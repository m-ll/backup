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
import os
import sys
sys.path.append( os.path.join( os.path.dirname( __file__ ), 'unireedsolomon' ) )

import unireedsolomon

# pour 1 mo:

# default:
# 4min
# 25s
exp = 8
result_size = 255
message_size = 223
errors = result_size - message_size
# (255-223 = 32 => 2*e+v)
# may correct 16o (if v==0)

prim = 0x11b
# prim = unireedsolomon.ff.find_prime_polynomials( c_exp=exp, fast_primes=True, single=True )
coder = unireedsolomon.rs.RSCoder( result_size, message_size, prim=prim, c_exp=exp )

src_name = f'zlib1.dll'
ecc_name = f'{src_name}.ecc-{exp}-{errors}'
dst_name = f'{src_name}.output'

print( '[{}] {} -> {}'.format( datetime.datetime.now().time(), src_name, dst_name ) )
print( '[{}] start reading'.format( datetime.datetime.now().time() ) )

with open( src_name, 'rb' ) as src, open( ecc_name, 'rb' ) as ecc, open( dst_name, 'wb' ) as dst:
	for src_chunk, ecc_chunk in zip( iter( lambda: src.read( message_size ), b'' ), iter( lambda: ecc.read( errors ), b'' ) ):
		padding = message_size - len( src_chunk )
		if padding:
			src_chunk = b'\0' * padding + src_chunk
		
		chunk = src_chunk + ecc_chunk

		if not coder.check_fast( chunk ):
			print( 'There is an error -_-' )
			print( 'Try to fix them with ecc-fix.py' )

print( '[{}] finish'.format( datetime.datetime.now().time() ) )

print( '(The result is not foolproof:' )
print( '(if it’s False, you’re sure the message was corrupted (or that you used the wrong RS parameters),' )
print( '(if it’s True, it’s either that the message is correct, or that there are too many errors)' )