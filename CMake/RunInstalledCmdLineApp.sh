#!/bin/sh
binpath=$(dirname "$(dirname "$(readlink -f "$0")")")
appname=$(basename "$0" .sh)
export LD_LIBRARY_PATH="$binpath/bin":"$LD_LIBRARY_PATH"
"$binpath/bin/$appname" "$@"
