#!/bin/bash

set -e

QTDIR=/usr/local/opt/qt

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"/..

# --- Dependency Checking ---

MISSING_DEPS=()

check_brew_dep() {
  local cmd="$1"
  local pkg="$2"

  if command -v "$cmd" >/dev/null 2>&1; then
    echo "  [OK] $cmd"
    return 0
  fi

  echo "  [MISSING] $cmd"
  read -r -p "  Install '$pkg' via Homebrew? [Y/n] " answer || answer="n"
  if [[ "$answer" =~ ^[Nn] ]]; then
    MISSING_DEPS+=("$cmd (brew install $pkg)")
    return 0
  fi

  brew install "$pkg" || true

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

echo "==> Checking build dependencies..."

# Homebrew itself
if ! command -v brew >/dev/null 2>&1; then
  echo "  [MISSING] Homebrew"
  echo "         Install from https://brew.sh/"
  MISSING_DEPS+=("Homebrew (https://brew.sh/)")
else
  echo "  [OK] Homebrew ($(brew --version | head -1))"
fi

check_brew_dep cmake cmake
check_brew_dep git git
check_manual_dep make \
  "Install Xcode Command Line Tools: xcode-select --install"
check_brew_dep 7za p7zip
check_brew_dep nproc coreutils

# Qt5: check qmake + macdeployqt
if [ -x "$QTDIR/bin/qmake" ]; then
  echo "  [OK] Qt5 qmake ($QTDIR/bin/qmake)"
else
  echo "  [MISSING] Qt5 qmake"
  MISSING_DEPS+=("Qt5 qmake (brew install qt5)")
fi

if [ -x "$QTDIR/bin/macdeployqt" ]; then
  echo "  [OK] Qt5 macdeployqt ($QTDIR/bin/macdeployqt)"
else
  echo "  [MISSING] Qt5 macdeployqt"
  MISSING_DEPS+=("Qt5 macdeployqt (brew install qt5)")
fi

check_manual_dep appdmg \
  "Install via: brew install node && npm install -g appdmg"

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
  echo ""
  echo "==> WARNING: Some dependencies are missing:"
  for dep in "${MISSING_DEPS[@]}"; do
    echo "    - $dep"
  done
  echo ""
  read -r -p "Continue anyway? [y/N] " answer || answer="n"
  if [[ ! "$answer" =~ ^[Yy] ]]; then
    echo "Aborting."
    exit 1
  fi
fi

echo "==> Dependency check complete."
echo ""

VERSION=$(cat "$ROOT"/VERSION)-$(git rev-parse --short HEAD)
BUILD="$ROOT"/build
TARGET=rclone-browser-$VERSION-macos
DMG=rclone-browser-$VERSION-macos
APP="$TARGET"/"Rclone Browser.app"

# clean from previous builds (if for the same version in releases)
if [ -d "$BUILD" ]; then
  rm -rf "$BUILD"
fi
if [ -d "$ROOT"/release/"$TARGET" ]; then
  rm -rf "$ROOT"/release/"$TARGET"*
fi
if [ -f "$ROOT"/release/"$DMG".dmg ]; then
  rm "$ROOT"/release/"$DMG".dmg
fi
if [ -d "$ROOT"/release/"Rclone Browser.app" ]; then
  rm -rf "$ROOT"/release/"Rclone Browser.app"
fi



mkdir -p "$BUILD"
cd "$BUILD"
# brew install cmake qt5
cmake .. -DCMAKE_PREFIX_PATH="$QTDIR" -DCMAKE_BUILD_TYPE=Release
# brew install coreutils
make --jobs=$(nproc --all)
cd build
"$QTDIR"/bin/macdeployqt rclone-browser.app -executable="rclone-browser.app/Contents/MacOS/rclone-browser" -qmldir=../src/
cd ../..


mkdir -p release
cd release
mkdir "$TARGET"
cp -R "$BUILD"/build/rclone-browser.app "$APP"
cp "$ROOT"/README.md "$APP"/Readme.md
cp "$ROOT"/CHANGELOG.md "$APP"/Changelog.md
cp "$ROOT"/LICENSE "$APP"/License.txt
mv "$APP"/Contents/MacOS/rclone-browser "$APP"/Contents/MacOS/"Rclone Browser"

sed -i .bak 's/rclone-browser/Rclone Browser/g' "$APP"/Contents/Info.plist
rm "$APP"/Contents/*.bak

cat >"$APP"/Contents/MacOS/qt.conf <<EOF
[Paths]
Plugins = Plugins
EOF

echo
echo "Preparing zip file"
# brew install p7zip
7za a -mx=9 -r -tzip "$TARGET".zip "$TARGET"

## gpg --detach-sign "$TARGET".zip.sig "$TARGET".zip

echo
echo "Preparing dmg file"
# brew install node && npm install -g appdmg
# https://github.com/LinusU/node-appdmg
cp -R "$TARGET"/"Rclone Browser.app" .
cd ../scripts
appdmg ../assets/appdmg.json ../release/"$DMG".dmg
cd ../release
rm -rf "Rclone Browser.app"
