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
from pathlib import Path
import subprocess

from colorama import init, deinit, Fore, Style
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
class cEccSchifra:

    ## The constructor
    #
    #  @param  iFileInput  pathlib.Path  The input file
    def __init__( self, iFileInput ):
        self.mFileInput = iFileInput

        self.mSizeResult = 255
        self.mSizeMessage = 223
        self.mSizeFec = self.mSizeResult - self.mSizeMessage
        self.mExp = 8

        executable_path = Path( __file__ ).resolve().parent.parent
        self.mExecutable = Path( executable_path ) / f'ecc-schifra-{self.mSizeResult}-{self.mSizeFec}-{self.mExp}'

        # Contains only the ecc data
        self.mFileEcc = Path( f'ecc-schifra-{self.mSizeResult}-{self.mSizeFec}-{self.mExp}' ) / self.mFileInput.with_suffix( self.mFileInput.suffix + f'.ecc-schifra-{self.mSizeResult}-{self.mSizeFec}-{self.mExp}' )
        self.mFileFix = Path( 'ecc-regenerated' ) / self.mFileInput.with_suffix( self.mFileInput.suffix + '.regenerated' )

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

    #---

    ## Create the ecc file
    def ProcessCreate( self ):
        size_to_process = self.mFileInput.stat().st_size

        if self.mFileEcc.is_file():
            print( Fore.CYAN + 'Skip: ecc file already exists: {}'.format( self.mFileEcc ) )
            return

        # start = datetime.datetime.now()
        # print( '[{}] {} -> {}'.format( now(), self.mFileInput, self.mFileEcc ) )

        self.mFileEcc.parent.mkdir( parents=True, exist_ok=True )

        #---

        command = [ self.mExecutable, 
                    '-v',
                    'encode',
                    '-i', self.mFileInput,
                    '-o', self.mFileEcc ]

        self._PrintHeader( command )
        completed_process = subprocess.run( command )
        self._PrintFooter( command )

        #---

        self.mFileEcc.chmod( 0o777 )
        # diff = datetime.datetime.now() - start
        # print( '[{}] time used: {} (at {:.2f} Mo/s)'.format( now(), diff, ( size_to_process / 1000000 ) / diff.total_seconds() ) )

    ## Fix the input file via its ecc file
    def ProcessFix( self ):
        size_to_process = self.mFileInput.stat().st_size

        if not self.mFileEcc.is_file():
            print( Fore.CYAN + 'Skip: ecc file doesn\'t exists: {}'.format( self.mFileEcc ) )
            return

        # start = datetime.datetime.now()
        # print( '[{}] {}, {} -> {}'.format( now(), self.mFileInput, self.mFileEcc, self.mFileFix ) )

        self.mFileFix.parent.mkdir( parents=True, exist_ok=True )

        #---

        command = [ self.mExecutable, 
                    '-v',
                    'decode',
                    '-i', self.mFileInput,
                    '-e', self.mFileEcc,
                    '-o', self.mFileFix ]

        self._PrintHeader( command )
        completed_process = subprocess.run( command )
        self._PrintFooter( command )

        #---

        self.mFileFix.chmod( 0o777 )
        # print( '[{}] time used: {}'.format( now(), datetime.datetime.now() - start ) )

    ## Check the ecc file size (proportional to the input file size)
    #
    #  @example: with mSizeResult = 255 & mSizeMessage = 223 & size_input = 411
    #
    #  block_size_ecc = 32 (=255-223)
    #  size_ecc = 64 (it's always a multiple of block_size_ecc)
    #  number_of_block_ecc = 2
    #  
    #            3 3        6 6
    #   0        1 2        3 4
    #   ---------------------
    #  |          |          |
    #   ---------------------
    #  
    #                  max_size_input = 446 (=223*2)
    #   .-------------------------------------------------.
    #               size_input = 411
    #   .-------------------------------------.
    #                                     end_input
    #                                         |
    #                           2 2           4           4 4
    #                           2 2           1           4 4
    #   0                       2 3           0           5 6
    #   ------------------------- ------------- -----------
    #  |                         |             :           |
    #   ------------------------- ------------- -----------
    #                             |                       | |
    #                             |                       | begin_of_next_of_last_block_input
    #                 begin_of_last_block_input end_of_last_block_input
    #
    def ProcessCheckSize( self ):
        size_input = self.mFileInput.stat().st_size

        if not self.mFileEcc.is_file():
            print( Fore.CYAN + 'Skip: ecc file doesn\'t exists: {}'.format( self.mFileEcc ) )
            return

        size_ecc = self.mFileEcc.stat().st_size
        
        block_size_ecc = self.mSizeResult - self.mSizeMessage
        block_size_input = self.mSizeMessage

        number_of_block_ecc, remainder = divmod( size_ecc, block_size_ecc )
        if remainder:
            print( Fore.RED + 'The ecc file size {} is not a multiple of {}'.format( size_ecc, block_size_ecc ) )
            return

        max_size_input = number_of_block_ecc * block_size_input

        #---

        end_input = size_input - 1

        begin_of_next_of_last_block_input = max_size_input
        end_of_last_block_input = begin_of_next_of_last_block_input - 1
        begin_of_last_block_input = begin_of_next_of_last_block_input - block_size_input

        if( end_input < begin_of_last_block_input or end_input > end_of_last_block_input ):
            print( Fore.RED + 'The input file size {} is not between {} and {}'.format( size_input, begin_of_last_block_input + 1, end_of_last_block_input + 1 ) )
            return

    #---

    ## Print information before running a command
    #
    #  @param  iCommand  string[]  The command which will be executed
    def _PrintHeader( self, iCommand ):
        pass
        # deinit()
        # init( autoreset=False )
        # print( Fore.YELLOW )
        # print( *iCommand )
        # print( Style.RESET_ALL )

    ## Execute a command
    #
    #  @param  iCommand  string[]  The command which will be executed
    def _Run( self, iCommand ):
        # return 'xxxxxxxxxxxxxxxxxx'
        return subprocess.run( iCommand )

    ## Print information after running a command
    #
    #  @param  iCompletedProcess  subprocess.cCompletedProcess  The result of the command which was executed
    def _PrintFooter( self, iCompletedProcess ):
        pass
        # print( Fore.YELLOW )
        # print( iCompletedProcess )
        # print( Style.RESET_ALL )
        # deinit()
        # init( autoreset=True )
