#!/bin/bash

set -x
set -e
set -u
set -o pipefail

if [ -f /etc/os-release ]; then
    cat /etc/os-release
    . /etc/os-release
    if [ "${OSTYPE}" = "msys" ]; then
        # MSYS2/MinGW doesn't have VERSION_ID in /etc/os-release
        VERSION_ID=$( uname -r )
    fi
else
    ID=$( uname -s )
    VERSION_ID=$( uname -r )
fi
COLLECTION=$( cat COLLECTION )
META_DATA_DIR=Metadata/${COLLECTION}/${ID}/${VERSION_ID}
mkdir -p ${META_DATA_DIR}
DIR=Packages/${COLLECTION}/${ID}/${VERSION_ID}
ARCH=$( uname -m )
mkdir -p ${DIR}/os/${ARCH}
mkdir -p ${DIR}/debug/${ARCH}
mkdir -p ${DIR}/source
build_system=${build_system:-autotools}
if [ "${build_system}" = "cmake" ]; then
    mkdir build
    cd build
    cmake ${CMAKE_OPTIONS:-} ..
    cmake --build .
    cpack
    cd ..
    if [ "${OSTYPE}" = "linux-gnu" ]; then
        if [ "${ID_LIKE:-}" = "debian" ]; then
            mv build/*.deb ${DIR}/os/${ARCH}/
        else
            mv build/*.rpm ${DIR}/os/${ARCH}/
        fi
    elif [[ "${OSTYPE}" =~ "darwin" ]]; then
        mv build/*.zip ${DIR}/os/${ARCH}/
    elif [ "${OSTYPE}" = "msys" ]; then
        mv build/*.zip ${DIR}/os/${ARCH}/
        mv build/*.exe ${DIR}/os/${ARCH}/
    else
        echo "Error: OSTYPE=${OSTYPE} is unknown" >&2
        exit 1
    fi
else
    GV_VERSION=$( cat VERSION )
    if [ "$OSTYPE" = "linux-gnu" ]; then
        if [ "${ID_LIKE:-}" = "debian" ]; then
            tar xfz graphviz-${GV_VERSION}.tar.gz
            (cd graphviz-${GV_VERSION}; fakeroot make -f debian/rules binary) | tee >(ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
            mv *.deb ${DIR}/os/${ARCH}/
            mv *.ddeb ${DIR}/debug/${ARCH}/
        else
            rm -rf ${HOME}/rpmbuild
            rpmbuild -ta graphviz-${GV_VERSION}.tar.gz | tee >(ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
            mv ${HOME}/rpmbuild/SRPMS/*.src.rpm ${DIR}/source/
            mv ${HOME}/rpmbuild/RPMS/*/*debuginfo*rpm ${DIR}/debug/${ARCH}/
            mv ${HOME}/rpmbuild/RPMS/*/*.rpm ${DIR}/os/${ARCH}/
        fi
    elif [[ "${OSTYPE}" =~ "darwin" ]]; then
        ./autogen.sh
        ./configure --prefix=$( pwd )/build --with-quartz=yes
        make
        make install
        tar cfz graphviz-${GV_VERSION}-${ARCH}.tar.gz --options gzip:compression-level=9 build
        mv graphviz-${GV_VERSION}-${ARCH}.tar.gz ${DIR}/os/${ARCH}/
    else
        echo "Error: OSTYPE=${OSTYPE} is unknown" >&2
        exit 1
    fi
fi
