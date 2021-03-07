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


#ifndef INCLUDE_ECC_ENCODER_HPP
#define INCLUDE_ECC_ENCODER_HPP


#include <cstring>
#include <iostream>
#include <fstream>

#include "schifra_reed_solomon_block.hpp"
#include "schifra_reed_solomon_encoder.hpp"
#include "schifra_fileio.hpp"


namespace schifra
{

   namespace reed_solomon
   {

      template <std::size_t code_length, std::size_t fec_length, std::size_t data_length = code_length - fec_length>
      class segment_encoder
      {
      public:

         typedef encoder<code_length,fec_length> encoder_type;
         typedef typename encoder_type::block_type block_type;

         segment_encoder(const encoder_type& encoder,
                              const std::vector<char>& input_segment,
                              std::vector<char>& output_segment)
         {
            std::size_t remaining_bytes = input_segment.size();
            if (remaining_bytes == 0)
            {
               std::cout << "reed_solomon::segment_encoder() - Error: empty segment." << std::endl;
               return;
            }

            std::size_t remaining_start = 0;

            while (remaining_bytes >= data_length)
            {
               process_block(encoder,input_segment,output_segment,remaining_start,data_length);
               remaining_bytes -= data_length;
               remaining_start += data_length;
            }

            if (remaining_bytes > 0)
            {
               process_block(encoder,input_segment,output_segment,remaining_start,remaining_bytes);
            }
         }

      private:

         inline void process_block(const encoder_type& encoder,
                                    const std::vector<char>& input_segment,
                                    std::vector<char>& output_segment,
                                    const std::size_t& read_start,
                                    const std::size_t& read_amount)
         {
            const char* data_buffer = &input_segment.data()[read_start];
            for (std::size_t i = 0; i < read_amount; ++i)
            {
               block_.data[i] = (data_buffer[i] & 0xFF);
            }

            if (read_amount < data_length)
            {
               for (std::size_t i = read_amount; i < data_length; ++i)
               {
                  block_.data[i] = 0x00;
               }
            }

            if (!encoder.encode(block_))
            {
               std::cout << "reed_solomon::segment_encoder.process_block() - Error during encoding of block!" << std::endl;
               return;
            }

            for (std::size_t i = 0; i < fec_length; ++i)
            {
               output_segment.push_back( static_cast<char>(block_.fec(i) & 0xFF) );
            }
         }

         block_type block_;
      };
   
   } // namespace reed_solomon

} // namespace schifra

#endif
