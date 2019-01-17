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
import os
import sys
sys.path.append( os.path.join( os.path.dirname( __file__ ), 'unireedsolomon' ) )

import unireedsolomon

parser = argparse.ArgumentParser()
parser.add_argument( 'action', choices=['create', 'check', 'fix'], help='The action to process' )
parser.add_argument( '-i', '--input', required=True, help='The input file' )
parser.add_argument( '-e', '--ecc', required=True, help='The corresponding ecc file' )
parser.add_argument( '-f', '--fix', help='The fixed output file (action=fix)' )
args = parser.parse_args()

if args.action == 'fix' and args.fix is None:
	parser.print_help( sys.stderr )
	sys.exit( 1 )

if not os.path.isfile( args.input ):
	print( 'The input file doesn\'t exist: {}'.format( args.input ), file=sys.stderr )
	sys.exit( 1 )

if args.action == 'create':
	if os.path.isfile( args.ecc ):
		print( 'An ecc file already exists: {}'.format( args.ecc ), file=sys.stderr )
		sys.exit( 1 )
elif args.action == 'check':
	if not os.path.isfile( args.ecc ):
		print( 'An ecc file doesn\'t exists: {}'.format( args.ecc ), file=sys.stderr )
		sys.exit( 1 )
elif args.action == 'fix':
	if not os.path.isfile( args.ecc ):
		print( 'An ecc file doesn\'t exists: {}'.format( args.ecc ), file=sys.stderr )
		sys.exit( 1 )
	elif os.path.isfile( args.fix ):
		print( 'A fix file already exists: {}'.format( args.fix ), file=sys.stderr )
		sys.exit( 1 )

#---

def now():
	return datetime.datetime.now().time()

def print_progress( iCurrent, iTotal ):
	print( '  processed: {:.2f}%'.format( iCurrent / iTotal * 100.0 ), end='\r' )

#---

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

src_name = args.input
ecc_name = args.ecc

size_to_process = os.path.getsize( src_name )

if args.action == 'create':

	print( '[{}] {} -> {}'.format( now(), src_name, ecc_name ) )
	print( '[{}] begin create ...'.format( now() ) )

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

	print( '[{}] end create'.format( now() ) )

elif args.action == 'check':

	print( '[{}] {} + {}'.format( now(), src_name, ecc_name ) )
	print( '[{}] begin check ...'.format( now() ) )

	with open( src_name, 'rb' ) as src, open( ecc_name, 'rb' ) as ecc:
		for src_chunk, ecc_chunk in zip( iter( lambda: src.read( message_size ), b'' ), iter( lambda: ecc.read( errors ), b'' ) ):
			padding = message_size - len( src_chunk )
			if padding:
				src_chunk = b'\0' * padding + src_chunk
			chunk = src_chunk + ecc_chunk

			if args.action == 'check':
				check = coder.check_fast( chunk )
				if not check:
					print()
					print( 'There are errors -_-' )
					sys.exit( 10 )
					
			print_progress( src.tell(), size_to_process )
		print()

	print( '[{}] end check'.format( now() ) )
	print()
	print( '(The result is not foolproof:' )
	print( '(if it’s False, you’re sure the message was corrupted (or that you used the wrong RS parameters),' )
	print( '(if it’s True, it’s either that the message is correct, or that there are too many errors)' )

elif args.action == 'fix':
	
	fix_name = args.fix

	print( '[{}] {} + {} -> {}'.format( now(), src_name, ecc_name, fix_name ) )
	print( '[{}] begin fix ...'.format( now() ) )

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

	print( '[{}] end fix'.format( now() ) )

