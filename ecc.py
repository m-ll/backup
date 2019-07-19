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
from pathlib import Path
import sys
# To use directly those inside the backup directory (but slower as unireedsolomon don't use cython in this case)
# sys.path.append( os.path.join( os.path.dirname( __file__ ), 'no-dep', 'unireedsolomon' ) )
# sys.path.append( os.path.join( os.path.dirname( __file__ ), 'no-dep', 'colorama' ) )

from colorama import init, Fore
init( autoreset=True )
import unireedsolomon

from ecc.ecc import cEcc

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

# The coder is outside the cEcc class, to not recreate it for each file

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
