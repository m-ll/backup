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


#include <cstddef>
#include <string>
#include <thread>

#include "schifra_galois_field.hpp"
#include "schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra_reed_solomon_encoder.hpp"
// #include "schifra_reed_solomon_file_encoder.hpp"
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

    if( argc != 1 + 2 )
    {
        std::cout << "Error - Bad arguments." << std::endl;
        std::cout << "./process-ecc input-file output-file" << std::endl;

        return -1;
    }

    // argv[0] == ./process-ecc
    std::string input_file_name = argv[1];
    std::string output_file_name = argv[2];

    // std::cout << "input: " << input_file_name << std::endl;
    // std::cout << "output: " << output_file_name << std::endl;

    //---

//    const unsigned int primitive_polynomial05b[]    = {1, 0, 0, 1, 1, 0, 0, 0, 1};
//    const unsigned int primitive_polynomial_size05b = 9;
//    const schifra::galois::field field( field_descriptor, primitive_polynomial_size05b, primitive_polynomial05b );

    const schifra::galois::field field( field_descriptor, schifra::galois::primitive_polynomial_size06, schifra::galois::primitive_polynomial06 );

    schifra::galois::field_polynomial generator_polynomial(field);

    if( !schifra::make_sequential_root_generator_polynomial( field, gen_poly_index, gen_poly_root_count, generator_polynomial ) )
    {
        std::cout << "Error - Failed to create sequential root generator!" << std::endl;
        return 1;
    }

    //---

    typedef schifra::reed_solomon::encoder<code_length,fec_length> encoder_t;

    const encoder_t rs_encoder(field,generator_polynomial);

    //---

    std::string now = get_current_time();
    std::cout << now << " Start encoding: " << input_file_name << " -> " << output_file_name << std::endl;

    //---

    std::size_t full_size = schifra::fileio::file_size(input_file_name);
    std::size_t small_size = full_size / ( code_length - fec_length ) / 4 * ( code_length - fec_length );

    std::size_t size1 = small_size;
    std::size_t size2 = small_size;
    std::size_t size3 = small_size;
    std::size_t size4 = full_size - ( size1 + size2 + size3 );

    std::ifstream in_stream( input_file_name.c_str(),std::ios::binary );
    if( !in_stream )
    {
        std::cout << "reed_solomon::file_encoder() - Error: input file could not be opened." << std::endl;
        return 1;
    }

    std::vector<char> data1( size1 );
    in_stream.read(data1.data(),static_cast<std::streamsize>(size1));
    std::vector<char> data2( size2 );
    in_stream.read(data2.data(),static_cast<std::streamsize>(size2));
    std::vector<char> data3( size3 );
    in_stream.read(data3.data(),static_cast<std::streamsize>(size3));
    std::vector<char> data4( size4 );
    in_stream.read(data4.data(),static_cast<std::streamsize>(size4));

    in_stream.close();

    //---

    typedef schifra::reed_solomon::segment_encoder<code_length,fec_length> segment_encoder_t;

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

    now = get_current_time();
    std::cout << now << " End encoding" << std::endl;

    //---

    std::ofstream out_stream(output_file_name.c_str(),std::ios::binary);
    if( !out_stream )
    {
        std::cout << "reed_solomon::file_encoder() - Error: output file could not be created." << std::endl;
        return 1;
    }

    out_stream.write( &ecc1[0], ecc1.size() );
    out_stream.write( &ecc2[0], ecc2.size() );
    out_stream.write( &ecc3[0], ecc3.size() );
    out_stream.write( &ecc4[0], ecc4.size() );

    out_stream.close();

    return 0;
}
