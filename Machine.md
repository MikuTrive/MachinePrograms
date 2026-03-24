# Machine Language Tutorial / Machine 语言教学

[README](./README.md) | [中文总览](./README.zh-CN.md)

---

## English

### 1. Overview

Machine is a compiled systems-language project with a C backend. Source files use the `.mne` suffix. Current language features include variables, constants, functions, structs, modules, pointers, arrays, lists, grids, control flow, math helpers, terminal/window helpers, and multiline constant tables.

### 2. Variables

```machine
var x = 10
var y: i64 = 20
```

Global variables may be declared at the top level. Local variables may be declared inside functions and blocks, subject to the compiler's current scoping rules.

### 3. Constants and constant tables

```machine
const answer = 42
const nums = [10, 20, 30]
const table = [
  [1, 2],
  [3, 4]
]
const cube = [
  [
    [1, 2],
    [3, 4]
  ],
  [
    [5, 6],
    [7, 8]
  ]
]
```

Supported patterns include:

- 1D constant arrays
- 2D constant tables
- 3D constant tables
- multiline nested constant literals

### 4. Numeric types and strings

Common built-in types currently include:

- `i64`
- `f64`
- `str`
- `bool`
- `ptr`
- `array`
- `list`
- user-defined `struct` types

### 5. Operators

Arithmetic:

- `+`
- `-`
- `*`
- `/`

Comparison:

- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`

Access / special operators:

- `@x` : address-of
- `p^` : dereference
- `obj.field`
- `table[i]`
- `table[i][j]`
- `table[i][j][k]`

### 6. Functions

```machine
func add(a: i64, b: i64) -> i64:
  ret a + b
```

Entry point:

```machine
main:
  print add(20, 22)
  ret 0
```

### 7. Structs

```machine
struct Point:
  x: i64
  y: i64

main:
  var p = Point()
  p.x = 10
  p.y = 20
  print p.x
  ret 0
```

Nested field access is supported.

### 8. Modules

```machine
module MathBox:
  func twice(x: i64) -> i64:
    ret x * 2

main:
  print MathBox.twice(21)
  ret 0
```

### 9. Control flow

#### if / elif / else

```machine
if x < 0:
  print "negative"
elif x == 0:
  print "zero"
else:
  print "positive"
```

#### while

```machine
var i: i64 = 0
while i < 5:
  print i
  i = i + 1
```

#### for-style loops

Machine currently relies on `while` for loop-style iteration:

```machine
var i: i64 = 0
while i < 10:
  print i
  i = i + 1
```

#### goto

```machine
label again:
print 1
goto again
```

### 10. Pointers and memory

```machine
main:
  var x: i64 = 10
  var p: ptr = @x
  p^ = 99
  print x

  var mem: ptr = alloc_bytes(8)
  store_i64(mem, 12345)
  print load_i64(mem)
  free_mem(mem)
  ret 0
```

### 11. Arrays

Dynamic arrays use the dedicated `array` type:

```machine
main:
  var arr: array = array_new()
  array_push(arr, 5)
  array_push(arr, 10)
  print array_len(arr)
  print array_get(arr, 1)
  array_free(arr)
  ret 0
```

### 12. Lists

Linked-list helpers use the dedicated `list` type:

```machine
main:
  var xs: list = list_new()
  list_push_back(xs, 11)
  list_push_back(xs, 22)
  print list_size(xs)
  print list_get(xs, 1)
  list_free(xs)
  ret 0
```

### 13. Terminal, window, image, and time helpers

Current runtime helpers include terminal functions, timing functions, grid helpers, window functions, image loading/drawing helpers, and first-stage video helpers. Exact availability depends on how the runtime was built and whether required system libraries are present.

### 14. Comments

Machine uses `--` line comments:

```machine
-- this is a comment
print "hello" -- inline comment
```

### 15. Diagnostics

The compiler emits structured diagnostics with file, line, column, message, and a caret marker. Current builds may also use colorized `ERROR:` and `WARN:` prefixes depending on the compiler/runtime version in use.

---

## 中文

### 1. 总览

Machine 是一个带 C 后端的编译型系统语言项目。源码后缀使用 `.mne`。当前语言已经包含变量、常量、函数、结构体、模块、指针、数组、链表、网格、控制流、数学辅助、终端/窗口辅助，以及多行常量表。

### 2. 变量

```machine
var x = 10
var y: i64 = 20
```

顶层可以定义全局变量。函数与代码块内可以定义局部变量，但要遵守当前编译器的作用域规则。

### 3. 常量与常量表

```machine
const answer = 42
const nums = [10, 20, 30]
const table = [
  [1, 2],
  [3, 4]
]
const cube = [
  [
    [1, 2],
    [3, 4]
  ],
  [
    [5, 6],
    [7, 8]
  ]
]
```

当前支持：

- 一维常量数组
- 二维常量表
- 三维常量表
- 跨多行嵌套常量字面量

### 4. 数值类型与字符串

当前常见内建类型包括：

- `i64`
- `f64`
- `str`
- `bool`
- `ptr`
- `array`
- `list`
- 用户自定义 `struct` 类型

### 5. 运算符

算术运算：

- `+`
- `-`
- `*`
- `/`

比较运算：

- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`

访问 / 特殊运算：

- `@x`：取地址
- `p^`：解引用
- `obj.field`
- `table[i]`
- `table[i][j]`
- `table[i][j][k]`

### 6. 函数

```machine
func add(a: i64, b: i64) -> i64:
  ret a + b
```

入口写法：

```machine
main:
  print add(20, 22)
  ret 0
```

### 7. 结构体

```machine
struct Point:
  x: i64
  y: i64

main:
  var p = Point()
  p.x = 10
  p.y = 20
  print p.x
  ret 0
```

支持嵌套字段访问。

### 8. 模块

```machine
module MathBox:
  func twice(x: i64) -> i64:
    ret x * 2

main:
  print MathBox.twice(21)
  ret 0
```

### 9. 控制流

#### if / elif / else

```machine
if x < 0:
  print "negative"
elif x == 0:
  print "zero"
else:
  print "positive"
```

#### while

```machine
var i: i64 = 0
while i < 5:
  print i
  i = i + 1
```

#### 用 while 模拟 for

Machine 当前主要依靠 `while` 实现 for 风格循环：

```machine
var i: i64 = 0
while i < 10:
  print i
  i = i + 1
```

#### goto

```machine
label again:
print 1
goto again
```

### 10. 指针与内存

```machine
main:
  var x: i64 = 10
  var p: ptr = @x
  p^ = 99
  print x

  var mem: ptr = alloc_bytes(8)
  store_i64(mem, 12345)
  print load_i64(mem)
  free_mem(mem)
  ret 0
```

### 11. 数组

动态数组使用专门的 `array` 类型：

```machine
main:
  var arr: array = array_new()
  array_push(arr, 5)
  array_push(arr, 10)
  print array_len(arr)
  print array_get(arr, 1)
  array_free(arr)
  ret 0
```

### 12. 链表

链表辅助函数使用专门的 `list` 类型：

```machine
main:
  var xs: list = list_new()
  list_push_back(xs, 11)
  list_push_back(xs, 22)
  print list_size(xs)
  print list_get(xs, 1)
  list_free(xs)
  ret 0
```

### 13. 终端、窗口、图像与时间辅助

当前 runtime 已包含终端函数、时间函数、网格辅助、窗口函数、图像加载/绘制辅助，以及第一阶段视频辅助。具体能否使用，取决于 runtime 的构建方式和系统依赖是否齐全。

### 14. 注释

Machine 使用 `--` 行注释：

```machine
-- 这是注释
print "hello" -- 行尾注释
```

### 15. 诊断

编译器会输出带文件名、行号、列号、消息和指针标记的诊断信息。根据当前构建版本不同，也可能带有彩色的 `ERROR:` 与 `WARN:` 前缀。
