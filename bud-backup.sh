#!/bin/bash

usage()
{
    echo "Usage: $0 [-f] [-d] -g /path/to/the/.gnupg/path -i /input/path | sagittarius-mike | sagittarius-family | sagittarius-videos | sagittarius-images | virgo-mike | virgo-family"
    echo '  -f: force a full backup'
    echo '  -d: dry run'
    echo '  -g: path to .gnupg'
    echo '  -i: input path'
    exit 2
}

FULL=
DRY=
GNUPG_PATH=
INPUT_PATH=
while getopts ":fdg:i:" option; do
    case "${option}" in
        f)
            FULL='full'
            ;;
        d)
            DRY='--dry-run'
            ;;
        g)
            GNUPG_PATH=${OPTARG}
            ;;
        i)
            INPUT_PATH=${OPTARG}
            ;;
        *)
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

OPTIONS=

case $INPUT_PATH in
    'sagittarius-mike') 
        INPUT_PATH=/home/mike
        OUTPUT_PATH="./$INPUT_PATH"
        ;;
    'sagittarius-family') 
        INPUT_PATH=/home/family
        OUTPUT_PATH="./$INPUT_PATH"
        OPTIONS='--exclude /home/family/Vidéos --exclude /home/family/Images'
        ;;
    'sagittarius-videos') 
        INPUT_PATH=/home/family/Vidéos
        OUTPUT_PATH="./videos"
        ;;
    'sagittarius-images') 
        INPUT_PATH=/home/family/Images
        OUTPUT_PATH="./images"
        ;;
    'virgo-mike') 
        INPUT_PATH=/cygdrive/d/Users/Mike
        OUTPUT_PATH="./$INPUT_PATH"
        OPTIONS='--exclude /cygdrive/d/Users/Mike/AppData/Local'
        ulimit -n 1024
        ;;
    'virgo-family') 
        INPUT_PATH=/cygdrive/d/Users/Family
        OUTPUT_PATH="./$INPUT_PATH"
        OPTIONS='--exclude /cygdrive/d/Users/Family/AppData/Local'
        ulimit -n 1024
        ;;
    *)
        OUTPUT_PATH="./$INPUT_PATH"
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
duplicity $FULL $DRY --progress --gpg-options "$OPTIONS_GPG" $OPTIONS \
            --encrypt-key 0DA52AFF --sign-key 62C590C4 \
            "$INPUT_PATH" "file://$OUTPUT_PATH"


read -p "Remove the .gnupg folder ($GNUPG_PATH) ? (y/n): " -r
if [[ "$REPLY" =~ ^[Yy]$ ]]; then
    read -p "Really ($GNUPG_PATH) ? (y/n): " -r
    if [[ "$REPLY" =~ ^[Yy]$ ]]; then
        rm -r $GNUPG_PATH
    fi
fi
