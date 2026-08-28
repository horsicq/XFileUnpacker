| Archive / container format | Common extensions or identity | Detect | List | Extract | Current supported scope |
|---|---|---:|---:|---:|---|
| 7-Zip | `.7z` | Yes | Yes | Yes | Self-contained archives using Copy, LZMA, LZMA2, Deflate, Deflate64, BZip2, PPMd7, Zstandard, Brotli, LZ4, LZ5, Lizard, branch filters, or AES-256; external folder and filename streams are unsupported |
| ZIP / ZIPX | `.zip`, `.zipx` | Yes | Yes | Yes | Single-disk, non-ZIP64 archives using Store, Shrink, Reduce, Implode, Deflate, Deflate64, BZip2, LZMA, XZ, Zstandard, PPMd8, WinZip JPEG (method 96), WavPack (method 97), ZipCrypto, or WinZip AES; WinZip MP3 (method 94) has no published specification and is not supported |
| RAR / RAR5 | `.rar` | Yes | Yes | Yes | Single-volume RAR 1.5, 2.x, 2.9, 5, and 7 method families; RAR5 AES |
| ARJ | `.arj` | Yes | Yes | Yes | Store, methods 1–4, and ARJ garble |
| LHA / LZH | `.lha`, `.lzh` | Yes | Yes | Bounded | `lh0`, `lh1`, and `lh4`–`lh7` |
| ZOO | `.zoo` | Yes | Yes | Yes | Store, LZD, and LZH records |
| SAR | `.sar` | Yes | Yes | Bounded | `LH0`, `LH4`, and `LH5` records |
| ARX | `.arx` | Yes | Yes | Yes | LZH1 records |
| ACE | `.ace` | Yes | Yes | Bounded | Stored and method-1 records; first independent solid member |
| SEA ARC | `.arc` | Yes | Yes | Bounded | Methods 2, 3, 4, 8, and 9 |
| FreeArc | `.arc`, `ArC\x01` archive header | Yes | Yes | Yes | Installed PeaZip `Arc` helper; metadata-only listing, deferred one-time extraction, private password-keyfile transport, exact manifest reconciliation, aggregate helper deadline, bounded private staging, and contained helper-process execution |
| ZPAQ | `.zpaq`, tagged or raw `zPQ` block header | Yes | Yes | Yes | Installed PeaZip `zpaq` helper; metadata-only listing, deferred one-time extraction, empty journals, implicit parent directories, safe remapping of stored absolute names, exact manifest reconciliation, aggregate helper deadline, bounded private staging, and contained helper-process execution |
| PEA | `.pea`, PEA header with `POD` stream | Yes | Yes | Yes | Installed PeaZip `pea` helper; contained execution and private staging published only after the helper integrity report verifies the archive |
| Microsoft CAB | `.cab` | Yes | Yes | Yes | Non-spanned cabinets using Store, MSZIP, LZX, or Quantum |
| Microsoft WIM | `.wim` | Yes | Yes | Bounded | Store, LZX, and XPRESS-Huffman resources in single-part, non-solid images |
| TAR | `.tar` | Yes | Yes | Yes | Regular files, directories, metadata, and safe path restoration |
| CPIO / AFIO | `.cpio`, `.afio` | Yes | Yes | Yes | Native archive records and payloads |
| Unix ar | `.a`, `.ar`, Debian ar containers | Yes | Yes | Yes | Standard ar members |
| XAR | `.xar` | Yes | Yes | Bounded | Store, zlib/gzip-style, BZip2, XZ, and LZMA payloads |
| ASAR | `.asar` | Yes | Yes | Yes | Electron ASAR records and payloads |
| Coktel Vision STK | `.stk`, `.itk`, `.ltk`, `.jtk` | Yes | Yes | Yes | Classic STK and STK 2.1; Store and Coktel LZ |
| WARC | `.warc`, `.warc.gz` after filtering | Yes | Yes | Yes | WARC 1.0 and 1.1 records |
| UUencoded data | `.uu`, `begin` and `begin-base64` envelopes | Yes | Yes | Yes | UUencode and begin-base64 payloads |
| Quake PAK | `.pak` | Yes | Yes | Yes | File-directory records and payloads |
| Doom WAD | `.wad` | Yes | Yes | Yes | Lump-directory records and payloads |
| Build GRP | `.grp` | Yes | Yes | Yes | File-directory records and payloads |
| CKP | `.ckp`, `.CKP\x00\x01` identity | Yes | Yes | Yes | Native store-only resource table, decoded names, and safe path restoration |
| EdgeDataPak | `.edp`, `.EDP\x00\x01` identity | Yes | Yes | Yes | Native store-only resource table with decoded UTF-16LE names and safe path restoration |
| Blizzard MPQ | `.mpq`, `MPQ\x1A` archive or `MPQ\x1B` user-data wrapper | Yes | Yes | Bounded | Classic hash/block tables; stored, zlib, bzip2, and PKWARE DCL sectors; absent listfiles use stable synthetic names; encrypted members use filename keys, sector-table recovery, or strongly validated known structures such as nested MPQ headers, otherwise extraction fails closed |
| Ptero BIGF / ZBL | `.cbf`, `BIGF` + `ZBL` identity | Yes | Yes | Yes | Stored and later-version obfuscated members plus multi-block Ptero LZW streams |
| Parsec resource archive | Nameless `.dat` resource table containing RIB or SM8 records | Yes | Yes | Yes | Strict `8*N+4` offset/size table, zero sentinel, contiguous records, exact EOF, and stable synthetic `.rib` / `.sm8` names |
| PSM 2.00 music module | `.pmm`, `MTCVTS PSM 2.00\0` identity | Yes | Yes | Yes | MDH metadata, PLX/PMA instrument bank, and one to eight exact-length SM8 samples; validated 6-, 7-, and 8-sample variants |
| CFBF / OLE compound file | `.cfb`, `.ole` and compound-document identities | Yes | Yes | Yes | Compound streams and storages |
| Mach-O universal / FAT | FAT32 and FAT64 Mach-O | Yes | Yes | Yes | Embedded architecture slices |
| Windows MiniDump | `.dmp`, MiniDump signature | Yes | Yes | Yes | Exposed dump streams and structures |
| DOS/16M | DOS/16M executable identity | Yes | Yes | Yes | Embedded executable structures |
| DOS/4G | DOS/4G executable identity | Yes | Yes | Yes | Embedded executable structures |
| mtree | `.mtree`, mtree manifest | Yes | Yes | Metadata | Directories and deterministic zero-byte metadata placeholders |

| Single-stream format | Common extensions or identity | Detect | List | Extract | Current supported scope |
|---|---|---:|---:|---:|---|
| gzip | `.gz` | Yes | Yes | Yes | gzip members and checksums |
| zlib | `.zlib`, zlib framing | Yes | Yes | Yes | zlib-wrapped Deflate |
| bzip2 | `.bz2` | Yes | Yes | Yes | bzip2 streams |
| XZ | `.xz` | Yes | Yes | Yes | XZ containers and filters |
| LZMA-alone | `.lzma` | Yes | Yes | Yes | LZMA-alone streams |
| lzip | `.lz` | Yes | Yes | Yes | lzip members and checksums |
| Unix compress | `.Z` | Yes | Yes | Yes | LZW-compress streams |
| Zstandard | `.zst`, `.zstd` | Yes | Yes | Yes | Zstandard frames |
| LZ4 | `.lz4` | Yes | Yes | Yes | LZ4 framing |
| LZ5 | `.lz5` | Yes | Yes | Yes | LZ5 framing |
| Lizard | `.lizard` | Yes | Yes | Yes | Lizard framing |
| Brotli | `.br` | Yes | Yes | Yes | Brotli streams |
| LZO / lzop | `.lzo` | Yes | Yes | Bounded | lzop methods 2 and 3 |
| SZDD | SZDD signature | Yes | Yes | Yes | SZDD streams |
| KWAJ | KWAJ signature | Yes | Yes | Yes | KWAJ streams |
| BCM | `.bcm`, `BCM1` stream | Yes | Yes | Yes | One logical output through the contained PeaZip BCM helper; bounded private staging; the format carries no filename or integrity metadata |
| LPAQ8 | `.lpaq8`, `.lpq`, `pQ` version-8 stream | Yes | Yes | Yes | One logical output through the contained PeaZip LPAQ8 helper; bounded private staging; declared size and configured memory ceiling are enforced |
| Parsec RIB | `.rib`, `.dat`, `RIB\0` stream | Yes | Yes | Yes | Native backward decoder with stored/empty handling, strict cursor convergence, reference/output bounds, cancellation, output ceilings, and inferred payload extensions |

| Compressed TAR format | Common extensions | Detect | List | Extract | Current supported route |
|---|---|---:|---:|---:|---|
| tar + gzip | `.tar.gz`, `.tgz` | Yes | Yes | Yes | gzip filter followed by TAR |
| tar + bzip2 | `.tar.bz2`, `.tbz`, `.tbz2` | Yes | Yes | Yes | bzip2 filter followed by TAR |
| tar + XZ | `.tar.xz`, `.txz` | Yes | Yes | Yes | XZ filter followed by TAR |
| tar + LZMA | `.tar.lzma` | Yes | Yes | Yes | LZMA filter followed by TAR |
| tar + lzip | `.tar.lz`, `.tlz` | Yes | Yes | Yes | lzip filter followed by TAR |
| tar + lzop | `.tar.lzo` | Yes | Yes | Yes | supported lzop filter followed by TAR |
| tar + Unix compress | `.tar.Z`, `.taz` | Yes | Yes | Yes | compress filter followed by TAR |
| tar + Zstandard | `.tar.zst`, `.tar.zstd`, `.tzst` | Yes | Yes | Yes | Zstandard filter followed by TAR |
| tar + LZ4 | `.tar.lz4` | Yes | Yes | Yes | LZ4 filter followed by TAR |

| Disk, filesystem, firmware, or structural format | Common extensions or identity | Detect | List | Extract | Current supported scope |
|---|---|---:|---:|---:|---|
| ISO 9660 / Joliet / CUE | `.iso`, `.cue`, raw optical images | Yes | Yes | Bounded | ISO/Joliet content from the first valid Mode-1 or Mode-2 Form-1 data track |
| UDF | UDF optical image | Yes | Bounded | Bounded | Basic single-partition images with directly usable allocation addresses and a single short/long extent or inline allocation; bridge images require explicit UDF routing |
| Apple DMG | `.dmg` | Yes | Yes | Bounded | Store, zlib, and BZip2 stripes; ADC, LZFSE, and XZ stripes are unsupported |

| Package format | Common extensions or identity | Detect | List | Extract | Current supported route |
|---|---|---:|---:|---:|---|
| Java archive | `.jar` | Yes | Yes | Yes | Semantic JAR identity over ZIP content |
| Android package | `.apk` | Yes | Yes | Yes | Semantic APK identity over ZIP content |
| Android APK Set | `.apks` | Yes | Yes | Yes | APK Set identity and contained APK records |
| Apple iOS package | `.ipa` | Yes | Yes | Yes | Semantic IPA identity over ZIP content |
| npm package | npm tarball identity | Yes | Yes | Yes | npm metadata and tar payload route |
| Debian package | `.deb` | Yes | Yes | Yes | ar members and nested control/data archives |
| RPM package | `.rpm` | Yes | Yes | Yes | RPM metadata and nested payload archive |
| macOS XAR package | `.pkg`, `.xip` with XAR identity | Yes | Yes | Yes | XAR container route |
| ZIP-based application packages | `.war`, `.ear`, `.xpi`, `.xapk` | Yes | Yes | Yes | ZIP container route |
| ZIP-based document packages | `.cbz`, `.appx`, `.msix`, `.whl`, OpenDocument, OOXML, EPUB | Yes | Yes | Yes | ZIP container route |
| Rust crate | `.crate` | Yes | Yes | Yes | gzip filter followed by TAR |

| Installer / SFX format | Common identity | Detect | List | Extract | Current supported scope |
|---|---|---:|---:|---:|---|
| Inno Setup | Inno Setup executable | Yes | Yes | Yes | Installer records; ARC4-MD5, ARC4-SHA1, and PBKDF2-XChaCha20 protected payloads |
| NSIS | NSIS executable | Yes | Yes | Yes | Installer records and payloads |
| InstallShield Cabinet | `.cab`, `ISc(` proprietary cabinet identity | Yes | Yes | Bounded | InstallShield 5 and 6+ catalog media; stored and chunked raw-Deflate members, obfuscation, links, MD5 verification, and adjacent volumes; secondary volumes require the companion `DATA1.HDR` or catalog-bearing `DATA1.CAB` |
| WiX Burn | WiX v3/v4 bootstrapper | Yes | Yes | Bounded | Attached and UX containers with version-appropriate hash validation |
| 7-Zip SFX | PE or ELF wrapper with an embedded 7-Zip archive; official 7-Zip modules have their own attribution | Yes | Yes | Yes | Configured and plain self-contained 7-Zip payloads; external folder/filename streams are unsupported; the official identity remains preferred when its stub attribution is present, and the stub is never executed |
| WinRAR SFX | PE32 or PE64 WinRAR-attributed wrapper with an embedded RAR archive | Yes | Yes | Yes | Single-volume RAR payloads; official WinRAR attribution remains preferred over the generic RAR SFX identity; the stub is never executed |
| ZIP SFX | PE or ELF wrapper with an embedded ZIP archive | Yes | Yes | Yes | Single-disk, non-ZIP64 payloads with strict central/local validation and delegated extraction; payload search is confined to the executable's unmapped suffix and the stub is never executed |
| RAR SFX | PE or ELF wrapper with an embedded RAR archive | Yes | Yes | Yes | Single-volume RAR payloads; family-specific generic identity distinct from WinRAR attribution; payload search is confined to the executable's unmapped suffix and the stub is never executed |
| CAB SFX | PE or ELF wrapper with an embedded Microsoft Cabinet archive | Yes | Yes | Yes | Non-spanned cabinet payloads with delegated extraction; payload search is confined to the executable's unmapped suffix and the stub is never executed |
| FreeArc SFX | PE or ELF wrapper with an embedded `ArC\x01` archive | Yes | Yes | Yes | Family-specific FreeArc SFX identity, authenticated header/footer selection, metadata-only listing, aggregate-deadline fallback only after a positively identified rejection and equivalent manifest, plus eager authentication of empty/directory-only payloads; bounded overlay carving uses the contained PeaZip `Arc` helper and the stub is never executed |
| ZPAQ SFX | PE or ELF wrapper with a tagged, raw `zPQ`, or encrypted ZPAQ payload | Yes | Yes | Yes | Family-specific ZPAQ SFX identity, exact zpaqfranz overlay framing, metadata-only listing, aggregate-deadline fallback only after a positively identified rejection and equivalent manifest, plus eager authentication of empty/directory-only payloads; password-protected and empty archives are supported through the contained PeaZip `zpaq` helper and the stub is never executed |
| IExpress | PE32 or PE64 IExpress package | Yes | Yes | Yes | Embedded package records and payloads |
| InstallForge | PE32 or PE64 InstallForge installer | Yes | Yes | Yes | Installer records and payloads |
| CreateInstall | PE32 or PE64 CreateInstall installer | Yes | Yes | Yes | Installer records and payloads |
| Actual Installer | PE32 or PE64 Actual Installer package | Yes | Yes | Yes | Installer records and payloads |
| Advanced Installer | PE32 or PE64 Advanced Installer package | Yes | Yes | Yes | Installer records and payloads |
| Smart Install Maker | PE32 or PE64 Smart Install Maker package | Yes | Yes | Yes | Installer records and payloads |
| Clickteam | PE32 or PE64 Clickteam package | Yes | Yes | Yes | Installer records and payloads |
| Tarma / InstallMate | PE32 or PE64 Tarma package | Yes | Yes | Yes | Installer records and payloads |
| AutoIt | AutoIt v2 or v3 executable | Yes | Yes | Yes | Script/resource records and payloads |
| Enigma Virtual Box | PE32 or PE64 EnigmaVB package | Yes | Yes | Yes | Virtualized file records and payloads |
| BoxedApp | PE32 or PE64 BoxedApp package | Yes | Yes | Yes | Virtualized file records and payloads |
| Install Simple | PE32 or PE64 Install Simple package | Yes | Yes | Yes | Emulator-enabled installer route |
| MSI | CFBF MSI database | Yes | Yes | Yes | MSI streams, tables, and payload records |
| WiX MSI | CFBF WiX MSI database | Yes | Yes | Yes | WiX/MSI streams, tables, and payload records |

| Executable packer / protector | Current identity | Detect | List | Extract | Current supported scope |
|---|---|---:|---:|---:|---|
| UPX | UPX-packed PE32, PE64, ELF, Mach-O, DOS COM, or DOS SYS | Yes | Yes | Bounded | Supported UPX image families and methods |
| ASPack | ASPack 2.x PE32 | Yes | Yes | Bounded | 2.00, 2.001, 2.1, 2.11 variants, 2.12, 2.2, and 2.42 |
| FSG | FSG-packed PE32 | Yes | Yes | Bounded | 1.0/1.3, 1.1, 1.2, 1.31, 1.33, and 2.0 analysis images |
| MEW | MEW-packed PE32 | Yes | Yes | Yes | Versions 10, 11, and 11 SE 1.2 |
| NsPack | NsPack-packed PE32 | Yes | Yes | Yes | Reconstructed executable image with imports, TLS, and resources |
| Petite | Petite-packed PE32 | Yes | Yes | Yes | Reconstructed PE32 image with explicit OEP state |
| Yoda's Protector | Yoda-protected PE32 | Yes | Yes | Yes | Reconstructed PE32 image |
