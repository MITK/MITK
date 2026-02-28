#!/bin/sh
binpath=$(dirname "$(dirname "$(readlink -f "$0")")")
appname=$(basename "$0" .sh)
export LD_LIBRARY_PATH="$binpath/python/lib":"$binpath/bin":"$binpath/bin/plugins":"$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$binpath/bin":"$binpath/bin/plugins"
export QT_QPA_PLATFORM=xcb
export QTWEBENGINEPROCESS_PATH="$binpath/bin/QtWebEngineProcess"
export QTWEBENGINE_RESOURCES_PATH="$binpath/bin/resources"
export QTWEBENGINE_LOCALES_PATH="$binpath/bin/translations/qtwebengine_locales"
"$binpath/bin/$appname" "$@"
