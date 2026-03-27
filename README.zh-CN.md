# Machine

[README](./README.md) | [Machine 中文纵览](./use_order.zh-CN.md)

Machine 是一个用 C17 实现的编译型系统编程语言项目。当前编译器前端会读取 `.mne` 源码，执行词法分析、语法分析、语义检查，然后根据所选后端生成 C 或 x86_64 汇编，最后再编译或链接成目标程序。

## 快速入口

- [构建与安装](#构建与安装)
- [编译器用法](#编译器用法)
- [英文指令说明](./use_order.md)
- [中文纵览](./use_order.zh-CN.md)

## 构建与安装

### 依赖

在 Fedora/RHEL/CentOS 上：

```bash
sudo dnf install -y gcc make SDL2-devel SDL2_image-devel nasm
```

在 Debian/Ubuntu/Kali 上：
```bash
sudo apt install -y gcc make libsdl2-dev libsdl2-image-dev nasm
```

在 ArchLinux/Manjaro 上：
```bash
sudo pacman -S gcc make sdl2-compat sdl2_image nasm
```

对于其他发行版手动编译安装依赖：

对于其他发行版手动编译安装依赖：

[![gcc](https://img.shields.io/badge/source-gcc-blue)](https://gcc.gnu.org/git/gcc.git)
[![make](https://img.shields.io/badge/source-make-blue)](https://git.savannah.gnu.org/git/make.git/)
[![SDL2](https://img.shields.io/badge/source-SDL2-blue)](https://github.com/libsdl-org/SDL)
[![SDL2-Image](https://img.shields.io/badge/source-SDL2--Image-blue)](https://github.com/libsdl-org/SDL_image)
[![nasm](https://img.shields.io/badge/source-nasm-blue)](https://github.com/netwide-assembler/nasm)


窗口、图像等 hosted runtime 功能依赖 SDL2 和 SDL2_image 在构建 hosted runtime 对象时可用。

### 构建编译器

```bash
make Compilation
```

会生成：

- `./machine`
- `build/machine_runtime.o`


### 清理构建产物

```bash
make clean
```

### 安装

```bash
sudo make install
```

现在安装目标会一并安装：

- `machine` 编译器
- hosted、freestanding、baremetal 三套 runtime 支持文件
- `.mne` / `.machine` 的 Vim 语法高亮文件
- 仓库自带的示例 `.mne` 程序
- 项目文档，包括头部指令说明

### 卸载

```bash
sudo make uninstall
```

## 编译器用法

### 查看帮助

```bash
./machine --help
```

### 查看版本

```bash
./machine --version
```

### 编译 Machine 程序

```bash
./machine path/to/file.mne -o output_name
```

例如：

```bash
./machine test/struct.mne -o struct
```

## Machine 属于什么类型的语言？

Machine 是一个编译型语言实现。

更准确地说：

- 编译器前端由 C17 编写
- Machine 源码可以降成生成的 C 或 x86_64 汇编
- 所选后端再被编译或链接成原生产物
- 根据 target 的不同，最终可以得到普通原生可执行文件或 baremetal ELF

所以 Machine 不是解释器，也不是“半解释半 JIT”的语言实现。

## `bin.runtime` 自动发现

当源码第一条有效语句是：

```machine
bin.runtime
```

编译器会切换到“安装版 / 项目版 runtime 自动发现”模式，并按以下顺序查找：

1. `/usr/local/lib/machine/machine_runtime.o` 与 `/usr/local/include/machine/machine_runtime.h`
2. `.mne` 文件旁边项目目录中的 `build/` runtime
3. 项目根目录中的 runtime 副本
4. 当前工作目录中的回退副本

这样应用项目就不需要把 runtime 文件手工复制到每个目录。

## 当前源码树

```text
MachinePrograms/
├── Compilation/                # CLI 入口与命令处理
├── include/                    # 公有与内部头文件
├── src/                        # 编译器与运行时实现
├── test/                       # 自带示例与测试 .mne 程序
├── vim/                        # Vim 语法、文件类型检测、ftplugin
├── build/                      # 生成的 runtime 对象和其他构建产物
├── LICENSE
├── README.md
├── README.zh-CN.md
├── use_order.md
├── use_order.zh-CN.md
├── Makefile
└── m.sh
```

## 主要模块

- **lexer**：Machine 源码分词
- **parser**：表达式、语句、顶层声明和指令解析
- **semantic analysis**：符号解析、类型检查与诊断
- **C backend**：生成 C 代码
- **x86_64 asm backend**：生成 GNU 风格 x86_64 汇编
- **runtime 分层**：
  - `runtime.c` 用于 Linux hosted 程序
  - `runtime_freestanding.c` 加入口汇编用于 freestanding target
  - `runtime_baremetal.c` 加入口汇编与链接脚本用于 baremetal target
- **CLI**：命令行解析、backend/target 选择、最终编译流程

## 缩进规则

Machine 同时接受 2 空格和 4 空格块缩进，而且一个文件里不同块可以混用。每深入一层块时，只能比上一层多 2 或 4 个空格；回退时必须回到之前真实出现过的缩进列。

## Vim 语法高亮

用户本地安装：

```bash
make vim-install
```

系统范围安装：`sudo make install` 现在也会把语法文件安装到系统 Vim 运行时目录。

## 示例和文档安装位置

执行 `sudo make install` 后，会填充这些目录：

- `/usr/local/bin/machine`
- `/usr/local/lib/machine/`
- `/usr/local/include/machine/`
- `/usr/local/share/machine/examples/`
- `/usr/local/share/doc/machine/`
- `/usr/local/share/vim/vimfiles/`


本项目使用了人工智能工具 ChatGPT 5.4
MikuTrive协助开发了这个Machine编程语言项目。


## 许可证

Machine 使用 GPL-3.0 协议，见 [LICENSE](./LICENSE)。
