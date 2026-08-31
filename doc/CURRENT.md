| Archive / container format | Common extensions or identity | Detect | List | Extract | Current supported scope |
|---|---|---:|---:|---:|---|
| 7-Zip | `.7z` | Yes | Yes | Yes | Self-contained archives using Copy, LZMA, LZMA2, Deflate, Deflate64, BZip2, PPMd7, Zstandard, Brotli, LZ4, LZ5, Lizard, branch filters, or AES-256; external folder and filename streams are unsupported |
| ZIP / ZIPX | `.zip`, `.zipx` | Yes | Yes | Yes | Single-disk, non-ZIP64 archives using Store, Shrink, Reduce, Implode, Deflate, Deflate64, BZip2, LZMA, XZ, Zstandard, PPMd8, WinZip JPEG (method 96), WavPack (method 97), ZipCrypto, or WinZip AES; WinZip MP3 (method 94) has no published specification and is not supported |
| RAR / RAR5 | `.rar` | Yes | Yes | Yes | Single-volume RAR 1.5, 2.x, 2.9, 5, and 7 method families; RAR5 AES |
| ARJ | `.arj` | Yes | Yes | Yes | Store, methods 1–4, and ARJ garble |
| LHA / LZH | `.lha`, `.lzh` | Yes | Yes | Bounded | Stored `lh0`, `lz4`, and `lhd` records plus `lh1` and `lh4`–`lh7`; LHA SFX also reaches the historical LArc `lz5` dialect through the contained Deark reader |
| ZOO | `.zoo` | Yes | Yes | Yes | Store, LZD, and LZH records |
| FLS (SaveRam/SaveRam2) | OS/2 SaveRam record table and `SaveRam` banner | Yes | Yes | Yes | Stored and `S`-tag adaptive-codec members; all 116 corpus records use exact packed/output bounds, CP437 names, and DOS timestamps; extended-attribute blobs are not exposed |
| RTPatch | `.rtp`, `.stp`, `K*` package header | Yes | Yes | Bounded | Versions 1.10, 2.00, 2.11, 4.10, 5.00, and 6.50; package banners and whole-file streams with validated source-free framing extract in every supported generation; binary deltas require the pre-patch source and remain listed but fail closed |
| SAR | `.sar` | Yes | Yes | Bounded | `LH0`, `LH4`, and `LH5` records |
| ARX | `.arx` | Yes | Yes | Yes | LZH1 records |
| ACE | `.ace` | Yes | Yes | Bounded | Stored and method-1 records; first independent solid member |
| SEA ARC / PAK 2.51 | `.arc`, ARC/PAK member headers | Yes | Yes | Bounded | Methods 2, 3, 4, 8, 9, 10 (PAK Crushed), and 11 (PAK Distilled); original Crunch methods 5–7 remain identified and fail closed |
| Crusher ARQ | `.arq`, `gW` container | Yes | Yes | Yes | LH5-compatible members with packed-stream CRC-32 validation, exact declared sizes, and safe path restoration |
| Squeeze It SQZ | `.sqz`, `HLSQZ` member chain | Yes | Yes | Yes | Store and methods 1–4 with CRC-32, exact output sizes, DOS timestamps, and bounded linked-record traversal |
| FoxPro FPAK | `.pak`, `FPAK` / `FPAC` volumes | Yes | Yes | Yes | Version-1 and version-2 FPPF members using FoxPro Implode; adjacent continuation volumes are joined and CRC-32 verified, while missing volumes fail closed |
| IBM OS/2 PACK2 (FTCOMP) | `FTCOMP` / `fT19` member records | Yes | Yes | Yes | Concatenated fT19 members with exact packed/output bounds; all 278 corpus members decode byte-exactly; extended-attribute blobs are bounded but not exposed |
| DOS Navigator installer archive | `.138`, DOS Navigator record/directory tables | Yes | Yes | Yes | Stored records and the raw-Deflate variant whose dynamic counts are ordered HDIST/HCLEN/HLIT; duplicate record/directory tables, declared sizes, and exact EOF are verified |
| PICTools SSM module | `.ssm`, `SSM\0` header | Yes | Yes | Yes | Pegasus/Accusoft PICTools methods 3 and 5 with strict target-name, codec-id, DOS-time, packed-stream, and output-size validation |
| Amiga MI10 | `.mi`, `MI10` block chain | Yes | Yes | Yes | Independently decoded backward escape-LZ blocks, synthetic `N.bin` names, big-endian bounds, and byte-sum checksums |
| Rob Northen RNC / RNCA | `RNC\x01`, `RNC\x02`, `RNCA`, Bullfrog carrier | Yes | Yes | Yes | Verified RNC1/RNC2 single streams plus RNCA multi-file containers with RNC0 stored members |
| Eschalon ARCV | `.arv`, `ARCV` 1.10 header | Yes | Yes | Yes | Compact and stock adaptive-Huffman/LZSS variants (F=32 and F=60), selected and verified by the appropriate compressed- or plaintext-JAMCRC |
| Shell archive | `.shar`, Bourne-shell here-documents | Yes | Yes | Bounded | Bounded `cat`, prefix-stripping `sed`, and uuencoded here-document members; unsafe or shell-expanded output targets are skipped rather than executed |
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
| SSBOB slideshow package | `.fss`, `SSBOB` identity | Yes | Yes | Yes | Fixed playback-script prefix and 74-byte resource records; type-`0x10` embedded files are exposed as stored payloads |
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
| TCompress SPIS | `SPIS\x1A` stream header | Yes | Yes | Yes | Single-stream (caNormal) and multi-file (caMulti) blobs; Store, RLE, and LH5 (LHA -lh5-) members; 32-bit sum checksum verified; LZH1 members provisional; CUS (application-supplied codec) members are listed and fail closed |
| Turbo Packer | `TPWM` Amiga stream | Yes | Yes | Yes | Turbo Packer streams using the zero-prefilled 4 KiB history window, strict input/output bounds, and Ancient-backed verification |

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
| Inno Setup | Inno Setup executable | Yes | Yes | Yes | Installer records, including strict 1.09/1.11 legacy layouts; bounded raw-data fallback for ISX variants; ARC4-MD5, ARC4-SHA1, and PBKDF2-XChaCha20 protected payloads |
| NSIS | NSIS executable | Yes | Yes | Yes | Installer records and payloads |
| JASC installer archive | `SETUP.INF`, `*.CMP` | Yes | Yes | Yes | Pre-InstallShield JASC archives with exactly framed raw LHA `-lh5-` members |
| INSA installer data | Nameless `.DAT`, `01 00` header | Yes | Yes | Yes | Multi-record LZHUF (`-lh1-`) payload chain with synthetic `File_N.bin` names |
| InstallShield setup skin | `.skin`, `.isn` | Yes | Yes | Yes | Stored wizard-skin resources in strict name/decimal-size records, decoded with the fixed InstallShield XOR/nibble transform keyed by absolute file offset |
| GP-Install SFX | `.exe`, PE overlay with `SPIS\x1a` + `LH5` | Yes | Yes | Yes | Individually framed raw LHA `-lh5-` members; the format's byte-sum checksums are not exposed as CRC-32 |
| GP-Install / SPIS SFX | PE32 Delphi stub with SPIS overlay chain or TCOMPRESS resources | Yes | Yes | Yes | Overlay blob chain and resource-embedded streams; flat merged listing |
| InstallShield Setup Launcher | PE32/PE64 with an `InstallShield\0` overlay table | Yes | Yes | Yes | Stored launcher support files (language INIs, `instmsi*.exe`, `isscript.msi`, `*.mst` transforms, and `Setup.ini`); the product MSI is external |
| CopyQM screens executable | MSDOS MZ with a `TX` overlay | Yes | Yes | Yes | Sydex 1993-95 tools; decoded help, menu, and registration screens are exposed as `NN.txt` |
| FlashJester Jugglor SFX | Delphi PE32 stub, `A3 61 4A 6A` member headers and a 220-byte EOF trailer | Yes | Yes | Yes | Jugglor 2.x zlib members with stored source paths; the stub is never executed |
| PackageForTheWeb | PE32/PE64 InstallShield PackageForTheWeb stub | Yes | Yes | Yes | Overlay and `_cabinet`-section packages; embedded Microsoft cabinet members via XCab; obfuscated settings header decoded for metadata; Authenticode trailer and the newer opaque header flavor are skipped |
| Disk eXPress SFX (.exe) / raw DXP | `.exe`, `.dxp` | Yes | Yes | Yes | One stored-track diskette image; per-track LH1, LH5, or store compression; versions 1.x and 2.x |
| InstallShield Cabinet | `.cab`, `ISc(` proprietary cabinet identity | Yes | Yes | Bounded | InstallShield 5 and 6+ catalog media; stored and chunked raw-Deflate members, obfuscation, links, MD5 verification, and adjacent volumes; secondary volumes require the companion `DATA1.HDR` or catalog-bearing `DATA1.CAB` |
| InstallShield Setup Player 2K2 SFX | PE32 launcher with a stored name/path/version/decimal-size overlay chain | Yes | Yes | Yes | InstallShield 7.x setup media exposed with its `Disk1` directory hierarchy; every record and stored payload is range-checked and the overlay must end exactly at EOF; the stub is never executed |
| InstallShield 3 SFX | PE32 or NE launcher with a type-1024 `MYRESOURCE` descriptor | Yes | Yes | Yes | Obfuscated source-path records with DOS timestamps; stored payloads and nested single-entry Deflate ZIP payloads; multi-disk sets are exposed as one flat member list; the stub is never executed |
| InstallShield 3 archive | `.z`, `_SETUP.LIB`, `_SETUP.1` and later volumes | Yes | Yes | Yes | Native InstallShield 3 compressed archive records, including recursively extracted SFX members |
| WiX Burn | WiX v3/v4 bootstrapper | Yes | Yes | Bounded | Attached and UX containers with version-appropriate hash validation |
| 7-Zip SFX | PE or ELF wrapper with an embedded 7-Zip archive; official 7-Zip modules have their own attribution | Yes | Yes | Yes | Configured and plain self-contained 7-Zip payloads; external folder/filename streams are unsupported; the official identity remains preferred when its stub attribution is present, and the stub is never executed |
| WinRAR SFX | PE32 or PE64 WinRAR-attributed wrapper with an embedded RAR archive | Yes | Yes | Yes | Single-volume RAR payloads; official WinRAR attribution remains preferred over the generic RAR SFX identity; the stub is never executed |
| ZIP SFX | PE, ELF, DOS MZ/NE, Atari GEMDOS/EXEC, or DOS COM wrapper with an embedded ZIP archive | Yes | Yes | Yes | Single-disk, non-ZIP64 payloads with strict central/local validation and delegated extraction; overlay-bearing carriers search the unmapped suffix, while COM and Atari carriers use a bounded complete-image scan; the stub is never executed |
| RAR SFX | PE, ELF, DOS MZ/NE, Atari GEMDOS/EXEC, or DOS COM wrapper with an embedded RAR archive | Yes | Yes | Yes | Single-volume RAR payloads with a family-specific generic identity distinct from WinRAR attribution; overlay-bearing carriers search the unmapped suffix, while COM and Atari carriers use a bounded complete-image scan; the stub is never executed |
| CAB SFX | PE, ELF, DOS MZ/NE, Atari GEMDOS/EXEC, or DOS COM wrapper with an embedded Microsoft Cabinet archive | Yes | Yes | Yes | Non-spanned cabinet payloads with delegated extraction; overlay-bearing carriers search the unmapped suffix, while COM and Atari carriers use a bounded complete-image scan; the stub is never executed |
| LHA SFX | DOS MZ, Atari GEMDOS/EXEC, or DOS COM stub with an appended LHA/LZH archive | Yes | Yes | Yes | LHarc 1.13 DOS stubs and Atari `LHA's SFX` v3.x stubs; `lh0`, `lh1`, and `lh4`–`lh7` payloads plus LArc `lz5` via the contained Deark reader; the stub is never executed |
| ARC SFX | Executable wrapper with an embedded SEA ARC or PAK 2.51 archive | Yes | Yes | Bounded | Methods 2, 3, 4, 8, 9, 10 (PAK Crushed), and 11 (PAK Distilled) are delegated to the native reader; methods 5–7 fail closed and the stub is never executed |
| ARJ SFX | Executable wrapper with an embedded ARJ archive | Yes | Yes | Yes | Store, methods 1–4, and ARJ garble through the native ARJ reader; the stub is never executed |
| Crusher ARQ SFX | Executable wrapper with an embedded `gW` Crusher archive | Yes | Yes | Yes | Packed-stream CRC-32 validation and LH5-compatible member extraction through the native ARQ reader; the stub is never executed |
| Squeeze It SQZ SFX | Executable wrapper with an embedded `HLSQZ` chain | Yes | Yes | Yes | Store and SQZ methods 1–4 with CRC-32 verification through the native SQZ reader; the stub is never executed |
| RTPatch SFX | PE wrapper with an embedded RTPatch 5.00 package | Yes | Yes | Bounded | Version-5 whole-file streams extract through the native RTPatch reader; source-dependent binary deltas remain listed and fail closed, and the stub is never executed |
| GZIP SFX | WinImage-style PE/NE self-extractor with a gzip member in a resource | Yes | Yes | Yes | Resource-backed gzip members are bounded by their executable resource record and decoded as Deflate; the stub is never executed |
| KWAJ SFX | PE/NE self-extractor with KWAJ-bearing resource groups | Yes | Yes | Yes | The KWAJ-bearing resource type is enumerated in file order; compressed KWAJ members and companion stored resources are extracted from their exact resource extents, and the stub is never executed |
| SZDD SFX | Executable wrapper with one or more embedded MS SZDD streams | Yes | Yes | Yes | All SZDD streams in the bounded image, standard and legacy headers, mode A LZSS; names are recovered from the stub when unambiguous and otherwise index-derived; the stub is never executed |
| PyInstaller SFX | PyInstaller executable with an embedded CArchive | Yes | Yes | Bounded | CArchive and contained PYZ records are listed; extraction depends on supported contained codecs and encryption state, and the stub is never executed |
| Wise SFX | Wise installer executable | Yes | Yes | Yes | Native Wise installer records and payloads; the stub is never executed |
| Nullsoft PiMP | PE stub with a `PIMPFILE` overlay directory | Yes | Yes | Yes | Both header variants are located by a bounded record-chain scan; per-member zlib streams and literal `$INSTDIR`/`$VISDIR` path components are preserved; the post-install command is ignored and the stub is never executed |
| VISE SFX | Installer VISE executable | Yes | Bounded | Bounded | Registered VISE archive records are parsed when their embedded directory validates; unsupported variants fail closed and the stub is never executed |
| SoftPaq 1 SFX | Compaq SoftPaq 1 executable | Yes | Yes | Yes | Native SoftPaq 1 archive records and payloads; the stub is never executed |
| Instalit SFX | Instalit `.001` media and Instalit-Shadow `EXEFILE`-resource NE self-extractors | Yes | Yes | Yes | CRC-32-verified PKWARE DCL members from the NE `EXEFILE` resource set plus optional `[PVL]`/`[PVM]` payload members; the XOR-0x67 `[SCRIPT]` block is metadata and the stub is never executed |
| Setup Factory | PE32/PE64 Setup Factory 6 installer | Yes | Yes | Yes | Strict `E0..E7` overlay, MFC-serialized `irsetup.dat` manifest, and CRC-32-verified stored or PKWARE DCL payloads |
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
