# Rclone Browser

<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/RcloneBrowserLongLogo1.png" width="80%" />
</p>

[![Travis CI Build Status][img1]][1] [![AppVeyor Build Status][img2]][2] [![Downloads][img3]][3] [![Release][img4]][4] <img src="https://img.shields.io/badge/Qt-cmake-green.svg"> [![License][img5]][5]

> rclone 命令行工具的跨平台图形化界面客户端

支持 macOS、GNU/Linux、BSD 系列以及 Windows 操作系统。

[🔙 返回英文README](README.md)

---

## ✨ 功能特性

* ✅ 支持浏览和管理所有类型的 rclone 远程存储，包括加密远程
* ✅ 直接使用 rclone 原生配置文件，无需额外配置
* ✅ 支持自定义 rclone.conf 配置文件路径和加密配置
* ✅ 多标签页同时管理多个远程存储库
* ✅ 文件列表支持分层树形结构，显示文件名、大小和修改时间
* ✅ 所有 rclone 命令异步执行，界面无卡顿
* ✅ 智能内存缓存，文件夹浏览更快速
* ✅ 完整的文件操作：上传、下载、新建文件夹、重命名、删除
* ✅ 文件夹大小计算、文件列表导出、rclone 命令复制
* ✅ 后台队列任务处理，支持同时多个上传/下载作业
* ✅ 拖拽上传支持，可直接从本地文件管理器拖入
* ✅ 媒体文件流式播放，可直接调用 VLC 等播放器
* ✅ 挂载/卸载远程文件夹（Windows 需要 winfsp，macOS 需要 osxfuse）
* ✅ 系统托盘最小化，传输完成桌面通知
* ✅ 便携模式支持，程序和配置可放在移动设备
* ✅ Google Drive 共享文件支持
* ✅ 远程存储公开链接生成
* ✅ 任务模板，可保存和重复使用传输任务
* ✅ 全系统深色主题支持
* ✅ ✨ **新增：完整中文界面支持**

## 📸 界面截图

### macOS
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot24.png" width="100%" />
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot23.png" width="75%" />
</p>

### Linux
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot21.png" width="65%" />
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot22.png" width="65%" />
</p>

### Windows
<p align="center">
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot25.PNG" width="100%" />
<img src="https://github.com/kapitainsky/RcloneBrowser/wiki/images/screenshot26.PNG" width="65%" />
</p>

## 📦 下载安装

可在 [Releases 页面][3] 下载 Windows、macOS 和 Linux 平台的预编译二进制文件。

* **Windows**: 64位/32位安装包，兼容 Windows 7 及以上版本
* **macOS**: 兼容 macOS 10.9 及以上版本
* **Linux**: 以 AppImage 格式发布，支持 x86_64, i386, armhf (树莓派) 架构

> 💡 Linux 用户推荐使用 [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher) 获得更好的桌面集成体验

## 🔨 编译说明

### Linux (Ubuntu/Debian)
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
make dmg
```

### Windows
使用 MSVC 编译器 + Qt5 环境进行编译

## 📜 版本历史

请查看 [CHANGELOG.md](CHANGELOG.md) 获取完整更新日志

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

---

[1]: https://travis-ci.org/kapitainsky/RcloneBrowser
[2]: https://ci.appveyor.com/project/kapitainsky/rclonebrowser
[3]: https://github.com/xrp001/RcloneBrowser/releases
[4]: https://github.com/xrp001/RcloneBrowser/releases/latest
[5]: LICENSE

[img1]: https://travis-ci.org/kapitainsky/RcloneBrowser.svg?branch=master
[img2]: https://ci.appveyor.com/api/projects/status/github/kapitainsky/RcloneBrowser?branch=master&svg=true
[img3]: https://img.shields.io/github/downloads/xrp001/RcloneBrowser/total.svg
[img4]: https://img.shields.io/github/release/xrp001/RcloneBrowser.svg
[img5]: https://img.shields.io/github/license/xrp001/RcloneBrowser.svg