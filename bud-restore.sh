#!/usr/bin/env bash
#
# Copyright (c) 2019 m-ll. All Rights Reserved.
#
# Licensed under the MIT License.
# See LICENSE file in the project root for full license information.
#
# 2b13c8312f53d4b9202b6c8c0f0e790d10044f9a00d8bab3edf3cd287457c979
# 29c355784a3921aa290371da87bce9c1617b8584ca6ac6fb17fb37ba4a07d191
#

usage()
{
    echo "Usage: $0 [-h] [-d] -i /input/path -o /output/path"
    echo '  -h: help me'
    echo '  -d: dry run'
    echo '  -i: input path'
    echo '  -o: output path'
    exit 2
}

# Fix for cygwin (but cygwin should not be used anymore, replaced by WSL)
OSNAME="$(uname -s)"
case "$OSNAME" in
    # Linux*) ;;
    # Darwin*) ;;
    CYGWIN*) ulimit -n 1024;;
    # MINGW*) ;;
    *) ;;
esac

# Simulate a restore without changing anything on disk
DRY=
# The path of the backup to restore
INPUT_PATH=
# The path of the restored backup
OUTPUT_PATH=

# Process all the parameters
while getopts ":hdg:i:o:" option; do
    case "${option}" in
        d)
            DRY='--dry-run'
            ;;
        i)
            INPUT_PATH=${OPTARG}
            ;;
        o)
            OUTPUT_PATH=${OPTARG}
            ;;
        h|*)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

# Check all the mandatory parameters

if [[ -z $INPUT_PATH ]]; then
    echo 'input path is empty !'
    usage
fi
if [[ -z $OUTPUT_PATH ]]; then
    echo 'output path is empty !'
    usage
fi

# Display all parameters containing a path and check its existence

echo 'input path: '$INPUT_PATH
if [[ ! -d "$INPUT_PATH" ]]; then
    echo 'The input path does not exist !'
    exit 5
fi

echo 'output path: '$OUTPUT_PATH
if [[ ! -d "$OUTPUT_PATH" ]]; then
    echo 'The output path does not exist.'
    echo 'Is it a first restore ?'
fi

# Confirm all the paths displayed above
read -p "Is it ok ? (y/n): " -r
if [[ ! "$REPLY" =~ ^[Yy]$ ]]; then
    exit 1
fi

# Start the restoration process

echo 'Start stuff...'
duplicity $DRY --progress --progress-rate 60 \
            "file://$INPUT_PATH" "$OUTPUT_PATH"
