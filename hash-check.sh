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

for f in *.sha512; do
    [ -e "$f" ] || exit 0
done

last_sha512sums=$(ls *.sha512 | sort | tail -n 1)

echo "Check: $last_sha512sums ..."
sha512sum --quiet --check $last_sha512sums
error=$?

exit $error
