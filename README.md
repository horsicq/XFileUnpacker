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
text-password bytes. Automatic archive/SFX detection has a 20,000 ms budget
per target; `--probe-timeout MILLISECONDS` changes it and `0` explicitly
disables it. Budget exhaustion prints `Detection budget exceeded` and returns
exit code 6, distinct from an unsupported archive (exit code 2):

```bash
xfileunpackerc --password legacy-password --extractarchive out setup.exe
xfileunpackerc --password-hex efe0f0eeebfc --codepage 1251 --extractarchive out setup.exe
xfileunpackerc --probe-timeout 5000 --listarchive setup.exe
```

### POSIX option syntax

The native command line follows the POSIX Utility Syntax Guidelines: single
character options, groupable when they take no argument (`-tv`), option
arguments as a separate or attached token (`-C out` or `-Cout`), `--` to end
the options, order between options irrelevant, and repeatable options that
accumulate. Operation letters follow POSIX tar -- `-t` is the table of
contents, `-x` extracts -- with GNU tar's spellings where POSIX has none.

Operations, at most one per invocation:

| Option | Long form | Operation |
|---|---|---|
| `-t` | `--list` | List archive contents |
| `-x` | `--extract` | Extract members |
| `-W` | `--verify` | Verify integrity, write nothing |
| `-O` | `--to-stdout` | Write selected members to standard output |
| `-i` | `--info` | Show file information (default for a bare target) |
| `-e` | `--entropy` | Show entropy |
| `-s NAME` | `--struct=NAME` | Show one named structure |
| `-S` | `--structs` | Show every available structure |
| `-L` | `--formats` | List container formats this build can open |

Modifiers:

| Option | Long form | Meaning |
|---|---|---|
| `-C DIR` | `--directory=DIR` | Extract into DIR |
| `-f FILE` | `--file=FILE` | Archive to operate on (repeatable) |
| `-X PAT` | `--exclude=PAT` | Skip matching members (repeatable) |
| | `--include=PAT` | Keep only matching members; operands do the same |
| `-k` | `--keep-old-files` | Keep existing destination files |
| | `--overwrite=MODE` | always (default), skip, rename |
| `-j` | `--flatten` | Drop stored directory components |
| `-I` | `--ignore-case` | Case-insensitive member matching |
| `-P PASS` | `--password=PASS` | Archive password |
| | `--password-stdin` | Read the password from standard input |
| `-H HEX` | `--password-hex=HEX` | Exact legacy password bytes |
| | `--codepage=NUM` | Code page for legacy names and passwords |
| | `--probe-timeout=MS` | Format-probe budget, 0 disables |
| | `--stop-on-error` | Strict all-or-nothing extraction |
| `-F TYPE` | `--filetype=TYPE` | Force the container type |
| `-o FMT` | `--format=FMT` | native, technical, unzip, unzip-verbose, zipinfo, json, xml, csv, tsv, text |
| `-v` | `--verbose` | Verbose output |
| `-q` | `--quiet` | Suppress progress and summary lines |
| `-N` | `--no-color` | Disable colour |
| `-h` | `--help` | Help |
| `-V` | `--version` | Version |

Member selection uses operands, as in tar and unzip: the first operand is the
archive and the rest are member patterns.

```bash
xfileunpackerc -tv archive.7z            # verbose table of contents
xfileunpackerc -x -C out archive.7z      # extract into out/
xfileunpackerc -x -C out a.zip 'docs/*'  # extract only docs/
xfileunpackerc -t -X '*.tmp' a.zip       # list, minus the temporaries
xfileunpackerc -W -P secret a.rar        # verify an encrypted archive
xfileunpackerc -O a.zip readme.txt       # one member to standard output
```

Every long option this tool shipped before is kept as an alias, so existing
scripts keep working: `--listarchive`/`--showarchive` (`-t`),
`--extractarchive DIR` (`-x -C DIR`), `--testarchive`/`--test` (`-W`),
`--stdout` (`-O`), `--showstructs` (`-S`), `--listformats` (`-L`),
`--nocolor` (`-N`), `--stoponerror`, and `--xml`/`--json`/`--csv`/`--tsv`/
`--plaintext` (`-o FMT`).

Note that `-V` is version and `-v` is verbose, per POSIX; Qt's built-in version
option would have taken `-v`, so this console registers its own.

### 7-Zip and Info-ZIP compatible syntax

`xfileunpackerc` also accepts 7-Zip and Info-ZIP `unzip` command lines, so
existing habits and scripts work unchanged. The grammar is chosen from the
program name first, then from a leading 7-Zip verb:

```bash
xfileunpackerc l archive.7z              # 7-Zip: list
xfileunpackerc x -oout -psecret a.7z     # 7-Zip: extract with paths
xfileunpackerc t archive.rar             # 7-Zip: test
xfileunpackerc unzip -l archive.zip      # Info-ZIP: list
xfileunpackerc unzip -o -d out a.zip     # Info-ZIP: extract, overwrite
xfileunpackerc unzip -p a.zip readme.txt # Info-ZIP: member to stdout
xfileunpackerc zipinfo archive.zip       # Info-ZIP: zipinfo listing
```

Copying or symlinking the executable to `7z`, `7za`, `7zr`, `unzip` or
`zipinfo` selects that dialect with no extra token, which makes it a drop-in
replacement in existing scripts. A leading verb is only taken as a command when
no file of that name exists, so `xfileunpackerc l` still opens a file called
`l`; `--` forces the file reading.

Supported 7-Zip form: commands `l x e t i` and switches `-o{dir}`,
`-p{password}`, `-y`, `-ao{a|s|u}`, `-i!{wildcard}`, `-x!{wildcard}`, `-so`,
`-slt`, `-t{Type}`, `-ssc[-]`, `--`. Compression, recursion and NTFS-metadata
switches are accepted and ignored, since this is a read-only unpacker.

Supported Info-ZIP form: `-l -v -Z -t -p -c -d dir -j -o -n -q -P password -x
-C --`, a trailing member list, and the zipinfo layout switches. Text
conversion, permission and pager switches are accepted and ignored.

Exit codes stay this project's own (0 success, 1 not found, 2 cannot open,
4 bad parameter, 5 partial, 6 probe timeout) in every dialect rather than
emulating three different schemes.

### Extras available in every dialect

Neither 7-Zip nor Info-ZIP uses GNU-style long options, so `--word` stays free
and this project's own options work inside a foreign command line:

```bash
xfileunpackerc l --format=json archive.7z          # machine-readable listing
xfileunpackerc unzip -l --format=json archive.zip
xfileunpackerc x -oout --password-hex efe0f0eeebfc setup.exe
```

`--format` selects `native` (default), `technical` (7-Zip `-slt`), `unzip`,
`unzip-verbose`, `zipinfo`, or `json`. `--testarchive`, `--stdout`,
`--listformats`, `--include`, `--exclude`, `--overwrite` and `--ignore-case`
are the native spellings of the same commands.

Not yet available: member selection (`--include`/`--exclude`/trailing names)
and path flattening (`-j`, 7-Zip `e`) apply to listing and `--stdout` but are
refused for extraction, and `--overwrite=rename` (7-Zip `-aou`) is refused.
Both need work in the extraction core rather than in the command layer; they
report an error instead of silently doing the wrong thing.

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
