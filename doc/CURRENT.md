# XFileUnpacker — Current Format Support

Status of format support in XFileUnpacker **0.1.0**, verified against the
source tree and the available regression corpora on **2026-08-20**. This is a
snapshot of this checkout; it is not a statement about every format supported
by an arbitrary system 7-Zip installation.

Authoritative sources for everything below:

| Role | Location |
|---|---|
| File-type enum (`FT_*`) | `_mylibs/Formats/xbinary.h` |
| **Primary format detection** | `XFormats::_getFileTypes()` in `_mylibs/Formats/xformats.cpp` — a chain of `XClass::isValid(pDevice)` probes. This is what the CLI uses. |
| Secondary signature detection | `XBinary::getFileTypes()` in `_mylibs/Formats/xbinary.cpp` — magic-byte chain, used by other callers |
| Type name table | `_mylibs/Formats/xbinary.cpp` (`FT_*` → display name) |
| Handler factory (type → class) | `XFormats::createClass()` in `_mylibs/Formats/xformats.cpp` |
| Advertised openable set | `XArchives::getArchiveOpenValidFileTypes()` in `_mylibs/XArchive/xarchives.cpp` |
| List / extract entry points | `XArchives::getRecords()`, `decompressToFolder()` in `_mylibs/XArchive/xarchives.cpp` |
| Generic codec chain | `XDecompress::multiDecompress()` in `_mylibs/XArchive/xdecompress.cpp` |
| Built-in 7-Zip bridge | `_mylibs/XArchive/xarchives_ip7z.cpp` |
| Static-unpacker regression runner | `_mylibs/XStaticUnpacker/tests/installer_corpus` |

There are three distinct support routes. Only a conventional native archive
needs the full detector/name/factory/openable-set chain. Static unpackers use a
separate PE/CFBF subtype probe and the `XBinary` streaming API. The compiled
ip7z bridge can open some physical files even when XFileUnpacker has no named
`FT_*` detector or native class for them.

## How to read this

- **Detect** — the archive operation can refine the input to the named type.
  The general `xfileunpackerc <file>` scan intentionally may still show the
  base `PE32`, `PE64`, `CFBF`, or container type when no semantic refinement
  exists for that surface.
- **List** — entries can be enumerated (`--listarchive`).
- **Extract** — entry data can actually be decompressed to disk (`--extractarchive DIR`).

`Yes` means the default filename-based route is implemented. Where available,
fixture evidence is stated in Notes or in the verification matrix. `Wired` means a
production route exists but has not been compared with a representative
fixture. `Partial` always carries its boundary in Notes. Enum membership alone
is never counted as support.

Evidence is described at four strengths: **source-traced** (registration and
routing only), **runtime-listed**, **container-verified** (listing/extraction
works through the base container), and **semantic regression** (the named type,
near-miss rejection, routing, and extracted bytes were all checked). A stronger
claim is never implied by a weaker label.

### Backend selection

| Operation / surface | Automatic order | Important boundary |
|---|---|---|
| CLI/GUI list from a file | static subtype → native-preferred type → ip7z → native fallback | With no password, listing may fall back to native metadata after an ip7z open failure. With a supplied password, only an unsupported-format/method result permits fallback. |
| Extract-all or test from a filename | static subtype → native-preferred type → ip7z → native fallback | Password, CRC, corruption, cancellation, resource-limit, and unsafe-path failures are terminal; fallback is only for a structured unsupported-format/method result. |
| Record and arbitrary `QIODevice` APIs | static or native class | ip7z needs a physical filename, so coverage is narrower for devices and memory buffers. |
| GUI with a user-forced type | selected native class | This bypasses automatic ip7z selection. |

Detection returns a **set** of possible types; `XBinary::_getPrefFileType()`
chooses the winner from the preference order. Static installer types precede
their generic PE/CFBF bases, application bundles precede ZIP, and typed tar
wrappers precede their outer compressor. Archive commands re-probe subtypes,
so a normal information scan and an archive listing can legitimately show
different labels.

### Engine note

Archive operations have a static-unpacker tier as well as the conventional
archive readers. The shared `XArchives` routing first normalizes a generic PE
to a more specific installer/packer type when an `XStaticUnpacker` probe
matches.  Static types list and extract through the streaming API
(`initUnpack` / `infoCurrent` / `unpackCurrent`), including extract-all,
single-record extraction, and test operations used by the console and GUI.

For conventional archives, `decompressToFolder(fileName, …)` and `testArchive`
try the **compiled-in 7-Zip source bridge (ip7z)** first and fall back to the
native `XArchive` class only on a structured "unsupported format" error.  A
handful of types listed in `XArchives::isNativeReaderPreferredFileType()`
deliberately bypass ip7z.  Device-based conventional-archive calls continue to
use the native `XFormats::createClass` path.

This is why a format may work from a filename while its native class is
header-only, and why the tables distinguish source-wired paths from
fixture-verified behavior.

---

## Multi-file archives

`Extract` describes the automatic filename route. Where the direct native
`QIODevice` reader is narrower, the Notes column says so.

| Format | Extensions | Detect | List | Extract | Codecs | Notes |
|---|---|---|---|---|---|---|
| 7-Zip | `.7z` | Yes | Yes | Yes | Copy, LZMA, LZMA2, Deflate, Deflate64, BZip2, PPMd7, ZSTD, Brotli, LZ4, LZ5, Lizard, Delta, BCJ, BCJ2, ARM, ARM64, ARMT, PPC, SPARC, IA64, RISCV, Swap2, Swap4 | AES-256 supported. RISCV, Swap2 and Swap4 are available through the compiled filename/ip7z route; the narrower native `XSevenZip` direct-device method map does not expose them. |
| ZIP | `.zip`, `.zipx`; common aliases `.odt`, `.ods`, `.docx`, `.xlsx`, `.epub` | Yes | Yes | Yes | Store, Shrink, Reduce 1–4, Implode, Deflate, Deflate64, BZip2, LZMA, XZ, ZSTD, PPMd8 | ZipCrypto and AES-128/192/256. The aliases remain ZIP unless a dedicated semantic type exists. |
| RAR | `.rar`, `.r00` | Yes | Yes | Yes | RAR 1.5, 2.0, 2.9, 5.0, 7.0 | RAR5 AES is verified by the compiled filename backend; legacy RAR 2.x/3.x crypto is also source-wired there, as qualified under Encryption. A real RAR5 container using RAR 7.0/v6 compression extracted seven files byte-identically to 7-Zip. |
| ARJ | `.arj` | Yes | Yes | Yes | Store, methods 1–4 (`fastest`) | A real method-4 archive extracted four files byte-identically to 7-Zip. |
| LHA / LZH | `.lzh`, `.lha`, `.lzs`, `.pma` | Yes | Yes | Partial | Store/lh0, lh1, lh4, lh5, lh6, lh7 | Coverage is split between backends: native handles lh1/lh5-lh7 but not lh4, while ip7z handles Store/lh4-lh7 but not lh1. Recognized lh2/lh3/lzs/pma methods may list but do not extract, and a mixed lh1+lh4 archive has no one backend that can extract every entry. |
| ZOO | `.zoo` | Yes | Yes | Partial | Store | Native parsing maps LZD and LZH records but has no decoder dispatch for either, and ip7z has no ZOO handler. Existing method-1/LZD and method-2/LZH fixtures therefore list but fail extraction. The linked-list scan is bounded by a roughly 100,001-entry guard. |
| ACE | `.ace` | Yes | Yes | Partial | ACE method 1 (LZ77+Huffman) | Solid archives, encrypted entries and ACE 2.0 `TECH.TYPE 2` are explicitly blocked |
| ARC (SEA) | `.arc` | Yes | Yes | Partial | Stored (methods 1–2) | Methods 3–9 (RLE, Squeezed/Huffman, Crunched/LZW, Squashed) map to `HANDLE_METHOD_UNKNOWN` and are listed but not extractable. CRC-16/ARC is verified on extract |
| FreeARC | `.arc` | Yes | No | No | — | `XFREEARC::getFileParts` emits raw blocks only; no per-file records |
| CAB | `.cab` | Yes | Yes | Yes | Store, MSZIP, LZX, Quantum | The filename route's compiled ip7z handler includes Quantum; the native `XCab` path remains Partial for Quantum. MSZIP/LZX fixtures are covered; add a Quantum fixture. |
| WIM | `.wim`, `.swm`, `.esd`, `.ppkg` | Yes | Yes | Yes | Store, LZX, XPRESS-Huffman, LZMS | The filename route's compiled ip7z handler includes LZMS; the native `XWIM` path remains Partial for LZMS. Store/LZX/XPRESS fixtures are covered; add an LZMS fixture. |
| TAR | `.tar`, `.ova` | Yes | Yes | Yes | Store | The default physical-file route can use ip7z. The direct native/device parser is ustar-oriented: it currently exposes POSIX PAX `x`/`g` and GNU `L`/`K` metadata records as files instead of applying/hiding them. |
| CPIO | `.cpio` | Yes | Yes | Yes | Store | Includes AFIO variants |
| ar | `.a`, `.udeb`, `.lib` | Yes | Yes | Yes | Store | |
| XAR | `.xar`, `.pkg`, `.xip` | Yes | Yes | Yes | none, zlib, bzip2, xz, lzma | macOS flat packages are XAR. A real zlib XAR exposed `hello` plus `[TOC].xml`; both outputs matched 7-Zip exactly. |
| ASAR | `.asar` | Yes | Yes | Partial | Store | Electron bundles. Inline file data is exposed; `unpacked: true` files and symbolic `link` nodes are omitted because no sidecar/source resolution is implemented. |
| WARC | `.warc` | Yes | Partial | Partial | — | Native reader, preferred over ip7z. The current version test accepts WARC/1.0 but rejects common WARC/1.1. Only qualifying `resource` and `response` records with bounded URI/name fields are emitted. A `response` entry contains the complete WARC payload, including embedded HTTP status/headers, rather than body-only data. |
| mtree | `.mtree` | Yes | Yes | Partial | — | Metadata manifest: extraction creates described directories and zero-byte placeholders, but mtree carries no payload bytes. A synthetic semantic regression verified listing, placeholder extraction, traversal/oversized-name rejection, and deterministic case-fold merging. Native reader preferred. |
| UU / begin-base64 | `.uu`, `.uue` | Yes | Partial | Partial | — | One transport block is decoded, but arbitrary UU payloads are not surfaced as a file: the decoded bytes must themselves be a recognized archive. Preamble, line, output-size, and nesting limits are listed below. |
| SquashFS | `.squashfs`, `.sfs`, `.sqsh`, `.snap` | Yes | Yes | Yes | gzip, lzma, lzo, xz, lz4, zstd, uncompressed | Both byte orders (`hsqs`/`sqsh`). Native `XSquashfs` is a superblock/structure reader; records and extraction come from ip7z's `SquashfsHandler`. Twelve generated filesystem fixtures passed. |
| CFBF / OLE compound file | `.doc`, `.xls`, `.ppt`, `.aaf`, `.ole`, `.msp`, `.msm`, generic `.msi`-like containers | Yes | Yes | Yes | Store | Generic CFBF mode exposes raw compound streams. Real MSI/WiX databases take the semantic static-unpacker route described below. |
| MiniDump | `.dmp` | Yes | Yes | Yes | Store | Structural extraction: valid minidump streams are emitted as entries; this does not reconstruct files from dumped process memory. |
| Mach-O FAT | `.fat`, `.mub` | Yes | Yes | Yes | Store | Classic 32-bit FAT headers only; FAT64 is unsupported. The native reader exposes at most 20 architecture slices as entries. |
| DOS/16M / DOS/4G | `.exe` | Yes | Yes | Yes | Store | Structural extraction of the DOS loader, `BW` segments, and optional nested MZ payload; it does not decompile or rebuild the program. DOS/16M is byte-verified. A 529,046-byte DOS/4G fixture listed Loader/VMM/4GWPRO/Payload and passed the central listing/full/name/partial/device/file extraction regression. |

## Single-stream compressors

These formats normally produce one logical output entry. A physical stream may
contain multiple concatenated members (notably lzip), which are decoded into
that logical output. If the decoded result is itself a supported archive,
`XFilteredArchive` can instead expose the inner records during the same archive
operation; this is a fully materialized nested-filter route, not streaming.

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| gzip | `.gz`, `.gzip` | Yes | Yes | Yes | |
| zlib | `.zlib` | Yes | Yes | Yes | RFC 1950 zlib wrapper around Deflate, not a raw RFC 1951 Deflate stream. Preset-dictionary (`FDICT`) streams are rejected. |
| bzip2 | `.bz2`, `.bzip2` | Yes | Yes | Yes | |
| XZ | `.xz` | Yes | Yes | Yes | |
| LZMA (alone) | `.lzma` | Yes | Yes | Yes | |
| lzip | `.lz` | Yes | Yes | Yes | Multi-member streams supported |
| compress | `.Z` | Yes | Yes | Yes | Unix LZW |
| Zstandard | `.zst`, `.zstd`, `.tzst`, `.tzstd` | Yes | Yes | Yes | Standard and legacy v0.4–v0.7 data frames stay on the native path. Concatenated data frames are decoded, bounded skippable frames may precede or separate them, at least one data frame is required, and trailing junk is rejected by full-consumption checks. The `t*` aliases may refine to an inner TAR through dynamic filtering. |
| LZ4 | `.lz4`, `.tlz4` | Yes | Yes | Yes | Native reader preferred |
| LZ5 | `.lz5`, `.tlz5` | Yes | Yes | Yes | Native reader preferred |
| Lizard | `.liz`, `.tliz` | Yes | Yes | Yes | Native reader preferred |
| Brotli | `.br`, `.brotli`, `.tbr` | Yes | Yes | Yes | Native reader preferred. Ordinary raw Brotli has no dependable magic and is extension-led; only the separate 16-byte `MT` envelope is signature-detectable after renaming. |
| LZO | `.lzo` | Yes | Yes | Yes | A real `.tar.lzo` reached its nested TAR and extracted `TOOLNAME.txt` byte-identically to the standalone ground-truth payload. |
| SZDD | `.szdd` | Yes | Yes | Yes | MS-DOS `compress.exe`; LZSS decoded inside `XSZDD`. A real 135-byte stream restored the adjacent 113-byte ground-truth file exactly. |
| KWAJ | `.kwaj` | Yes | Yes | Partial | Store, XOR and MSZIP extract. The recognized SZDD-LZSS and LZH method IDs currently have no dispatch and fail closed. |

### Dynamic nested filters

The native filter adapter can unwrap gzip, bzip2, XZ, lzip, LZMA, LZO,
compress, Brotli, Zstandard, LZ4, LZ5, Lizard, and RPM layers, then open a
recognized archive inside. This is why a `.tar.br` can list TAR members even
though there is no `FT_TAR_BROTLI`, why a Zstandard-wrapped CPIO can expose
CPIO entries while retaining an outer `Zstandard` label, and why `.warc.gz`
can reach the native WARC reader (with all of the WARC limitations above).

Filter-envelope normalization is important when detection refines an inner
gzip to `tar+gzip` or npm: the adapter unwraps the physical gzip layer before
opening the TAR, rather than asking the typed TAR class to decode from the wrong
device offset. A deterministic regression covers plain Brotli, Brotli(TAR),
gzip(TAR), Brotli(gzip(TAR)), four gzip layers around TAR, the five-layer depth
boundary, Brotli-wrapped npm, corrupt inner-gzip rejection, and byte-identical
extraction. A separate real `.tar.lzo` fixture confirms the LZO-to-TAR route
against an independent ground-truth payload. Ordinary Brotli nested inside
another filter remains non-discoverable without an extension because raw
Brotli lacks stable magic.

## Compressed tar wrappers

The outer compressor is unwrapped transparently and the inner TAR is read in the
same archive operation, so entries appear directly.

| Format | Extensions | Detect | List | Extract |
|---|---|---|---|---|
| tar + gzip | `.tar.gz`, `.tgz`, `.tpz` | Yes | Yes | Partial |
| tar + bzip2 | `.tar.bz2`, `.tbz2`, `.tbz`, `.tb2`, `.tz2` | Yes | Yes | Partial |
| tar + xz | `.tar.xz`, `.txz` | Yes | Yes | Partial |
| tar + lzma | `.tar.lzma`, `.tlz` | Yes | Yes | Partial |
| tar + lzip | `.tar.lz` | Yes | Yes | Partial |
| tar + lzop | `.tar.lzo` | Yes | Yes | Partial |
| tar + compress | `.tar.Z`, `.taz` | Yes | Yes | Partial |
| tar + zstd | `.tar.zst`, `.tzst`, `.tzstd` | Outer `Zstandard` | Yes | Partial |
| tar + lz4 | `.tar.lz4`, `.tlz4` | Yes | Yes | Partial |

`tar + zstd` is operational through nested filtering, but the primary detector
currently returns `FT_ZSTD` rather than refining to `FT_TAR_ZSTD`; its Detect
cell is therefore intentionally not `Yes` under this document's definition.
All typed wrapper rows are `Partial` for native extraction because their inner
TAR parser is currently ustar-oriented and does not apply PAX/GNU long-name
metadata. The UI metadata also advertises `.taz` for tar+gzip while automatic
routing conventionally treats `.taz` as tar+Unix-compress; that alias conflict
is retained here as planned work rather than duplicated in the gzip row.

## Disk images

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| ISO 9660 | `.iso`, `.img` | Yes | Yes | Yes | Stored content; `.img` is only an extension hint and is shared with other signature handlers. |
| UDF | `.udf`, `.iso`, `.img` | Yes | Yes | Yes | Stored content; detection, not the shared alias, decides the format. |
| Apple Disk Image | `.dmg` | Yes | Yes | Yes | The filename route's ip7z handler includes Store, zlib, bzip2, ADC and LZFSE. Native `XDMG` remains Partial because it does not decode ADC/LZFSE; targeted fixtures for those two codecs are still needed. |
| Raw disk / virtual-disk image | `.img`, `.simg`, `.lpimg`, `.vdi`, `.vhd`, `.vhdx`, `.avhdx`, `.vmdk`, `.qcow`, `.qcow2`, `.qcow2c` | Generic | Wired | Wired | No generic native image class. Several filesystems/containers are reachable only through the compiled filename-based handlers listed below. |

### Additional compiled ip7z filename handlers

These handlers do not necessarily have an XFileUnpacker `FT_*` name, detector,
factory arm, or arbitrary-device route. `Wired` means the handler is compiled
and the CLI will try it for a physical filename; it is not a claim that every
variant has a local fixture.

| Family | Compiled handlers | General scan | List / extract | Device route | Evidence |
|---|---|---|---|---|---|
| Help/document containers | CHM (`.chm`, `.chi`, `.chq`, `.chw`), HXS (`.hxs`, `.hxi`, `.hxr`, `.hxq`, `.hxw`, `.lit`) | Usually `Binary` | Yes | No native class | Multiple real CHM files list successfully; one was byte-extracted. No local HXS fixture was found. |
| Filesystems and partition maps | Android LP (`.lpimg`, `.img`), APFS (`.apfs`, `.img`), APM (`.apm`), CramFS (`.cramfs`), Ext (`.ext`, `.ext2`, `.ext3`, `.ext4`, `.img`), FAT (`.fat`, `.img`), GPT (`.gpt`, `.mbr`), HFS (`.hfs`, `.hfsx`), MBR (`.mbr`), NTFS (`.ntfs`, `.img`) | Usually generic | Wired | No native class | FAT and Ext2 are byte-verified; a truncated GPT fixture is list-only. HFS registration is source-traced, but the available classic-HFS fixture currently fails to open. Shared `.img`/`.mbr` hints do not override structural detection. |
| Virtual and raw disks | Android Sparse (`.simg`, `.img`), QCOW, VDI, VHD, VHDX, VMDK | Usually generic | Wired | No native class | Compiled registry/source trace; add geometry and sparse fixtures. |
| Split volume / transport streams | Split (zero-padded numeric starts ending `0`/`1`, or alphabetic starts ending `aa`), raw Base64 (`.b64`), MsLZ (`.mslz`), LZMA86 (`.lzma86`), PPMd (`.pmd`) | Usually generic or extension-led | Partial | No native class | Raw Base64 plus dotless and two-/three-/four-digit numeric and dotless/two-/three-/four-letter/prefixed alphabetic Split starts have semantic regressions; MsLZ/LZMA86/PPMd are only source-traced. Split is sibling-dependent and its full start-name grammar is preferred before a truncated root signature. Base64 is extension-only, distinct from UU/`begin-base64`, and currently buffers the full encoded input without a dedicated input cap. |
| Firmware/executable structures | COFF, ELF (`.elf`), PE (`.exe`, `.dll`, `.sys`), Mach-O (`.macho`), Intel HEX (`.ihex`), TE (`.te`), UEFI (`.scap`, `.uefif`) | Base executable or generic | Wired | Format-specific structural readers may exist, but not as archive records | A real UEFI firmware volume listed 162 files + 19 folders, including nested LZMA modules. The local Intel HEX fixture fails to open; no TE fixture was found. |
| Media containers | FLV, SWF | Media or generic | Wired | No conventional archive class | Runtime listing passed for two SWF fixtures (354 and 2 structural records) and one FLV fixture (1 record). |

Local filename-only evidence is deliberately recorded per handler rather than
promoting the whole family:

| Handler / fixture | Result |
|---|---|
| FAT `fat.img` and Ext2 `ext2.img` | Both listed and extracted `bin/ls` (126,584 bytes) and `etc/services` (19,605 bytes); corresponding SHA-256 values matched across the two filesystems. |
| Android Ext partition images | Five real 8–67 MiB images listed successfully, from 5 to 371 files. |
| GPT `gpt.img` | Listed three partitions, but the 32 KiB truncated fixture declares 116.48 GiB of logical partition output. It is list-only and must not be used for extraction. |
| UEFI firmware volume | Listed 162 files + 19 folders / 5.44 MiB from a 2 MiB ROM. |
| Split ZIP `volume.zip.001` + `.002` | Listed and byte-extracted `Hello.txt` (26 bytes, CRC `4C5322A6`); output matched 7-Zip. With only `.001`, local-header listing still succeeds but extraction fails without publishing a file; `.002` alone cannot list or extract. Source hashes remained unchanged. |
| Synthetic raw Base64, raw Split and split-ZIP | Raw Base64 and a two-part byte stream listed/extracted exactly. A stored ZIP deliberately split inside its payload listed/extracted/hash-matched across dotless and two-/three-/four-digit numeric and dotless/two-/three-/four-letter/prefixed alphabetic starts; incomplete/gapped sets failed extraction without output. |
| Classic HFS and Intel HEX | Representative local files were recognized by their format signatures but rejected cleanly by the current archive route. These stay `Wired`, not fixture-verified. |

## Package and application bundles

ZIP-compatible bundles use the ZIP reader even when they do not have a
dedicated semantic type. APKS identification follows bundletool's documented
output shape: a root `toc.pb` plus generated `.apk` payloads (see the
[Android bundletool example](https://developer.android.com/guide/playcore/asset-delivery/texture-compression#verify-the-contents-of-the-app-bundle)).

| Format | Extensions | Dedicated type | General detection | List | Extract | Notes |
|---|---|---|---|---|---|---|
| JAR | `.jar` | Yes | JAR | Yes | Yes | ZIP refinement requires an exact, non-empty root `META-INF/MANIFEST.MF`. Packed and unpacked manifest sizes are each capped at 16 MiB; encryption, a second codec layer, incomplete decoding or CRC failure keeps the file generic ZIP. APK is no longer co-reported as JAR merely because both inherit ZIP handling. |
| APK | `.apk` | Yes | APK | Yes | Yes | ZIP refinement requires an exact, non-empty root `AndroidManifest.xml`; `classes.dex` alone is insufficient, while a resource-only APK is valid. The manifest is plaintext, single-stage, fully decoded/CRC-authenticated, and capped at 16 MiB packed and unpacked. Empty, nested, case-mismatched, corrupt or encrypted manifest lookalikes stay ZIP. |
| APKS | `.apks` | Yes | APKS | Yes | Yes | Primary refinement requires an exact, non-empty, CRC-authenticated root `toc.pb` capped at 16 MiB plus a complete inner APK whose root manifest passes the APK contract above. At most eight APK candidates are inspected. Stored outer members are CRC-checked in place; outer Deflate is materialized only when packed and unpacked sizes are at most 64 MiB, then fully authenticated. Encrypted, multi-stage and other outer codecs are refused. Tests cover stored/deflated and resource-only positives plus missing/empty/corrupt metadata, corrupt outer or inner CRCs, encrypted outer or inner identities, incomplete ZIP shapes and unsupported codecs. |
| IPA | `.ipa` | Yes | IPA | Yes | Yes | ZIP refinement requires a non-empty direct `Payload/<one bundle>.app/Info.plist`, with packed and unpacked sizes capped at 16 MiB. The plist must be plaintext, single-stage and fully decoded/CRC-authenticated; root, unrelated, empty, corrupt and encrypted plist lookalikes stay ZIP. A real 34,910-byte IPA refined correctly and all six extracted files matched 7-Zip by SHA-256. |
| WAR / EAR | `.war`, `.ear` | No | ZIP | Yes | Yes | |
| XPI | `.xpi` | No | ZIP | Yes | Yes | |
| CBZ | `.cbz` | No | ZIP | Yes | Yes | |
| APPX / MSIX | `.appx`, `.msix` | No | ZIP | Yes | Yes | |
| Python wheel | `.whl` | No | ZIP | Yes | Yes | |
| OpenDocument | `.odt`, `.ods`, `.odp` | No | ZIP | Yes | Yes | One ODS fixture listed 8 files + 9 folders. |
| OOXML | `.docx`, `.xlsx`, `.pptx` | No | ZIP | Yes | Yes | Two XLSX fixtures listed 9 files each. |
| EPUB | `.epub` | No | ZIP | Yes | Yes | Compiled ZIP alias; no dedicated semantic type. |
| XAPK | `.xapk` | No | ZIP | Yes | Yes | No dedicated semantic refinement. A real 145,203,787-byte APKCombo bundle listed generically as ZIP with eight stored entries (six APKs, `icon.png`, and `manifest.json`); this large fixture was list-only. |

TAR-derived packages are a separate family:

| Format | Extensions | Dedicated type | General detection | List | Extract | Notes |
|---|---|---|---|---|---|---|
| npm package | `.tgz` | Yes | NPM | Yes | Yes | Primary refinement uses npm's conventional `package/` subfolder and required fields from the [npm tarball requirements](https://docs.npmjs.com/cli/install/#npm-install-tarball-file): `package/package.json` must be valid JSON no larger than 1 MiB, with non-empty string `name` and `version`. NPM is native-reader-preferred so the outer gzip does not hide TAR members. Tests cover the positive route, root-path/missing-version/invalid-JSON/numeric/blank near misses, exact 1 MiB acceptance, 1 MiB+1 rejection, nested filtering, and byte-identical extraction. |
| Rust crate | `.crate` | No | tar + gzip | Yes | Yes | A gzip-compressed TAR, not ZIP. |

Other packages:

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| Debian package | `.deb` | Yes | Yes | Yes | Semantic `ar` validation requires the first member to be `debian-binary` with exact contents `2.0\n`, followed by `control.tar*` before `data.tar*`; underscore-prefixed extensions are ignorable and the scan is capped at 65,536 members. Accepted payload suffixes are uncompressed TAR, gzip, xz, Zstandard, bzip2 and LZMA. |
| RPM package | `.rpm` | Yes | Yes | Yes | Payload CPIO may be stored or compressed with gzip, xz, Zstandard, bzip2 or LZMA. Header/index sizes and counts are bounded; when the non-empty `RPMTAG_PAYLOADCOMPRESSOR` tag exists, its normalized codec must agree with payload magic or validation fails. Stored payloads must have valid CPIO magic and structure. |
| macOS package | `.pkg` | Yes | Yes | Yes | XAR container |
| Snap | `.snap` | Yes | Yes | Yes | Snap is a SquashFS image; handled by the SquashFS reader |

## Installers, self-extractors and packers

Handled by the `XStaticUnpacker` classes, wired into the app in this revision.
Detection, listing, extraction, and testing go through the shared `XBinary`
streaming API (`initUnpack` / `infoCurrent` / `moveToNext` / `unpackCurrent`).
The CLI and GUI probe these types before the ip7z bridge so an installer is not
misread as a plain PE and listed as raw sections.

For **List** and **Extract/Unpack**, bold **Yes** means a real fixture confirmed
the production handler. `Wired` means the class and common route exist, but its
output has not been independently compared. The 150-sample strict corpus
validates direct production handlers rather than every CLI/GUI presentation
path; it excludes Inno, NSIS, AutoIt, generic SFX, and executable packers. A
normal information scan may show only PE/CFBF; the **Archive probe** column is
the subtype selected for the archive operation.

### Installers / self-extractors (multi-file)

| Format | Archive type | Archive probe | List | Extract | Notes |
|---|---|---|---|---|---|
| Inno Setup | `PE32/PE64: Inno Setup` | Yes | **Yes** | **Yes** | Deterministic layouts cover modelled 5.x, 6.0-6.4, 6.7, and 7.0 schemas. Inno 5.6.1 passed all 11 compression/solid variants byte-identically; cross-version fixtures cover shared locations, external records, non-`{app}` destinations, loader integrity, 64-bit fields, and resource limits. Older loaders, unmodelled 6.5/6.6 layouts, encryption, and multi-slice installers fail closed; ANSI names use a CP1252 fallback |
| NSIS | `PE32/PE64: NSIS` | Yes | Partial | Partial | LZMA/zlib/bzip2, solid + non-solid are source-wired, and `SetOutPath` is surfaced as the advanced **Path** column. In the available 20-file runtime sweep all detected/listed, but only 13 produced non-zero records; seven non-solid 2.51/3.12 bzip2/LZMA/zlib or zip2exe-classic cases reported zero sizes. The earlier unqualified 7-Zip parity claim is therefore withdrawn pending repair and byte checks. |
| WiX Burn v3 | `PE32/PE64: WiX Burn bundle` | Yes | **Yes** | **Yes** | PE `.wixburn` bootstrapper with a CAB UX container and zero or one attached CAB; fixture extraction is byte-identical. Additional attached containers, external/detached payloads, and WiX v4 LZMA fail closed |
| 7-Zip SFX | `PE32/PE64: 7-Zip SFX` | Yes | **Yes** | **Yes** | 20 fixtures; 7z payload with MD5/name contracts. |
| WinRAR SFX | `PE32/PE64: WinRAR SFX` | Yes | **Yes** | **Yes** | 24 fixtures; RAR payload with MD5/name contracts. A separate 259,725-byte real RAR4 overlay listed/extracted `README.txt` (67 bytes, CRC `A6DCD3A2`) byte-identically to 7-Zip. |
| Generic SFX | `PE32/PE64: SFX` | Yes | Wired | Wired | |
| IExpress | `PE32/PE64: IExpress` | Yes | **Yes** | **Yes** | 12 fixtures; embedded CAB with Store/MSZIP/LZX and MD5/name contracts. |
| InstallForge | `PE32/PE64: InstallForge` | Yes | **Yes** | **Yes** | 11 fixtures; 7z, bzip2, and gzip payloads. |
| CreateInstall | `PE32/PE64: CreateInstall` | Yes | **Yes** | **Yes** | 9 fixtures; Store, LZGE and Gentee PPMd-I, including solid streams and sibling volumes. |
| Actual Installer | `PE32/PE64: Actual Installer` | Yes | **Yes** | **Yes** | 8 fixtures; ZIP payloads with strict MD5 verification. |
| Advanced Installer | `PE32/PE64: Advanced Installer` | Yes | **Yes** | **Yes** | 8 direct-MSI and EXE-bootstrapper fixtures; embedded/external MSI/CAB payloads. |
| Smart Install Maker | `PE32/PE64: SmartInstall` | Yes | **Yes** | **Yes** | 5 fixtures; Store, deflate and LZX, filenames and split volumes. |
| Clickteam Install | `PE32/PE64: Clickteam` | Yes | **Yes** | **Yes** | 7 fixtures; proprietary Store/zlib container, names and volumes. |
| Tarma (InstallMate) | `PE32/PE64: Tarma` | Yes | **Yes** | **Yes** | 6 fixtures; `tiz2z`/`tiz3z`, metadata paths, volumes and loose-file graph. |
| AutoIt | `PE32/PE64: AutoIt` | Partial | Partial | Partial | AutoIt3 is compiled and v3 samples reach the type, but current runtime samples expose zero records; v2 samples fail the probe. No usable extraction is presently verified. |
| Enigma Virtual Box | `PE32/PE64: Enigma Virtual Box` | Yes | **Yes** | **Yes** | 10 fixtures; VFS Store/aPLib and installed filenames. |
| BoxedApp | `PE32/PE64: BoxedApp` | Yes | **Yes** | **Yes** | 10 fixtures; VFS Store/zlib and installed filenames. |
| Install Simple | `PE32/PE64: Install Simple` | only with `WITH_XEMULATOR` | **Yes** (optional build) | **Yes** (optional build) | 14 fixtures pass the bounded x86-emulated range decoder. The default app build does not enable this class. |
| MSI | `MSI` (CFBF) | Yes | **Yes** | **Yes** | Parses StringPool/Columns/File/Media plus Component/Directory, resolves installed names, and extracts embedded/sibling CABs or loose files. Patches, transforms, and databases without File/Media intentionally fall back to generic CFBF streams. |
| WiX (MSI) | `MSI: WiX` (CFBF) | Yes | **Yes** | **Yes** | WiX marker plus the semantic `XMSI` route. Six WiX fixtures and overlapping Advanced Installer forms passed; a CLI fixture extracted `app.exe` with the expected SHA-256. Distinct from the PE Burn-v3 row. |

### Packers / protectors (single-file → restored executable)

For this table, `Not working in corpus` means a production class is reachable,
but every available family sample either failed or fell back to raw PE-section
records; it is more specific than the source-only `Wired` label.

| Format | Type shown | Detect | Unpack | Notes |
|---|---|---|---|---|
| UPX | `PE32/PE64: UPX` | Yes | Wired | One sample from each of 54 local UPX version directories routed and listed as UPX with no failures; restored-executable bytes still need comparison. App routing currently covers PE32/PE64 only. `XUPX` contains ELF/Mach-O/DOS implementations, but generic non-PE `FT_UPX` is not wired through named detection/factory selection. |
| ASPack | `PE32: ASPack` | Yes | Partial | PE32 only. Of 17 local samples, 15 fell back to raw PE-section records and two failed; no restored image was verified. |
| FSG | `PE32: FSG` | Yes | Not working in corpus | PE32 only; 5/5 available samples failed. |
| MEW | `PE32: MEW` | Yes | Not working in corpus | PE32 only; 3/3 available samples failed. |
| NsPack | `PE32: NsPack` | Yes | Not working in corpus | PE32 only; 7/7 available samples failed. |
| Petite | `PE32: Petite` | Yes | Not working in corpus | PE32 only; 2/2 available samples failed. |
| Yoda's Protector | `PE32: Yoda's Protector` | Yes | Not working in corpus | PE32 only; available samples either exposed raw PE sections or failed. |

## Planned / to add later

| Item | Kind | Notes |
|---|---|---|
| Repair runtime-failing static routes | Correctness | Fix the seven zero-record/zero-size NSIS cases, AutoIt v2/v3, ASPack fallback, and the currently failing FSG/MEW/NsPack/Petite/Yoda corpora before promoting them. Preserve family-specific negative checks. Tracker #12 |
| Byte-verify remaining source-wired routes | Coverage | Add restored-image comparisons for UPX, generic SFX, and every repaired packer/installer, then exercise the same fixtures through both CLI and GUI. The 54-directory UPX sweep currently proves listing/routing only. Tracker #12 |
| GUI runtime end-to-end smoke | Coverage | Current Qt 6 GUI evidence is compile/link/no-op rebuild only. Add automated open → list → extract → test verification with model-field checks. |
| Real-world package breadth | Coverage | Add bundletool-generated universal/split/asset-slice APK Sets, modern signed IPAs, more XAPK producers, and registry-produced scoped npm packages. Synthetic semantic/near-miss coverage and one legacy real IPA now exist. |
| Remaining ip7z-only handler fixtures | Coverage | Add HXS/additional CHM variants, Android LP/Sparse, APM/CramFS/MBR, VDI/VHD/VHDX/VMDK/QCOW, APFS/HFS/NTFS, MsLZ/LZMA86/PPMd, Intel HEX and TE fixtures; verify names, hashes and sparse handling. FAT, Ext, UEFI, SWF, FLV, raw Base64 and numeric/alphabetic Split now have runtime evidence. |
| Advanced native-vs-ip7z codecs | Coverage | Add CAB Quantum, WIM LZMS, and DMG ADC/LZFSE fixtures to prove both automatic filename success and the documented native-device boundary. |
| Backend fallback and remaining hostile-path matrix | Safety | Unsupported → native fallback and terminal password/CRC/corruption behavior still need a dedicated matrix. Path preflight now covers traversal in both separator styles, normalized absolute/drive/UNC-like paths, case/Unicode duplicates, file/directory collisions, ADS, reserved device names, oversized metadata, empty destinations, and source immutability; link/hardlink/reparse and mid-commit rollback cases remain. |
| POSIX/GNU TAR metadata | Capability | Apply and hide PAX `x`/`g` and GNU `L`/`K` pseudo-records in the native TAR parser, with long-path/link-path, global-header, malformed-length, size-limit, and traversal regressions. Typed compressed wrappers currently inherit this limitation. |
| Native legacy archive codecs | Capability | Wire ZOO LZD/LZH and KWAJ SZDD-LZSS/LZH, reconcile per-entry LHA lh1/lh4 routing, and add byte-checked mixed-method fixtures. Existing ZOO fixtures already cover method 1/LZD and method 2/LZH failure cases. |
| WARC 1.1 and response semantics | Correctness | Accept WARC/1.1, define whether `response` extraction should expose the full embedded HTTP message or body-only bytes, broaden safe record types/URIs if intended, and add `.warc`/`.warc.gz` fixtures. |
| Transport bounds and alias cleanup | Safety / dispatch | Add a dedicated raw-Base64 input cap, cover malformed/trailing data, and resolve the `.taz` tar+gzip UI alias versus tar+compress automatic route. Split start styles and a missing-middle volume are now covered; add long carry/rollover and large volume-count cases. |
| Native-device state contracts | Coverage | Add direct C++ `QBuffer` tests proving semantic APK/JAR/APKS/IPA/npm probes and nested-filter failures preserve caller position and do not leak temporary state. |
| Generic non-PE UPX routing | Dispatch | Wire `FT_UPX` for ELF/Mach-O/DOS through detection, preference and factory paths, then add end-to-end fixtures. |
| WiX Burn v3 multi-container, v4, and detached containers | Enhancement | generalize the current fixed UX/attached contexts for multiple v3 attached CABs, add the v4 LZMA schema, and provide an explicit source for detached/external content. Tracker #9 |
| Inno older-loader and newer-schema coverage | Coverage | add fixtures for pre-5.1.5 loaders and model currently unsupported 6.5/6.6 layouts. Tracker #11 |
| Inno encrypted / multi-slice installers | Capability | add explicit key/slice handling; both currently fail closed. Tracker #11 |
| Inno legacy code-page recovery | Enhancement | derive the ANSI build code page when possible; current fallback is CP1252. Tracker #11 |
| Inno Setup `OPTIONAL_PATH` | Enhancement | surface Inno's `{const}\…` destination directory as an advanced Path column, like NSIS's SetOutPath |
| Install Simple in the default build | Build | enable `WITH_XEMULATOR` (or a non-emulator path) so `XInstallSimple` ships in the app |
| ALZ, EGG, StuffIt, PEA, PAK | New formats | no detector or reader today (see Not supported) |

## Encryption

| Scheme | Formats | Extract | Listing without password |
|---|---|---|---|
| ZipCrypto | ZIP | Yes | Yes — headers are not encrypted |
| AES-128 / 192 / 256 (WinZip) | ZIP | Yes | Yes |
| AES-256 | 7-Zip | Yes | Only when headers are unencrypted |
| Legacy RAR 2.x crypto | RAR | Source-wired in the ip7z filename backend | Depends on whether archive headers are encrypted |
| RAR 3.x AES/password | RAR | Source-wired in the ip7z filename backend | Depends on whether archive headers are encrypted |
| AES | RAR5 | Yes | Only when headers are unencrypted |
| ARJ garble | ARJ | Yes | Yes |
| ACE encryption | ACE | No | Yes — explicitly blocked for extraction |

The CLI takes the archive password either as a normal argument or from standard
input (use one, not both):

```bash
xfileunpackerc --password 123456 --extractarchive out archive.7z
echo 123456 | xfileunpackerc --password-stdin --extractarchive out archive.7z
```

`--password` is the convenient form; `--password-stdin` avoids the password
appearing in the process list or shell history. The same options apply to
`--listarchive`.

The deterministic encryption regression generates ZipCrypto, WinZip AES-256,
7-Zip AES with visible headers, and 7-Zip AES with encrypted headers. ZIP and
visible-header 7-Zip listings remain available without a password; encrypted
7-Zip headers require the correct password. Extraction with a missing or wrong
password fails without publishing files, while both `--password` and
`--password-stdin` restore byte-identical payloads with the correct value. The
test also verifies that every source archive remains unchanged.

On the native AES path, a missing password reports
`[XAESDecoder] Password is required for AES decryption`. The default ip7z path
uses its own backend-specific wording, so callers should key off failure state,
not that exact string.

The legacy RAR rows describe instantiated decoders and selection logic in the
compiled filename backend, not fixture verification and not equivalent coverage
in the narrower native `XRar`/arbitrary-device path.

## Codecs reachable through the generic chain

`XDecompress::multiDecompress()` handles Store/Copy, zlib, Deflate,
Deflate64, BZip2, LZMA/LZMA2, PPMd7/8, ZSTD, LZ4/LZ5/Lizard, Brotli, XZ,
lzip, Unix compress (LZW), LZO, Shrink, Reduce, Implode, RAR
1.5/2.0/2.9/5.0/7.0, LHA lh1/lh5/lh6/lh7, ACE method 1, ARJ methods and
`fastest`, KWAJ XOR, and IT214/IT215 8/16-bit transforms. It also dispatches
ZIP AES/ZipCrypto, 7-Zip AES and RAR5 AES layers; PDF LZW, ASCII85,
ASCIIHex, RunLength and image transforms; CAB Store/MSZIP/LZX-CAB framing;
and the Delta/BCJ/BCJ2/ARM/ARM64/ARMT/PPC/SPARC/IA64 branch filters.

These enum values are **not** generic-chain methods:

`LZSS_SZDD`, `KWAJ_LZSS`, `KWAJ_LZH`, `ZOO_LZD`, `ZOO_LZH`, `LZX`, `XPRESS`,
`XPRESS_HUFF`, `ANDROID_XML`, `ACE_DELTA`, `ARCHIVE_STREAM`, `FILE`.

- `LZSS_SZDD` is owned by `XSZDD`; `LZX` is framed by the CAB/WIM readers; and
  `XPRESS_HUFF` is owned by the WIM reader.
- `KWAJ_LZSS`, `KWAJ_LZH`, `ZOO_LZD`, `ZOO_LZH`, and plain `XPRESS` currently
  have no reachable decoder dispatch. Recognizing or listing those method IDs
  must not be read as extraction support.
- `ANDROID_XML` is a domain transform rather than a general compressor.
- `ARCHIVE_STREAM` and `FILE` are routing sentinels.
- `ACE_DELTA` is not implemented.

`ARCHIVE_STREAM` is deliberately refused before codec dispatch because it
requires the owning archive's indexed state. Other unsupported enum values
reach the unknown-method result. Callers must retain the owning reader and its
private state.

## Not supported

No native detector/class and no registered compiled ip7z handler:

| Format | Extensions |
|---|---|
| ALZ | `.alz` |
| EGG | `.egg` |
| StuffIt | `.sit`, `.sitx` |
| PEA | `.pea` |
| PAK | `.pak` |
| SAR (as a distinct type) | `.sar` |

Partially blocked or route-limited, listed above for detail: ACE beyond method
1, FreeARC file records, ZOO LZD/LZH, KWAJ SZDD-LZSS/LZH, mixed/legacy LHA
methods, native TAR PAX/GNU metadata, ASAR sidecars/links, WARC 1.1/record
semantics, arbitrary UU payloads, native CAB Quantum, native WIM LZMS, native
DMG ADC/LZFSE, and non-PE UPX application routing.

## Metadata fidelity

Archive properties are best-effort and backend-specific; a blank field means
the provider did not expose a trustworthy value.

- The record model preserves the numeric handler method and the provider's
  exact method text separately. Unknown text therefore remains visible instead
  of becoming an empty Method column.
- Host OS, CRC/checksum, POSIX/Windows attributes, modified/created/accessed
  times, encryption, solid/block state, and path are carried when available.
- Packed size may be a block-level value rather than a per-file value. A solid
  block is counted once in totals, and per-file ratios are suppressed when the
  denominator would be misleading.
- Offsets and packed sizes are provider-defined and may be unavailable for
  solid, synthesized, or semantic installer records.
- Listing a property does not promise that extraction restores it on every
  platform. The ip7z merge path preserves supported permissions and modified
  time; other readers restore only the fields they implement.

## Extraction safety and resource limits

The compiled ip7z route scans at most 8 MiB for a handler, follows at most 32
handler-declared main-subfile layers, accepts at most 100,000 items, caps one
text property at 32,768 UTF-16 characters and aggregate metadata at 32 Mi
characters, and caps decoder memory at 4 GiB in a 64-bit process or 512 MiB in
a 32-bit process. This 32-step bound is specifically the handler main-subfile
chain, not a promise to recursively unpack any 32 arbitrary archives.
Inno separately caps header data and its LZMA dictionary at 64 MiB. Burn caps
its manifest at 16 MiB and its record count at 100,000.

The native dynamic-filter route permits at most four decoded layers, caps each
materialized layer at 512 MiB, and automatically probes only inputs up to
256 MiB. Compressed-TAR materialization has the same 512 MiB decoded ceiling.
The fifth-layer boundary is regression-tested and stays as one outer stream
rather than being partially misidentified.

APK/JAR/APKS/IPA/npm semantic refinement enumerates at most 20,000 container
records. APK and JAR identity manifests and IPA `Info.plist` are each capped at
16 MiB packed and unpacked and must fully decode with a valid CRC. APKS applies
the same 16 MiB bound and authentication to `toc.pb`, then inspects at most
eight APK candidates: Store is CRC-checked and parsed in place, Deflate is
accepted only when both packed and unpacked sizes are at most 64 MiB and is
then fully CRC-authenticated, and every other outer method is refused. npm caps
`package.json` at 1 MiB. UU caps its preamble at 128 KiB, one line
at 1 MiB, decoded data at 512 MiB, and recursive transport/archive depth at
four. WARC caps a header block at 1 MiB, scans at most 100,000 physical records,
and caps a generated name at 32,768 bytes. mtree caps a logical line at 1 MiB,
continuations at 1,024, entries at 100,000, input lines at 1,000,000, and one
decoded name at 32,768 bytes. ASAR caps JSON recursion at 256 and items at one
million, but its declared 32-bit header JSON is only bounded by the source size,
not by a smaller dedicated allocation limit. Raw ip7z Base64 likewise buffers
the complete encoded input without a dedicated cap.

Before ip7z extraction, the complete entry set is preflighted. `..` traversal
with either separator style, unsupported links/hardlinks, alternate streams,
anti/deleted/reparse entries, Windows reserved device names, case-folded or
Unicode-normalized duplicates, and file/directory path collisions reject the
archive. Drive, UNC-like and leading-root prefixes are normalized to a path
below the destination; `.` and duplicate separators are collapsed.
Extraction first uses a private staging directory beside the destination and
then commits each member with `QSaveFile`; a member is atomic, but the whole
archive is not a transaction, so earlier committed files/directories can remain
after a later failure. Native/static routes also normalize names, prevent
symlink escapes, suffix ordinary collisions, and use temporary/atomic member
writes where implemented.

There is no universal decompressed-byte or compression-ratio ceiling across all
backends. Callers still need destination free-space policy and may cancel via
`PDSTRUCT`.

## Archive/listing/static-unpacker changes (2026-08-20)

- Archive methods retain both the numeric handler method and its reported text,
  so the console and GUI no longer lose the Method column during conversion.
- APK, JAR, IPA and APKS now authenticate their mandatory ZIP members instead
  of trusting filenames or empty placeholders. APK requires a root manifest
  (not merely `classes.dex`), JAR is no longer added automatically to APK, IPA
  authenticates `Payload/<bundle>.app/Info.plist`, and APKS authenticates both
  `toc.pb` and the chosen inner APK identity. npm is likewise refined from
  tar+gzip after bounded `package.json` validation and is kept on the native
  reader so the gzip envelope cannot hide TAR members. All semantic probes
  preserve the caller's device position.
- Nested filtering now normalizes refined typed wrappers such as `tar+gzip` and
  npm back to their physical compressor envelope before materializing a layer.
  This fixes valid Brotli(gzip(TAR/npm)) inputs while retaining depth, size,
  corruption, and cancellation checks.
- The Split handler's complete numeric/alphabetic first-part grammar is tried
  before an equally ranked truncated root signature. This makes a ZIP split
  inside its payload route through the sibling-volume joiner instead of being
  misopened as the root ZIP.
- Solid packed-size totals are block-aware; a shared block contributes once and
  can produce a useful overall ratio.
- Host OS, fuller attributes, per-file ratio, and available checksum metadata
  are exposed by the archive record model.
- Static-unpacker detection, listing, extract-all, indexed extraction, and test
  are routed centrally for the CLI and GUI.
- Console archive listing now performs archive/static subtype refinement even
  when the preliminary scan returned `Binary`, `MSDOS`, PE, or CFBF. This makes
  native-only structural readers and semantic MSI/WiX listing reachable without
  breaking filename-only ip7z formats.
- `XMSI` resolves File/Media/Component/Directory semantics and streams installed
  payloads from embedded or sibling CABs and loose sources; `XWiX` delegates to
  that path after authenticating its WiX marker.
- WiX Burn v3 embedded-CAB bootstrappers have a dedicated PE32/PE64 static
  unpacker. Additional attached CABs and unsupported v4/detached/external
  forms reject cleanly.
- Inno parsing uses deterministic version schemas, CRC/full-width loader tables,
  independent file/location counts, and bounded header decompression; it
  validates data-block CRCs, filters, and hash layout before committing output.
- ASN.1 UTCTime/GeneralizedTime parsing now returns canonical `Qt::UTC` on Qt 5
  and uses `QTimeZone::UTC` on Qt 6.8+, restoring the deterministic primitive
  decoder contract without the newer Qt deprecation.

### Verification for this change set

| Case | Result |
|---|---|
| Deterministic in-memory `test_installer_corpus --selftest` | Passed in 56.1 s wall time on the final run; covers lifecycle, short/stalled I/O, cancellation, rollback, exact codec results, framing/limits, ZIP/CAB corruption, path/state rejection and related parser contracts. |
| Strict production-handler installer corpus | 150/150 expected detections and 161/161 extraction contracts passed; zero skips, open failures, or cross-family hits. Includes 7-Zip SFX, WinRAR SFX, IExpress, MSI/WiX, Advanced/Actual Installer, InstallForge, Clickteam, CreateInstall, EnigmaVB, BoxedApp, optional InstallSimple, SmartInstall and Tarma. |
| Packed-binary negative installer scan | 603/603 opened; zero static-installer hits and zero open failures. |
| Inno Setup 5.6.1, 11 Store/zlib/bzip2/LZMA/LZMA2 and solid/filter variants | All extracted byte-identically |
| Inno Setup 5/6/7 location and loader-integrity fixture | Shared locations, external records, non-`{app}` paths, CRC/decoy/full-width/overlay/limit cases passed |
| Central legacy APIs on ordinary ZIP, Inno 5.6.1/7.0.2, and WiX 3.14 MSI | Listing; full/name/partial/device/file extraction; presence; and forged-record rejection passed |
| Fresh Qt 5 and Qt 6 console, Qt 6 GUI, and corpus-runner MSVC/Ninja builds after the parser/routing changes | All compile and link successfully; immediate rebuilds are clean no-ops. |
| WiX Burn v3 embedded-CAB and external-payload fixtures | Attached extraction matched the source payload by SHA-256; the unsupported external form failed closed |
| WiX 3.14 MSI through the shipped console | Refined CFBF to `MSI: WiX`, listed `app.exe`, and extracted bytes whose SHA-256 matched the ground-truth payload. |
| Filename-only CHM through the shipped console | General scan remained `Binary`; archive mode listed 23 files + 3 folders and extracted 23 files / 81,930 bytes. |
| DOS/16M structural fixture | Listed 4 records and extracted 293,119 bytes; every output matched its exact source slice by SHA-256. The 5,303-byte MF information gap intentionally remains outside the records. |
| MiniDump structural fixture | Listed and extracted 14 stored streams / 11,600 bytes; every output matched its directory extent by SHA-256, including safe suffixing of four duplicate `Unused` names. |
| APK/APKS/IPA/JAR/npm semantic console regression | Direct APK and resource-only APK positives selected APK; classes-only, empty, nested, case-mismatched, CRC-corrupt and encrypted manifests stayed ZIP. Stored/deflated APKS and a resource-only inner APK selected APKS; missing metadata/payload, incomplete ZIPs, unsupported/encrypted outer members, corrupt outer/toc/inner CRCs and encrypted/empty inner identities stayed ZIP. IPA and JAR positives refined correctly, while empty, misplaced, corrupt or encrypted identity records stayed ZIP. Positive APK/APKS/IPA/JAR/npm payloads extracted byte-identically; npm near misses and exact 1 MiB/1 MiB+1 metadata boundaries also passed. |
| ZIP/7-Zip encryption console regression | Generated ZipCrypto, WinZip AES-256, visible-header 7-Zip AES and encrypted-header 7-Zip AES fixtures exercised listing with no/wrong/correct passwords. Missing and wrong-password extraction published no files; correct `--password` and `--password-stdin` extraction was byte-identical, and all source hashes stayed unchanged. |
| DEB/RPM semantic self-tests | DEB 2.0 member order, post-data extensions, preference/factory routing and caller-position preservation passed; wrong versions/order/counts were rejected. Stored and gzip RPM payloads restored exact CPIO bytes, while malformed header bounds, compressor-tag/magic mismatch and a corrupt gzip footer failed with clean state reset. |
| Native nested-filter regression | Plain Brotli, Brotli(TAR), gzip(TAR), Brotli(gzip(TAR)), four gzip layers, Brotli-wrapped npm, and byte-identical extraction passed. Five layers stopped at the documented boundary, and a corrupted inner gzip footer did not expose TAR records. |
| ip7z raw-stream/Split regression | Raw Base64, a two-part byte stream, and a stored ZIP split inside its payload listed/extracted exactly across dotless and two-/three-/four-digit numeric and dotless/two-/three-/four-letter/prefixed alphabetic starts. Incomplete and gapped volume sets failed extraction without output; all source hashes stayed unchanged. |
| mtree semantic console regression | Listed one directory + one file, created a zero-byte placeholder, rejected traversal and a 32,769-byte name, and merged a case-folded update to one record. |
| FAT/Ext2 filename-only ip7z regression | Both images listed and extracted; `bin/ls` (126,584 bytes) and `etc/services` (19,605 bytes) matched across the filesystems by SHA-256. |
| ip7z extraction-preflight regression | Safe absolute/drive/UNC-like and redundant-separator names normalized below the destination. Traversal in both separator styles, case-fold and Unicode-normalization duplicates, file/directory collision, ADS, `CON`, and 32,769-character metadata fixtures all failed before publishing entries; no traversal output escaped. |
| DOS/4G structural fixture | Listed Loader, VMM.EXP, 4GWPRO.EXP and Payload from a 529,046-byte executable; the central archive API's full/name/partial/device/file extraction contract passed. |
| Additional ip7z-only listing evidence | Five Android Ext partition images, a UEFI firmware volume (162 files + 19 folders), two SWFs, one FLV, and additional CHMs listed successfully. A classic-HFS and an Intel HEX fixture failed cleanly and remain unverified. |
| Targeted missing-fixture inventory | A bounded local search found no usable CAB Quantum, WIM LZMS, DMG ADC/LZFSE, HXS or virtual-disk codec fixtures. Those capabilities therefore remain explicitly source-traced or `Wired`, not promoted to fixture-verified. |
| Real IPA, RAR4 SFX and Split ZIP | The 34,910-byte IPA refined to IPA and all six files matched 7-Zip by SHA-256. A 259,725-byte WinRAR SFX extracted its 67-byte stored payload identically. A 178-byte two-volume ZIP extracted its 26-byte payload identically; incomplete-volume extraction failed without output. Every source hash was unchanged. |
| Additional real archive ground truth | A zlib XAR (two outputs), ARJ method 4 (four files), and a RAR5 container using RAR 7.0/v6 compression (seven files) matched 7-Zip by relative path, length and SHA-256. A 135-byte SZDD restored its adjacent 113-byte source exactly, and nested `.tar.lzo` restored `TOOLNAME.txt` exactly. Source hashes remained unchanged; the RAR source timestamp was also unchanged. |
| Static packer breadth audit | One sample from each of 54 UPX version directories listed as UPX. Separate family sweeps exposed the documented ASPack fallback and complete FSG/MEW/NsPack/Petite/Yoda failures rather than silently retaining `Wired` claims. |
| NSIS metadata audit | 20/20 samples detected/listed, 13 with non-zero records; seven non-solid 2.51/3.12 compression/zip2exe cases yielded zero sizes and are now explicitly Partial. |
| Bounded 317-file archive safety smoke | 289 successful listings; 28 clean nonzero results (2 password-required 7z, 25 embedded non-standalone raw-deflate candidates, 1 split-ZIP `.z01`); zero crashes, timeouts, output overruns, source mutations, or corpus changes. |
| Documentation/test-source audit | Every Markdown table had a consistent column count, code fences were balanced, no trailing whitespace was present, all reproduced test paths existed, and all 12 PowerShell test scripts parsed successfully. |

The Qt 6 GUI result above is a build/link check, not an interactive runtime
test. A real GUI open/list/extract/test smoke remains in Planned work.

### Reproduce the main gates

These are the commands for this workstation's current local build and corpora.
For a new checkout, configure/build the runner first as described in
`_mylibs/XStaticUnpacker/tests/installer_corpus/README.md`, then update
`$runner` and the corpus roots.

```powershell
$env:PATH = "C:\Qt\5.15.2\msvc2019_64\bin;" + $env:PATH
$runner = "$env:TEMP\xfu_burn_cmake_build\test_installer_corpus.exe"

& $runner --selftest
& $runner --corpus=F:\tests\installers --expect-count=150 --expect-extractions=161
& $runner --fpscan=F:\ownCloud\binary_examples\packed --expect-count=603

$console = "$env:TEMP\xfileunpacker_pristine_build\src\console\xfileunpackerc.exe"
& .\tests\console_package_type_regression.ps1 -ConsoleExe $console
& .\tests\console_encryption_regression.ps1 -ConsoleExe $console
& .\tests\console_nested_filter_regression.ps1 -ConsoleExe $console
& .\tests\console_ip7z_stream_handlers_regression.ps1 -ConsoleExe $console
& .\tests\console_listing_extract_regression.ps1 -ConsoleExe $console
& .\tests\console_mtree_regression.ps1 -ConsoleExe $console
& .\tests\console_ip7z_safety_regression.ps1 -ConsoleExe $console
& .\tests\console_ip7z_filesystem_regression.ps1 -ConsoleExe $console `
    -FixtureRoot F:\ownCloud\binary_examples\GIT\rizin-testbins\fs
```

The corpus runner intentionally builds with `WITH_XEMULATOR=ON` so it can test
Install Simple; that does not mean the default application ships that class.

## Earlier archive-dispatch changes

Eight defects in the detection/dispatch path were fixed. SquashFS support was
entirely non-functional and ARC could be neither opened nor extracted.

**Dispatch**

1. **`FT_ARC`, `FT_FREEARC`, `FT_SQUASHFS` and `FT_MINIDUMP` were advertised as
   openable but had no arm in `XFormats::createClass()`.** The
   `dynamic_cast<XArchive*>` in `XArchives::getRecords()` therefore failed and
   listing silently returned nothing. All four classes already existed, compiled
   and were `#include`d — only the factory arms were missing.
2. **`FT_DMG` and `FT_UDF` were constructible but absent from
   `XArchives::getArchiveOpenValidFileTypes()`**, so `isArchiveOpenValid()`
   rejected them under the default set.

**Detection**

3. **`XSquashfs::isValid()` was never called.** It was missing from the
   `XFormats::_getFileTypes()` probe chain, which is the detection the CLI
   actually uses. This was the root cause of SquashFS being undetectable.
4. **`FT_SQUASHFS` had no entry in the file-type name table**, so it could not
   surface as a named type even once detected.
5. **`FT_SQUASHFS` was missing from `XFormats::getAvailableFileTypes()`.**
6. **`FT_UDF` and `FT_SQUASHFS` were missing from `g_arrPrefFileTypeOrder`**, so
   `XBinary::_getPrefFileType()` returned `FT_UNKNOWN` for a correctly detected
   UDF image; `FT_SQUASHFS` was also missing from `_getFileTypeListFromSet()`.
7. **The ARC arm in `XBinary::getFileTypes()` was unreachable.** It was guarded
   only by `nSize >= 29` and placed after an ACE arm guarded only by
   `nSize >= 14`, so every file large enough to be ARC was absorbed by the ACE
   arm first. Both arms now carry their magic in the condition. (This is the
   secondary signature chain; the CLI path is item 3.)

**Format structure**

8. **`XSquashfs::isValid()` accepted only the big-endian `sqsh` magic**,
   rejecting every standard little-endian image, and `SQUASHFS_HEADER` was 104
   bytes instead of the on-disk 96 because `s_major`/`s_minor` were declared
   32-bit rather than 16-bit. All field offsets derive from `offsetof`/`sizeof`,
   so correcting the struct corrected them too.

### Verification

| Case | Result |
|---|---|
| 12 real `mksquashfs` images (gzip, gzip@4k/16k/64k/1M blocks, allroot, lz4, lzo, uncompressed, xz, xz@64k, zstd) | All detect as SquashFS, list 5 files + 1 folder, extract 5 files / 19,562 bytes |
| Big-endian SquashFS superblock | Detected (was rejected before) |
| SEA ARC, method 2 (stored), valid CRC-16/ARC | Detects as ARC, lists 1 entry, extracts with byte-exact content |
| Historical 362-file archive corpus | Swept before and after the earlier dispatch fixes; distinct from the bounded 317-file safety smoke above. |
