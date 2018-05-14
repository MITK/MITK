FROM python
MAINTAINER Insight Software Consortium <community@itk.org>

ENV http_proxy http://www-int2.dkfz-heidelberg.de:3128/
ENV https_proxy https://www-int2.dkfz-heidelberg.de:3128/


RUN apt-get update && \ 
    yes | apt-get install libgl1-mesa-glx

# User is expected to mount directory to "/work"
ENTRYPOINT ["bash", "-c", "groupadd -o -g $_GROUPID $_USER && useradd -m -o -g $_GROUPID $_USER -u $_USERID && su $_USER /work/io/imagefiles/cmd.sh" ]

