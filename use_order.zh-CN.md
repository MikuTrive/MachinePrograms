# Machine 头部指令与构建引导说明

[English Overview](./use_order.md) | [项目 README](./README.zh-CN.md)

> 适用范围：当前 Machine `.mne` 文件最开头可以出现的那些头部指令。
>
> 本文重点说明：**什么时候该写、什么时候不该写、怎么组合、适合什么场景、示例代码怎么写**。

## 这些指令分别控制什么

Machine 的头部指令控制的是构建链路中的不同层级：

- `bin.runtime`
  - 控制 **运行时从哪里来**
  - 告诉编译器自动寻找 Machine runtime，而不是要求你手工复制 runtime 文件

- `unsafe.enable`
  - 控制 **是否允许危险/底层能力**
  - 允许使用 `unsafe:` 块和一批 unsafe builtin

- `backend.c`
  - 选择 **C 后端**

- `backend.x86_64-asm`
  - 选择 **x86_64 汇编后端**

- `target.linux-hosted`
  - 选择 **普通 Linux 用户态程序**

- `target.freestanding-x86_64`
  - 选择 **freestanding x86_64 目标**

- `target.baremetal-x86_64`
  - 选择 **baremetal x86_64 目标**，用于启动链或内核式产物

## 它们应该写在什么位置

这些指令应该放在源文件**最开头**，位于普通代码之前。

推荐顺序：

```machine
bin.runtime
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  ...
```

它们不是强制模板。只写当前程序真正需要的那些即可。

## 什么时候用 `bin.runtime`

当程序依赖标准 Machine runtime，而且你希望编译器自动找到 runtime 时，就用 `bin.runtime`。

典型场景：

- `print`
- `alloc_bytes`
- `free_mem`
- `load_* / store_*`
- `mmap_anon`
- `fd_open_*`、`fd_read`、`fd_write`
- 终端、时间、窗口、图像等 hosted runtime 功能

示例：

```machine
bin.runtime

main:
  print "Hello, Machine"
  ret 0
```

对于 baremetal 目标，通常**不要依赖** `bin.runtime`，因为 baremetal 会走自己的 runtime 源码、入口汇编和链接脚本。

## 什么时候用 `unsafe.enable`

只要程序开始做原始、危险或者底层操作，就应该用 `unsafe.enable`。

典型 unsafe builtin 包括：

- `ptr_offset`
- `ptr_to_i64`
- `ptr_from_i64`
- `load_u8/u16/u32/u64`
- `store_u8/u16/u32/u64`
- `volatile_load_*`
- `volatile_store_*`
- `syscall*`
- `mmap_anon`
- `ioctl_i64`
- 部分 CPU、汇编、端口 I/O builtin

示例：

```machine
bin.runtime
unsafe.enable

main:
  unsafe:
    var mem: ptr = alloc_bytes(16)

    store_u32(mem, 111)
    store_u32(ptr_offset(mem, 4), 222)
    store_u32(ptr_offset(mem, 8), 333)

    print load_u32(mem)
    print load_u32(ptr_offset(mem, 4))
    print load_u32(ptr_offset(mem, 8))

    free_mem(mem)

  ret 0
```

如果程序只是普通打印或者高层逻辑，没有碰到底层 builtin，就不需要 `unsafe.enable`。

## 什么时候用 `backend.c`

当你希望在源码里把“走 C 后端”这件事写清楚时，可以用 `backend.c`。虽然多数情况下 C 后端本来就是默认值。

示例：

```machine
bin.runtime
backend.c

main:
  print "use c backend"
  ret 0
```

## 什么时候用 `backend.x86_64-asm`

当你明确希望走 x86_64 汇编后端时，用 `backend.x86_64-asm`。

典型场景：

- 后端实验
- 更底层的 runtime 工作
- freestanding 实验
- baremetal 产物

示例：

```machine
bin.runtime
unsafe.enable
backend.x86_64-asm

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid
  ret 0
```

对于普通应用程序，C 后端通常还是更简单、覆盖更全的选择。

## 什么时候用 `target.linux-hosted`

当你想明确声明“这是普通 Linux 用户态程序”时，可以用 `target.linux-hosted`。

示例：

```machine
bin.runtime
backend.c
target.linux-hosted

main:
  print "linux hosted"
  ret 0
```

对普通 Linux 程序来说，这个指令通常是可选的，因为常见工作流里它仍然是默认目标。

## 什么时候用 `target.freestanding-x86_64`

当你要写更底层、尽量不走完整 hosted runtime 模型的 x86_64 程序时，用 `target.freestanding-x86_64`。

示例：

```machine
unsafe.enable
backend.x86_64-asm
target.freestanding-x86_64

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid > 0
    print pid
  ret 0
```

适合：

- syscall 风格实验
- freestanding runtime 验证
- 比普通 hosted 更底层，但还不是 baremetal 的程序

## 什么时候用 `target.baremetal-x86_64`

只有当程序明确是给 baremetal 启动或内核式执行环境准备时，才用 `target.baremetal-x86_64`。

典型场景：

- baremetal hello-world
- VGA 显存实验
- 页表、GDT、IDT、中断实验
- QEMU、GRUB 或真实引导链测试

示例：

```machine
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  print "Machine baremetal hello"
  ret 0
```

固定 VGA 地址写显存示例：

```machine
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  print "Machine baremetal hello"

  unsafe:
    var text: ptr = ptr_from_i64(753664)
    volatile_store_u16(text, 8013)
    volatile_store_u16(ptr_offset(text, 2), 8014)
    volatile_store_u16(ptr_offset(text, 4), 8015)

  ret 0
```

重要说明：**不要**期待 baremetal 目标可以像普通 Linux 程序那样直接 `./program` 运行。baremetal 产物不是普通 hosted 可执行文件。

## 常见组合模板

### 1. 普通 Linux 程序

```machine
bin.runtime

main:
  print "hello"
  ret 0
```

### 2. Linux 下的底层实验

```machine
bin.runtime
unsafe.enable

main:
  unsafe:
    var mem: ptr = mmap_anon(16)
    store_u32(mem, 123)
    print load_u32(mem)
    munmap_mem(mem, 16)
  ret 0
```

### 3. 使用 asm 后端的 Linux 程序

```machine
bin.runtime
unsafe.enable
backend.x86_64-asm

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid
  ret 0
```

### 4. Freestanding 程序

```machine
unsafe.enable
backend.x86_64-asm
target.freestanding-x86_64

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid > 0
  ret 0
```

### 5. Baremetal 程序

```machine
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  print "baremetal hello"
  ret 0
```

## 实用决策顺序

如果你只是想写普通 Linux 程序，先从：

```machine
bin.runtime
```

开始。

如果你开始使用指针、原始内存、`mmap`、`syscall`、`volatile` 或其他底层 builtin，再加：

```machine
unsafe.enable
```

如果你明确想要汇编输出，再加：

```machine
backend.x86_64-asm
```

如果程序不再是普通 Linux 程序，而是 freestanding 或 baremetal 环境，就切换到对应的 `target.*`。

## 常见误区

### 把所有指令都当成固定模板

不是。应该只使用当前程序所需的最小正确集合。

### 把 baremetal 目标当成普通 Linux 程序执行

baremetal 产物不是普通 Linux hosted 可执行文件。

### 忘记写 `unsafe.enable`

许多原始指针和内存 builtin 故意被放在 unsafe 模式之后。

### 以为 asm 后端一定更适合所有程序

对很多普通程序来说，C 后端仍然是更简单、更完整的路线。


## 指针地址格式化输出：二进制与十六进制

Machine 现在提供两个新的指针地址格式化 builtin，用来直接显示地址：

- `ptr_hex(pointer_value)`
  - 以十六进制字符串形式返回指针地址
  - 例如：`0x00007ffd1c2a4b80`

- `ptr_bin(pointer_value)`
  - 以二进制字符串形式返回指针地址
  - 例如：`0b000000000000000001111111...`

这两个 builtin 主要用于底层观察、调试、内存布局实验以及地址跟踪。

### 什么时候用

当你想：

- 看栈变量地址
- 看堆地址或 `mmap` 地址
- 比较地址之间的字节间隔
- 直接以更好读的进制显示地址，而不是手动把十进制再转换

就适合使用它们。

因为它们会暴露原始指针信息，所以它们被归类为 unsafe builtin。

### 必要指令

建议配合：

```machine
bin.runtime
unsafe.enable
```

并把真正的调用放在 `unsafe:` 代码块里。

### 十六进制示例

```machine
bin.runtime
unsafe.enable

main:
  var x: i64 = 123

  unsafe:
    print ptr_hex(@x)

  ret 0
```

### 二进制示例

```machine
bin.runtime
unsafe.enable

main:
  var x: i64 = 123

  unsafe:
    print ptr_bin(@x)

  ret 0
```

### 堆地址十六进制布局示例

```machine
bin.runtime
unsafe.enable

main:
  unsafe:
    var mem: ptr = alloc_bytes(16)

    print ptr_hex(mem)
    print ptr_hex(ptr_offset(mem, 4))
    print ptr_hex(ptr_offset(mem, 8))

    free_mem(mem)

  ret 0
```

### 说明

- `ptr_hex(...)` 和 `ptr_bin(...)` 的返回值是字符串，所以最适合直接配合 `print` 使用。
- 它们不会改变地址本身，只是改变地址显示方式。
- 如果你仍然需要十进制整数形式的地址，继续使用 `ptr_to_i64(...)`。
