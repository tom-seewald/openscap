#!/usr/bin/env bash
set -o errexit
set -o nounset

if [ "$#" -ne 1 ]; then
	echo "Usage: ./convert /path/to/directory/"
	exit
fi

new_str='/usr/bin/env bash'
orig_str='/bin/bash'

for f in $1/*.sh; do
       echo $f
       sed -i "s~$orig_str~$new_str~" $f
done

