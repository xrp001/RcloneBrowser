# RcloneBrowser

Qt5 + CMake cross-platform GUI for [rclone](https://rclone.org/).

## Build

```
mkdir build && cd build && cmake .. && make -j$(nproc)
```

`CMakeLists.txt` in root + `src/`. Requires Qt5Widgets, Qt5LinguistTools (+ Qt5WinExtras on Windows, Qt5MacExtras on macOS). Version from `VERSION` file + `git rev-parse --short HEAD`.

## Project layout

```
src/           — .cpp, .h, .ui (main_window, remote_widget, transfer_dialog, etc.)
scripts/       — Per-platform release scripts
translations/  — zh_CN translation (.ts → .qm via lrelease)
assets/        — Icons, appdmg.json
```

## Release scripts

All three check dependencies interactively (prompt to install, ask to continue if missing):

| Script | Platform | Output |
|--------|----------|--------|
| `scripts/release_AppImage.sh` | Linux | AppImage (x86_64, i686, armv7l) |
| `scripts/release_macOS.sh` | macOS | .dmg + .zip |
| `scripts/release_windows.cmd` | Windows | .zip + Inno Setup installer |

### Key deps by platform

- **Linux**: cmake, gcc/g++/make, qtbase5-dev, qttools5-dev, git, linuxdeploy + plugins
- **macOS**: Homebrew, cmake, Qt5, Xcode CLT, p7zip, coreutils, appdmg
- **Windows**: Windows 10 1703+, VS2019, Qt 5.13.2 (msvc2017), cmake, git, 7-Zip, Inno Setup 6, OpenSSL (x86)
