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

         segment_encoder( const encoder_type& iEncoder,
                          const std::vector<char>& iInputSegment,
                          std::vector<char>& oOutputSegment )
         {
            std::size_t remaining_bytes = iInputSegment.size();
            if( remaining_bytes == 0 )
            {
               std::cout << "reed_solomon::segment_encoder() - Error: empty segment." << std::endl;
               return;
            }

            std::size_t chunk_start = 0;

            while( remaining_bytes >= data_length )
            {
               process_block( iEncoder, iInputSegment, oOutputSegment, chunk_start, data_length );
               remaining_bytes -= data_length;
               chunk_start += data_length;
            }

            if( remaining_bytes > 0 )
            {
               process_block( iEncoder, iInputSegment, oOutputSegment, chunk_start, remaining_bytes );
            }
         }

      private:

         inline void process_block( const encoder_type& iEncoder,
                                    const std::vector<char>& iInputSegment,
                                    std::vector<char>& oOutputSegment,
                                    const std::size_t& iChunkStart,
                                    const std::size_t& iChunkSize )
         {
            const char* data_buffer = &iInputSegment.data()[iChunkStart];
            for( std::size_t i = 0; i < iChunkSize; ++i )
            {
               mBlock.data[i] = ( data_buffer[i] & 0xFF );
            }

            if ( iChunkSize < data_length )
            {
               for( std::size_t i = iChunkSize; i < data_length; ++i )
               {
                  mBlock.data[i] = 0x00;
               }
            }

            if( !iEncoder.encode( mBlock ) )
            {
               std::cout << "reed_solomon::segment_encoder.process_block() - Error during encoding of block!" << std::endl;
               return;
            }

            for( std::size_t i = 0; i < fec_length; ++i )
            {
               oOutputSegment.push_back( static_cast<char>( mBlock.fec( i ) & 0xFF ) );
            }
         }

         block_type mBlock;
      };
   
   } // namespace reed_solomon

} // namespace schifra

#endif
