# XFileUnpacker — Current Format Support

Status of format support in XFileUnpacker **0.1.0**, derived by reading the
dispatch and decoder code rather than from documentation.

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

A format needs **all** of the following to be usable end to end, and each has
been a real source of gaps: an `isValid` probe in `XFormats::_getFileTypes`, an
entry in the type name table, an arm in `XFormats::createClass`, and membership
in `XArchives::getArchiveOpenValidFileTypes()`.

## How to read this

- **Detect** — the file is recognised as a named type (shown by `xfileunpackerc <file>`).
- **List** — entries can be enumerated (`--listarchive`).
- **Extract** — entry data can actually be decompressed to disk (`--extractarchive DIR`).

`Partial` always carries a qualifier in Notes. Enum membership alone is never
counted as support; each `Yes` was traced to a reachable decoder.

### Engine note

Extraction has two tiers. `decompressToFolder(fileName, …)` and `testArchive`
try the **compiled-in 7-Zip source bridge (ip7z)** first and fall back to the
native `XArchive` class only on a structured "unsupported format" error.
Device-based calls (`getRecords`, `decompress`) use **only** the native
`XFormats::createClass` path. A handful of types listed in
`XArchives::isNativeReaderPreferredFileType()` deliberately bypass ip7z.

This is why a few formats can extract through ip7z while their native class is
header-only — those rows are marked accordingly.

---

## Multi-file archives

| Format | Extensions | Detect | List | Extract | Codecs | Notes |
|---|---|---|---|---|---|---|
| 7-Zip | `.7z` | Yes | Yes | Yes | Copy, LZMA, LZMA2, Deflate, Deflate64, BZip2, PPMd7, ZSTD, Brotli, LZ4, LZ5, Lizard, Delta, BCJ, BCJ2, ARM, ARM64, ARMT, PPC, SPARC, IA64 | AES-256 supported |
| ZIP | `.zip`, `.zipx` | Yes | Yes | Yes | Store, Shrink, Reduce 1–4, Implode, Deflate, Deflate64, BZip2, LZMA, XZ, ZSTD, PPMd8 | ZipCrypto and AES-128/192/256 |
| RAR | `.rar` | Yes | Yes | Yes | RAR 1.5, 2.0, 2.9, 5.0, 7.0 | AES for RAR5 |
| ARJ | `.arj` | Yes | Yes | Yes | Store, method 1–3, fastest | |
| LHA / LZH | `.lzh`, `.lha` | Yes | Yes | Yes | lh0, lh1, lh5, lh6, lh7 | |
| ZOO | `.zoo` | Yes | Yes | Yes | Store, LZD, LZH | Decoded inside `XZOO`, not via the generic chain |
| ACE | `.ace` | Yes | Yes | Partial | ACE method 1 (LZ77+Huffman) | Solid archives, encrypted entries and ACE 2.0 `TECH.TYPE 2` are explicitly blocked |
| ARC (SEA) | `.arc` | Yes | Yes | Partial | Stored (methods 1–2) | Methods 3–9 (RLE, Squeezed/Huffman, Crunched/LZW, Squashed) map to `HANDLE_METHOD_UNKNOWN` and are listed but not extractable. CRC-16/ARC is verified on extract |
| FreeARC | `.arc` | Yes | No | No | — | `XFREEARC::getFileParts` emits raw blocks only; no per-file records |
| CAB | `.cab` | Yes | Yes | Partial | Store, MSZIP, LZX | Quantum entries are listed but not extractable |
| WIM | `.wim` | Yes | Yes | Partial | Store, LZX, XPRESS-Huffman | LZMS entries are listed but not extractable |
| TAR | `.tar` | Yes | Yes | Yes | Store | |
| CPIO | `.cpio` | Yes | Yes | Yes | Store | Includes AFIO variants |
| ar | `.a` | Yes | Yes | Yes | Store | |
| XAR | `.xar`, `.pkg` | Yes | Yes | Yes | none, zlib, bzip2, xz, lzma | macOS flat packages are XAR |
| ASAR | `.asar` | Yes | Yes | Yes | Store | Electron bundles |
| WARC | `.warc` | Yes | Yes | Yes | — | Native reader, preferred over ip7z |
| mtree | — | Yes | Yes | Yes | — | Self-contained records only |
| UU / begin-base64 | `.uu` | Yes | Yes | Yes | — | Transport decoding |
| SquashFS | `.squashfs`, `.sfs`, `.snap` | Yes | Yes | Yes | gzip, lzma, lzo, xz, lz4, zstd, uncompressed | Both byte orders (`hsqs`/`sqsh`). Native `XSquashfs` is a superblock/structure reader; records and extraction come from ip7z's `SquashfsHandler` |
| CFBF / MSI | `.msi`, `.doc` | Yes | Yes | Partial | Store | Container streams only. MSI table semantics and embedded CAB payloads are not resolved |
| MiniDump | `.dmp` | Yes | Yes | — | — | Structure regions, not file entries |
| Mach-O FAT | — | Yes | Yes | Yes | Store | Slices exposed as entries |

## Single-stream compressors

Each wraps exactly one payload. All are detected, listed as a single entry, and
extracted.

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| gzip | `.gz` | Yes | Yes | Yes | |
| zlib | — | Yes | Yes | Yes | Raw zlib streams |
| bzip2 | `.bz2` | Yes | Yes | Yes | |
| XZ | `.xz` | Yes | Yes | Yes | |
| LZMA (alone) | `.lzma` | Yes | Yes | Yes | |
| lzip | `.lz` | Yes | Yes | Yes | Multi-member streams supported |
| compress | `.Z` | Yes | Yes | Yes | Unix LZW |
| Zstandard | `.zst` | Yes | Yes | Yes | Legacy v0.4–v0.7 frames stay on the native path |
| LZ4 | `.lz4` | Yes | Yes | Yes | Native reader preferred |
| LZ5 | `.lz5` | Yes | Yes | Yes | Native reader preferred |
| Lizard | `.liz` | Yes | Yes | Yes | Native reader preferred |
| Brotli | `.br` | Yes | Yes | Yes | Native reader preferred |
| LZO | `.lzo` | Yes | Yes | Yes | |
| SZDD | — | Yes | Yes | Yes | MS-DOS `compress.exe`; LZSS decoded inside `XSZDD` |
| KWAJ | — | Yes | Yes | Yes | LZSS/LZH decoded inside `XKWAJ` |

## Compressed tar wrappers

The outer compressor is unwrapped transparently and the inner tar is read in the
same pass, so entries appear directly.

| Format | Extensions | Detect | List | Extract |
|---|---|---|---|---|
| tar + gzip | `.tar.gz`, `.tgz` | Yes | Yes | Yes |
| tar + bzip2 | `.tar.bz2`, `.tbz2` | Yes | Yes | Yes |
| tar + xz | `.tar.xz`, `.txz` | Yes | Yes | Yes |
| tar + lzma | `.tar.lzma` | Yes | Yes | Yes |
| tar + lzip | `.tar.lz` | Yes | Yes | Yes |
| tar + lzop | `.tar.lzo` | Yes | Yes | Yes |
| tar + compress | `.tar.Z`, `.taz` | Yes | Yes | Yes |
| tar + zstd | `.tar.zst` | Yes | Yes | Yes |
| tar + lz4 | `.tar.lz4` | Yes | Yes | Yes |

## Disk images

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| ISO 9660 | `.iso` | Yes | Yes | Yes | Stored content |
| UDF | `.udf` | Yes | Yes | Yes | Stored content |
| Apple Disk Image | `.dmg` | Yes | Yes | Partial | Store, zlib and bzip2 stripes only; ADC and LZFSE stripes are not decoded |
| Raw disk image | `.img` | — | — | — | Only if the image *is* ISO 9660 or UDF. Raw FAT/ext are reachable through ip7z only |

## Package and application bundles

All of these are ZIP containers and are handled by the ZIP reader. Several have
their own `FT_*` type; the rest are detected as plain `FT_ZIP`.

| Format | Extensions | Own type | Detect | List | Extract |
|---|---|---|---|---|---|
| JAR | `.jar` | Yes | Yes | Yes | Yes |
| APK | `.apk` | Yes | Yes | Yes | Yes |
| APKS | `.apks` | Yes | Yes | Yes | Yes |
| IPA | `.ipa` | Yes | Yes | Yes | Yes |
| npm | `.tgz` | Yes | Yes | Yes | Yes |
| WAR / EAR | `.war`, `.ear` | No | Yes | Yes | Yes |
| XPI | `.xpi` | No | Yes | Yes | Yes |
| CBZ | `.cbz` | No | Yes | Yes | Yes |
| APPX / MSIX | `.appx`, `.msix` | No | Yes | Yes | Yes |
| Python wheel | `.whl` | No | Yes | Yes | Yes |
| Rust crate | `.crate` | No | Yes | Yes | Yes | 

Non-ZIP packages:

| Format | Extensions | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| Debian package | `.deb` | Yes | Yes | Yes | `ar` container |
| RPM package | `.rpm` | Yes | Yes | Yes | |
| macOS package | `.pkg` | Yes | Yes | Yes | XAR container |
| Snap | `.snap` | Yes | Yes | Yes | Snap is a SquashFS image; handled by the SquashFS reader |

## Installers, self-extractors and packers

Handled by the `XStaticUnpacker` classes, wired into the app in this revision.
Detection and **listing** go through the shared `XBinary` streaming API
(`initUnpack` / `infoCurrent` / `moveToNext`). These types are probed only when a
caller requests `FT_FLAG_STATICUNPACKERS` (the GUI and the CLI `--listarchive`
now do), and they are matched **before** the ip7z bridge so an installer is not
misread as a plain PE and listed as raw sections.

**Extraction is not yet wired for this group.** The classes implement
`unpackCurrent`, but `--extractarchive` and the GUI Extract/Test buttons still
route through `XArchive`, so the current end-to-end status is **detect + list,
not extract** (see Planned, and tracker items #10 / #12). Only **Inno Setup**
and **NSIS** are verified through the app; the rest are wired and rely on the
class-level `isValid`/`initUnpack` (exercised by the `installer_corpus` test) but
are not yet confirmed through the CLI/GUI — marked *Untested* below.

### Installers / self-extractors (multi-file)

| Format | Type shown | Detect | List | Extract | Notes |
|---|---|---|---|---|---|
| Inno Setup | `PE32/PE64: Inno Setup` | Yes | **Yes** | Planned | Both **5.1.x (ANSI)** and **6.x (Unicode)** setup-data parsers; verified on real installers (5.1.10 → 31 files, 6.0.0 → 86, 6.4.2 → 1386) |
| NSIS | `PE32/PE64: NSIS` | Yes | **Yes** | Planned | LZMA/zlib/bzip2, solid + non-solid; file list & sizes match 7-Zip; `SetOutPath` dir surfaced as the advanced **Path** column |
| 7-Zip SFX | `PE32/PE64: 7-Zip SFX` | Yes | Untested | Planned | payload is a 7z archive |
| WinRAR SFX | `PE32/PE64: WinRAR SFX` | Yes | Untested | Planned | payload is a RAR archive |
| Generic SFX | `PE32/PE64: SFX` | Yes | Untested | Planned | |
| IExpress | `PE32/PE64: IExpress` | Yes | Untested | Planned | Windows `wextract` |
| InstallForge | `PE32/PE64: InstallForge` | Yes | Untested | Planned | |
| CreateInstall | `PE32/PE64: CreateInstall` | Yes | Untested | Planned | |
| Actual Installer | `PE32/PE64: Actual Installer` | Yes | Untested | Planned | |
| Advanced Installer | `PE32/PE64: Advanced Installer` | Yes | Untested | Planned | EXE bootstrapper |
| Smart Install Maker | `PE32/PE64: SmartInstall` | Yes | Untested | Planned | |
| Clickteam Install | `PE32/PE64: Clickteam` | Yes | Untested | Planned | |
| Tarma (ExpressInstall) | `PE32/PE64: Tarma` | Yes | Untested | Planned | |
| AutoIt | `PE32/PE64: AutoIt` | Yes | Untested | Planned | compiled AutoIt3 |
| Enigma Virtual Box | `PE32/PE64: Enigma Virtual Box` | Yes | Untested | Planned | file virtualization |
| BoxedApp | `PE32/PE64: BoxedApp` | Yes | Untested | Planned | file virtualization |
| Install Simple | `PE32/PE64: Install Simple` | only w/ `WITH_XEMULATOR` | — | — | **not in the default app build** — the class needs the XEmulator option |
| MSI | `MSI` (CFBF) | Yes | Yes | Partial | CFBF database; see the CFBF/MSI row above — container streams only |
| WiX (MSI) | `MSI: WiX` (CFBF) | Yes | Yes | Partial | WiX-authored MSI database (CFBF). **Not** WiX Burn bundles — those are PE `.wixburn` bootstrappers (see Planned) |

### Packers / protectors (single-file → restored executable)

| Format | Type shown | Detect | Unpack | Notes |
|---|---|---|---|---|
| UPX | `PE32/PE64: UPX` | Yes | Planned | also unpacks ELF/Mach-O/DOS UPX (kept as generic `FT_UPX`) |
| ASPack | `PE32: ASPack` | Yes | Planned | PE32 only |
| FSG | `PE32: FSG` | Yes | Planned | PE32 only |
| MEW | `PE32: MEW` | Yes | Planned | PE32 only |
| NsPack | `PE32: NsPack` | Yes | Planned | PE32 only |
| Petite | `PE32: Petite` | Yes | Planned | PE32 only |
| Yoda's Protector | `PE32: Yoda's Protector` | Yes | Planned | PE32 only |

## Planned / to add later

| Item | Kind | Notes |
|---|---|---|
| Installer/packer **extraction** | Capability | wire the `XStaticUnpacker` `unpackCurrent` path into `--extractarchive` and the GUI Extract/Test buttons — this group is list-only today. Tracker #10 |
| Verify wired installers/packers | Coverage | run the SFX / AutoIt / Clickteam / EnigmaVB / UPX / ASPack / … classes against real samples through the app; only Inno + NSIS are verified so far. Tracker #12 |
| **WiX Burn bundle** (`.exe`) | New format | PE bootstrapper with a `.wixburn` section → attached CAB/LZMA container → child MSIs/payloads (distinct from the CFBF WiX/MSI rows). Tracker #9 |
| Inno Setup 5.0.x / 5.2.x / 5.3.x-ANSI | Coverage | verify the ANSI parser on other 5.x point releases; relax the empty-source / sequential-location heuristics. Tracker #11 |
| Inno Setup `OPTIONAL_PATH` | Enhancement | surface Inno's `{const}\…` destination directory as an advanced Path column, like NSIS's SetOutPath |
| Install Simple in the default build | Build | enable `WITH_XEMULATOR` (or a non-emulator path) so `XInstallSimple` ships in the app |
| MSI table semantics + embedded CAB | Enhancement | resolve MSI install layout and extract embedded CAB payloads (CFBF row is container-streams-only today) |
| ALZ, EGG, StuffIt, PEA, PAK | New formats | no detector or reader today (see Not supported) |

## Encryption

| Scheme | Formats | Extract | Listing without password |
|---|---|---|---|
| ZipCrypto | ZIP | Yes | Yes — headers are not encrypted |
| AES-128 / 192 / 256 (WinZip) | ZIP | Yes | Yes |
| AES-256 | 7-Zip | Yes | Only when headers are unencrypted |
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

When a password is required but absent, the engine reports
`[XAESDecoder] Password is required for AES decryption`.

## Codecs reachable through the generic chain

`XDecompress::multiDecompress()` handles: Store/Copy, Deflate, Deflate64,
BZip2, LZMA, LZMA2, PPMd7, PPMd8, ZSTD, LZ4, LZ5, Lizard, Brotli, XZ, lzip,
Unix compress (LZW), LZO, Shrink, Reduce, Implode, RAR 1.5/2.0/2.9/5.0, Delta
and the BCJ/BCJ2/ARM/ARM64/ARMT/PPC/SPARC/IA64 branch filters.

These enum values are **not** in that chain and reaching it with them yields
"Unknown compression method":

`LZSS_SZDD`, `KWAJ_LZSS`, `KWAJ_LZH`, `ZOO_LZD`, `ZOO_LZH`, `LZX`, `XPRESS`,
`XPRESS_HUFF`, `ANDROID_XML`, `ACE_DELTA`, `ARCHIVE_STREAM`, `FILE`.

That is by design — each is decoded privately inside its owning class (`XSZDD`,
`XKWAJ`, `XZOO`, `XWIM`, `XCab`). It only matters for callers that route a
record through the generic chain instead of the owning reader.

## Not supported

No detector and no reader:

| Format | Extensions |
|---|---|
| ALZ | `.alz` |
| EGG | `.egg` |
| StuffIt | `.sit`, `.sitx` |
| PEA | `.pea` |
| PAK | `.pak` |
| SAR (as a distinct type) | `.sar` |

Partially blocked, listed above for detail: ACE beyond method 1, CAB Quantum,
WIM LZMS, DMG ADC/LZFSE stripes, FreeARC records, MSI embedded CAB payloads.

## Changes in this revision

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
| 362-file archive corpus | Swept before and after to confirm no regression |
