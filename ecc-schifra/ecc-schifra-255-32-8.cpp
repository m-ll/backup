/*
(**************************************************************************)
(*                                                                        *)
(*                                Schifra                                 *)
(*                Reed-Solomon Error Correcting Code Library              *)
(*                                                                        *)
(* Release Version 0.0.1                                                  *)
(* http://www.schifra.com                                                 *)
(* Copyright (c) 2000-2020 Arash Partow, All Rights Reserved.             *)
(*                                                                        *)
(* The Schifra Reed-Solomon error correcting code library and all its     *)
(* components are supplied under the terms of the General Schifra License *)
(* agreement. The contents of the Schifra Reed-Solomon error correcting   *)
(* code library and all its components may not be copied or disclosed     *)
(* except in accordance with the terms of that agreement.                 *)
(*                                                                        *)
(* URL: http://www.schifra.com/license.html                               *)
(*                                                                        *)
(**************************************************************************)
*/


#include <any>
#include <cstring>
#include <cstddef>
#include <deque>
#include <math.h>
#include <map>
#include <string>
#include <thread>

#include "schifra_galois_field.hpp"
#include "schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra_reed_solomon_decoder.hpp"
#include "schifra_reed_solomon_encoder.hpp"
#include "ecc-decoder.hpp"
#include "ecc-encoder.hpp"

//---

typedef std::vector<char> tBigChunk;

//---

std::string
get_current_time()
{
    time_t now;
    time( &now );

    struct tm* local = localtime( &now );

    int hours = local->tm_hour;          // get hours since midnight (0-23)
    int minutes = local->tm_min;         // get minutes passed after the hour (0-59)
    int seconds = local->tm_sec;         // get seconds passed after minute (0-59)

    char buffer[100];
    sprintf( buffer, "[%02d:%02d:%02d]", hours, minutes, seconds );

    return buffer;
}

//---

int
AllocateEmptyBigChunks( int iBigChunkCount, std::vector<tBigChunk>& oBigChunks )
{
    for( int i = 0; i < iBigChunkCount; i++ )
    {
        oBigChunks.push_back( tBigChunk() );
    }

    return 0;
}

int
AllocateBigChunks( std::size_t iFullSize, std::size_t iBigChunkSize, std::vector<tBigChunk>& oBigChunks )
{
    // Get size for each data big chunk
    std::vector<std::size_t> big_chunk_sizes;
    std::size_t remaining_size = iFullSize;
    while( remaining_size >= iBigChunkSize )
    {
        big_chunk_sizes.push_back( iBigChunkSize );
        remaining_size -= iBigChunkSize;
    }
    if( remaining_size )
        big_chunk_sizes.push_back( remaining_size );

    assert( remaining_size < iBigChunkSize );

    // Alocate each big chunk
    for( auto big_chunk_size : big_chunk_sizes )
    {
        oBigChunks.push_back( tBigChunk( big_chunk_size ) );
    }

    return 0;
}

int
ReadFileToChunks( std::string iPathFile, std::vector<tBigChunk>& oBigChunks )
{
    std::ifstream in_stream( iPathFile.c_str(), std::ios::binary );
    if( !in_stream )
    {
        std::cout << "reed_solomon::ReadFileToChunks() - Error: input file could not be opened: " << iPathFile << std::endl;
        return 1;
    }

    // Read all big chunks from data input file
    for( auto& big_chunk : oBigChunks )
    {
        in_stream.read( big_chunk.data(), static_cast<std::streamsize>( big_chunk.size() ) );
    }

    in_stream.close();

    return 0;
}

int
WriteChunksToFile( const std::vector<tBigChunk>& iBigChunks, std::string iPathFile )
{
    std::ofstream out_stream( iPathFile.c_str(), std::ios::binary );
    if( !out_stream )
    {
        std::cout << "reed_solomon::WriteChunksToFile() - Error: output file could not be created: " << iPathFile << std::endl;
        return 1;
    }

    // Write ecc to the output file
    for( const auto& big_chunk : iBigChunks )
    {
        out_stream.write( big_chunk.data(), big_chunk.size() );
    }

    out_stream.close();
    
    return 0;
}

//---

enum class eAction
{
    kNone,
    kEncode,
    kDecode,
};

enum class eArgument
{
    kVerbose,
    kAction,
    kInputDataFile,
    kOutputEccFile,
    kInputEccFile,
    kOutputDataDecodedFile,
};

int
ParseArgs( std::deque<std::string>& ioArgs, std::map<eArgument, std::any>& oArgs )
{
    //--- Optional arguments

    int verbose = 0;
    std::string input_data_file_name;   // encode + decode
    std::string output_ecc_file_name;   // encode
    std::string input_ecc_file_name;    // decode
    std::string output_data_file_name;  // decode

    std::deque<std::string> args_positional;
    while( ioArgs.size() )
    {
        std::string arg = ioArgs[0];
        ioArgs.pop_front();

        if( arg == "-v" )
        {
            verbose++;
        }
        else if( arg == "-i" )
        {
            input_data_file_name = ioArgs[0];
            ioArgs.pop_front();
        }
        else if( arg == "-e" )
        {
            input_ecc_file_name = ioArgs[0];
            ioArgs.pop_front();
        }
        else if( arg == "-o" )
        {
            output_ecc_file_name = ioArgs[0];
            output_data_file_name = ioArgs[0];
            ioArgs.pop_front();
        }
        else
        {
            args_positional.push_back( arg );
        }
    }

    //--- Positional arguments

    eAction action = eAction::kNone;

    if( args_positional.size() )
    {
        std::string arg = args_positional[0];
        args_positional.pop_front();

        if( arg == "e" || arg == "encode" )
            action = eAction::kEncode;
        if( arg == "d" || arg == "decode" )
            action = eAction::kDecode;
    }

    //--- Error checking

    if( args_positional.size() // remaining arguments
        || action == eAction::kNone 
        || ( action == eAction::kEncode && ( !input_data_file_name.length() || !output_ecc_file_name.length() ) )
        || ( action == eAction::kDecode && ( !input_data_file_name.length() || !input_ecc_file_name.length() || !output_data_file_name.length() ) ) )
    {
        std::cout << "Error - Bad arguments." << std::endl;
        std::cout << "./ecc-schifra-255-32-8 [-v] {e, encode} -i input-data-file -o output-ecc-file" << std::endl;
        std::cout << "./ecc-schifra-255-32-8 [-v] {d, decode} -i input-data-file -e input-ecc-file -o output-datafile" << std::endl;
        std::cout << std::endl;
        std::cout << args_positional.size() << std::endl;
        std::cout << int(action) << " " << input_data_file_name << " " << output_ecc_file_name << std::endl;
        std::cout << int(action) << " " << input_data_file_name << " " << input_ecc_file_name << " " << output_data_file_name << " " << std::endl;

        return 1;
    }

    //--- Build argument parameters

    oArgs[eArgument::kAction] = action;
    oArgs[eArgument::kVerbose] = verbose;
    oArgs[eArgument::kInputDataFile] = input_data_file_name;
    oArgs[eArgument::kOutputEccFile] = output_ecc_file_name;
    oArgs[eArgument::kInputEccFile] = input_ecc_file_name;
    oArgs[eArgument::kOutputDataDecodedFile] = output_data_file_name;

    return 0;
}

int
main( int argc, char *argv[] )
{
    const std::size_t field_descriptor    =   8;
    const std::size_t gen_poly_index      = 120;
    const std::size_t gen_poly_root_count =  32;
    const std::size_t code_length         = 255;
    const std::size_t fec_length          =  32;
    const std::size_t data_length         = code_length - fec_length;

    int error = 0;

    std::deque<std::string> string_args( argv, argv + argc );
    string_args.pop_front(); // Remove program path

    std::map<eArgument, std::any> args;
    error = ParseArgs( string_args, args );
    if( error )
        return 1;

    int verbose = std::any_cast<int>( args[eArgument::kVerbose] );
    eAction action = std::any_cast<eAction>( args[eArgument::kAction] );
    std::string input_data_file_name = std::any_cast<std::string>( args[eArgument::kInputDataFile] );
    std::string output_ecc_file_name = std::any_cast<std::string>( args[eArgument::kOutputEccFile] );
    std::string input_ecc_file_name = std::any_cast<std::string>( args[eArgument::kInputEccFile] );
    std::string output_data_file_name = std::any_cast<std::string>( args[eArgument::kOutputDataDecodedFile] );

    //---

//      const unsigned int primitive_polynomial05b[]    = {1, 0, 0, 1, 1, 0, 0, 0, 1};
//      const unsigned int primitive_polynomial_size05b = 9;
//      const schifra::galois::field field( field_descriptor, primitive_polynomial_size05b, primitive_polynomial05b );

    const schifra::galois::field field( field_descriptor, schifra::galois::primitive_polynomial_size06, schifra::galois::primitive_polynomial06 );

    //---

    const auto processor_count = std::thread::hardware_concurrency();

    std::size_t data_full_size = schifra::fileio::file_size( input_data_file_name );
    // Number of big chunk
    float big_chunk_sizef = data_full_size / float( processor_count );
    // Number of data chunk (223o) per big chunk
    float nbf_data_chunk_per_big_chunk = big_chunk_sizef / data_length;
    // Number of data chunk (223o) per big chunk by rounding to upper value
    int nb_data_chunk_per_big_chunk = int( ceil( nbf_data_chunk_per_big_chunk ) );
    // Size of a big chunk containing a multiple of data_length chunk (except for the last one)
    std::size_t data_big_chunk_size = nb_data_chunk_per_big_chunk * data_length;

    //---

    std::vector<tBigChunk> data_big_chunks;
    error = AllocateBigChunks( data_full_size, data_big_chunk_size, data_big_chunks );
    if( error )
        return 1;
    
    assert( data_big_chunks.size() <= processor_count );

    error = ReadFileToChunks( input_data_file_name, data_big_chunks );
    if( error )
        return 1;

    //---

    if( action == eAction::kEncode )
    {
        if( verbose >= 1 )
            std::cout << get_current_time() << " Start encoding: " << input_data_file_name << " -> " << output_ecc_file_name << std::endl;

        schifra::galois::field_polynomial generator_polynomial( field );

        if( !schifra::make_sequential_root_generator_polynomial( field, gen_poly_index, gen_poly_root_count, generator_polynomial ) )
        {
            std::cout << "Error - Failed to create sequential root generator!" << std::endl;
            return 1;
        }

        //---

        std::vector<tBigChunk> ecc_big_chunks;
        error = AllocateEmptyBigChunks( data_big_chunks.size(), ecc_big_chunks );
        if( error )
            return 1;

        //---

        typedef schifra::reed_solomon::encoder<code_length,fec_length> encoder_t;
        typedef schifra::reed_solomon::segment_encoder<code_length,fec_length> segment_encoder_t;

        // Create the encoder
        const encoder_t rs_encoder( field, generator_polynomial );

        //---

        // Callback for the thread
        auto encode = [&]( const tBigChunk& iData, tBigChunk& oEcc )
        {
            segment_encoder_t( rs_encoder, iData, oEcc );
        };

        // Create the threads
        std::vector<std::thread> threads;
        for( std::vector<tBigChunk>::size_type i = 0; i < data_big_chunks.size(); i++ )
        {
            threads.push_back( std::thread( encode, std::ref( data_big_chunks[i] ), std::ref( ecc_big_chunks[i] ) ) );
        }

        // Wait all threads
        for( auto& thread : threads )
        {
            if( thread.joinable() )
                thread.join();
        }

        //---

        if( verbose >= 2 )
            std::cout << get_current_time() << " Write output file" << std::endl;

        error = WriteChunksToFile( ecc_big_chunks, output_ecc_file_name );
        if( error )
            return 1;
    }
    else if( action == eAction::kDecode )
    {
        if( verbose >= 1 )
            std::cout << get_current_time() << " Start decoding: " << input_data_file_name << " + " << input_ecc_file_name << " -> " << output_data_file_name << std::endl;

        std::size_t ecc_full_size = schifra::fileio::file_size( input_ecc_file_name );
        assert( !( ecc_full_size % fec_length ) );
        
        std::size_t ecc_big_chunk_size = nb_data_chunk_per_big_chunk * fec_length; // Must use the same nb_data_chunk_per_big_chunk as the data big_chunk, to have the same number of chunk in both side

        //---

        std::vector<tBigChunk> ecc_big_chunks;
        error = AllocateBigChunks( ecc_full_size, ecc_big_chunk_size, ecc_big_chunks );
        if( error )
            return 1;

        assert( ecc_big_chunks.size() <= processor_count );

        error = ReadFileToChunks( input_ecc_file_name, ecc_big_chunks );
        if( error )
            return 1;

        std::vector<tBigChunk> data_decoded_big_chunks;
        error = AllocateEmptyBigChunks( data_big_chunks.size(), data_decoded_big_chunks );
        if( error )
            return 1;

        //---

        typedef schifra::reed_solomon::decoder<code_length,fec_length> decoder_t;
        typedef schifra::reed_solomon::segment_decoder<code_length,fec_length> segment_decoder_t;

        // Create the encoder
        const decoder_t rs_decoder( field, gen_poly_index );

        //---

        // Callback for the thread
        auto decode = [&]( const tBigChunk& iData, const tBigChunk& iEcc, tBigChunk& oDataDecoded )
        {
            segment_decoder_t( rs_decoder, iData, iEcc, oDataDecoded );
        };

        // Create the threads
        std::vector<std::thread> threads;
        for( std::vector<tBigChunk>::size_type i = 0; i < data_big_chunks.size(); i++ )
        {
            threads.push_back( std::thread( decode, std::ref( data_big_chunks[i] ), std::ref( ecc_big_chunks[i] ), std::ref( data_decoded_big_chunks[i] ) ) );
        }

        // Wait all threads
        for( auto& thread : threads )
        {
            if( thread.joinable() )
                thread.join();
        }

        //---

        if( verbose >= 2 )
            std::cout << get_current_time() << " Write output file" << std::endl;

        error = WriteChunksToFile( data_decoded_big_chunks, output_data_file_name );
        if( error )
            return 1;
    }

    return 0;
}
