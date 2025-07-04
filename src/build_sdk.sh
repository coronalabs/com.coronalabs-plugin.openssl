#!/bin/bash

# This option is used to exit the script as
# soon as a command returns a non-zero value.
set -o errexit

path=`dirname $0`

BUILD_DIR=$1
PLUGIN_NAME=$2
PRODUCT=sdk

# Verify parameters.
if [ $# -ne 2 ]
then
	echo "Usage: $0 <BUILD_DIR> <PLUGIN_NAME>"
	exit 1
fi

# 
# Canonicalize relative paths to absolute paths
# 
pushd $path > /dev/null
dir=`pwd`
path=$dir
popd > /dev/null

#
# OUTPUT_DIR
# 
OUTPUT_DIR=$BUILD_DIR/$PRODUCT

# Clean build
if [ -e "$OUTPUT_DIR" ]
then
	rm -rf "$OUTPUT_DIR"
fi

# Plugins
OUTPUT_PLUGINS_DIR=$OUTPUT_DIR/plugins/2100.9999
OUTPUT_DIR_IOS=$OUTPUT_PLUGINS_DIR/iphone
OUTPUT_DIR_IOS_SIM=$OUTPUT_PLUGINS_DIR/iphone-sim
OUTPUT_DIR_TVOS=$OUTPUT_PLUGINS_DIR/tvos
OUTPUT_DIR_TVOS_SIM=$OUTPUT_PLUGINS_DIR/tvos-sim
OUTPUT_DIR_MAC=$OUTPUT_PLUGINS_DIR/mac-sim
OUTPUT_DIR_ANDROID=$OUTPUT_PLUGINS_DIR/android
OUTPUT_DIR_WIN32=$OUTPUT_PLUGINS_DIR/win32-sim

# Docs
OUTPUT_DIR_DOCS=$OUTPUT_DIR/docs

# Samples
OUTPUT_DIR_SAMPLES=$OUTPUT_DIR/samples

# Create directories
mkdir "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR_IOS"
mkdir -p "$OUTPUT_DIR_IOS_SIM"
mkdir -p "$OUTPUT_DIR_TVOS"
mkdir -p "$OUTPUT_DIR_TVOS_SIM"
mkdir -p "$OUTPUT_DIR_MAC"
mkdir -p "$OUTPUT_DIR_ANDROID"
mkdir -p "$OUTPUT_DIR_WIN32"
mkdir -p "$OUTPUT_DIR_SAMPLES"

#
# Build
#

echo "------------------------------------------------------------------------"
echo "[ios]"
cd "$path/ios"
	./build.sh "$OUTPUT_DIR_IOS" $PLUGIN_NAME

	cp -v metadata.lua "$OUTPUT_DIR_IOS"

	cp -rv "$OUTPUT_DIR_IOS/" "$OUTPUT_DIR_IOS_SIM"

	# Remove i386 from ios build
	find "$OUTPUT_DIR_IOS" -name \*.a | xargs -n 1 -I % lipo -remove i386 % -output %

	# Remove x86_64 from ios build
	find "$OUTPUT_DIR_IOS" -name \*.a | xargs -n 1 -I % lipo -remove x86_64 % -output %

	# Remove armv7 from ios-sim build
	find "$OUTPUT_DIR_IOS_SIM" -name \*.a | xargs -n 1 -I % lipo -remove armv7 % -output %

	# Remove arm64 from ios-sim build
	find "$OUTPUT_DIR_IOS_SIM" -name \*.a | xargs -n 1 -I % lipo -remove arm64 % -output %
cd -

echo "------------------------------------------------------------------------"
echo "[tvos]"
cd "$path/tvos"
	./build.sh "$OUTPUT_DIR_TVOS" Corona_plugin_openssl

	cp -v metadata.lua "$OUTPUT_DIR_TVOS"

	cp -rv "$OUTPUT_DIR_TVOS/" "$OUTPUT_DIR_TVOS_SIM"

	# Remove i386 from TVos build
	find "$OUTPUT_DIR_TVOS" -name \*.a | xargs -n 1 -I % lipo -remove i386 % -output %

	# Remove x86_64 from TVos build
	find "$OUTPUT_DIR_TVOS" -name \*.a | xargs -n 1 -I % lipo -remove x86_64 % -output %

	# Remove armv7 from TVos-sim build
	find "$OUTPUT_DIR_TVOS_SIM" -name \*.a | xargs -n 1 -I % lipo -remove armv7 % -output %

	# Remove arm64 from TVos-sim build
	find "$OUTPUT_DIR_TVOS_SIM" -name \*.a | xargs -n 1 -I % lipo -remove arm64 % -output %
cd -

echo "------------------------------------------------------------------------"
echo "[osx]"
cd "$path/osx"
	./build.sh "$OUTPUT_DIR_MAC" $PLUGIN_NAME
cd -

echo "------------------------------------------------------------------------"
echo "[android]"
cd "$path/android"
	export OUTPUT_PLUGIN_DIR_ANDROID="$OUTPUT_DIR_ANDROID"
	# The parameters of this build.sh are optional.
	./build.sh
	cp -v libs/armeabi-v7a/* "$OUTPUT_DIR_ANDROID"
cd -

if [ ! -f win32/Release/plugin_openssl.dll ]
then
	echo "Build plugin for win32 before packaging"
	exit 1
fi

echo "------------------------------------------------------------------------"
echo "[win32]"
cd "$path"
	cp -v sdk-openssl/win32/bin/*.dll "$OUTPUT_DIR_WIN32"
	cp -v win32/Release/plugin_openssl.dll "$OUTPUT_DIR_WIN32"
cd -

if [ -d "$path/docs" ]
then
	echo "------------------------------------------------------------------------"
	echo "[docs]"
	cp -Rfv "$path/docs" "$OUTPUT_DIR"
fi

echo "------------------------------------------------------------------------"
echo "[samples]"
cp -Rfv "$path/Corona/" "$OUTPUT_DIR_SAMPLES"

echo "------------------------------------------------------------------------"
echo "[metadata.json]"
cp -Rfv "$path/metadata.json" "$OUTPUT_DIR"

echo "------------------------------------------------------------------------"
echo "Generating plugin zip"
ZIP_FILE=$BUILD_DIR/${PRODUCT}-${PLUGIN_NAME}.zip
cd "$OUTPUT_DIR"
	rm -f "$ZIP_FILE"
	zip -rv "$ZIP_FILE" *
cd -

echo "------------------------------------------------------------------------"
echo "Plugin build succeeded."
echo "Zip file located at: '$ZIP_FILE'"
