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
	echo "Usage: $0 [-h] [-f] directory..."
	echo '  -h: help me'
	echo '  -f: force overwriting exsting hash file'
	echo '  directory...: directories to compute hash (default should be: cygdrive home nas)'
    exit 2
}

force=0
while getopts ":hf" option; do
    case "${option}" in
        f)
            force=1
            ;;
        h|*)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [[ x"$@" == x ]]; then
	usage
fi

#---

echo "Create new hashes for: $@..."

for dir in "$@"; do 
	find "$dir" -type f ! -iname "*.sha512" -print0 | 
	while IFS= read -r -d $'\0' file; do 
		file_hash="$file.sha512"
		
		if [[ -f "$file_hash" && $force -ne 1 ]]; then
			echo "  Skip: hash file ($file_hash) already exist."
			continue
		fi

		echo "  Process: $file..."
		sha512sum "$file" > "$file_hash"
	done
done
