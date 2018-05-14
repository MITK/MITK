#!/bin/sh

set -x

export SRC_DIR="/tmp/mitk"
export BLD_DIR="/tmp/bin"
export OUT_DIR="/var/io"

MITK_GIT_TAG=T24046-SwigBasedPythonWrapping

PYTHON_VERSIONS=${PYTHON_VERSIONS:-$(ls /opt/python | sed -e 's/cp2[0-6][^ ]\+ \?//g')}

NPROC=$(grep -c processor /proc/cpuinfo)
export MAKEFLAGS="-j ${NPROC}"


function build_mitk {

    echo "MITK_GIT_TAG: ${MITK_GIT_TAG}"

    git clone https://phabricator.mitk.org/source/mitk.git ${SRC_DIR} &&
    (cd ${SRC_DIR}  && git checkout ${MITK_GIT_TAG}  ) &&
    rm -rf ${BLD_DIR} &&
    mkdir -p ${BLD_DIR} && cd ${BLD_DIR} &&
    cmake \
        -DBUILD_TESTING:BOOL=OFF \
        -DMITK_USE_SWIG:BOOL=ON \
        -DMITK_USE_Qt5:BOOL=OFF \
        -DMITK_USE_CTK:BOOL=OFF \
        -DMITK_USE_BLUEBERRY:BOOL=OFF \
        -DCMAKE_C_COMPILER:FILEPATH=/usr/local/bin/gcc \
        -DCMAKE_CXX_COMPILER:FILEPATH=/usr/local/bin/g++ \
        -DMITK_WHITELIST:STRING=Wrapping  \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        ${SRC_DIR} &&
    make -j8
}

function build_mitk_python {

    PYTHON_EXECUTABLE=/opt/python/${PYTHON}/bin/python
    PYTHON_INCLUDE_DIR="$( find -L /opt/python/${PYTHON}/include/ -name Python.h -exec dirname {} \; )"

    echo ""
    echo "PYTHON_EXECUTABLE:${PYTHON_EXECUTABLE}"
    echo "PYTHON_INCLUDE_DIR:${PYTHON_INCLUDE_DIR}"
    echo "PYTHON_LIBRARY:${PYTHON_LIBRARY}"

    cd ${BLD_DIR}/MITK-build &&
    cmake \
        -DWRAP_PYTHON:BOOL=ON \
        -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR} \
        -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE} \
        .
    make dist -B
}

build_mitk || exit 1


for PYTHON in ${PYTHON_VERSIONS}; do
    build_mitk_python &&
    auditwheel repair $(find ${BLD_DIR}/MITK-build/Wrapping/Python/dist -name pyMITK*.whl) -w ${OUT_DIR}/wheelhouse/
    rm $(find ${BLD_DIR}/MITK-build/Wrapping/Python/dist/ -name pyMITK*.whl)
done

