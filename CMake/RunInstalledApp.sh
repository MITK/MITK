#!/bin/sh
binpath=$(dirname $(readlink -f $0))
appname=$(basename $0 .sh)
cd $binpath/bin
./$appname
