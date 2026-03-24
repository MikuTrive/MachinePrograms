# Machine

[README](./README.md) | [Machine](./Machine.md)

Machine 是一个用 C17 实现的编译型系统编程语言项目。它不是解释器，也不是“半解释半运行时”的语言实现。当前编译器前端会读取 `.mne` 源码，完成词法分析、语法分析、语义检查，生成 C 代码，然后调用系统 C 编译器生成原生可执行文件。

## 编译

### 依赖

在 Fedora 上：

```bash
sudo dnf install gcc make SDL2-devel SDL2_image-devel
```

窗口、图像等运行时功能依赖在构建 runtime 对象时可用的 SDL2/SDL2_image。

### 编译 Machine 编译器

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

### 卸载

```bash
sudo make uninstall
```

## 编译器参数与使用方式

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
./machine ceshi/struct.mne -o struct
```

## Machine 是什么类型的语言？

Machine 是一个**编译型语言实现**。

更准确地说：

- 语言前端由 C17 编写
- Machine 源码会先编译成生成的 C
- 生成的 C 再由系统 C 编译器编译
- 最终得到原生可执行文件

所以 Machine **不是解释型语言**，也**不是半解释半运行时语言**。它当前采用的是**源到源编译 + 原生 C 后端**的实现策略。

## 目录结构说明

```text
Machine_project_v17/
├── Compilation/          # CLI 入口与用户可见命令处理
├── test/                 # 供 m.sh 使用的测试源码
├── include/              # 编译器/运行时头文件
├── src/                  # 编译器与运行时代码
├── build/                # 生成的 runtime 对象等构建产物
├── LICENSE               # GPL-3.0 协议文本
├── Machine.md            # 中英双语教学文档
├── README.md             # 英文项目总览与构建说明
├── README.zh-CN.md       # 中文项目总览与构建说明
└── m.sh                  # test/ 测试辅助脚本
```

## 功能模块介绍

当前项目主要由这些模块组成：

- **lexer**：Machine 源码分词
- **parser**：表达式、语句、顶层声明、常量表解析
- **semantic analysis**：符号解析、类型检查、const 检查、容器检查、诊断输出
- **code generator**：将用户程序生成 C 代码
- **runtime**：内存、数组、链表、网格、数学、终端、窗口/媒体辅助
- **CLI**：`machine --help`、`machine --version`、输入输出与最终编译流程
- **test helper**：`m.sh` 用于自动编译/运行 `ceshi/` 中的测试

## 源代码规范与仓库约定

当前仓库遵循这些实用规则：

- 编译器/运行时代码以 C17 为基准。
- 头文件统一放在 `include/`。
- 编译器/运行时实现文件统一放在 `src/`。
- CLI 入口文件统一放在 `Compilation/`。
- 教学或仓库文档统一放在项目根目录。
- 供脚本测试的 `.mne` 文件统一放在 `ceshi/`。
- 单个源文件应保持在项目规定的行数上限之下，并在变得难读前及时拆分。
- 优先使用清晰直接的命名、边界检查和小型辅助函数，不追求晦涩压缩写法。
- 诊断输出要保持可读、统一。

## 测试脚本

`m.sh` 是项目级测试辅助脚本。

### 显示帮助

```bash
./m.sh
```

### 编译并运行 `ceshi/` 中的全部预定义测试

```bash
./m.sh -c
```

### 仅删除测试生成的二进制文件

```bash
./m.sh -d
```

它**不会**删除 `./machine` 本身。

### 选择脚本输出语言

```bash
./m.sh -l
```

目前支持：

- `en_US`
- `zh_CN`

## 协议

本开源项目采用 **GPL-3.0** 协议。

本项目包含 **ChatGPT 5.4** 参与。

此项目由 **MikuTrive 与 GPT 共同维护**。
