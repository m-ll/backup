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


#ifndef INCLUDE_ECC_DECODER_HPP
#define INCLUDE_ECC_DECODER_HPP


#include <iostream>
#include <fstream>

#include "schifra_reed_solomon_block.hpp"
#include "schifra_reed_solomon_decoder.hpp"
#include "schifra_fileio.hpp"


namespace schifra
{

   namespace reed_solomon
   {

      template <std::size_t code_length, std::size_t fec_length, std::size_t data_length = code_length - fec_length>
      class segment_decoder
      {
      public:

         typedef decoder<code_length,fec_length> decoder_type;
         typedef typename decoder_type::block_type block_type;

         segment_decoder( const decoder_type& iDecoder,
                          const std::vector<char>& iInputDataSegment,
                          const std::vector<char>& iInputEccSegment,
                          std::vector<char>& oOutputDataSegment )
         {
            std::size_t remaining_bytes = iInputDataSegment.size();
            if( remaining_bytes == 0 )
            {
               std::cout << "reed_solomon::segment_decoder() - Error: empty segment." << std::endl;
               return;
            }

            mCurrentBlockIndex = 0;

            while( remaining_bytes >= data_length )
            {
               process_complete_block( iDecoder, iInputDataSegment, iInputEccSegment, oOutputDataSegment );
               remaining_bytes -= data_length;
               mCurrentBlockIndex++;
            }

            if( remaining_bytes > 0 )
            {
               process_partial_block( iDecoder, iInputDataSegment, iInputEccSegment, oOutputDataSegment, remaining_bytes );
            }
         }

      private:

         inline void process_complete_block( const decoder_type& iDecoder,
                                             const std::vector<char>& iInputDataSegment,
                                             const std::vector<char>& iInputEccSegment,
                                             std::vector<char>& oOutputDataSegment )
         {
            int start_data = mCurrentBlockIndex * data_length;
            const char* data = &iInputDataSegment.data()[start_data];
            int start_fec = mCurrentBlockIndex * fec_length;
            const char* fec = &iInputEccSegment.data()[start_fec];

            for( std::size_t i = 0; i < data_length; ++i )
            {
               mBlock.data[i] = static_cast<typename block_type::symbol_type>( data[i] );
            }
            for( std::size_t i = 0; i < fec_length; ++i )
            {
               mBlock.fec( i ) = static_cast<typename block_type::symbol_type>( fec[i] );
            }

            if( !iDecoder.decode( mBlock ) )
            {
               std::cout << "reed_solomon::segment_decoder.process_complete_block() - Error during decoding of block " << mCurrentBlockIndex << "!" << std::endl;
               return;
            }

            for( std::size_t i = 0; i < data_length; ++i )
            {
               oOutputDataSegment.push_back( static_cast<char>( mBlock[i] ) );
            }
         }

         inline void process_partial_block( const decoder_type& iDecoder,
                                            const std::vector<char>& iInputDataSegment,
                                            const std::vector<char>& iInputEccSegment,
                                            std::vector<char>& oOutputDataSegment,
                                            const std::size_t& iDataRemainingSize )
         {
            int start_data = mCurrentBlockIndex * data_length;
            const char* data = &iInputDataSegment.data()[start_data];
            int start_fec = mCurrentBlockIndex * fec_length;
            const char* fec = &iInputEccSegment.data()[start_fec];

            for( std::size_t i = 0; i < iDataRemainingSize; ++i )
            {
               mBlock.data[i] = static_cast<typename block_type::symbol_type>( data[i] );
            }

            if( iDataRemainingSize < data_length )
            {
               for( std::size_t i = iDataRemainingSize; i < data_length; ++i )
               {
                  mBlock.data[i] = 0;
               }
            }

            for( std::size_t i = 0; i < fec_length; ++i )
            {
               mBlock.fec( i ) = static_cast<typename block_type::symbol_type>( fec[i] );
            }

            if( !iDecoder.decode( mBlock ) )
            {
               std::cout << "reed_solomon::segment_decoder.process_partial_block() - Error during decoding of block " << mCurrentBlockIndex << "!" << std::endl;
               return;
            }

            for( std::size_t i = 0; i < iDataRemainingSize; ++i )
            {
               oOutputDataSegment.push_back( static_cast<char>( mBlock.data[i] ) );
            }
         }

         block_type mBlock;
         std::size_t mCurrentBlockIndex;
      };

   } // namespace reed_solomon

} // namespace schifra

#endif
