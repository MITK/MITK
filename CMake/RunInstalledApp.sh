#!/bin/sh
binpath=$(dirname "$(readlink -f "$0")")
appname=$(basename "$0" .sh)
export LD_LIBRARY_PATH="$binpath/bin":"$binpath/bin/plugins":$LD_LIBRARY_PATH
"$binpath/bin/$appname" "$@"
