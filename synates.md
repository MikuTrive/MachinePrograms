# Machine Lang VS Code Extension

## install
- install the .vsix file and right-click it in vscode explore, click "install extension by vsix"
- or use npm to pack the project, then right-click the output .vsix file in vscode explore, click "install extension by vsix"
```shell
npm install
npx @vscode/vsce package
```

## Features

- Syntax highlighting for Machine programming language
- Support for comments, strings, numbers, keywords, booleans, types, built-in functions, and operators
- Auto-closing brackets and quotes
- Comment toggling support

## File Extensions

- `.mne` - Machine language files

## Syntax Support

### Keywords
```
main func struct var const if else elif while print ret goto label switch case default module unsafe
```

### Boolean Values
```
true false
```

### Types
```
i64 f64 hp str ptr list array bool void
```

### Directives
```
bin.runtime
unsafe.enable
target.*
backend.*
```

### Built-in Functions

The extension supports a wide range of built-in functions including:
- Math functions: `len`, `sqrt`, `sin`, `cos`, `pow`
- Memory operations: `alloc_bytes`, `free_mem`, `store_*`, `load_*`
- System calls: `syscall*`
- File operations: `fd_open_*`, `fd_close`, `fd_read`, `fd_write`
- Terminal operations: `term_*`
- Window operations: `win_*`
- Image operations: `image_*`
- And many more...

## Installation

1. Download the extension package from the VS Code Marketplace
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Click "Install from VSIX..."
5. Select the downloaded .vsix file

## Contributing

If you find any issues or have suggestions for improvements, please feel free to contribute to the project.

## License

MIT License
