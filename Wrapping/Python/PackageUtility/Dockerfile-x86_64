FROM quay.io/pypa/manylinux1_x86_64
MAINTAINER Insight Software Consortium <community@itk.org>

ENV http_proxy http://www-int2.dkfz-heidelberg.de:3128/
ENV https_proxy https://www-int2.dkfz-heidelberg.de:3128/

ADD https://cmake.org/files/v3.7/cmake-3.7.2.tar.gz \
    https://www.openssl.org/source/openssl-1.0.2h.tar.gz \
    /tmp/

ADD http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz \
    https://gmplib.org/download/gmp/gmp-6.0.0a.tar.bz2 \
    http://www.mpfr.org/mpfr-3.1.3/mpfr-3.1.3.tar.bz2 \
    https://ftp.gnu.org/gnu/mpc/mpc-1.0.2.tar.gz \
    http://bugseng.com/products/ppl/download/ftp/releases/1.1/ppl-1.1.tar.bz2 \
    http://www.bastoul.net/cloog/pages/download/cloog-0.18.1.tar.gz \
    http://ftp.gnu.org/gnu/gcc/gcc-4.9.4/gcc-4.9.4.tar.bz2 \
    http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.bz2 \
    http://download.osgeo.org/libtiff/tiff-4.0.9.tar.gz \
    /tmp/archives/

RUN yum -y install nano libXt-devel tcp_wrappers

WORKDIR /tmp/
COPY ./imagefiles/install.sh ./
COPY ./imagefiles/install-gcc.sh ./
COPY ./imagefiles/install-libtiff.sh ./

RUN bash -v install-gcc.sh /usr/local && \
    bash -v install-libtiff.sh && \
    bash -v install.sh && \
    rm -rf /tmp/*

# User is expected to mount directory to "/work"
ENTRYPOINT ["bash", "-c", "groupadd -o -g $_GROUPID $_USER && useradd -m -o -g $_GROUPID $_USER -u $_USERID && su $_USER /work/io/imagefiles/cmd.sh" ]

