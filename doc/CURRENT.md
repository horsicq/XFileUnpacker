# XFileUnpacker — Current Format Support

Status of format support in XFileUnpacker **0.1.0**, verified against the
source tree and the available regression corpora on **2026-08-23**. This is a
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
| LHA / LZH | `.lzh`, `.lha`, `.lzs`, `.pma` | Yes | Yes | Partial | Store/lh0, lh1, lh4, lh5, lh6, lh7 | Native extraction handles Store/lh0, lh1 and lh4-lh7; ip7z handles Store/lh4-lh7. Recognized lh2/lh3/lzs/pma methods may list but do not extract, and now fail closed on every route instead of publishing compressed bytes. Local fixtures cover lh0/lh5/lh6/lh7 and one 47-entry `-pm2-` archive; there is no local lh2/lh3/lzs/pm0 sample. |
| ZOO | `.zoo` | Yes | Yes | Yes | Store, method 1 (LZD), method 2 (LZH) | Native method 1 uses Zoo's continuous LSB-first 9-to-13-bit LZD stream with mandatory clear/end codes, strict padding and full-consumption checks. Method 2 uses the format's 8 KiB LZH dictionary and explicit zero-block terminator. Real Zoo 2.10 fixtures extracted the same 5,040-byte payload exactly through native, shared and legacy routes; one-byte streaming, full-table reset, malformed framing, exact size, and CRC-16 failures are covered. ip7z has no ZOO handler. The linked-list scan is bounded by a roughly 100,001-entry guard. |
| SAR | `.sar` | Yes | Yes | Yes | Store/` LH0 `, ` LH5 ` | Streamline Design's SAR is LHA with a differently spelled method tag, so `XSAR` inherits the LHA container reader. A byte comparison against an LHA 2.13 archive of the same payload showed the two differ in ten bytes — the tag, the header checksum that follows from it, the OS identifier and a zeroed timestamp — while the header fields, CRC-16, the whole compressed stream and the terminator were identical, and the stream decoded to the same SHA-256 as an independent copy of the payload. Detection is stricter than LHA's because SAR carries no magic number and `.sar` is a heavily overloaded extension: a supported tag, header level 0 or 1, and a verified header checksum are all required. Only the stored and ` LH5 ` tags have been observed; any other tag and header level 2 fail closed. |
| ARX | `.arx` | Yes | Yes | Yes | `-lh1-` (LZHUF: adaptive Huffman + 4 KiB LZSS) | ARX is LHA with one byte inserted at offset 7, so every field from there on is shifted: the compressed size, the sizes, the timestamp, the name, and a checksum truncated to the CRC-16's low byte. The tag is a genuine `-lh1-` and the LHA header checksum still verifies, so the tag alone cannot separate the two — the shift is what does, because an ARX header read as LHA yields a compressed size far larger than the file. `XLHA::isValid` now requires the first record to fit, which is the same test its member walk applies, so ARX no longer reports as LHA and then refuses to open. Both fixtures extract `TEST.TXT` at 5,040 bytes with SHA-256 `583279d2...441e37` — the same payload the SAR, ZOO and ACE fixtures produce, so four unrelated readers agree on it. No `FPART_PROP_RESULTCRC` is published: only the CRC's low byte is stored and there is no record-model type for a truncated checksum. |
| ACE | `.ace` | Yes | Yes | Partial | Stored, ACE method 1 (LZ77+Huffman) | Solidity is judged per member rather than per archive. ACE marks every member of a solid archive as solid, the first one included, so treating the archive flag as decisive refused even a single-entry stored archive and left the method-1 decoder unreachable from every available sample. A member is now refused only when it genuinely continues an earlier member's decoder state, which for a continuation volume includes its first member. Encrypted entries, later members of a solid archive, and ACE 2.0 `TECH.TYPE 2` remain blocked. |
| ARC (SEA) | `.arc` | Yes | Yes | Partial | Stored (1–2), Packed/RLE (3), Squeezed/Huffman (4), Crunched dynamic LZW (8), Squashed (9) | Every method except the obsolete 5/6/7 extracts. 337 entries across the 18 real ARC archives available here restore with the exact declared size and a matching CRC-16/ARC, recomputed independently of the application. Methods 5–7 are ARC's original hash-table crunch — a different decompressor, not the LZW below with a fixed width — and no surviving archive using them was found on disk or in a 212-item sweep of the Internet Archive's DOS shareware collection, so they map to `HANDLE_METHOD_UNKNOWN` and are refused on every route rather than published as their compressed bytes. |
| FreeARC | `.arc` | Yes | No | No | — | `XFREEARC::getFileParts` emits raw blocks only; no per-file records |
| CAB | `.cab` | Yes | Yes | Yes | Store, MSZIP, LZX, Quantum | The filename route's compiled ip7z handler includes Quantum; the native `XCab` path remains Partial for Quantum. MSZIP/LZX fixtures are covered; add a Quantum fixture. |
| WIM | `.wim`, `.swm`, `.esd`, `.ppkg` | Yes | Yes | Yes | Store, LZX, XPRESS-Huffman, LZMS | The filename route's compiled ip7z handler includes LZMS; the native `XWIM` path remains Partial for LZMS. Store/LZX/XPRESS fixtures are covered; add an LZMS fixture. |
| TAR | `.tar`, `.ova` | Yes | Yes | Yes | Store | The native/device parser applies and hides POSIX PAX `x`/`g` and GNU `L`/`K` metadata. It handles local/global precedence and deletion, long paths/links, size/owner/time fields, and rejects malformed, oversized, or dangling metadata. The shared extraction guard still confines metadata-supplied paths below the destination. |
| CPIO | `.cpio` | Yes | Yes | Yes | Store | Includes AFIO variants |
| ar | `.a`, `.udeb`, `.lib` | Yes | Yes | Yes | Store | |
| XAR | `.xar`, `.pkg`, `.xip` | Yes | Yes | Yes | none, zlib, bzip2, xz, lzma | macOS flat packages are XAR. A real zlib XAR exposed `hello` plus `[TOC].xml`; both outputs matched 7-Zip exactly. |
| ASAR | `.asar` | Yes | Yes | Yes | Store | Electron bundles. Inline data, named-`QFile` `.asar.unpacked` sidecars, and file/directory `link` nodes are extracted; links are safely materialized. Sidecars are size/hash checked at initialization and extraction, resolved below a pinned sibling root, and authenticated by their opened handle. Anonymous/exotic devices with external records fail closed. |
| Quake PAK | `.pak` | Yes | Yes | Yes | Store | Native reader for the id Software `PACK` layout: 56-byte path plus little-endian offset/size directory records. Other unrelated formats that also use `.pak` are not implied. |
| Doom WAD | `.wad` | Yes | Yes | Yes | Store | Native reader for `IWAD` and `PWAD` lump directories. Duplicate lump names receive deterministic safe suffixes so extraction does not overwrite an earlier lump. |
| Build GRP | `.grp` | Yes | Yes | Yes | Store | Native reader for the `KenSilverman` group-file layout used by Build-engine games. |
| WARC | `.warc` | Yes | Yes | Yes | — | Native reader, preferred over ip7z. WARC 1.0 and 1.1 are accepted with their version-specific UTC date grammars; 1.1 field names, type values, folding/LWS, reduced precision, and 1–9 fractional digits are handled. Mandatory fields, strict record-ID URIs, duplicate IDs, bounds, and unsafe paths fail closed. Qualifying `resource` and `response` records are emitted; a `response` preserves the complete archived HTTP message rather than silently discarding its status line/headers. Other standard record types are validated and skipped. |
| mtree | `.mtree` | Yes | Yes | Partial | — | Metadata manifest: extraction creates described directories and zero-byte placeholders, but mtree carries no payload bytes. A synthetic semantic regression verified listing, placeholder extraction, traversal/oversized-name rejection, and deterministic case-fold merging. Native reader preferred. |
| UU / begin-base64 | `.uu`, `.uue` | Yes | Yes | Yes | — | One transport block is decoded. An arbitrary payload, including an empty file, is exposed under the declared name; a recognized nested archive that opens successfully instead exposes its members. Preamble, line, output-size, and nesting limits are listed below. |
| SquashFS | `.squashfs`, `.sfs`, `.sqsh`, `.snap` | Yes | Yes | Yes | gzip, lzma, lzo, xz, lz4, zstd, uncompressed | Both byte orders (`hsqs`/`sqsh`). Native `XSquashfs` is a superblock/structure reader; records and extraction come from ip7z's `SquashfsHandler`. Twelve generated filesystem fixtures passed. |
| CFBF / OLE compound file | `.doc`, `.xls`, `.ppt`, `.aaf`, `.ole`, `.msp`, `.msm`, generic `.msi`-like containers | Yes | Yes | Yes | Store | Generic CFBF mode exposes raw compound streams. Real MSI/WiX databases take the semantic static-unpacker route described below. |
| MiniDump | `.dmp` | Yes | Yes | Yes | Store | Structural extraction: valid minidump streams are emitted as entries; this does not reconstruct files from dumped process memory. |
| Mach-O FAT | `.fat`, `.mub` | Yes | Yes | Yes | Store | Classic FAT32 and FAT64 headers are supported in both byte orders. Up to 1,000,000 structurally bounded slices may be exposed; table/range/overflow/alignment/reserved-field failures are rejected. A 21-slice FAT32 case and big-/little-endian FAT64 extraction are regression-tested. |
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
| LZO | `.lzo` | Yes | Yes | Yes | Both lzop compressors: method 2 (LZO1X-1, `-1`..`-6`/`--fast`) and method 3 (LZO1X-999, `-7`..`-9`/`--best`). They share one bitstream, but only method 3 emits the M1 short-match opcode, which was rejected — so every `lzop -7` and above archive failed to open while the common levels worked. A real `.tar.lzo` reached its nested TAR and extracted `TOOLNAME.txt` byte-identically to the standalone ground-truth payload. `--filter` delta filters remain unsupported and fail closed. |
| SZDD | `.szdd` | Yes | Yes | Yes | MS-DOS `compress.exe`; LZSS decoded inside `XSZDD`. A real 135-byte stream restored the adjacent 113-byte ground-truth file exactly. |
| KWAJ | `.kwaj` | Yes | Yes | Yes | Store, XOR, QBasic-LZSS method 2, LZH method 3, and CK-framed MSZIP method 4. Method 2 uses the format's 4 KiB space-filled history with the QBasic `4096-18` starting position; method 3 uses KWAJ's five canonical Huffman trees. Method 4 enforces each declared CK/raw-DEFLATE frame boundary, the 32 KiB frame ceiling, shared cross-frame history, and explicit-zero or clean-boundary-EOF termination. An authentic two-frame Word 6.0a `DIALOG.FO_` restores 48,624 bytes byte-identically to independent libmspack and raw-zlib oracles. Compressed records without a declared output length use disk-backed private staging before native publication. |

### Dynamic nested filters

The native filter adapter can unwrap gzip, bzip2, XZ, lzip, LZMA, LZO,
compress, Brotli, Zstandard, LZ4, LZ5, Lizard, and RPM layers, then open a
recognized archive inside. This is why a `.tar.br` can list TAR members even
though there is no `FT_TAR_BROTLI`, why a Zstandard-wrapped CPIO can expose
CPIO entries while retaining an outer `Zstandard` label, and why `.warc.gz`
can reach the native WARC reader. Nested WARC listing and byte extraction are
covered by the same WARC 1.0/1.1 regression.

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
| tar + gzip | `.tar.gz`, `.tgz`, `.tpz` | Yes | Yes | Yes |
| tar + bzip2 | `.tar.bz2`, `.tbz2`, `.tbz`, `.tb2`, `.tz2` | Yes | Yes | Yes |
| tar + xz | `.tar.xz`, `.txz` | Yes | Yes | Yes |
| tar + lzma | `.tar.lzma`, `.tlz` | Yes | Yes | Yes |
| tar + lzip | `.tar.lz` | Yes | Yes | Yes |
| tar + lzop | `.tar.lzo` | Yes | Yes | Yes |
| tar + compress | `.tar.Z`, `.taz` | Yes | Yes | Yes |
| tar + zstd | `.tar.zst`, `.tzst`, `.tzstd` | Outer `Zstandard` | Yes | Yes |
| tar + lz4 | `.tar.lz4`, `.tlz4` | Yes | Yes | Yes |

`tar + zstd` is operational through nested filtering, but the primary detector
currently returns `FT_ZSTD` rather than refining to `FT_TAR_ZSTD`; its Detect
cell is therefore intentionally not `Yes` under this document's definition.
Typed wrappers inherit the native parser's PAX/GNU metadata handling and its
malformed/dangling-header checks. `.taz` consistently denotes tar + Unix
`compress`; content magic still wins when a gzip-wrapped TAR is given that
suffix.

## Disk images

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| ISO 9660 / CD data track | `.iso`, `.img`, `.bin`, `.raw`, `.cue` | Yes | Yes | Yes | Stored ISO/Joliet content. CUE sheets map `MODE1/2048`, `MODE1/2352`, `MODE2/2336`, and Form-1 `MODE2/2352` data tracks; direct 2048/2336/2352/2448-byte sector images are recognized structurally. Audio and Mode-2 Form-2 sectors are not exposed as archive files. |
| UDF | `.udf`, `.iso`, `.img` | Yes | Yes | Yes | Stored content; detection, not the shared alias, decides the format. |
| Apple Disk Image | `.dmg` | Yes | Yes | Yes | The filename route's ip7z handler includes Store, zlib, bzip2, ADC and LZFSE. Native `XDMG` remains Partial because it does not decode ADC/LZFSE; targeted fixtures for those two codecs are still needed. |
| Raw disk / virtual-disk image | `.img`, `.simg`, `.lpimg`, `.vdi`, `.vhd`, `.vhdx`, `.avhdx`, `.vmdk`, `.qcow`, `.qcow2`, `.qcow2c` | Generic | Wired | Wired | No generic native image class. Several filesystems/containers are reachable only through the compiled filename-based handlers listed below. |

CD-image evidence covers all 54 image sets under the local 1998 corpus: 53 CUE
sheets (51 BIN and 2 RAW companions, 220 tracks) plus the standalone
`CMRALLY.iso`. The CUE tracks comprise 47 `MODE1/2352`, six `MODE2/2352`, one
`MODE1/2048`, and 166 audio tracks. Their native listing counts match an
independent sector/ISO parser, including mixed data/audio discs, legacy rooted
`FILE` tokens, a wrong historical filename, a data track that extends through
the next track's 150-sector pregap, and Carmageddon II's poisoned primary
descriptor with a valid Joliet supplementary descriptor.
Copy-protection directory entries with impossible extents are retained as
annotated zero-byte placeholders, so they remain visible without discarding
or blocking extraction of valid siblings.
Semantic regressions exercise direct 2048/2336/2352/2448-sector and CUE
`MODE1/2048`, `MODE1/2352`, `MODE2/2336`, and `MODE2/2352` routing,
cross-sector byte extraction, a recoverable duplicated-XA-subheader mismatch,
Unicode Joliet names, bounded sibling resolution, and hostile
external-path/Form-2 rejection.

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
| Split volume / transport streams | Split (zero-padded numeric starts ending `0`/`1`, or alphabetic starts ending `aa`), raw Base64 (`.b64`), MsLZ (`.mslz`), LZMA86 (`.lzma86`), PPMd (`.pmd`) | Usually generic or extension-led | Partial | No native class | Raw Base64 plus dotless and two-/three-/four-digit numeric and dotless/two-/three-/four-letter/prefixed alphabetic Split starts have semantic regressions; MsLZ/LZMA86/PPMd are only source-traced. Split is sibling-dependent and its full start-name grammar is preferred before a truncated root signature. Base64 is extension-only, distinct from UU/`begin-base64`, and its fully buffered encoded input is capped at 256 MiB before the handler opens. |
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
| Inno Setup | `NE / PE32/PE64: Inno Setup` | Yes | **Yes** | **Yes** | Deterministic layouts cover both native-width 1.2.10 IDs and every standard JR Software setup-data schema from 1.3.3 through 4.2.6, plus modelled 5.x through 7.0 schemas, including 6.5/6.6. Historical loaders S02/S04/S05/S06/S07, Adler-32/CRC-32/MD5 location records, old/new block framing, and the four lagged-ID schema pairs are handled. The Win16 path supports generic NE routing and `idska16`/`base.N` slices; the Win32 twin uses `idska32`. Destination directories, including `{const}` paths, are surfaced as the advanced **Path** column. Encryption supports ARC4-MD5, ARC4-SHA1, and PBKDF2-XChaCha20; embedded and sibling multi-slice data are handled. ANSI metadata uses Windows language/code-page inference and defaults to Windows-1252 on other hosts; `--codepage` overrides either path, while `--password-hex` supplies exact legacy password bytes. ISX/custom IDs and unarchived formats older than `i1.2.10` fail closed. |
| NSIS | `PE32/PE64: NSIS` | Yes | **Yes** | **Yes** | LZMA/zlib/bzip2, solid + non-solid are handled, and `SetOutPath` is surfaced as the advanced **Path** column. All 20 available installers list and extract. The seven historical non-solid NSIS 2.51/3.12 bzip2/LZMA/zlib and zip2exe-classic failures now preserve honest unknown-size metadata and match 7-Zip byte-for-byte. Nineteen corpus cases have identical file sets; the MUI2 case additionally reconstructs its source-declared uninstaller while its five shared files match 7-Zip. |
| WiX Burn v3/v4 | `PE32/PE64: WiX Burn bundle` | Yes | **Yes** | **Yes** | PE `.wixburn` bootstrapper with CAB UX/attached containers. The pinned v3 fixture extracts byte-identically; bounded multiple-container parsing, v4 namespaces, SHA-512 validation and attached-payload mapping are implemented. Detached/external payloads fail closed without an explicit source resolver; the generated-v4 regression is optional where WiX 4+ tooling is installed. |
| 7-Zip SFX | `PE32/PE64: 7-Zip SFX` | Yes | **Yes** | **Yes** | 20 fixtures; 7z payload with MD5/name contracts. |
| WinRAR SFX | `PE32/PE64: WinRAR SFX` | Yes | **Yes** | **Yes** | 24 fixtures; RAR payload with MD5/name contracts. A separate 259,725-byte real RAR4 overlay listed/extracted `README.txt` (67 bytes, CRC `A6DCD3A2`) byte-identically to 7-Zip. |
| Generic SFX | `PE32/PE64: SFX` | Yes | Wired | Wired | A central synthetic PE+7z-overlay regression extracts the exact payload and checks output limits; real CLI/GUI byte-parity fixtures remain open. |
| IExpress | `PE32/PE64: IExpress` | Yes | **Yes** | **Yes** | 12 fixtures; embedded CAB with Store/MSZIP/LZX and MD5/name contracts. |
| InstallForge | `PE32/PE64: InstallForge` | Yes | **Yes** | **Yes** | 11 fixtures; 7z, bzip2, and gzip payloads. |
| CreateInstall | `PE32/PE64: CreateInstall` | Yes | **Yes** | **Yes** | 9 fixtures; Store, LZGE and Gentee PPMd-I, including solid streams and sibling volumes. |
| Actual Installer | `PE32/PE64: Actual Installer` | Yes | **Yes** | **Yes** | 8 fixtures; ZIP payloads with strict MD5 verification. |
| Advanced Installer | `PE32/PE64: Advanced Installer` | Yes | **Yes** | **Yes** | 8 direct-MSI and EXE-bootstrapper fixtures; embedded/external MSI/CAB payloads. |
| Smart Install Maker | `PE32/PE64: SmartInstall` | Yes | **Yes** | **Yes** | 5 fixtures; Store, deflate and LZX, filenames and split volumes. |
| Clickteam Install | `PE32/PE64: Clickteam` | Yes | **Yes** | **Yes** | 7 fixtures; proprietary Store/zlib container, names and volumes. |
| Tarma (InstallMate) | `PE32/PE64: Tarma` | Yes | **Yes** | **Yes** | 6 fixtures; `tiz2z`/`tiz3z`, metadata paths, volumes and loose-file graph. |
| AutoIt | `PE32/PE64: AutoIt` | Yes | **Yes** | **Yes** | AutoIt v2 and v3/EA06 are handled. An official v2.64 compiler plus Exe2Aut oracle verifies LCG-encrypted record parsing, JB01 stored/compressed data, exact script and `FileInstall` payloads, routing, quotas, and malformed rejection. An official AutoIt 3.3.18.0 EA06 fixture independently verifies two records and an exact `FileInstall` payload. |
| Enigma Virtual Box | `PE32/PE64: Enigma Virtual Box` | Yes | **Yes** | **Yes** | 10 fixtures; VFS Store/aPLib and installed filenames. |
| BoxedApp | `PE32/PE64: BoxedApp` | Yes | **Yes** | **Yes** | 10 fixtures; VFS Store/zlib and installed filenames. |
| Install Simple | `PE32/PE64: Install Simple` | Yes | **Yes** | **Yes** | 14 fixtures pass the bounded x86-emulated range decoder. CMake and qmake application builds enable the in-tree emulator by default; opt out with `-DWITH_XEMULATOR=OFF` or qmake `XCONFIG+=no_xemulator`. |
| MSI | `MSI` (CFBF) | Yes | **Yes** | **Yes** | Parses StringPool/Columns/File/Media plus Component/Directory, resolves installed names, and extracts embedded/sibling CABs or loose files. Patches, transforms, and databases without File/Media intentionally fall back to generic CFBF streams. |
| WiX (MSI) | `MSI: WiX` (CFBF) | Yes | **Yes** | **Yes** | WiX marker plus the semantic `XMSI` route. Six WiX fixtures and overlapping Advanced Installer forms passed; a CLI fixture extracted `app.exe` with the expected SHA-256. Distinct from the PE Burn-v3 row. |

### Packers / protectors (single-file → restored executable)

For this table, `Not working in corpus` means a production class is reachable,
but every available family sample either failed or fell back to raw PE-section
records; it is more specific than the source-only `Wired` label.

| Format | Type shown | Detect | Unpack | Notes |
|---|---|---|---|---|
| UPX | `PE32/PE64: UPX`; generic `UPX` for non-PE | Yes | Partial | One sample from each of 54 local PE UPX version directories routes and lists as UPX, but that breadth sweep still lacks restored-image comparison. Generic `FT_UPX` is now wired through detection, preference, factory, CLI, and GUI archive paths for ELF, Mach-O executables, DOS COM, and DOS SYS. Two ELF, two Mach-O, COM, and SYS fixtures restore byte-identically; malformed headers, corrupt gap streams, truncated markers, and false-positive `UPX!` overlays fail closed. Unsupported Mach-O dylibs are intentionally not admitted. |
| ASPack | `PE32: ASPack` | Yes | Partial | PE32 only. Stub generations 2.00, 2.001/2.1, 2.12 and 2.2 are handled: the production CLI reconstructs 56/128 current corpus executables, and every one of those 56 is a valid restored PE carrying the original OEP and 64 entry-point bytes; the pinned 2.12 fixture additionally has its whole code section byte-identical to the pre-pack input. The 1.0x stubs (40 samples) use a different pre-2.0 compressor and fall back to raw PE-section records; the 2.11 family (32 samples) shares the 2.x decoder but hides the entry-point marker and the stored OEP, so it stays unhandled. Each generation is selected by an exact entry signature plus the marker and the constant decoder table at that generation's offsets. |
| FSG | `PE32: FSG` | Yes | Partial | PE32 only. Stub generations 1.0/1.3, 1.31, 1.33 and 2.0 are handled; the production CLI reconstructs 9/12 current corpus executables, all valid restored PEs with the original OEP and 64 entry-point bytes; the pinned 1.3.3 fixture additionally has its whole code section byte-identical to the pre-pack input. Pre-1.33 stubs use a 16-bit support-record list and entry-point immediates instead of the 1.33 32-bit table, and the original entry point is located through its `FE 0F`/`0F 84` anchor rather than a fixed constant. FSG 1.1/1.2 (3 samples) hides the loader behind a polymorphic byte-decryption loop and is deliberately not detected. |
| MEW | `PE32: MEW` | Yes | **Yes** | PE32 only. All 16/16 current corpus executables reconstruct as valid restored PEs with the original OEP and 64 entry-point bytes, across MEW 10, MEW 11 and MEW 11 SE 1.2, aPLib and LZMA blocks, and the single-block "special" LZMA form; the pinned fixture additionally has its whole code section byte-identical to the pre-pack input. Imports are not re-fixed, so the restored image is an analysis dump and does not execute. MEW 5 has no runnable corpus sample. |
| NsPack | `PE32: NsPack` | Yes | **Yes** | PE32 only. All 34 current corpus executables route and list a reconstructed record through the production CLI. One 3.7 output is a valid restored PE with the original OEP, 64 entry-point bytes and a byte-identical code section. Imports are not re-fixed, so the restored image is an analysis dump and does not execute. |
| Petite | `PE32: Petite` | Yes | **Yes** | PE32 only. Both current corpus executables route and list a reconstructed record through the production CLI. One output is a valid restored PE with the original OEP, 64 entry-point bytes and a byte-identical code section. Imports are not re-fixed, so the restored image is an analysis dump and does not execute. |
| Yoda's Protector | `PE32: Yoda's Protector` | Yes | **Yes** | PE32 only. The official Cisco-Talos ClamAV yC 1.3 fixture restores its 3,072-byte embedded UPX image byte-identically from the 6,226-byte packed input; pinned-oracle, PE-fixup, quota, fail-closed and family-transition checks pass. |

## Planned / to add later

| Item | Kind | Notes |
|---|---|---|
| Repair runtime-failing static routes | Correctness | MEW is complete (16/16). ASPack is at 56/128 and FSG at 9/12; what remains is the ASPack 1.0x compressor, the ASPack 2.11 family's hidden marker/OEP, and the FSG 1.1/1.2 polymorphic stub decryptor. Preserve family-specific negative checks. Tracker #12 |
| Byte-verify remaining source-wired routes | Coverage | Add restored-image comparisons for the 54-directory PE UPX breadth sweep and noncanonical successful packer variants, plus real CLI/GUI byte parity for generic SFX, AutoIt v2/v3, and executable packers. Non-PE UPX and the seven repaired NSIS cases already have CLI byte checks. Tracker #12 |
| GUI runtime end-to-end smoke | Coverage | Current Qt 6 GUI evidence is compile/link/no-op rebuild only. Add automated open → list → extract → test verification with model-field checks. |
| Real-world package breadth | Coverage | Add bundletool-generated universal/split/asset-slice APK Sets, modern signed IPAs, more XAPK producers, and registry-produced scoped npm packages. Synthetic semantic/near-miss coverage and one legacy real IPA now exist. |
| Remaining ip7z-only handler fixtures | Coverage | Add HXS/additional CHM variants, Android LP/Sparse, APM/CramFS/MBR, VDI/VHD/VHDX/VMDK/QCOW, APFS/HFS/NTFS, MsLZ/LZMA86/PPMd, Intel HEX and TE fixtures; verify names, hashes and sparse handling. FAT, Ext, UEFI, SWF, FLV, raw Base64 and numeric/alphabetic Split now have runtime evidence. |
| Advanced native-vs-ip7z codecs | Coverage | Add CAB Quantum, WIM LZMS, and DMG ADC/LZFSE fixtures to byte-verify the compiled filename/ip7z handlers and the documented native-device failure boundary. |
| Backend fallback and remaining hostile-path matrix | Safety | Unsupported → native fallback and terminal password/CRC/corruption behavior still need end-to-end fixtures. Path preflight now covers traversal in both separator styles, normalized absolute/drive/UNC-like paths, case/Unicode duplicates, file/directory collisions, ADS, ASCII and superscript-digit reserved device names, oversized metadata, empty destinations, and source immutability; link/hardlink/reparse and hostile concurrent destination-replacement cases remain. |
| Native legacy archive codecs | Capability | Wire the remaining LHA lh2/lh3/lzs/pma methods with byte-checked fixtures. |
| Transport edge cases | Coverage | Add malformed/trailing raw-Base64 cases plus long Split carry/rollover and large volume-count cases. The Base64 input ceiling, `.taz` mapping, Split start styles, and missing-middle behavior are covered. |
| Native-device state contracts | Coverage | Add direct C++ `QBuffer` tests proving semantic APK/JAR/APKS/IPA/npm probes and nested-filter failures preserve caller position and do not leak temporary state. |
| WiX Burn detached/external payloads | Enhancement | Provide an explicit acquisition/source resolver for detached containers and external payloads, retain fail-closed behavior without a supplied source, and keep generated-v4 runtime coverage available. Tracker #9 |
| ALZ, EGG, StuffIt, PEA | New formats | no detector or reader today, and no local fixture for any of the four (see Not supported) |

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
| ARC4-MD5 / ARC4-SHA1 / PBKDF2-XChaCha20 | Inno Setup | Yes | Depends on whether setup headers are encrypted |
| ACE encryption | ACE | No | Yes — explicitly blocked for extraction |

The CLI accepts exactly one password form: normal text, one UTF-8 line from
standard input, or exact legacy bytes written as hexadecimal:

```bash
xfileunpackerc --password 123456 --extractarchive out archive.7z
echo 123456 | xfileunpackerc --password-stdin --extractarchive out archive.7z
xfileunpackerc --password-hex efe0f0eeebfc --codepage 1251 --extractarchive out legacy-inno.exe
```

`--password` is the convenient form; `--password-stdin` avoids the password
appearing in the process list or shell history. `--password-hex` bypasses text
encoding and supplies the exact bytes expected by legacy non-Unicode Inno Setup.
`--codepage NUMBER` selects the Windows code page used for legacy filenames and
for converting a text password; with `--password-hex`, it still controls legacy
filename decoding. Without an override, a legacy encrypted setup accepts a text
password only when its stored salted check hash identifies one unique encoding
from a bounded Windows-ACP set. Filename decoding still requires inference or
the explicit override because the compiler machine's active code page is not
identified by the installer. The same options apply to `--listarchive`.

The deterministic encryption regression generates ZipCrypto, WinZip AES-256,
7-Zip AES with visible headers, and 7-Zip AES with encrypted headers. ZIP and
visible-header 7-Zip listings remain available without a password; encrypted
7-Zip headers require the correct password. Extraction with a missing or wrong
password fails without publishing files, while both `--password` and
`--password-stdin` restore byte-identical payloads with the correct value. The
test also verifies that every source archive remains unchanged.

The Inno regression covers official 5.3.8 ARC4-MD5, 6.3.3 ARC4-SHA1, and
6.4.3 PBKDF2-XChaCha20 installers. The 5.3.8 case includes a CP1251 filename
and verifies both strict text-password conversion and exact raw bytes. The
regression also exercises encrypted sibling slices; the external slice files
are validated before extraction and remain unchanged.

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
1.5/2.0/2.9/5.0/7.0, LHA lh1/lh4/lh5/lh6/lh7, ZOO method 1/LZD and
method 2/LZH, ACE method 1, SEA ARC packed/squeezed/crunched-dynamic/squashed,
ARJ methods and `fastest`, KWAJ XOR/QBasic-LZSS/LZH/MSZIP, and IT214/IT215 8/16-bit transforms. It also dispatches
ZIP AES/ZipCrypto, 7-Zip AES and RAR5 AES layers; PDF LZW, ASCII85,
ASCIIHex, RunLength and image transforms; CAB Store/MSZIP/LZX-CAB framing;
and the Delta/BCJ/BCJ2/ARM/ARM64/ARMT/PPC/SPARC/IA64 branch filters.

These enum values are **not** generic-chain methods:

`LZSS_SZDD`, `LZX`, `XPRESS`,
`XPRESS_HUFF`, `ANDROID_XML`, `ACE_DELTA`, `ARCHIVE_STREAM`, `FILE`.

- `LZSS_SZDD` is dispatched by the archive layer and used by `XSZDD`;
  `LZX` is framed by the CAB/WIM readers; and `XPRESS_HUFF` is owned by the WIM
  reader.
- `KWAJ_LZSS`, `KWAJ_LZH`, and `KWAJ_MSZIP` have dedicated bounded decoder dispatches. Plain
  `XPRESS` currently has no reachable decoder dispatch; recognizing or listing
  that method ID must not be read as extraction support.
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

The supported `.pak` type is specifically the Quake `PACK` format. Other game
and application archive layouts that reuse the `.pak` extension remain
unsupported unless they match another explicitly documented format.

Partially blocked or route-limited, listed above for detail: ACE encrypted
entries, later members of a solid archive and `TECH.TYPE 2`; FreeARC file
records, LHA lh2/lh3/lzs/pma methods, SEA ARC methods 5-7,
native CAB Quantum, native WIM LZMS, and native DMG ADC/LZFSE.

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
decoded name at 32,768 bytes. TAR scans at most 100,000 physical records, caps
one PAX/GNU metadata payload at 1 MiB, and caps recognized metadata text at
64 KiB. ASAR caps JSON recursion at 256, items at one million, declared header
JSON at 16 MiB, and link traversal at 40 hops. Raw ip7z Base64 rejects an
encoded input above 256 MiB before its fully buffering handler opens.

Before ip7z extraction, the complete entry set is preflighted. `..` traversal
with either separator style, unsupported links/hardlinks, alternate streams,
anti/deleted/reparse entries, Windows reserved device names (including the
superscript aliases documented in the [Win32 naming rules](https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file)), case-folded or
Unicode-normalized duplicates, and file/directory path collisions reject the
archive. Drive, UNC-like and leading-root prefixes are normalized to a path
below the destination; `.` and duplicate separators are collapsed.
Filename/ip7z extraction first decodes into a private staging directory beside
the destination. Publication for filename/ip7z, native/static, legacy archive,
and XFormats routes is a whole-archive transaction: originals are moved into an
owner-private, same-filesystem `.xunpack-rollback-*` journal, while newly
published files and directories are recorded. A later decoder, CRC,
cancellation, path, enumeration, disk/publication, or finalization failure
restores the prior child state; `QSaveFile` still makes each file replacement
atomic. An incomplete rollback retains the journal and reports its recovery
path. If the API created the requested destination root, that root itself may
remain as an empty directory. This is single-writer process-level rollback, not
crash recovery or isolation from hostile concurrent destination mutation.

For filename/ip7z and property-aware native/static extract-all, a caller-supplied
`UNPACK_PROP_MAX_OUTPUT_SIZE` now applies to every staged member: a trustworthy
declared size is rejected before decoding, and the actual write is bounded as
well, so dishonest/unknown sizes cannot cross the limit or merge partial output.
This is an opt-in per-member budget, not a universal policy. There is still no
default decompressed-byte limit, aggregate decompressed-output budget, or
equivalent coverage across legacy/list-record and every direct
single-record/arbitrary-device API.
Callers still need destination free-space policy and may cancel via `PDSTRUCT`.

Two routes that accepted the limit but did not apply it were repaired. The ARJ
decoder writes through its own `arjWriteAll`, which reimplements
`XBinary::_writeDevice`'s window arithmetic but omitted its output-limit gate, so
an ARJ member ignored a correctly supplied limit; an exact-boundary regression
now covers acceptance at the limit and refusal one byte over. The compiled ip7z
`GetStream` wrapped its runtime bound around the staging file only, so a
caller-supplied output device was covered by nothing but the declared-size
preflight — which is skipped whenever the handler reports no trustworthy size,
that is, precisely the single-stream gzip/xz/bzip2 handlers. The bound now wraps
whichever device was selected.

A compression-ratio ceiling is deliberately not offered. DEFLATE's structural
maximum is roughly 1032:1 and a benign local fixture reaches 1023.5:1
(`PCem-ROMs-master.zip`, a 2,097,152-byte member from 2,049 bytes), so a deflate
ratio ceiling is either inert or refuses real content. Absolute byte budgets are
the workable control.

## Whole-archive rollback changes (2026-08-23)

- A shared destination rollback journal now covers filename/ip7z, native/static,
  legacy archive, list-record, and XFormats publication routes. It saves
  overwritten targets before publication and removes archive-created children
  on a later failure.
- Deterministic regressions inject decoder, premature-enumeration, cancellation,
  finalization, unsafe-path, CRC, and simulated disk/publication failures after
  an earlier member succeeds. They verify restored sentinel content, removal of
  new child directories, successful commit behavior, and no rollback-journal
  residue after success or complete rollback.
- Cleanup or incomplete-rollback diagnostics preserve the recovery path. The
  transaction intentionally does not claim crash recovery or protection from a
  concurrent hostile writer.

## Archive/listing/static-unpacker changes (through 2026-08-23)

- Archive methods retain both the numeric handler method and its reported text,
  so the console and GUI no longer lose the Method column during conversion.
- The SEA ARC and LHA file-part records now carry their resolved handler method.
  Previously they left it unset on that route, and the shared decompressor's
  missing-method default is `Store`, so an ARC method 3-9 entry — and any LHA
  entry at all — could be published as its own compressed bytes without an
  error. Unsupported methods now fail closed. `XLHA::isValid()` also enforces
  its method whitelist again instead of accepting any `-l??-`/`-pm?-` tag.
- ASPack, FSG and MEW recognize their pre-2.12 / pre-1.33 / MEW 10 stub
  generations; see the packer table for the per-family corpus counts.
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
- Windows entry-path preflight now rejects the documented superscript-digit
  device aliases as well as ASCII `COM1`–`COM9`/`LPT1`–`LPT9`, closing the
  `COM¹`/`LPT²` namespace bypass before staging begins.
- Solid packed-size totals are block-aware; a shared block contributes once and
  can produce a useful overall ratio.
- Host OS, fuller attributes, per-file ratio, and available checksum metadata
  are exposed by the archive record model.
- WARC now implements the WARC 1.0/1.1 version boundary, version-specific
  W3CDTF timestamps, case-insensitive/folded fields, mandatory strict record
  IDs with bounded duplicate detection, safe URI mapping, and lossless
  `resource`/full-`response` streaming through plain and gzip-wrapped archives.
- Universal Mach-O parsing now accepts FAT32/FAT64 in either byte order without
  the former 20-slice ceiling and rejects malformed tables/ranges/alignment.
- NSIS non-solid block lookup was repaired for the seven historical 2.51/3.12
  failures. AutoIt now handles official v2 LCG/JB01 records as well as v3 EA06.
- Generic UPX routing and in-process rebuilding now cover ELF, Mach-O
  executables, DOS COM, and DOS SYS. Header truncation, reconstruction-size,
  corrupt-gap, and false-positive marker paths fail closed; unsupported Mach-O
  dylibs are not advertised.
- Filename/ip7z and property-aware native/static extract-all enforce the
  caller's maximum output size with declared-size preflight and runtime-bounded
  staging before destination merge.
- Static-unpacker detection, listing, extract-all, indexed extraction, and test
  are routed centrally for the CLI and GUI.
- Console record collection now follows the streaming cursor contract and calls
  `moveToNext()` only between declared records. Single-record ASPack, FSG, MEW,
  NsPack, and Petite outputs are no longer discarded after successful parsing.
- Console archive listing now performs archive/static subtype refinement even
  when the preliminary scan returned `Binary`, `MSDOS`, PE, or CFBF. This makes
  native-only structural readers and semantic MSI/WiX listing reachable without
  breaking filename-only ip7z formats.
- `XMSI` resolves File/Media/Component/Directory semantics and streams installed
  payloads from embedded or sibling CABs and loose sources; `XWiX` delegates to
  that path after authenticating its WiX marker.
- WiX Burn v3/v4 embedded-CAB bootstrappers have a dedicated PE32/PE64 static
  unpacker with bounded multiple-container parsing and version-specific hash
  validation. Detached/external forms reject cleanly until an explicit source
  resolver is supplied.
- Inno parsing uses deterministic version schemas from both native-width 1.2.10
  layouts through 7.0, all five historical loader-table generations,
  independent file/location counts, and bounded header decompression. It
  validates external slice headers,
  data-block CRCs, encryption metadata, filters, and hash layout before
  committing output.
- ASN.1 UTCTime/GeneralizedTime parsing now returns canonical `Qt::UTC` on Qt 5
  and uses `QTimeZone::UTC` on Qt 6.8+, restoring the deterministic primitive
  decoder contract without the newer Qt deprecation.

### Verification for this change set

| Case | Result |
|---|---|
| Deterministic in-memory `test_installer_corpus --selftest` | The final consolidated 2026-08-23 MSVC/Qt 5 run completed in 51.0 s with `internal regressions: PASS`. New boundaries include whole-archive rollback after decoder, premature-enumeration, cancellation, finalization, unsafe-path, CRC, and simulated disk/publication failures across streaming, legacy, list-record, XFormats, and ip7z merge routes, plus native PAX/GNU TAR metadata, compressed-wrapper inheritance, ASAR sidecars/links, WARC 1.0/1.1, FAT32/FAT64 Mach-O tables, filename/ip7z and native/static streaming output limits, AutoIt/NSIS support code, UPX safety paths, and the existing lifecycle, codec, corruption, path, quota, and state contracts. |
| Strict production-handler installer corpus | 150/150 expected detections and 161/161 extraction contracts passed; zero skips, open failures, or cross-family hits. Includes 7-Zip SFX, WinRAR SFX, IExpress, MSI/WiX, Advanced/Actual Installer, InstallForge, Clickteam, CreateInstall, EnigmaVB, BoxedApp, default-enabled InstallSimple, SmartInstall and Tarma. |
| Packed-binary negative installer scan | 603/603 opened; zero static-installer hits and zero open failures. |
| Inno Setup 5.6.1, 11 Store/zlib/bzip2/LZMA/LZMA2 and solid/filter variants | All extracted byte-identically |
| Official Inno Setup 1.2.16 through 4.2.7 historical installers | 40/40 SHA-256-pinned fixtures passed full listing, checksum-metadata validation, and extraction across S02/S04/S05/S06/S07. This includes every archived 3.x and 4.x installer and both structurally ambiguous raw-ID pairs. The 1.2.16 NE installer extracted 13 records / 843,036 bytes through the generic factory/archive route; the official 1.2.16 source-derived Win32 twin has separate exact-layout regression coverage. |
| Inno Setup 5/6/7 location and loader-integrity fixture | Shared locations, external records, non-`{app}` paths, CRC/decoy/full-width/overlay/limit cases passed |
| Central legacy APIs on ordinary ZIP, Inno 5.6.1/7.0.2, and WiX 3.14 MSI | Listing; full/name/partial/device/file extraction; presence; and forged-record rejection passed |
| Fresh Qt 5 and Qt 6 console, Qt 6 GUI, and corpus-runner MSVC/Ninja builds after the parser/routing changes | All compile and link successfully; immediate rebuilds are clean no-ops. A fresh Qt 5 application cache selected `WITH_XEMULATOR:BOOL=ON`, generated `USE_XEMULATOR` plus Install Simple/emulator sources, and linked the final Release console with the new TAR/ASAR and whole-archive rollback code. |
| Native POSIX/GNU TAR regression | PAX `x`/`g` and GNU `L`/`K` records are applied and hidden across direct TAR and gzip-wrapped TAR. Global/local precedence, zero-length deletion, owner-name/ID precedence, long paths/links, negative fractional time, opaque unknown keys, malformed/oversized/dangling records, source state, and extraction confinement pass. |
| ASAR sidecar/link regression | Canonical Pickle framing, strict trees/integrity, inline and external bytes, zero-length sidecars, file and directory-link materialization, normalized/root-escape paths, cycles/missing targets, exact 40/41-hop boundary, output quotas/aliasing, sidecar mutation/hash mismatch, opened-handle containment, rollback, and caller positions pass. |
| WiX Burn v3 embedded-CAB and external-payload fixtures | Attached extraction matched the source payload by SHA-256; the unsupported external form failed closed |
| WiX 3.14 MSI through the shipped console | Refined CFBF to `MSI: WiX`, listed `app.exe`, and extracted bytes whose SHA-256 matched the ground-truth payload. |
| Filename-only CHM through the shipped console | General scan remained `Binary`; archive mode listed 23 files + 3 folders and extracted 23 files / 81,930 bytes. |
| DOS/16M structural fixture | Listed 4 records and extracted 293,119 bytes; every output matched its exact source slice by SHA-256. The 5,303-byte MF information gap intentionally remains outside the records. |
| MiniDump structural fixture | Listed and extracted 14 stored streams / 11,600 bytes; every output matched its directory extent by SHA-256, including safe suffixing of four duplicate `Unused` names. |
| APK/APKS/IPA/JAR/npm semantic console regression | Direct APK and resource-only APK positives selected APK; classes-only, empty, nested, case-mismatched, CRC-corrupt and encrypted manifests stayed ZIP. Stored/deflated APKS and a resource-only inner APK selected APKS; missing metadata/payload, incomplete ZIPs, unsupported/encrypted outer members, corrupt outer/toc/inner CRCs and encrypted/empty inner identities stayed ZIP. IPA and JAR positives refined correctly, while empty, misplaced, corrupt or encrypted identity records stayed ZIP. Positive APK/APKS/IPA/JAR/npm payloads extracted byte-identically; npm near misses and exact 1 MiB/1 MiB+1 metadata boundaries also passed. |
| ZIP/7-Zip encryption console regression | Generated ZipCrypto, WinZip AES-256, visible-header 7-Zip AES and encrypted-header 7-Zip AES fixtures exercised listing with no/wrong/correct passwords. Missing and wrong-password extraction published no files; correct `--password` and `--password-stdin` extraction was byte-identical, and all source hashes stayed unchanged. |
| DEB/RPM semantic self-tests | DEB 2.0 member order, post-data extensions, preference/factory routing and caller-position preservation passed; wrong versions/order/counts were rejected. Stored and gzip RPM payloads restored exact CPIO bytes, while malformed header bounds, compressor-tag/magic mismatch and a corrupt gzip footer failed with clean state reset. |
| Native nested-filter regression | Plain Brotli, Brotli(TAR), gzip(TAR), Brotli(gzip(TAR)), four gzip layers, Brotli-wrapped npm, and byte-identical extraction passed. Five layers stopped at the documented boundary, and a corrupted inner gzip footer did not expose TAR records. |
| ip7z raw-stream/Split regression | Raw Base64, a two-part byte stream, and a stored ZIP split inside its payload listed/extracted exactly across dotless and two-/three-/four-digit numeric and dotless/two-/three-/four-letter/prefixed alphabetic starts. Base64 opened at exactly 256 MiB; the bridge returned its explicit pre-backend safety error at 256 MiB + 1, and CLI extraction published no output. Incomplete and gapped volume sets failed extraction without output; all source hashes stayed unchanged. |
| mtree semantic console regression | Listed one directory + one file, created a zero-byte placeholder, rejected traversal and a 32,769-byte name, and merged a case-folded update to one record. |
| Quake PAK, Doom WAD, and Build GRP native regression | Synthetic specification-shaped fixtures were detected through the generic factory, listed, streamed, and extracted byte-identically. Tests cover truncation, range/count ceilings, PAK traversal, deterministic duplicate names, stable renamed-parent reuse, canonical empty containers, zero-size-member overlays, strict GRP end-of-file accounting, and the 100,000-record duplicate boundary. The shipped console regression passed for all three formats and also verified source immutability. |
| 1998 CD-image corpus | All 54 image sets matched an independent parser's file/folder counts: 53 CUE-described discs plus standalone `CMRALLY.iso` (5,724/670). Other representative counts include Falcon 4 (561/14), Tomb Raider 3 (1,550/36), Carmageddon II (2,379/78), and Excessive Speed (265/4). A real damaged-subheader Mode-2 extraction produced Abe's `SV.LVL` at 13,551,616 bytes with SHA-256 `ada47fd846e7847c18fd783a4822826d79f109c7aad097e1974d1c9a66e03aa0`, exactly matching an independent logical-sector extraction. |
| FAT/Ext2 filename-only ip7z regression | Both images listed and extracted; `bin/ls` (126,584 bytes) and `etc/services` (19,605 bytes) matched across the filesystems by SHA-256. |
| ip7z extraction-preflight regression | Safe absolute/drive/UNC-like and redundant-separator names normalized below the destination. Traversal in both separator styles, case-fold and Unicode-normalization duplicates, file/directory collision, ADS, `CON`, `COM¹`, `LPT²`, and 32,769-character metadata fixtures all failed before publishing entries; 11 hostile archives published no output and no traversal escaped. |
| DOS/4G structural fixture | Listed Loader, VMM.EXP, 4GWPRO.EXP and Payload from a 529,046-byte executable; the central archive API's full/name/partial/device/file extraction contract passed. |
| Additional ip7z-only listing evidence | Five Android Ext partition images, a UEFI firmware volume (162 files + 19 folders), two SWFs, one FLV, and additional CHMs listed successfully. A classic-HFS and an Intel HEX fixture failed cleanly and remain unverified. |
| Targeted missing-fixture inventory | A bounded local search found no usable CAB Quantum, WIM LZMS, DMG ADC/LZFSE, HXS or virtual-disk codec fixtures. Those capabilities therefore remain explicitly source-traced or `Wired`, not promoted to fixture-verified. |
| Real IPA, RAR4 SFX and Split ZIP | The 34,910-byte IPA refined to IPA and all six files matched 7-Zip by SHA-256. A 259,725-byte WinRAR SFX extracted its 67-byte stored payload identically. A 178-byte two-volume ZIP extracted its 26-byte payload identically; incomplete-volume extraction failed without output. Every source hash was unchanged. |
| Additional real archive ground truth | A zlib XAR (two outputs), ARJ method 4 (four files), and a RAR5 container using RAR 7.0/v6 compression (seven files) matched 7-Zip by relative path, length and SHA-256. A 135-byte SZDD restored its adjacent 113-byte source exactly, and nested `.tar.lzo` restored `TOOLNAME.txt` exactly. Source hashes remained unchanged; the RAR source timestamp was also unchanged. |
| WARC 1.0/1.1 native regression | Plain and gzip-wrapped multi-record archives list/extract exact `resource` and complete `response` blocks. Version-specific dates, 1.1 case/folding/LWS, strict unique record IDs, unsafe paths, malformed fields/URIs, and source immutability passed. |
| Mach-O FAT32/FAT64 console regression | A 21-slice classic FAT32 archive plus big- and little-endian FAT64 archives routed and extracted exact slices. Overflow, truncation, excessive count, and source-mutation checks passed. |
| AutoIt official fixtures | AutoIt v2.64 compiler/Exe2Aut produced exact encrypted script and `FileInstall` outputs; AutoIt 3.3.18.0 EA06 produced two exact records. Central routing, aggregate quotas, malformed records, and fail-closed behavior passed. |
| Non-PE UPX console regression | Two ELF and two Mach-O executables, DOS COM, and DOS SYS restored byte-identically. The main scripted BZR/Mach/COM/SYS gate passed against UPX 3.95; truncated/fake markers, five malformed Mach headers, and a corrupt ELF gap stream failed closed without source changes or partial output. |
| Extract-all output-limit regressions | Filename/ip7z accepts a 5-byte member at an exact 5-byte caller limit and rejects it at 4 bytes without replacing the destination sentinel. Shared native/static folder staging rejects an invalid limit before destination creation, preflights declared oversize before decoder entry, and caps write/resize/seek for dishonest or unknown sizes; exact-boundary commit and whole-archive rollback both pass. |
| Static packer route and breadth regression | Five canonical ASPack/FSG/MEW/NsPack/Petite CLI routes list reconstructed records; seven direct packer fixtures produce valid PEs, preserve the oracle OEP, and reproduce the oracle's ENTIRE code section byte-for-byte - 512 bytes for the five analysis-dump families and 15,872 / 32,256 bytes for UPX PE32 / PE64, replacing a 64-byte entry-point sample that a truncated or mis-mapped rebuild could pass. The two UPX outputs additionally execute and exit 0x12345678; the other five fault with 0xC0000005 because their unpackers deliberately do not rebuild the import directory, so execution is not a reachable gate for them (XFU-031). NsPack passed 34/34 corpus listings and Petite 2/2. ASPack is 56/128, FSG 9/12, and MEW 16/16; every reconstructed output in those three families was extracted through the shipped console and compared against the staged pre-pack input, matching the original entry-point RVA and its first 64 bytes in all 81 cases. The remaining ASPack 1.0x/2.11 and FSG 1.1/1.2 variants fall back or fail closed as documented. One sample from each of 54 PE UPX version directories still lists as UPX, with breadth byte comparison open. |
| SEA ARC codec regression | Every entry in all 18 genuine SEA ARC archives on this machine — 337 records spanning methods 2, 3, 4, 8 and 9 — extracted with the exact declared original size and a CRC-16/ARC recomputed by the test itself rather than taken from the application. Coverage includes the LZW `CLEAR` code, which is the only place ARC's compress-style group padding is observable: a flat bit reader passes every archive without one and corrupts every archive with one. Thirteen of the archives are pinned in `tests/console_seaarc_regression.ps1` (309 entries); source archives were unchanged. Methods 5-7 have no surviving sample and are refused. |
| SAR detection and extraction | Both genuine SAR archives listed and extracted `TEST.TXT` at 5,040 bytes with SHA-256 `583279d2...441e37`, matching the same 5,040-byte payload the ZOO and ACE fixtures extract to. Three unrelated readers now agree on that payload, which is what makes it a usable cross-format oracle. All 32 remaining files named `*.sar` on this machine - game resources, bitmaps, plain text and two ZIPs - were refused, giving zero false positives across the extension. LHA listing was unaffected by the shared member walk. |
| ARX detection and extraction | Both ARX fixtures are reported as ARX, list one `LZH1` member, and extract `TEST.TXT` at 5,040 bytes with SHA-256 `583279d2...441e37` — the same payload the SAR, ZOO and ACE fixtures produce, bringing the cross-format oracle to four independent readers. `tests/console_arx_regression.ps1` re-derives ARX-versus-LHA from the raw header bytes in PowerShell and requires the console to agree in both directions: no ARX archive may report as LHA, and none of the 27 genuine LHA archives may be claimed as ARX. Level 2 LHA headers are the trap that direction guards — their compressed-size low byte at offset 7 is legitimately zero, which is the byte ARX detection keys on. `XLHA`, `XSAR` and `XARX` validation now also restore the caller's device cursor explicitly; the LHA lh4 selftest had been passing only because the tag read happened to land on the probe position. |
| Packer cross-family false-positive sweep | All 1,846 executables under the 66-family packer corpus were listed through the shipped console. ASPack fired on exactly its own 56, FSG on its own 9, and MEW on its own 16; no sample outside a family's own directory was claimed by that family, and there were no crashes or open overruns. |
| Yoda's Protector official fixture | The Cisco-Talos ClamAV yC 1.3 sample restored its 3,072-byte embedded UPX image byte-identically from a 6,226-byte packed input. The pinned oracle/hash, PE fixups, output quotas, fail-closed cases, source immutability and family transition passed. |
| NSIS extraction audit | All 20 samples detect/list/extract. The seven historical non-solid 2.51/3.12 bzip2/LZMA/zlib and zip2exe-classic failures now preserve honest unknown sizes and match 7-Zip byte-for-byte; the other corpus cases retain the file-set qualifications documented above. |
| Bounded 317-file archive safety smoke | 289 successful listings; 28 clean nonzero results (2 password-required 7z, 25 embedded non-standalone raw-deflate candidates, 1 split-ZIP `.z01`); zero crashes, timeouts, output overruns, source mutations, or corpus changes. |
| ACE per-member solidity regression | All three ACE fixtures extract to the shared 5,040-byte payload, SHA-256 `583279d2...441e37` - the same bytes the ZOO and SAR fixtures produce, so the method-1 LZ77+Huffman decoder is confirmed against two unrelated readers rather than against itself. None of the three extracted before: the archive-level SOLID flag refused them all, the stored one included. The selftest now asserts the new contract in both directions - a solid archive's first stored and first compressed member decode, while the first member of a solid *continuation volume* is still refused, since it does depend on the previous volume's dictionary. Password, split-before/after and unsupported-TECH.TYPE members still reject. |
| lzop compression-level regression | All 15 local `.lzo` archives extract, up from 11: `lzop -7`, `-8`, `-9` and `--best` are LZO1X-999 (method 3), which is the only compressor that emits the M1 short-match opcode, and that opcode was rejected outright. The twelve level fixtures are one source file at every level, so they are their own oracle - all twelve now decode to a single identical 19,400-byte payload, SHA-256 `ddd6b4fd...41efaa1`. The five malformed-LZO negative cases in the selftest still reject. |
| Qt 5 GUI build and start-up | A dedicated `-DBUILD_GUI=ON` cache (`xfu_gui_build`, Qt 5.15.2, MSVC 2022 x64) configured and built both targets with zero errors, producing `src/gui/Release/xfileunpacker.exe` linked against the current library — including the new SAR type, the appended handler-method values and the SEA ARC decoder. After `windeployqt`, the application launched and was still running after eight seconds, so the main window is constructed without faulting. This is initialization evidence; no archive operation was driven through the UI. |
| Documentation/test-source audit | Every Markdown table had a consistent column count, code fences were balanced, no trailing whitespace was present, all reproduced test paths existed, and all 23 PowerShell test scripts parsed successfully. |

The GUI results above are build/link and start-up checks, not interactive
runtime tests. A real GUI open/list/extract/test smoke remains in Planned work,
and `run/build.bat` still builds the console only, so the GUI compile path has
to be selected deliberately.

### Reproduce the main gates

These are the commands for this workstation's current local build and corpora.
For a new checkout, configure/build the runner first as described in
`_mylibs/XStaticUnpacker/tests/installer_corpus/README.md`, then update
`$runner` and the corpus roots.

```powershell
$env:PATH = "C:\Qt\5.15.2\msvc2019_64\bin;" + $env:PATH
$runner = "F:\ownCloud\prepare\qt5\game_archive_selftest_build\Release\test_installer_corpus.exe"

& $runner --selftest
& $runner --corpus=F:\tests\installers --expect-count=150 --expect-extractions=161
& $runner --fpscan=F:\ownCloud\binary_examples\packed --expect-count=603

$console = "F:\ownCloud\prepare\qt5\game_archive_cli_build\src\console\Release\xfileunpackerc.exe"
& .\tests\console_package_type_regression.ps1 -ConsoleExe $console
& .\tests\console_encryption_regression.ps1 -ConsoleExe $console
& .\tests\console_nested_filter_regression.ps1 -ConsoleExe $console
& .\tests\console_ip7z_stream_handlers_regression.ps1 -ConsoleExe $console
& .\tests\console_listing_extract_regression.ps1 -ConsoleExe $console
& .\tests\console_packed_pe_regression.ps1 -ConsoleExe $console `
    -PackerRoot F:\tests\packers -RunnerExe $runner
& .\tests\console_mtree_regression.ps1 -ConsoleExe $console
& .\tests\console_game_archives_regression.ps1 -ConsoleExe $console
& .\tests\console_ip7z_safety_regression.ps1 -ConsoleExe $console
& .\tests\console_ip7z_filesystem_regression.ps1 -ConsoleExe $console `
    -FixtureRoot F:\ownCloud\binary_examples\GIT\rizin-testbins\fs
& .\tests\console_seaarc_regression.ps1 -ConsoleExe $console
& .\tests\console_sar_regression.ps1 -ConsoleExe $console
& .\tests\console_lzo_regression.ps1 -ConsoleExe $console
& .\tests\console_ace_regression.ps1 -ConsoleExe $console
& .\tests\nsis_extract_regression.ps1 -ConsoleExe $console
& .\tests\console_machofat64_regression.ps1 -ConsoleExe $console
& .\tests\autoit_v264_regression.ps1 -TestExe $runner `
    -QtBin C:\Qt\5.15.2\msvc2019_64\bin
& .\tests\autoit_ea06_regression.ps1 -TestExe $runner `
    -QtBin C:\Qt\5.15.2\msvc2019_64\bin
& .\tests\upx_nonpe_regression.ps1 -CliExe $console `
    -UpxExe F:\ownCloud\packers\done\UPX\upx-3.95-win32\upx.exe `
    -OriginalElf F:\ownCloud\binary_examples\BZR-Player-2.0.77_linux\BZRPlayer `
    -OriginalMach F:\ownCloud\binary_examples\GIT\rizin-testbins\mach0\ls-osx-x86_64 `
    -OriginalDos F:\ownCloud\binary_examples\SOU\SOUNDRV.COM `
    -PackedSys F:\ownCloud\file_formats2\dos_sys_filter\packed\ver_5_1_1_upx_best_dos_sys_filter.sys `
    -OriginalSys F:\ownCloud\file_formats2\dos_sys_filter\clear\dos_sys_filter.sys `
    -QtBin C:\Qt\5.15.2\msvc2019_64\bin
```

Application builds now default `WITH_XEMULATOR=ON`, matching the corpus runner,
so Install Simple ships by default. Use `-DWITH_XEMULATOR=OFF` for CMake or
qmake `XCONFIG+=no_xemulator` for an explicit opt-out.

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
