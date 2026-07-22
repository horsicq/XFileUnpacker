# XFileUnpacker

A generic file unpacking utility with both Console (CLI) and Graphical User Interface (GUI) versions built with Qt5/Qt6.

## Features

- **Console Application**: Command-line interface for batch processing
- **GUI Application**: User-friendly graphical interface
- **Qt5/Qt6 Support**: Automatically detects and uses available Qt version
- **Cross-platform**: Works on Windows, macOS, and Linux

## Version

**0.1.0** - Initial release

## Requirements

- CMake 3.16 or higher
- Qt5 (Core, Gui, Widgets) or Qt6 (Core, Gui, Widgets)
- C++17 compatible compiler

## Building

### Clone and Build

```bash
git clone https://github.com/horsicq/XFileUnpacker.git --recursive
cd XFileUnpacker
mkdir build
cd build
cmake ..
cmake --build .
```

### Installation

```bash
cmake --install .
```

## Usage

### GUI Application

```bash
XFileUnpacker-GUI
```

### CLI Application

```bash
XFileUnpacker-CLI [options] <file>
```

## Project Structure

```
XFileUnpacker/
├── src/
│   ├── gui/          # GUI application
│   ├── cli/          # CLI application
│   ├── global.h      # Global definitions
│   └── CMakeLists.txt
├── test/             # Test files
├── doc/              # Documentation
├── res/              # Resources (icons, images, etc.)
├── dep/              # Dependencies
├── tools/            # Build tools and scripts
├── CMakeLists.txt
├── README.md
├── LICENSE.txt
├── changelog.txt
└── release_version.txt
```

## License

MIT License - See LICENSE.txt for details

## Contributing

Contributions are welcome! Please feel free to submit pull requests.

## Support

For issues and feature requests, please use the issue tracker.
