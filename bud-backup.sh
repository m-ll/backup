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
    echo "Usage: $0 [-h] [-f] [-d] -i /input/path | \
sagittarius-mike | sagittarius-family | \
virgo-wsl-mike | virgo-wsl-family | \
virgo-wsl-video | virgo-wsl-music | virgo-wsl-photo"
    echo '  -h: help me'
    echo '  -f: force a full backup'
    echo '  -d: dry run'
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

if [[ -z $INPUT_PATH ]]; then
    echo 'input path is empty !'
    usage
fi

# Fill/Convert some parameters
# - when the input path is not a real path, convert the 'code' to a real input/output paths
# - fill the duplicity options for each 'code' (exclude paths/...)

# Remove the interpretation of wildcard when setting --exclude parameters
GLOBIGNORE="*"

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
        OPTIONS=' --exclude /mnt/d/Users/Mike/AppData/Local'
        # ue projects exclude
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/**/Binaries'
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/**/Build'
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/**/DerivedDataCache'
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/**/Intermediate'
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/**/Saved'
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/**/Script'
        # ue package exclude
        OPTIONS+=' --exclude /mnt/d/Users/Mike/work/Unreal*/*package*'
        ;;
    'virgo-wsl-family') 
        INPUT_PATH=/mnt/d/Users/Family
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /mnt/d/Users/Family/AppData/Local'
        ;;

    'virgo-wsl-video') 
        INPUT_PATH=/mnt/f/Video
        OUTPUT_PATH="$(pwd)/nas/video"
        OPTIONS=' --no-compression'
        ;;
    'virgo-wsl-photo') 
        INPUT_PATH=/mnt/f/Photo
        OUTPUT_PATH="$(pwd)/nas/photo"
        OPTIONS=' --no-compression'
        ;;
    'virgo-wsl-music') 
        INPUT_PATH=/mnt/f/Music
        OUTPUT_PATH="$(pwd)/nas/music"
        OPTIONS=' --no-compression'
        ;;
    *)
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        ;;
esac

# Display all parameters containing a path and check its existence

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

echo 'Start stuff...'
# PATCH:
# - add --allow-source-mismatch
#   when problem with domain name in an incremental backup
#   (But try to avoid it if possible)
duplicity $FULL $DRY --volsize 2000 --progress --progress-rate 60 $OPTIONS \
            --encrypt-key 58771CEB5DE165CEA883EE80C2584942FBB9903F --sign-key A35008C5AFA617D1AD021782F265C63126F28B81 \
            "$INPUT_PATH" "file://$OUTPUT_PATH"

# Set (again) the interpretation of wildcard to manage chmod
GLOBIGNORE=

chmod -R 777 "$OUTPUT_PATH/"*
