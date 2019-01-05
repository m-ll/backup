#!/bin/bash

usage()
{
    echo "Usage: $0 [-f] [-d] -i /input/path | sagittarius-mike | sagittarius-family | sagittarius-videos | sagittarius-images | virgo-mike | virgo-family"
    echo '  -f: force a full backup'
    echo '  -d: dry run'
    echo '  -i: input path or preset'
    exit 2
}

FULL=
DRY=
INPUT_PATH=
while getopts ":fdi:o:p:" option; do
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
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

OPTIONS=

[[ -z $INPUT_PATH ]] && usage

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
        ;;
    'virgo-family') 
        INPUT_PATH=/cygdrive/d/Users/Family
        OUTPUT_PATH="./$INPUT_PATH"
        OPTIONS='--exclude /cygdrive/d/Users/Family/AppData/Local'
        ;;
    *)
        ;;
esac

echo 'input path: '$INPUT_PATH
if [[ ! -d $INPUT_PATH ]]; then
    echo 'The input path does not exist !'
    exit 5
fi

echo 'output path: '$OUTPUT_PATH
if [[ ! -d $OUTPUT_PATH ]]; then
    echo 'The output path does not exist.'
    echo 'Is it a first backup ?'
fi

read -p "Is it ok ? (y/n): " -r
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

echo 'Start stuff...'
duplicity $FULL $DRY --progress $OPTIONS \
            --encrypt-key 0DA52AFF --sign-key 62C590C4 \
            "$INPUT_PATH" "file://$OUTPUT_PATH"


# to restore
# duplicity "file://$INPUT_PATH" "$OUTPUT_PATH"

echo
echo 'REMOVE the .gnupg directory !!!'
echo
