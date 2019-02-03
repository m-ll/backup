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
sagittarius-video | sagittarius-music | sagittarius-photo | \
virgo-cyg-mike | virgo-cyg-family | \
virgo-wsl-mike | virgo-wsl-family \
virgo-wsl-video | virgo-wsl-music | virgo-wsl-photo"
    echo '  -h: help me'
    echo '  -f: force a full backup'
    echo '  -d: dry run'
    echo '  -k: keep .gnupg files'
    echo '  -g: path to .gnupg'
    echo '  -i: input path'
    exit 2
}

OSNAME="$(uname -s)"
case "$OSNAME" in
    # Linux*) ;;
    # Darwin*) ;;
    CYGWIN*) ulimit -n 1024;;
    # MINGW*) ;;
    *) ;;
esac

FULL=
DRY=
KEEP_GNUPG=
GNUPG_PATH=
INPUT_PATH=
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

OPTIONS=

case $INPUT_PATH in
    'sagittarius-mike') 
        INPUT_PATH=/home/mike
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        ;;
    'sagittarius-family') 
        INPUT_PATH=/home/family
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /home/family/Vidéos --exclude /home/family/Images --exclude /home/family/Musique --exclude /home/family/.cache --exclude /home/family/.macromedia'
        ;;                                                                                                                                # .macromedia + .local = infinite loop
        
    'sagittarius-video') 
        INPUT_PATH=/home/family/Vidéos
        OUTPUT_PATH="$(pwd)/nas/video"
        ;;
    'sagittarius-photo') 
        INPUT_PATH=/home/family/Images
        OUTPUT_PATH="$(pwd)/nas/photo"
        ;;
    'sagittarius-music') 
        INPUT_PATH=/home/family/Musique
        OUTPUT_PATH="$(pwd)/nas/music"
        ;;
        
    'virgo-cyg-mike') 
        INPUT_PATH=/cygdrive/d/Users/Mike
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /cygdrive/d/Users/Mike/AppData/Local'
        ;;
    'virgo-cyg-family') 
        INPUT_PATH=/cygdrive/d/Users/Family
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        OPTIONS='--exclude /cygdrive/d/Users/Family/AppData/Local'
        ;;
        
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
        INPUT_PATH=/mnt/d/Video
        OUTPUT_PATH="$(pwd)/nas/video"
        ;;
    'virgo-wsl-photo') 
        INPUT_PATH=/mnt/d/Photo
        OUTPUT_PATH="$(pwd)/nas/photo"
        ;;
    'virgo-wsl-music') 
        INPUT_PATH=/mnt/d/Music
        OUTPUT_PATH="$(pwd)/nas/music"
        ;;
    *)
        OUTPUT_PATH="$(pwd)/$INPUT_PATH"
        ;;
esac

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

OPTIONS_GPG="--homedir=$GNUPG_PATH"

read -p "Is it ok ? (y/n): " -r
if [[ ! "$REPLY" =~ ^[Yy]$ ]]; then
    exit 1
fi

echo 'Start stuff...'
duplicity $FULL $DRY --volsize 2000 --progress --progress-rate 60 --gpg-options "$OPTIONS_GPG" $OPTIONS \
            --encrypt-key 0DA52AFF --sign-key 62C590C4 \
            "$INPUT_PATH" "file://$OUTPUT_PATH"

chmod -R 777 "$OUTPUT_PATH/"*

#---

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
