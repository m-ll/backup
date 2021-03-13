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

    if( argc != 1 + 3 && argc != 1 + 4 )
    {
        std::cout << "Error - Bad arguments." << std::endl;
        std::cout << "./process-ecc {e, encode, d, decode} input-file output-file" << std::endl;

        return -1;
    }

    // argv[0] == ./process-ecc
    enum class eAction
    {
        kNone,
        kEncode,
        kDecode,
    };
    std::string argv1 = argv[1];
    eAction action = ( argv1 == "e" || argv1 == "encode" )
                        ?
                            eAction::kEncode
                        :
                            ( argv1 == "d" || argv1 == "decode" )
                            ?
                                eAction::kDecode
                            :
                                eAction::kNone;
    std::string input_data_file_name = argv[2];

    // std::cout << "input: " << input_data_file_name << std::endl;
    // std::cout << "output: " << output_file_name << std::endl;

//      const unsigned int primitive_polynomial05b[]    = {1, 0, 0, 1, 1, 0, 0, 0, 1};
//      const unsigned int primitive_polynomial_size05b = 9;
//      const schifra::galois::field field( field_descriptor, primitive_polynomial_size05b, primitive_polynomial05b );

    const schifra::galois::field field( field_descriptor, schifra::galois::primitive_polynomial_size06, schifra::galois::primitive_polynomial06 );

    //---

    std::size_t full_size = schifra::fileio::file_size( input_data_file_name );
    const std::size_t data_length = code_length - fec_length;
    std::size_t small_size = full_size / data_length / 4 * data_length;

    std::size_t size1 = small_size;
    std::size_t size2 = small_size;
    std::size_t size3 = small_size;
    std::size_t size4 = full_size - ( size1 + size2 + size3 );

    std::ifstream in_stream( input_data_file_name.c_str(), std::ios::binary );
    if( !in_stream )
    {
        std::cout << "reed_solomon::file_encoder() - Error: input data file could not be opened." << std::endl;
        return 1;
    }

    std::vector<char> data1( size1 );
    in_stream.read( data1.data(), static_cast<std::streamsize>( size1 ) );
    std::vector<char> data2( size2 );
    in_stream.read( data2.data(), static_cast<std::streamsize>( size2 ) );
    std::vector<char> data3( size3 );
    in_stream.read( data3.data(), static_cast<std::streamsize>( size3 ) );
    std::vector<char> data4( size4 );
    in_stream.read( data4.data(), static_cast<std::streamsize>( size4 ) );

    in_stream.close();

    //---

    if( action == eAction::kEncode )
    {
        std::string output_ecc_file_name = argv[3];

        //---

        schifra::galois::field_polynomial generator_polynomial( field );

        if( !schifra::make_sequential_root_generator_polynomial( field, gen_poly_index, gen_poly_root_count, generator_polynomial ) )
        {
            std::cout << "Error - Failed to create sequential root generator!" << std::endl;
            return 1;
        }

        //---

        std::string now = get_current_time();
        std::cout << now << " Start encoding: " << input_data_file_name << " -> " << output_ecc_file_name << std::endl;

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

        now = get_current_time();
        std::cout << now << " Write output file" << std::endl;

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
        std::string input_ecc_file_name = argv[3];
        std::string output_data_file_name = argv[4];

        //---

        full_size = schifra::fileio::file_size( input_ecc_file_name );
        small_size = full_size / fec_length / 4 * fec_length;
        assert( !( full_size % fec_length ) );

        size1 = small_size;
        size2 = small_size;
        size3 = small_size;
        size4 = full_size - ( size1 + size2 + size3 );

        std::ifstream in_ecc_stream( input_ecc_file_name.c_str(), std::ios::binary );
        if( !in_ecc_stream )
        {
            std::cout << "reed_solomon::file_decoder() - Error: input ecc file could not be opened." << std::endl;
            return 1;
        }

        std::vector<char> ecc1( size1 );
        in_ecc_stream.read( ecc1.data(), static_cast<std::streamsize>( size1 ) );
        std::vector<char> ecc2( size2 );
        in_ecc_stream.read( ecc2.data(), static_cast<std::streamsize>( size2 ) );
        std::vector<char> ecc3( size3 );
        in_ecc_stream.read( ecc3.data(), static_cast<std::streamsize>( size3 ) );
        std::vector<char> ecc4( size4 );
        in_ecc_stream.read( ecc4.data(), static_cast<std::streamsize>( size4 ) );

        in_ecc_stream.close();

        //---

        std::string now = get_current_time();
        std::cout << now << " Start decoding: " << input_data_file_name << " + " << input_ecc_file_name << " -> " << output_data_file_name << std::endl;

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

        now = get_current_time();
        std::cout << now << " Write output file" << std::endl;

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
