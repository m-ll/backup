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
    echo "Usage: $0 [-h] [-f] [-d] -k {yes|no|ask} -g /path/to/the/.gnupg/path -i /input/path | \
sagittarius-mike | sagittarius-family | \
virgo-wsl-mike | virgo-wsl-family | \
virgo-wsl-video | virgo-wsl-music | virgo-wsl-photo"
    echo '  -h: help me'
    echo '  -f: force a full backup'
    echo '  -d: dry run'
    echo '  -k: keep .gnupg files'
    echo '  -g: path to .gnupg'
    echo '  -i: input path'
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

# Make a full backup
FULL=
# Simulate a backup without changing anything on disk
DRY=
# Answer to the last question to keep or not the gnupg directory
KEEP_GNUPG=
# The gnupg directory with keys
GNUPG_PATH=
# The path to backup
INPUT_PATH=

# Process all the parameters
while getopts ":hfdk:g:i:" option; do
    case "${option}" in
        f)
            FULL='full'
            ;;
        d)
            DRY='--dry-run'
            ;;
        k)
            KEEP_GNUPG=${OPTARG}
            ;;
        g)
            GNUPG_PATH=${OPTARG}
            ;;
        i)
            INPUT_PATH=${OPTARG}
            ;;
        h|*)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

# Check all the mandatory parameters

if [[ -z $KEEP_GNUPG ]]; then
    echo 'keep gnupg is empty !'
    usage
fi
if [[ x"$KEEP_GNUPG" != x'yes' && x"$KEEP_GNUPG" != x'no' && x"$KEEP_GNUPG" != x'ask' ]]; then
    echo 'wrong keep argument !'
    usage
fi
if [[ -z $GNUPG_PATH ]]; then
    echo 'gnupg path is empty !'
    usage
fi
if [[ -z $INPUT_PATH ]]; then
    echo 'input path is empty !'
    usage
fi

# Fill/Convert some parameters
# - when the input path is not a real path, convert the 'code' to a real input/output paths
# - fill the duplicity options for each 'code' (exclude paths/...)

OPTIONS=

case $INPUT_PATH in
    'sagittarius-mike') 
        INPUT_PATH=/home/mike
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /home/mike/.cache --exclude /home/mike/.macromedia'
        ;;                                   # .macromedia + .local = infinite loop
    'sagittarius-family') 
        INPUT_PATH=/home/family
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /home/family/Vidéos --exclude /home/family/Images --exclude /home/family/Musique --exclude /home/family/.cache --exclude /home/family/.macromedia'
        ;;                                                                                                                                # .macromedia + .local = infinite loop

    # 'sagittarius-video') 
    #     INPUT_PATH=/home/family/Vidéos
    #     OUTPUT_PATH="$(pwd)/nas/video"
    #     ;;
    # 'sagittarius-photo') 
    #     INPUT_PATH=/home/family/Images
    #     OUTPUT_PATH="$(pwd)/nas/photo"
    #     ;;
    # 'sagittarius-music') 
    #     INPUT_PATH=/home/family/Musique
    #     OUTPUT_PATH="$(pwd)/nas/music"
    #     ;;

    # 'virgo-cyg-mike') 
    #     INPUT_PATH=/cygdrive/d/Users/Mike
    #     OUTPUT_PATH="$(pwd)/$INPUT_PATH"
    #     OPTIONS='--exclude /cygdrive/d/Users/Mike/AppData/Local'
    #     ;;
    # 'virgo-cyg-family') 
    #     INPUT_PATH=/cygdrive/d/Users/Family
    #     OUTPUT_PATH="$(pwd)/$INPUT_PATH"
    #     OPTIONS='--exclude /cygdrive/d/Users/Family/AppData/Local'
    #     ;;

    'virgo-wsl-mike') 
        INPUT_PATH=/mnt/d/Users/Mike
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /mnt/d/Users/Mike/AppData/Local'
        ;;
    'virgo-wsl-family') 
        INPUT_PATH=/mnt/d/Users/Family
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /mnt/d/Users/Family/AppData/Local'
        ;;

    'virgo-wsl-video') 
        INPUT_PATH=/mnt/f/Video
        OUTPUT_PATH="$(pwd)/nas/video"
        ;;
    'virgo-wsl-photo') 
        INPUT_PATH=/mnt/f/Photo
        OUTPUT_PATH="$(pwd)/nas/photo"
        ;;
    'virgo-wsl-music') 
        INPUT_PATH=/mnt/f/Music
        OUTPUT_PATH="$(pwd)/nas/music"
        ;;
    *)
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        ;;
esac

# Display all parameters containing a path and check its existence

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
    echo 'Is it a first backup ?'
fi

# Confirm all the paths displayed above
read -p "Is it ok ? (y/n): " -r
if [[ ! "$REPLY" =~ ^[Yy]$ ]]; then
    exit 1
fi

if [[ ! -d "$OUTPUT_PATH" ]]; then              # if first backup with none existing directories
    if [[ "$INPUT_PATH" =~ ^/home/.* ]]; then   # only when done on ubuntu (improve the condition)
        mkdir -p "$OUTPUT_PATH"
        chmod -R 777 "$(pwd)/home"              # set the permissions to be readable on wsl
    fi
fi

# Start the backup process

OPTIONS_GPG="--homedir=$GNUPG_PATH"

echo 'Start stuff...'
duplicity $FULL $DRY --volsize 2000 --progress --progress-rate 60 --gpg-binary gpg1 --gpg-options "$OPTIONS_GPG" $OPTIONS \
            --encrypt-key 63BAF710 --sign-key CA12167B \
            "$INPUT_PATH" "file://$OUTPUT_PATH"

chmod -R 777 "$OUTPUT_PATH/"*

#---

# Ask to keep/remove the gnupg directory

if [[ $KEEP_GNUPG == 'ask' ]]; then
    read -p "Remove the .gnupg folder ($GNUPG_PATH) ? (y/n): " -r
    if [[ "$REPLY" =~ ^[Yy]$ ]]; then
        read -p "Really ($GNUPG_PATH) ? (y/n): " -r
        if [[ "$REPLY" =~ ^[Yy]$ ]]; then
            rm -r $GNUPG_PATH
        fi
    fi
elif [[ $KEEP_GNUPG == 'yes' ]]; then
    echo '.gnupg folder still here, think to remove it when finished'
    echo
elif [[ $KEEP_GNUPG == 'no' ]]; then
    rm -r $GNUPG_PATH
    echo '.gnupg folder has been removed'
    echo
fi
