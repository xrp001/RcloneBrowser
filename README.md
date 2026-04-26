<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/RcloneBrowserLongLogo1.png" width="80%" />

[![Travis CI Build Status][img1]][1] [![AppVeyor Build Status][img2]][2] [![Downloads][img3]][3] [![Release][img4]][4] <img src="https://img.shields.io/badge/Qt-cmake-green.svg"> [![License][img5]][5]

Rclone browser
==============
Simple cross platform GUI for [rclone](https://rclone.org/) command line tool.

Supports macOS, GNU/Linux, BSD family and Windows.

[🇨🇳 中文版本](README.zh-CN.md)

Table of contents
-------------------
*   [Features](https://github.com/xrp001/RcloneBrowser#features)
*   [Sample screenshots](https://github.com/xrp001/RcloneBrowser#sample-screenshots)
*   [How to get it](https://github.com/xrp001/RcloneBrowser#how-to-get-it)
*   [Why AppImage only for Linux](https://github.com/xrp001/RcloneBrowser#why-appimage-only-for-linux)
*   [Build instructions](https://github.com/xrp001/RcloneBrowser#build-instructions)
    *   [Linux](https://github.com/xrp001/RcloneBrowser#linux)
    *   [FreeBSD](https://github.com/xrp001/RcloneBrowser#freebsd)
    *   [OpenBSD](https://github.com/xrp001/RcloneBrowser#openbsd)
    *   [NetBSD](https://github.com/xrp001/RcloneBrowser#netbsd)
    *   [macOS](https://github.com/xrp001/RcloneBrowser#macos)
    *   [Windows](https://github.com/xrp001/RcloneBrowser#windows)
*   [Portable vs standard mode](https://github.com/xrp001/RcloneBrowser#portable-vs-standard-mode)
*   [History](https://github.com/xrp001/RcloneBrowser#history)

Features
--------
*   Allows to browse and modify any rclone remote, including encrypted ones
*   Uses same configuration file as rclone, no extra configuration required
*   Supports custom location and encryption for rclone.conf configuration file
*   Simultaneously navigate multiple repositories in separate tabs
*   Lists files hierarchically with file name, size and modify date
*   All rclone commands are executed asynchronously, no freezing GUI
*   File hierarchy is lazily cached in memory, for faster traversal of folders
*   Allows to upload, download, create new folders, rename or delete files and folders
*   Allows to calculate size of folder, export list of files and copy rclone command to clipboard
*   Can process multiple upload or download jobs in background
*   Drag & drop support for dragging files from local file explorer for uploading
*   Streaming media files for playback in player like [vlc][6] or similar
*   Mount and unmount folders on macOS, GNU/Linux and Windows (for Windows requires [winfsp](http://www.secfs.net/winfsp/) and for mac [fuse for macOS](https://osxfuse.github.io/))
*   Optionally minimizes to tray, with notifications when upload/download finishes
*   Supports portable mode (create .ini file next to executable with same name), rclone and rclone.conf path now can be relative to executable
*   Supports drive-shared-with-me (Google Drive specific)
*   For remotes supporting public link sharing has an option (right-click menu) to fetch it
*   Supports tasks. Created jobs can be saved and run or edited later.
*   Configurable dark mode for all systems
*   ✨ **New: Complete Chinese language interface support**

Sample screenshots
-------------------
**macOS**
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot24.png" width="100%" />
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot23.png" width="75%" />
</p>

**Linux**
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot21.png" width="65%" />
</p>
&nbsp;
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot22.png" width="65%" />
</p>

&nbsp;
**Windows**
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot25.PNG" width="100%" />
</p>
&nbsp;
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot26.PNG" width="65%" />
</p>

&nbsp;

How to get it
--------------
Get binaries for Windows, macOS and Linux on [releases][3]' page.

Windows installers (64-bit and 32-bit) are compatible with all x86 based Windows OS starting with Windows 7.

Mac version is compiled to run on all versions of macOS starting with 10.9.

Linux binary ([AppImage](https://appimage.org/)) for armhf architecture runs on any Raspberry Pi hardware.
Linux x86_64 and i386 AppImages work on most modern Linux distributions.

> 💡 Linux users are recommended to use [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher) for better desktop integration.

Build instructions
------------------

### Linux (Debian/Ubuntu)
```bash
sudo apt install build-essential cmake qtbase5-dev qttools5-dev
git clone https://github.com/xrp001/RcloneBrowser.git
cd RcloneBrowser
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS
```bash
brew install qt5 cmake
git clone https://github.com/xrp001/RcloneBrowser.git
cd RcloneBrowser
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Windows
Build with MSVC compiler + Qt5 environment.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for full version history.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

[1]: https://travis-ci.org/kapitainsky/RcloneBrowser
[2]: https://ci.appveyor.com/project/kapitainsky/RcloneBrowser
[3]: https://github.com/xrp001/RcloneBrowser/releases
[4]: https://github.com/xrp001/RcloneBrowser/releases/latest
[5]: LICENSE
[6]: https://www.videolan.org

[img1]: https://travis-ci.org/kapitainsky/RcloneBrowser.svg?branch=master
[img2]: https://ci.appveyor.com/api/projects/status/cclx7jc48t4u4x9u?svg=true
[img3]: https://img.shields.io/github/downloads/xrp001/RcloneBrowser/total.svg
[img4]: https://img.shields.io/github/release/xrp001/RcloneBrowser.svg
[img5]: https://img.shields.io/github/license/xrp001/RcloneBrowser.svg