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

from ecc.ecc_schifra import cEccSchifra

#---

parser = argparse.ArgumentParser()
parser.add_argument( 'action', choices=['create', 'check-size', 'fix', 'fix-and-compare'], help='The action to process' )
parser.add_argument( '-i', '--input', nargs='+', type=Path, required=True, help='The input file' )
args = parser.parse_args()

#---

eccs = []

not_existing_input = []

for input in args.input:
    if input.is_file():
        ecc = cEccSchifra( input )
        if ecc.IsValidInput():
            eccs.append( ecc )
    elif input.is_dir():
        for entry in input.rglob( '*' ):
            ecc = cEccSchifra( entry )
            if ecc.IsValidInput():
                eccs.append( ecc )
    else:
        not_existing_input.append( input )

if not_existing_input:
    print( Fore.RED + 'Some input files don\'t exist:' )
    [ print( Fore.RED + str(input) ) for input in not_existing_input ]
    sys.exit( 1 )

#---

print( Fore.GREEN + f'Info: {len(eccs)} files to process' )

for i, ecc in enumerate( eccs ):
    progress_ratio = i / ( len(eccs) - 1 )
    progress_percent = int( progress_ratio * 100 )
    print( Fore.GREEN + f'Info: file {i}/{len(eccs) - 1} - {progress_percent}%' )

    if args.action == 'create':
        ecc.ProcessCreate()
    elif args.action == 'check-size':
        ecc.ProcessCheckSize()
    elif args.action == 'fix':
        ecc.ProcessFix( False )
    elif args.action == 'fix-and-compare':
        ecc.ProcessFix( True )
