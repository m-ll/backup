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
    echo "Usage: $0 [-h] [-d] -g /path/to/the/.gnupg/path -i /input/path -o /output/path"
    echo '  -h: help me'
    echo '  -d: dry run'
    echo '  -g: path to .gnupg'
    echo '  -i: input path'
    echo '  -o: output path'
    exit 2
}

OSNAME="$(uname -s)"
case "$OSNAME" in
    # Linux*)
        # machine=Linux
        # ;;
    # Darwin*)
        # machine=Mac
        # ;;
    CYGWIN*)
        ulimit -n 1024
        ;;
    # MINGW*)
        # machine=MinGw
        # ;;
    *)
        ;;
esac

DRY=
GNUPG_PATH=
INPUT_PATH=
OUTPUT_PATH=
while getopts ":hdg:i:o:" option; do
    case "${option}" in
        d)
            DRY='--dry-run'
            ;;
        g)
            GNUPG_PATH=${OPTARG}
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

if [[ -z $GNUPG_PATH ]]; then
    echo 'gnupg path is empty !'
    usage
fi
if [[ -z $INPUT_PATH ]]; then
    echo 'input path is empty !'
    usage
fi
if [[ -z $OUTPUT_PATH ]]; then
    echo 'output path is empty !'
    usage
fi

echo '.gnupg path: '$GNUPG_PATH
if [[ ! -d "$GNUPG_PATH" ]]; then
    echo 'The .gnupg path does not exist !'
    exit 7
fi

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

OPTIONS_GPG="--homedir=$GNUPG_PATH"

read -p "Is it ok ? (y/n): " -r
if [[ ! "$REPLY" =~ ^[Yy]$ ]]; then
    exit 1
fi

echo 'Start stuff...'
duplicity $DRY --progress --progress-rate 60 --gpg-options "$OPTIONS_GPG" \
            "file://$INPUT_PATH" "$OUTPUT_PATH"

#---

read -p "Remove the .gnupg folder ($GNUPG_PATH) ? (y/n): " -r
if [[ "$REPLY" =~ ^[Yy]$ ]]; then
    read -p "Really ($GNUPG_PATH) ? (y/n): " -r
    if [[ "$REPLY" =~ ^[Yy]$ ]]; then
        rm -r $GNUPG_PATH
    fi
fi
