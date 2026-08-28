# XFileUnpacker

A generic file unpacking utility with both Console (CLI) and Graphical User Interface (GUI) versions built with Qt5/Qt6.

## Features

- **Console Application**: Command-line interface for batch processing
- **GUI Application**: User-friendly graphical interface
- **Native archive engine**: ZIP, 7z, RAR, CAB, TAR and the other documented
  formats are parsed and unpacked by XArchive without a `7z.exe` or `7z.dll`
  runtime dependency
- **Encrypted archives**: Password entry is available in the archive-content
  view; the CLI accepts text, secure standard-input, or exact legacy password
  bytes and can override the code page used by non-Unicode Inno Setup data
- **Native supplemental readers**: Quake PAK, Doom WAD, Build GRP, WARC,
  safe self-contained mtree, UU and begin-base64 transport, AFIO CPIO,
  LZ4/TAR.LZ4, and nested compression filters are handled in-process without
  linking or loading libarchive
- **Qt5/Qt6 Support**: Automatically detects and uses available Qt version
- **Cross-platform**: Works on Windows, macOS, and Linux

## Supported formats

See [doc/CURRENT.md](doc/CURRENT.md) for the current format support matrix —
every archive, compressor, disk image and package format, with per-format
detect / list / extract status, the codecs behind each one, and the known
limitations.

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

Archive operations accept one of `--password`, `--password-stdin`, or
`--password-hex`. For legacy non-Unicode Inno Setup installers,
`--codepage NUMBER` selects the Windows code page used for filenames and
text-password bytes:

```bash
xfileunpackerc --password legacy-password --extractarchive out setup.exe
xfileunpackerc --password-hex efe0f0eeebfc --codepage 1251 --extractarchive out setup.exe
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

XFileUnpacker is MIT-licensed; see [LICENSE](LICENSE). Supplemental
source-derived codecs have separate terms described in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Contributing

Contributions are welcome! Please feel free to submit pull requests.

## Support

For issues and feature requests, please use the issue tracker.
