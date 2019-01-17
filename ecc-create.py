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

# 4min: find_prime_polynomials
# 30s: encode_fast
# exp = 14
# result_size = 2**exp
# message_size = result_size-16

# 4min: find_prime_polynomials
# 25s: encode_fast
# exp = 14
# result_size = 2**exp
# message_size = result_size-28

# 4min: find_prime_polynomials
# 8min: encode_fast
# exp = 14
# result_size = 2**exp
# message_size = result_size-512

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

src_name = 'zlib1.dll'
dst_name = f'{src_name}.wo-ecc-{exp}-{errors}'
ecc_name = f'{src_name}.ecc-{exp}-{errors}'

print( '[{}] {} -> {}'.format( datetime.datetime.now().time(), src_name, dst_name ) )
print( '[{}] start reading'.format( datetime.datetime.now().time() ) )

with open( src_name, 'rb' ) as src, open( dst_name, 'wb' ) as dst, open( ecc_name, 'wb' ) as ecc:
	for chunk in iter( lambda: src.read( message_size ), b'' ):
		padding = message_size - len( chunk )
		if padding:
			chunk = b'\0' * padding + chunk

		c = coder.encode_fast( chunk, return_string=False )
		
		# If using exp > 8 -> value may be > 255 -> can't store them inside 1 byte
		# b = bytearray()
		# for n in c:
			# b.extend( n.to_bytes( ( n.bit_length() + 7 ) // 8, byteorder='little' ) )
		# dst.write( b )

		b = bytes( c )
		dst.write( b[padding:message_size] )
		ecc.write( b[message_size:] )

		# print( dst.tell() )
		# print()

print( '[{}] finish'.format( datetime.datetime.now().time() ) )
