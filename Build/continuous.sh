#!/bin/bash
# endless loop to run in dart client directory
# TODO: automatic respawn if process dies
uname | grep -i cygwin && WIN32=1 || WIN32=0
if [ $WIN32 == 1 ]; then
  echo "Win32/Cygwin detected"
  if [ -z "$TMP" ]; then 
    export TMP=C:\\temp
  fi;
  MAKECMD=nmake
export PATH=/cygdrive/g/opt/share/Dart/mbi:$PATH  
else
  echo "True Unix detected"  
  MAKECMD=make
export PATH=/opt/share/Dart/mbi:$PATH  
fi;	
while true; do 
  echo "=========================================="
  date
  if [ ! -f suspend ]; then
    echo "Now doing a make clean and make Continuous"
    $MAKECMD clean; 	
    $MAKECMD ContinuousStart ContinuousUpdate ContinuousBuild ContinuousSubmit;
	echo "Sleeping for 5 minutes now...";
        sleep 300;
  else 
    echo "Existing 'suspend' file detected."
    echo "Sleeping for 1 minute now...";
    sleep 60;
  fi;
  echo "=========================================="
done
