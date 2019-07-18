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
	echo '  directory...: directories to check hash (default should be: mnt home nas)'
	exit 2
}

now()
{
	date '+%H:%M:%S'
}

while getopts ":h" option; do
    case "${option}" in
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

echo "Check hashes for: $@..."

for dir in "$@"; do 
	find "sha512/$dir" -type f -iname "*.sha512" -print0 | 
	while IFS= read -r -d $'\0' file_hash; do 
		echo "  [$(now)] Check: $file_hash..."
		sha512sum --quiet --check "$file_hash"
	done
done



