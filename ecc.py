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

import argparse
import datetime
import io
from pathlib import Path
import sys
# To use directly those inside the backup directory (but slower as unireedsolomon don't use cython in this case)
# sys.path.append( os.path.join( os.path.dirname( __file__ ), 'no-dep', 'unireedsolomon' ) )
# sys.path.append( os.path.join( os.path.dirname( __file__ ), 'no-dep', 'colorama' ) )

from colorama import init, Fore
init( autoreset=True )
import unireedsolomon

#---

def now():
	return datetime.datetime.now().time()

def print_progress( iCurrent, iTotal ):
	print( '[{}]   processed: {:.2f}%'.format( now(), iCurrent / iTotal * 100.0 ), end='\r' )

#---

class cEcc:
	def __init__( self, iFileInput ):
		self.mFileInput = iFileInput
		self.mFileEcc = '' # Contains only the ecc data
		self.mFileFix = ''

		self.mSizeResult = 0
		self.mSizeMessage = 0
		self.mExp = 0
		self.mThreshold = 0
	
	def __repr__( self ):
		return f'input: {self.mFileInput}\n  ecc: {self.mFileEcc}\n  fix: {self.mFileFix}'
	
	#---
	
	def IsValidInput( self, iLimitSize ):
		if not self.mFileInput.is_file():
			return False

		if self.mFileInput.stat().st_size > iLimitSize * 1000000:
			return False
		
		if self.mFileInput.suffix != '.gpg':
			return False
		
		return True
	
	def Init( self, iResultSize, iMessageSize, iExp ):
		self.mFileEcc = Path( f'ecc-{iResultSize}-{iMessageSize}-{iExp}' ) / self.mFileInput.with_suffix( self.mFileInput.suffix + f'.ecc-{iResultSize}-{iMessageSize}-{iExp}' )
		self.mFileFix = Path( 'ecc-regenerated' ) / self.mFileInput.with_suffix( self.mFileInput.suffix + '.regenerated' )

		self.mSizeResult = iResultSize
		self.mSizeMessage = iMessageSize
		self.mCorrectionThreshold = iResultSize - iMessageSize
		self.mExp = iExp
	
	#---
	
	def ProcessCreate( self, iCoder ):
		size_to_process = self.mFileInput.stat().st_size
		
		if self.mFileEcc.is_file():
			print( Fore.CYAN + 'Skip: ecc file already exists: {}'.format( self.mFileEcc ) )
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

#---

parser = argparse.ArgumentParser()
parser.add_argument( 'action', choices=['create', 'check', 'fix'], help='The action to process' )
parser.add_argument( '-i', '--input', required=True, help='The input file' )
parser.add_argument( '-l', '--limit', type=int, default=100, help='Process only files below this limit size (Mo)' )
args = parser.parse_args()

#---

# Initialize after argparse, as it would be possible to get those values as parameters

# for 1 Mo:
#
# 4min: find_prime_polynomials      4min: find_prime_polynomials      4min: find_prime_polynomials
# 30s: encode_fast                  25s: encode_fast                  8min: encode_fast
# exp = 14                          exp = 14                          exp = 14
# result_size = 2**exp              result_size = 2**exp              result_size = 2**exp
# message_size = result_size-16     message_size = result_size-28     message_size = result_size-512

# default:
exp = 8
result_size = 255
message_size = 223
errors = result_size - message_size
# (255-223 = 32 => 2*e+v)
# may correct 16o (if v==0)

prim = 0x11b
# prim = unireedsolomon.ff.find_prime_polynomials( c_exp=exp, fast_primes=True, single=True )
coder = unireedsolomon.rs.RSCoder( result_size, message_size, prim=prim, c_exp=exp )

#---

eccs = []

input = Path( args.input )

if input.is_file():
	ecc = cEcc( input )
	if ecc.IsValidInput( args.limit ):
		ecc.Init( result_size, message_size, exp )
		eccs.append( ecc )
elif input.is_dir():
	for entry in input.rglob( '*' ):
		ecc = cEcc( entry )
		if ecc.IsValidInput( args.limit ):
			ecc.Init( result_size, message_size, exp )
			eccs.append( ecc )
else:
	print( Fore.RED + 'The input file doesn\'t exist: {}'.format( args.input ) )
	sys.exit( 1 )

#---

for ecc in eccs:
	if args.action == 'create':
		ecc.ProcessCreate( coder )
	elif args.action == 'check':
		ecc.ProcessCheck( coder )
	elif args.action == 'fix':
		ecc.ProcessFix( coder )

if args.action == 'check':
	print()
	print( '(The result is not foolproof:' )
	print( '(if it’s False, you’re sure the message was corrupted (or that you used the wrong RS parameters),' )
	print( '(if it’s True, it’s either that the message is correct, or that there are too many errors)' )
