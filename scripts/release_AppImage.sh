#!/bin/bash

set -euo pipefail

# x86_64 build on CentOS 7.7
#   gcc 7 installed
#   sudo yum install -y centos-release-scl
#   sudo yum install -y devtoolset-7-gcc*
#   run below command before build
#   scl enable devtoolset-7 bash

# newer cmake is required than one included in CentOS 7
# download from http://www.cmake.org/download
# sudo mkdir /opt/cmake
# sudo sh cmake-$version.$build-Linux-x86_64.sh --prefix=/opt/cmake

# i686 build on Ubuntu 16.04 LTS

# armv7l build on raspbian stretch

# Qt path and flags set in env e.g.:
# export PATH="/opt/Qt/5.14.0/bin/:$PATH"
# export CPPFLAGS="-I/opt/Qt/5.14.0/bin/include/"
# export LDFLAGS="-L/opt/Qt/5.14.0/bin/lib/"
# export LD_LIBRARY_PATH="/opt/Qt/5.14.0/bin/lib/:$LD_LIBRARY_PATH"

# for x86_64 and i686 platform
# Qt 5.14.0 uses openssl 1.1 and some older distros still use 1.0
# we build openssl 1.1.1d from source using following setup:
# ./config shared --prefix=/opt/openssl-1.1.1/ && make --jobs=`nproc --all` && sudo make install
# and add to build env
# export LD_LIBRARY_PATH="/opt/openssl-1.1.1/lib/:$LD_LIBRARY_PATH"

ARCH="$(arch)"

if [ "${1:-}" = "SIGN" ]; then
  export SIGN="1"
fi

require_command() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "required command not found: $1" >&2
    exit 1
  fi
}

if [ -x /opt/cmake/bin/cmake ]; then
  CMAKE="/opt/cmake/bin/cmake"
else
  require_command cmake
  CMAKE="$(command -v cmake)"
fi

require_command git
require_command gcc
require_command make
require_command linuxdeploy
require_command linuxdeploy-plugin-qt
require_command linuxdeploy-plugin-appimage

# check gcc version on CentOS if using devtoolset style build environment
if [ "$ARCH" = "x86_64" ]; then
  currentver="$(gcc -dumpversion)"
  if [ "${currentver:0:1}" -lt "7" ] && [ -f /etc/centos-release ]; then
    echo "gcc version 7 or newer required"
    echo "on CentOS 7 run"
    echo "scl enable devtoolset-7 bash"
    exit 1
  fi
fi

# building AppImage in temporary directory to keep system clean
# use RAM disk if possible (as in: not building on CI system like Travis, and RAM disk is available)
if [ "${CI:-}" = "" ] && [ -d /dev/shm ]; then
  TEMP_BASE=/dev/shm
else
  TEMP_BASE=/tmp
fi

# we run it from our project scripts folder
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"/..
VERSION=$(cat "$ROOT"/VERSION)-$(git rev-parse --short HEAD)
# linuxdeploy uses $VERSION env variable for AppImage name
export VERSION=$VERSION
BUILD="$ROOT"/build
TARGET=rclone-browser-$VERSION.AppImage

case "$ARCH" in
  armv7l)
    RELEASE_TARGET="$ROOT/release/rclone-browser-$VERSION-raspberrypi-armhf.AppImage"
    ;;
  i686)
    RELEASE_TARGET="$ROOT/release/rclone-browser-$VERSION-linux-i386.AppImage"
    ;;
  x86_64)
    RELEASE_TARGET="$ROOT/release/rclone-browser-$VERSION-linux-x86_64.AppImage"
    ;;
  *)
    echo "unsupported architecture: $ARCH" >&2
    exit 1
    ;;
esac

# clean AppImage temporary folder
if [ -d "$TEMP_BASE/$TARGET" ]; then
  rm -rf "$TEMP_BASE/$TARGET"
fi
mkdir "$TEMP_BASE/$TARGET"

# clean build folder
if [ -d "$BUILD" ]; then
  rm -rf "$BUILD"
fi
mkdir "$BUILD"

# create release folder if does not exist
mkdir -p "$ROOT"/release

# clean current version previous build
if [ -f "$RELEASE_TARGET" ]; then
  rm "$RELEASE_TARGET"
fi

# build and install to temporary AppDir folder
cd "$BUILD"

if [ "$ARCH" = "armv7l" ]; then
  # more threads need swap on 1GB RAM RPi
  "$CMAKE" .. -DCMAKE_INSTALL_PREFIX=/usr
  make -j 2
fi

if [ "$ARCH" = "x86_64" ]; then
  "$CMAKE" .. -DCMAKE_INSTALL_PREFIX=/usr
  make --jobs=$(nproc --all)
fi

if [ "$ARCH" = "i686" ]; then
  "$CMAKE" .. -DCMAKE_INSTALL_PREFIX=/usr
  make --jobs=$(nproc --all)
fi

make install DESTDIR="$TEMP_BASE"/"$TARGET"/AppDir

# prepare AppImage
cd "$TEMP_BASE/$TARGET"

# metainfo file
#mkdir $TEMP_BASE/$TARGET/AppDir/usr/share/metainfo
#cp $ROOT/assets/rclone-browser.appdata.xml $TEMP_BASE/$TARGET/AppDir/usr/share/metainfo/

# copy info files to AppImage
cp "$ROOT"/README.md "$TEMP_BASE"/"$TARGET"/AppDir/Readme.md
cp "$ROOT"/CHANGELOG.md "$TEMP_BASE"/"$TARGET"/AppDir/Changelog.md
cp "$ROOT"/LICENSE "$TEMP_BASE"/"$TARGET"/AppDir/License.txt

# https://github.com/linuxdeploy/linuxdeploy
# https://github.com/linuxdeploy/linuxdeploy-plugin-qt
linuxdeploy --appdir AppDir --desktop-file=AppDir/usr/share/applications/rclone-browser.desktop --plugin qt
#linuxdeploy-plugin-qt --appdir AppDir

if [ "$ARCH" != "armv7l" ]; then
  # we add openssl 1.1.1 libs needed for distros still using openssl 1.0
  if [ -f /opt/openssl-1.1.1/lib/libssl.so.1.1 ] && [ -f /opt/openssl-1.1.1/lib/libcrypto.so.1.1 ]; then
    cp /opt/openssl-1.1.1/lib/libssl.so.1.1 ./AppDir/usr/bin/
    cp /opt/openssl-1.1.1/lib/libcrypto.so.1.1 ./AppDir/usr/bin/
  else
    echo "warning: OpenSSL 1.1 runtime libraries not found under /opt/openssl-1.1.1/lib/; skipping bundled OpenSSL copy" >&2
  fi
fi

# https://github.com/linuxdeploy/linuxdeploy-plugin-appimage
linuxdeploy-plugin-appimage --appdir=AppDir

GENERATED_APPIMAGE="$(find . -maxdepth 1 -type f -name '*.AppImage' | head -n 1)"
if [ -z "$GENERATED_APPIMAGE" ]; then
  echo "AppImage was not generated" >&2
  exit 1
fi

cp "$GENERATED_APPIMAGE" "$RELEASE_TARGET"

# clean AppImage temporary folder
cd ..
rm -rf "$TARGET"
