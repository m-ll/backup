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
	echo '  -f: force overwriting existing hash file'
	echo '  directory...: directories to compute hash (default should be: mnt home nas)'
	echo
	echo '  Output stuff will always be inside ./sha512/'
    exit 2
}

now()
{
	date '+%H:%M:%S'
}

# Create the sha512 file even if it already exists
force=0

# Process all the parameters
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

# At least, one directory to hash must be set
if [[ x"$@" == x ]]; then
	usage
fi

#---

echo "Create new hashes for *.gpg files inside: $@..."

# For each directory given
for dir in "$@"; do 
	# Find every .gpg files inside the directory and loop over each
	find "$dir" -type f -iname "*.gpg" -print0 | 
	while IFS= read -r -d $'\0' file; do 
		# All the new .sha512 files will be create inside ./sha512/ directory
		file_hash="sha512/$file.sha512"
		
		# Skip or overwrite the existing .sha512 file
		if [[ -s "$file_hash" ]]; then
			if [[ $force -eq 0 ]]; then
				echo "  [$(now)] Skip: hash file ($file_hash) already exist."
				continue
			fi
		fi

		echo "  [$(now)] Process: $file..."
		mkdir -p "$(dirname "$file_hash")"
		sha512sum "$file" > "$file_hash"
		chmod 777 "$file_hash"
	done
done
