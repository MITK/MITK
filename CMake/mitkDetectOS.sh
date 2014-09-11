#!/bin/sh
# Detects which OS and if it is Linux then it will detect which Linux Distribution.

OS=`uname -s`
MACH=`uname -m`

if [ "${OS}" = "SunOS" ] ; then
  OS=Solaris
  ARCH=`uname -p`
  OSSTR="${OS} ${REV} (${ARCH} `uname -v`)"
elif [ "${OS}" = "AIX" ] ; then
  OSSTR="${OS} `oslevel` (`oslevel -r`)"
elif [ "${OS}" = "Linux" ] ; then
  if [ -f /etc/os-release ] ; then
    DIST=`cat /etc/os-release | tr "\n" ' ' | sed s/.*PRETTY_NAME=\"// | sed s/\".*//`
  elif [ -f /etc/redhat-release ] ; then
    DIST=`cat /etc/redhat-release | sed s/\ release.*//`
    VERSION=`cat /etc/redhat-release | sed s/.*release\ //`
  elif [ -f /etc/SUSE-release ] ; then
    DIST=`cat /etc/SUSE-release | tr "\n" ' '| sed s/VERSION.*//`
    VERSION=`cat /etc/SUSE-release | tr "\n" ' ' | sed s/.*=\ //`
  elif [ -f /etc/mandrake-release ] ; then
    DIST='Mandrake'
    VERSION=`cat /etc/mandrake-release | sed s/.*release\ //`
  elif [ -f /etc/debian_version ] ; then
    DIST="Debian"
    VERSION=`cat /etc/debian_version`
  fi

  OSSTR="${DIST} ${VERSION} ${MACH}"
fi

echo ${OSSTR}
