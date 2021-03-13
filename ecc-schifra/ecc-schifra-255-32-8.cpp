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


#include <cstring>
#include <cstddef>
#include <deque>
#include <string>
#include <thread>

#include "schifra_galois_field.hpp"
#include "schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra_reed_solomon_decoder.hpp"
#include "schifra_reed_solomon_encoder.hpp"
#include "ecc-decoder.hpp"
#include "ecc-encoder.hpp"


std::string get_current_time()
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

int main( int argc, char *argv[] )
{
    const std::size_t field_descriptor    =   8;
    const std::size_t gen_poly_index      = 120;
    const std::size_t gen_poly_root_count =  32;
    const std::size_t code_length         = 255;
    const std::size_t fec_length          =  32;

    std::deque<std::string> args( argv, argv + argc );
    args.pop_front(); // Remove program path

    int verbose = 0;
    std::string input_data_file_name;   // encode + decode
    std::string output_ecc_file_name;   // encode
    std::string input_ecc_file_name;    // decode
    std::string output_data_file_name;  // decode

    std::deque<std::string> args_positional;
    while( args.size() )
    {
        std::string arg = args[0];
        args.pop_front();

        if( arg == "-v" )
        {
            verbose++;
        }
        else if( arg == "-i" )
        {
            input_data_file_name = args[0];
            args.pop_front();
        }
        else if( arg == "-e" )
        {
            input_ecc_file_name = args[0];
            args.pop_front();
        }
        else if( arg == "-o" )
        {
            output_ecc_file_name = args[0];
            output_data_file_name = args[0];
            args.pop_front();
        }
        else
        {
            args_positional.push_back( arg );
        }
    }

    enum class eAction
    {
        kNone,
        kEncode,
        kDecode,
    };
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

        return -1;
    }

//      const unsigned int primitive_polynomial05b[]    = {1, 0, 0, 1, 1, 0, 0, 0, 1};
//      const unsigned int primitive_polynomial_size05b = 9;
//      const schifra::galois::field field( field_descriptor, primitive_polynomial_size05b, primitive_polynomial05b );

    const schifra::galois::field field( field_descriptor, schifra::galois::primitive_polynomial_size06, schifra::galois::primitive_polynomial06 );

    //---

    //const auto processor_count = std::thread::hardware_concurrency();
    int nb_big_chunk = 4;

    std::size_t full_size = schifra::fileio::file_size( input_data_file_name );
    const std::size_t data_length = code_length - fec_length;
    int nb_data_chunk = full_size / data_length; // Number of data chunk (223o) inside input file, doesn't count the last partial one (as integer division) but it's not a problem, it's just have an idea to split for each thread

    int nb_data_chunk_per_big_chunk = nb_data_chunk / nb_big_chunk; // Number of data chunk for each big (thread) chunk

    std::size_t big_chunk_size1 = nb_data_chunk_per_big_chunk * data_length;
    std::size_t big_chunk_size2 = big_chunk_size1;
    std::size_t big_chunk_size3 = big_chunk_size2;
    std::size_t big_chunk_size4 = full_size - ( big_chunk_size1 + big_chunk_size2 + big_chunk_size3 );

    std::ifstream in_stream( input_data_file_name.c_str(), std::ios::binary );
    if( !in_stream )
    {
        std::cout << "reed_solomon::file_encoder() - Error: input data file could not be opened." << std::endl;
        return 1;
    }

    std::vector<char> data1( big_chunk_size1 );
    in_stream.read( data1.data(), static_cast<std::streamsize>( big_chunk_size1 ) );
    std::vector<char> data2( big_chunk_size2 );
    in_stream.read( data2.data(), static_cast<std::streamsize>( big_chunk_size2 ) );
    std::vector<char> data3( big_chunk_size3 );
    in_stream.read( data3.data(), static_cast<std::streamsize>( big_chunk_size3 ) );
    std::vector<char> data4( big_chunk_size4 );
    in_stream.read( data4.data(), static_cast<std::streamsize>( big_chunk_size4 ) );

    in_stream.close();

    //---

    if( action == eAction::kEncode )
    {
        schifra::galois::field_polynomial generator_polynomial( field );

        if( !schifra::make_sequential_root_generator_polynomial( field, gen_poly_index, gen_poly_root_count, generator_polynomial ) )
        {
            std::cout << "Error - Failed to create sequential root generator!" << std::endl;
            return 1;
        }

        //---

        if( verbose >= 1 )
            std::cout << get_current_time() << " Start encoding: " << input_data_file_name << " -> " << output_ecc_file_name << std::endl;

        typedef schifra::reed_solomon::encoder<code_length,fec_length> encoder_t;
        typedef schifra::reed_solomon::segment_encoder<code_length,fec_length> segment_encoder_t;

        const encoder_t rs_encoder( field, generator_polynomial );

        std::vector<char> ecc1;
        std::vector<char> ecc2;
        std::vector<char> ecc3;
        std::vector<char> ecc4;

        std::thread first ( [&](){ segment_encoder_t( rs_encoder, data1, ecc1 ); } );
        std::thread second( [&](){ segment_encoder_t( rs_encoder, data2, ecc2 ); } );
        std::thread third ( [&](){ segment_encoder_t( rs_encoder, data3, ecc3 ); } );
        std::thread forth ( [&](){ segment_encoder_t( rs_encoder, data4, ecc4 ); } );

        first.join();
        second.join();
        third.join();
        forth.join();

        //---

        if( verbose >= 2 )
            std::cout << get_current_time() << " Write output file" << std::endl;

        std::ofstream out_stream( output_ecc_file_name.c_str(), std::ios::binary );
        if( !out_stream )
        {
            std::cout << "reed_solomon::file_encoder() - Error: output ecc file could not be created." << std::endl;
            return 1;
        }

        out_stream.write( &ecc1[0], ecc1.size() );
        out_stream.write( &ecc2[0], ecc2.size() );
        out_stream.write( &ecc3[0], ecc3.size() );
        out_stream.write( &ecc4[0], ecc4.size() );

        out_stream.close();
    }
    else if( action == eAction::kDecode )
    {
        full_size = schifra::fileio::file_size( input_ecc_file_name );
        assert( !( full_size % fec_length ) );

        big_chunk_size1 = nb_data_chunk_per_big_chunk * fec_length; // Must use the same nb_data_chunk_per_big_chunk as the data big_chunk, to have the same number of chunk in both side
        big_chunk_size2 = big_chunk_size1;
        big_chunk_size3 = big_chunk_size2;
        big_chunk_size4 = full_size - ( big_chunk_size1 + big_chunk_size2 + big_chunk_size3 );

        std::ifstream in_ecc_stream( input_ecc_file_name.c_str(), std::ios::binary );
        if( !in_ecc_stream )
        {
            std::cout << "reed_solomon::file_decoder() - Error: input ecc file could not be opened." << std::endl;
            return 1;
        }

        std::vector<char> ecc1( big_chunk_size1 );
        in_ecc_stream.read( ecc1.data(), static_cast<std::streamsize>( big_chunk_size1 ) );
        std::vector<char> ecc2( big_chunk_size2 );
        in_ecc_stream.read( ecc2.data(), static_cast<std::streamsize>( big_chunk_size2 ) );
        std::vector<char> ecc3( big_chunk_size3 );
        in_ecc_stream.read( ecc3.data(), static_cast<std::streamsize>( big_chunk_size3 ) );
        std::vector<char> ecc4( big_chunk_size4 );
        in_ecc_stream.read( ecc4.data(), static_cast<std::streamsize>( big_chunk_size4 ) );

        in_ecc_stream.close();

        //---

        if( verbose >= 1 )
            std::cout << get_current_time() << " Start decoding: " << input_data_file_name << " + " << input_ecc_file_name << " -> " << output_data_file_name << std::endl;

        typedef schifra::reed_solomon::decoder<code_length,fec_length> decoder_t;
        typedef schifra::reed_solomon::segment_decoder<code_length,fec_length> segment_decoder_t;

        const decoder_t rs_decoder( field, gen_poly_index );

        std::vector<char> datadecoded1;
        std::vector<char> datadecoded2;
        std::vector<char> datadecoded3;
        std::vector<char> datadecoded4;

        std::thread first ( [&](){ segment_decoder_t( rs_decoder, data1, ecc1, datadecoded1 ); } );
        std::thread second( [&](){ segment_decoder_t( rs_decoder, data2, ecc2, datadecoded2 ); } );
        std::thread third ( [&](){ segment_decoder_t( rs_decoder, data3, ecc3, datadecoded3 ); } );
        std::thread forth ( [&](){ segment_decoder_t( rs_decoder, data4, ecc4, datadecoded4 ); } );

        first.join();
        second.join();
        third.join();
        forth.join();

        //---

        if( verbose >= 2 )
            std::cout << get_current_time() << " Write output file" << std::endl;

        std::ofstream out_stream( output_data_file_name.c_str(), std::ios::binary );
        if( !out_stream )
        {
            std::cout << "reed_solomon::file_decoder() - Error: output data file could not be created." << std::endl;
            return 1;
        }

        out_stream.write( &datadecoded1[0], datadecoded1.size() );
        out_stream.write( &datadecoded2[0], datadecoded2.size() );
        out_stream.write( &datadecoded3[0], datadecoded3.size() );
        out_stream.write( &datadecoded4[0], datadecoded4.size() );

        out_stream.close();
    }

    return 0;
}
