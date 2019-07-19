#!/usr/bin/env python3
#
# Copyright (c) 2019 m-ll. All Rights Reserved.
#
# Licensed under the MIT License.
# See LICENSE file in the project root for full license information.
#
# 2b13c8312f53d4b9202b6c8c0f0e790d10044f9a00d8bab3edf3cd287457c979
# 29c355784a3921aa290371da87bce9c1617b8584ca6ac6fb17fb37ba4a07d191
#

import datetime
import io
from pathlib import Path

from colorama import init, Fore
init( autoreset=True )

#---

## The current time
#
#  @return  datetime.time  The current time
def now():
	return datetime.datetime.now().time()

## Display a progress bar (as text)
#
#  @param  iCurrent  int  The number of byte already processed
#  @param  iTotal    int  The total number of byte to process
def print_progress( iCurrent, iTotal ):
	print( '[{}]   processed: {:.2f}%'.format( now(), iCurrent / iTotal * 100.0 ), end='\r' )

#---

## Manage the ecc file
#
#  Create/Check/Fix the ecc file corresponding to the input file
class cEcc:

	## The constructor
	#
	#  @param  iFileInput  pathlib.Path  The input file
	def __init__( self, iFileInput ):
		self.mLimitSize = 0
		self.mFileInput = iFileInput
		self.mFileEcc = '' # Contains only the ecc data
		self.mFileFix = ''

		self.mSizeResult = 0
		self.mSizeMessage = 0
		self.mExp = 0
		self.mThreshold = 0
	
	## Convert the object to string to display it
	#
	#  @return  string  A string containing the members
	def __repr__( self ):
		return f'input: {self.mFileInput}\n  ecc: {self.mFileEcc}\n  fix: {self.mFileFix}'
	
	#---
	
	## Check if the input file is valid for managing its ecc file
	#
	#  @return             bool  The input file can be process or not
	def IsValidInput( self ):
		if not self.mFileInput.is_file():
			return False

		if self.mFileInput.suffix != '.gpg':
			return False
		
		return True
	
	## Check if the input file is not too big for managing its ecc file
	#
	#  @return             bool  The input file can be process or not
	def _IsValidInputSize( self ):
		if self.mFileInput.stat().st_size > self.mLimitSize * 1000000:
			return False
		
		return True
	
	## Initialize data for processing the input file
	#
	#  @param  iLimitSize    int  The limit size (in Mo) of the input file (otherwise the process is too slow)
	#  @param  iResultSize   int  The result size
	#  @param  iMessageSize  int  The message size
	#  @param  iExp          int  The exp
	def Init( self, iLimitSize, iResultSize, iMessageSize, iExp ):
		self.mLimitSize = iLimitSize
		self.mFileEcc = Path( f'ecc-{iResultSize}-{iMessageSize}-{iExp}' ) / self.mFileInput.with_suffix( self.mFileInput.suffix + f'.ecc-{iResultSize}-{iMessageSize}-{iExp}' )
		self.mFileFix = Path( 'ecc-regenerated' ) / self.mFileInput.with_suffix( self.mFileInput.suffix + '.regenerated' )

		self.mSizeResult = iResultSize
		self.mSizeMessage = iMessageSize
		self.mCorrectionThreshold = iResultSize - iMessageSize
		self.mExp = iExp
	
	#---
	
	## Create the ecc file
	#
	#  @param  iCoder  unireedsolomon.rs.RSCoder  The coder
	def ProcessCreate( self, iCoder ):
		size_to_process = self.mFileInput.stat().st_size
		
		if self.mFileEcc.is_file():
			print( Fore.CYAN + 'Skip: ecc file already exists: {}'.format( self.mFileEcc ) )
			return
		if not self._IsValidInputSize():
			print( Fore.RED + 'Skip: file too big: {:5} Mo - {}'.format( self.mFileInput.stat().st_size // 1000000, self.mFileInput ) )
			return
		
		start = datetime.datetime.now()
		print( '[{}] {} -> {}'.format( now(), self.mFileInput, self.mFileEcc ) )

		self.mFileEcc.parent.mkdir( parents=True, exist_ok=True )

		# with self.mFileInput.open( 'rb' ) as src, open( dst_name, 'wb' ) as dst, self.mFileEcc.open( 'wb' ) as ecc:
		with self.mFileInput.open( 'rb' ) as src, self.mFileEcc.open( 'wb' ) as ecc:
			for big_chunk_in in iter( lambda: src.read( self.mSizeMessage * 4000 ), b'' ):
				big_chunk_in_as_file = io.BytesIO( big_chunk_in )
				big_chunk_out_as_file = io.BytesIO()
				for chunk in iter( lambda: big_chunk_in_as_file.read( self.mSizeMessage ), b'' ):
					padding = self.mSizeMessage - len( chunk )
					if padding:
						chunk = b'\0' * padding + chunk

					c = iCoder.encode_fast( chunk, return_string=False )
					
					# If using exp > 8 -> value may be > 255 -> can't store them inside 1 byte
					# b = bytearray()
					# for n in c:
						# b.extend( n.to_bytes( ( n.bit_length() + 7 ) // 8, byteorder='little' ) )
					# # dst.write( b )

					message_and_ecc = bytes( c )
					ecc_only = message_and_ecc[self.mSizeMessage:]
					big_chunk_out_as_file.write( ecc_only )

				ecc.write( big_chunk_out_as_file.getvalue() )

				print_progress( src.tell(), size_to_process )
			print() # To go to next line after the last previous \r

		self.mFileEcc.chmod( 0o777 )
		diff = datetime.datetime.now() - start
		print( '[{}] time used: {} (at {:.2f} Mo/s)'.format( now(), diff, ( size_to_process / 1000000 ) / diff.total_seconds() ) )
		
	## Check the input file via its ecc file
	#
	#  @param  iCoder  unireedsolomon.rs.RSCoder  The coder
	def ProcessCheck( self, iCoder ):
		size_to_process = self.mFileInput.stat().st_size

		if not self.mFileEcc.is_file():
			print( Fore.CYAN + 'Skip: ecc file doesn\'t exists: {}'.format( self.mFileEcc ) )
			return

		start = datetime.datetime.now()
		print( '[{}] {}, {}'.format( now(), self.mFileInput, self.mFileEcc ) )

		with self.mFileInput.open( 'rb' ) as src, self.mFileEcc.open( 'rb' ) as ecc:
			for src_chunk, ecc_chunk in zip( iter( lambda: src.read( self.mSizeMessage ), b'' ), iter( lambda: ecc.read( self.mCorrectionThreshold ), b'' ) ):
				padding = self.mSizeMessage - len( src_chunk )
				if padding:
					src_chunk = b'\0' * padding + src_chunk
				chunk = src_chunk + ecc_chunk

				check = iCoder.check_fast( chunk )
				if not check:
					print()
					print( Fore.RED + 'There are errors -_-' )
						
				print_progress( src.tell(), size_to_process )
			print()

		print( '[{}] time used: {}'.format( now(), datetime.datetime.now() - start ) )
			
	## Fix the input file via its ecc file
	#
	#  @param  iCoder  unireedsolomon.rs.RSCoder  The coder
	def ProcessFix( self, iCoder ):
		size_to_process = self.mFileInput.stat().st_size

		if not self.mFileEcc.is_file():
			print( Fore.CYAN + 'Skip: ecc file doesn\'t exists: {}'.format( self.mFileEcc ) )
			return

		start = datetime.datetime.now()
		print( '[{}] {}, {} -> {}'.format( now(), self.mFileInput, self.mFileEcc, self.mFileFix ) )

		self.mFileFix.parent.mkdir( parents=True, exist_ok=True )

		with self.mFileInput.open( 'rb' ) as src, self.mFileEcc.open( 'rb' ) as ecc, self.mFileFix.open( 'wb' ) as fix:
			for src_chunk, ecc_chunk in zip( iter( lambda: src.read( self.mSizeMessage ), b'' ), iter( lambda: ecc.read( self.mCorrectionThreshold ), b'' ) ):
				padding = self.mSizeMessage - len( src_chunk )
				if padding:
					src_chunk = b'\0' * padding + src_chunk
				chunk = src_chunk + ecc_chunk

				d, code = iCoder.decode_fast( chunk, nostrip=True, return_string=False )

				b = bytes( d )
				fix.write( b[padding:] )

				print_progress( src.tell(), size_to_process )
			print()

		self.mFileFix.chmod( 0o777 )
		print( '[{}] time used: {}'.format( now(), datetime.datetime.now() - start ) )
