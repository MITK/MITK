FROM mitk_manylinux_x86_64
MAINTAINER MITK.org <community@mitk.org>


ENV http_proxy http://www-int2.dkfz-heidelberg.de:3128/
ENV https_proxy https://www-int2.dkfz-heidelberg.de:3128/

WORKDIR /tmp/

# User is expected to mount directory to "/work"
ENTRYPOINT ["bash", "-c", "groupadd -o -g $_GROUPID $_USER && useradd -m -o -g $_GROUPID $_USER -u $_USERID && su $_USER /work/io/imagefiles/cmd.sh" ]

