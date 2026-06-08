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

# --- Dependency Checking ---

MISSING_DEPS=()
APT_UPDATED=0
LRELEASE=lrelease

install_packages() {
  local apt_pkgs="$1"
  local rpm_pkgs="$2"
  local zypper_pkgs="$3"
  local pacman_pkgs="$4"

  if command -v apt-get >/dev/null 2>&1; then
    if [ "$APT_UPDATED" -eq 0 ]; then
      sudo apt-get update || true
      APT_UPDATED=1
    fi
    sudo apt-get install -y $apt_pkgs || true
  elif command -v dnf >/dev/null 2>&1; then
    sudo dnf install -y $rpm_pkgs || true
  elif command -v yum >/dev/null 2>&1; then
    sudo yum install -y $rpm_pkgs || true
  elif command -v zypper >/dev/null 2>&1; then
    sudo zypper --non-interactive install $zypper_pkgs || true
  elif command -v pacman >/dev/null 2>&1; then
    sudo pacman -Sy --noconfirm --needed $pacman_pkgs || true
  else
    return 1
  fi
}

check_system_dep() {
  local cmd="$1"
  local apt_pkgs="$2"
  local rpm_pkgs="$3"
  local zypper_pkgs="$4"
  local pacman_pkgs="$5"

  if command -v "$cmd" >/dev/null 2>&1; then
    echo "  [OK] $cmd"
    return 0
  fi

  echo "  [MISSING] $cmd"
  if [ "${CI:-}" != "" ] || [ ! -t 0 ]; then
    MISSING_DEPS+=("$cmd (install packages: $apt_pkgs / $rpm_pkgs / $zypper_pkgs / $pacman_pkgs)")
    return 0
  fi

  read -r -p "  Install missing package(s) for '$cmd'? [Y/n] " answer || answer="n"
  if [[ "$answer" =~ ^[Nn] ]]; then
    MISSING_DEPS+=("$cmd (install packages: $apt_pkgs / $rpm_pkgs / $zypper_pkgs / $pacman_pkgs)")
    return 0
  fi

  if ! install_packages "$apt_pkgs" "$rpm_pkgs" "$zypper_pkgs" "$pacman_pkgs"; then
    echo "  Cannot detect supported package manager. Install manually."
    MISSING_DEPS+=("$cmd (manual install required)")
    return 0
  fi

  if command -v "$cmd" >/dev/null 2>&1; then
    echo "  [OK] $cmd installed successfully"
  else
    echo "  [WARN] Installation may have failed for $cmd"
    MISSING_DEPS+=("$cmd (installation failed)")
  fi
}

check_manual_dep() {
  local cmd="$1"
  local desc="$2"

  if command -v "$cmd" >/dev/null 2>&1; then
    echo "  [OK] $cmd"
    return 0
  fi

  echo "  [MISSING] $cmd"
  echo "         $desc"
  MISSING_DEPS+=("$cmd ($desc)")
}

check_command_group() {
  local label="$1"
  local result_var="$2"
  local apt_pkgs="$3"
  local rpm_pkgs="$4"
  local zypper_pkgs="$5"
  local pacman_pkgs="$6"
  shift 6

  for cmd in "$@"; do
    if command -v "$cmd" >/dev/null 2>&1; then
      printf -v "$result_var" '%s' "$cmd"
      echo "  [OK] $cmd"
      return 0
    fi
  done

  echo "  [MISSING] $label"
  if [ "${CI:-}" != "" ] || [ ! -t 0 ]; then
    MISSING_DEPS+=("$label (install packages: $apt_pkgs / $rpm_pkgs / $zypper_pkgs / $pacman_pkgs)")
    return 0
  fi

  read -r -p "  Install missing package(s) for '$label'? [Y/n] " answer || answer="n"
  if [[ "$answer" =~ ^[Nn] ]]; then
    MISSING_DEPS+=("$label (install packages: $apt_pkgs / $rpm_pkgs / $zypper_pkgs / $pacman_pkgs)")
    return 0
  fi

  if ! install_packages "$apt_pkgs" "$rpm_pkgs" "$zypper_pkgs" "$pacman_pkgs"; then
    echo "  Cannot detect supported package manager. Install manually."
    MISSING_DEPS+=("$label (manual install required)")
    return 0
  fi

  for cmd in "$@"; do
    if command -v "$cmd" >/dev/null 2>&1; then
      printf -v "$result_var" '%s' "$cmd"
      echo "  [OK] $cmd installed successfully"
      return 0
    fi
  done

  echo "  [WARN] Installation may have failed for $label"
  MISSING_DEPS+=("$label (installation failed)")
}

echo "==> Checking build dependencies..."

check_system_dep cmake cmake cmake cmake cmake
check_system_dep git git git git git
check_system_dep gcc gcc gcc gcc gcc
check_system_dep g++ g++ gcc-c++ gcc-c++ gcc
check_system_dep make make make make make

check_command_group "lrelease / lrelease-qt5" LRELEASE \
  qttools5-dev-tools qt5-qttools libqt5-qttools-devel qt5-tools \
  lrelease lrelease-qt5

check_command_group "qmake / qmake-qt5" QMAKE \
  qt5-qmake qt5-qtbase-devel libqt5-qtbase-devel qt5-base \
  qmake qmake-qt5

check_manual_dep linuxdeploy \
  "Download from https://github.com/linuxdeploy/linuxdeploy/releases"
check_manual_dep linuxdeploy-plugin-qt \
  "Download from https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases"
check_manual_dep linuxdeploy-plugin-appimage \
  "Download from https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases"

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
  echo ""
  echo "==> WARNING: Some dependencies are missing:"
  for dep in "${MISSING_DEPS[@]}"; do
    echo "    - $dep"
  done
  echo ""
  if [ "${CI:-}" != "" ] || [ ! -t 0 ]; then
    echo "Aborting in non-interactive mode. Install the missing dependencies before running this script."
    exit 1
  fi
  read -r -p "Continue anyway? [y/N] " answer || answer="n"
  if [[ ! "$answer" =~ ^[Yy] ]]; then
    echo "Aborting."
    exit 1
  fi
fi

echo "==> Dependency check complete."
echo ""

# cmake path
if [ -x /opt/cmake/bin/cmake ]; then
  CMAKE="/opt/cmake/bin/cmake"
else
  CMAKE="$(command -v cmake)"
fi

# check gcc version on CentOS if using devtoolset style build environment
if [ "$ARCH" = "x86_64" ]; then
  currentver="$(gcc -dumpversion)"
  if [ "${currentver:0:1}" -lt "7" ] && [ -f /etc/centos-release ]; then
    echo "WARNING: gcc version 7 or newer required (current: $currentver)"
    echo "On CentOS 7 run: scl enable devtoolset-7 bash"
    read -r -p "Continue anyway? [y/N] " answer || answer="n"
    if [[ ! "$answer" =~ ^[Yy] ]]; then
      echo "Aborting."
      exit 1
    fi
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

# compile translations explicitly before build/package
"$LRELEASE" "$ROOT"/translations/rclonebrowser_zh_CN.ts \
  -qm "$ROOT"/translations/rclonebrowser_zh_CN.qm

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
