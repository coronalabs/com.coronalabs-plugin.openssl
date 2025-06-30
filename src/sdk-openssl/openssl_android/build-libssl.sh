#!/bin/bash

#  Automatic build script for libssl and libcrypto
#  for Android
#
#  Based on a script for iOS by:
#  Created by Felix Schulze on 16.12.10.
#  Copyright 2010-2015 Felix Schulze. All rights reserved.
#  https://github.com/x2on/OpenSSL-for-iPhone
#
#  Also based on a script for Android:
#  https://github.com/aluvalasuman/openssl1.0.1g-android
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################
#  Change values here                                                     #
#                                                                         #
VERSION=${1:-'1.0.2h'}
ARCH=${2:-x64}
ANDROID_NDK=${3:-$ANDROID_NDK}
PLATFORM=Android
CURL_OPTIONS=""

#                                                                         #
###########################################################################
#                                                                         #
# Don't change anything under this line!                                  #
#                                                                         #
###########################################################################

OUTPUT_DIR="$(pwd)/../android"

set -e
OPENSSL_ARCHIVE_BASE_NAME=OpenSSL_${VERSION//./_}
OPENSSL_ARCHIVE_FILE_NAME=${OPENSSL_ARCHIVE_BASE_NAME}.tar.gz
if [ ! -e "${OPENSSL_ARCHIVE_FILE_NAME}" ]
then
  echo "Downloading ${OPENSSL_ARCHIVE_FILE_NAME}"
  curl ${CURL_OPTIONS} -L -O https://github.com/openssl/openssl/archive/"${OPENSSL_ARCHIVE_FILE_NAME}"
else
  echo "Using ${OPENSSL_ARCHIVE_FILE_NAME}"
fi

export NDK="${ANDROID_NDK}"

case "$ARCH" in
	arm)
		ARCH_TOOLCHAIN="arm-linux-androideabi-4.9"
		ARCH_TOOLCHAIN_NAME="arm-linux-androideabi"
		ARCH_CC_FLAGS="-mthumb"
		ARCH_LD_FLAGS=
		;;
	armv7)
		ARCH_TOOLCHAIN="arm-linux-androideabi-4.9"
		ARCH_TOOLCHAIN_NAME="arm-linux-androideabi"
		ARCH_CC_FLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
		ARCH_LD_FLAGS="-march=armv7-a -Wl,--fix-cortex-a8"
		;;
	armv64)
		ARCH_TOOLCHAIN="arm-linux-androideabi-4.9"
		ARCH_TOOLCHAIN_NAME="aarch64-linux-android"
		ARCH_CC_FLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
		ARCH_LD_FLAGS="-march=armv7-a -Wl,--fix-cortex-a8"
		;;
	x86)
		ARCH_TOOLCHAIN="x86-4.9"
		ARCH_TOOLCHAIN_NAME="i686-linux-android"
		ARCH_CC_FLAGS="-march=i686 -msse3 -mstackrealign -mfpmath=sse"
		ARCH_LD_FLAGS=
		;;
	x64)
		ARCH_TOOLCHAIN="x86-4.9"
		ARCH_TOOLCHAIN_NAME="x86_64-linux-android"
		ARCH_CC_FLAGS="-march=i686 -msse3 -mstackrealign -mfpmath=sse"
		ARCH_LD_FLAGS=
		;;
	*)
		echo "ERROR: unrecogized architecture: $ARCH"
		exit 1
esac

TOOLCHAIN_INSTALL_DIR="${OUTPUT_DIR}/android-toolchain-${ARCH}"
export TOOLCHAIN_INSTALL_DIR

"${ANDROID_NDK}/build/tools/make-standalone-toolchain.sh" --platform=android-16 --toolchain="${ARCH_TOOLCHAIN}" --install-dir="$TOOLCHAIN_INSTALL_DIR"  --verbose --force

export TOOLCHAIN_PATH="${TOOLCHAIN_INSTALL_DIR}/bin"
export NDK_TOOLCHAIN_BASENAME="${TOOLCHAIN_PATH}/${ARCH_TOOLCHAIN_NAME}"
export CC="${NDK_TOOLCHAIN_BASENAME}-gcc"
export CXX="${NDK_TOOLCHAIN_BASENAME}-g++"
export CPP="${NDK_TOOLCHAIN_BASENAME}-cpp"
export LINK="${CXX}"
export LD="${NDK_TOOLCHAIN_BASENAME}-ld"
export AR="${NDK_TOOLCHAIN_BASENAME}-ar"
export AS="${NDK_TOOLCHAIN_BASENAME}-as"
export RANLIB="${NDK_TOOLCHAIN_BASENAME}-ranlib"
export STRIP="${NDK_TOOLCHAIN_BASENAME}-strip"
export NM="${NDK_TOOLCHAIN_BASENAME}-nm"
export OBJDUMP="${NDK_TOOLCHAIN_BASENAME}-objdump"
export ARCH_LINK=
export CPPFLAGS=" ${ARCH_CC_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export CXXFLAGS=" ${ARCH_CC_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
export CFLAGS=" ${ARCH_CC_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export LDFLAGS=" ${ARCH_LD_FLAGS} "

WORK_DIR="$(pwd)/src/${PLATFORM}-${ARCH}"
mkdir -p "${WORK_DIR}"
tar xzf "$(pwd)/${OPENSSL_ARCHIVE_FILE_NAME}" -C "${WORK_DIR}"
cd "${WORK_DIR}/openssl-${OPENSSL_ARCHIVE_BASE_NAME}"

# Clean output directory
rm -rf "${OUTPUT_DIR}/libs"

case "$ARCH" in
	arm)
		./Configure android shared --prefix="${OUTPUT_DIR}/libs" --openssldir=openssl
		;;
	armv7)
		./Configure android-armv7 no-ui no-stdio no-shared --prefix="${OUTPUT_DIR}/libs/armeabi-v7a" --openssldir=openssl
		;;
	x86)
		./Configure android-x86 shared --prefix="${OUTPUT_DIR}/libs/x86"  --openssldir=openssl
		;;
    x64)
        ./Configure android-x86_64 shared --prefix="${OUTPUT_DIR}/libs/x64"  --openssldir=openssl
        ;;
esac

make clean && make && make install

if [ $? != 0 ]
then
	echo "If 'make' fails, try running it again to work around OpenSSL issue: make && make install"
	exit 1
fi

echo
echo "OpenSSL $VERSION for Android $ARCH built in $OUTPUT_DIR"
