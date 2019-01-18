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
import glob
import os
import re
import sys
sys.path.append( os.path.join( os.path.dirname( __file__ ), 'unireedsolomon' ) )
sys.path.append( os.path.join( os.path.dirname( __file__ ), 'colorama' ) )

from colorama import init, Fore
init( autoreset=True )
import unireedsolomon

#---

def now():
	return datetime.datetime.now().time()

def print_progress( iCurrent, iTotal ):
	print( '[{}]   processed: {:.2f}%'.format( now(), iCurrent / iTotal * 100.0 ), end='\r' )

#---

parser = argparse.ArgumentParser()
parser.add_argument( 'action', choices=['create', 'check', 'fix'], help='The action to process' )
parser.add_argument( '-i', '--input', required=True, help='The input file' )
args = parser.parse_args()

#---

# Initialize after argparse, as it would be possible to get values as parameters

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

pathfiles = []

if os.path.isfile( args.input ):
	pathfiles.append( { 
						'input': args.input, 
						'ecc': f'{args.input}.ecc-{result_size}-{message_size}-{exp}', 
						'fix': f'{args.input}.regenerated'
					  } )
elif os.path.isdir( args.input ):
	root = args.input
	for entry in glob.iglob( root + '**/.*', recursive=True ):
		if re.match( r'.+\.regenerated$', entry ) or re.match( r'.+\.ecc-.+-.+', entry ):
			continue
		if not os.path.isfile( entry ):
			continue
		pathfiles.append( { 
							'input': entry, 
							'ecc': f'{entry}.ecc-{result_size}-{message_size}-{exp}', 
							'fix': f'{entry}.regenerated'
						} )
	for entry in glob.iglob( root + '**/*', recursive=True ):
		if re.match( r'.+\.regenerated$', entry ) or re.match( r'.+\.ecc-.+-.+', entry ):
			continue
		if not os.path.isfile( entry ):
			continue
		pathfiles.append( { 
							'input': entry, 
							'ecc': f'{entry}.ecc-{result_size}-{message_size}-{exp}', 
							'fix': f'{entry}.regenerated'
						} )
else:
	print( Fore.RED + 'The input file doesn\'t exist: {}'.format( args.input ) )
	sys.exit( 1 )

#---

for pathfile in pathfiles:

	src_name = pathfile['input']
	ecc_name = pathfile['ecc']
	fix_name = pathfile['fix']

	size_to_process = os.path.getsize( src_name )

	if args.action == 'create':
		if os.path.isfile( ecc_name ):
			print( Fore.CYAN + '  Skip: ecc file already exists: {}'.format( ecc_name ) )
			continue
	elif args.action == 'check':
		if not os.path.isfile( ecc_name ):
			print( Fore.CYAN + '  Skip: ecc file doesn\'t exists: {}'.format( ecc_name ) )
			continue
	elif args.action == 'fix':
		if not os.path.isfile( ecc_name ):
			print( Fore.CYAN + '  Skip: ecc file doesn\'t exists: {}'.format( ecc_name ) )
			continue

	#---

	if args.action == 'create':

		print( '[{}] {} -> {}'.format( now(), src_name, ecc_name ) )

		# with open( src_name, 'rb' ) as src, open( dst_name, 'wb' ) as dst, open( ecc_name, 'wb' ) as ecc:
		with open( src_name, 'rb' ) as src, open( ecc_name, 'wb' ) as ecc:
			for chunk in iter( lambda: src.read( message_size ), b'' ):
				padding = message_size - len( chunk )
				if padding:
					chunk = b'\0' * padding + chunk

				c = coder.encode_fast( chunk, return_string=False )
				
				# If using exp > 8 -> value may be > 255 -> can't store them inside 1 byte
				# b = bytearray()
				# for n in c:
					# b.extend( n.to_bytes( ( n.bit_length() + 7 ) // 8, byteorder='little' ) )
				# # dst.write( b )

				b = bytes( c )
				# dst.write( b[padding:message_size] )
				ecc.write( b[message_size:] )

				print_progress( src.tell(), size_to_process )
			print() # To go to next line after previous \r

	elif args.action == 'check':

		print( '[{}] {} + {}'.format( now(), src_name, ecc_name ) )

		with open( src_name, 'rb' ) as src, open( ecc_name, 'rb' ) as ecc:
			for src_chunk, ecc_chunk in zip( iter( lambda: src.read( message_size ), b'' ), iter( lambda: ecc.read( errors ), b'' ) ):
				padding = message_size - len( src_chunk )
				if padding:
					src_chunk = b'\0' * padding + src_chunk
				chunk = src_chunk + ecc_chunk

				check = coder.check_fast( chunk )
				if not check:
					print()
					print( Fore.RED + 'There are errors -_-' )
						
				print_progress( src.tell(), size_to_process )
			print()
			
	elif args.action == 'fix':
		
		print( '[{}] {} + {} -> {}'.format( now(), src_name, ecc_name, fix_name ) )

		with open( src_name, 'rb' ) as src, open( ecc_name, 'rb' ) as ecc, open( fix_name, 'wb' ) as fix:
			for src_chunk, ecc_chunk in zip( iter( lambda: src.read( message_size ), b'' ), iter( lambda: ecc.read( errors ), b'' ) ):
				padding = message_size - len( src_chunk )
				if padding:
					src_chunk = b'\0' * padding + src_chunk
				chunk = src_chunk + ecc_chunk

				d, code = coder.decode_fast( chunk, nostrip=True, return_string=False )

				b = bytes( d )
				fix.write( b[padding:] )

				print_progress( src.tell(), size_to_process )
			print()

if args.action == 'check':
	print()
	print( '(The result is not foolproof:' )
	print( '(if it’s False, you’re sure the message was corrupted (or that you used the wrong RS parameters),' )
	print( '(if it’s True, it’s either that the message is correct, or that there are too many errors)' )
