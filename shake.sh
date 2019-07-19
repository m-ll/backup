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
	echo "Usage: $0 [-h] directory..."
	echo '  -h: help me'
	echo '  directory...: directories to shake files in place (default should be: mnt home nas)'
    exit 2
}

now()
{
	date '+%H:%M:%S'
}

# Process all the parameters
while getopts ":h" option; do
    case "${option}" in
        h|*)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

# At least, one directory to shake must be set
if [[ x"$@" == x ]]; then
	usage
fi

#---

echo "Shake files for: $@..."

# For each directory given
for dir in "$@"; do 
	# Find every files inside the directory and loop over each
	find "$dir" -type f -print0 | 
	while IFS= read -r -d $'\0' file; do 
        subs="$file".substitute
        echo "  [$(now)] $file ->  $subs"
    
        # In a row: rename the file -> duplicate it -> remove the renaming file
        mv "$file" "$subs" && cp "$subs" "$file" && rm "$subs"

        # Too slow -_-
        # diff "$subs" "$file"
        # if [[ $? -ne 0 ]]; then
        #     break
        # fi
	done
done