# U3 compatibility

XFU contains native C++/Qt translations of selected U3 archive handlers and an
explicit U3 command adapter. Full capability parity is still incomplete. The
recovered executable contains 810 archive entries, 162 SFX entries and 41
filesystem entries, plus shared FAT12/16/32 branches. Detection, an archive
name in a menu, or exporting `disk.raw` does not establish file extraction.

The complete per-entry inventory is in [U3_COVERAGE.csv](U3_COVERAGE.csv).
`verified_subset` means that the stated samples/methods have matching output;
it does not mean that every U3 variant or workflow is supported.

## Commands

Start the positional compatibility grammar with `--u3`:

```text
xfileunpackerc --u3 type archive
xfileunpackerc --u3 list archive
xfileunpackerc --u3 test archive
xfileunpackerc --u3 testlist archive
xfileunpackerc --u3 x archive output
xfileunpackerc --u3 x2 "inputs/*" output
xfileunpackerc --u3 x3 "inputs/*" output
xfileunpackerc --u3 help
```

The default command is extraction and the default destination is `./Output`.
Aliases, case-insensitive commands and a single `-` or `/` command prefix are
accepted. Batch output directory names retain each archive's extension. `x2`
also preserves relative input directories; `x3` renames collisions with
`(0)name` through `(999)name`. `x` and `x2` preserve existing files. Interactive
U3 overwrite prompts are not implemented.

`sort` and `testsort` **move source archives** into `destination/type/`;
`testsort` verifies an archive before moving it. Existing names receive `.1`,
`.2` and later suffixes. Sorting operates only on recognized archives.

On Windows, U3 applies the same wildcard to files and directories. `*.zip`
therefore descends only into directories whose names also match `*.zip`;
use `*` for a full recursive traversal. Paths are collected before writing or
moving. Nested output directories are excluded from that traversal.

Output formatting, type names, exit codes and recognition ordering remain
XFU's. Positional `--u3` commands do not accept arbitrary native options.

## Files inside disks

Native CLI operations accept `--filesystem`:

```text
xfileunpackerc --filesystem --listarchive volume.vhd
xfileunpackerc --filesystem --extractarchive output volume.vhd
```

U3 commands enable this option automatically. Native operations retain raw
disk export as their default. The GUI's **Files inside disk (NTFS)** option
uses the same choice for listing, selected-file access, verification and
extraction. Raw NTFS volumes are opened as filesystems directly.

The reader supports NTFS user files in raw volumes and primary type-7 MBR
partitions, including resident, fragmented, sparse and uninitialized-tail
data. The allocation view can read supported virtual disks without expanding
the entire disk merely to list its files. It exports unnamed DATA and follows
U3's filename preference and reserved-metadata filtering. A volume containing
only reserved metadata can correctly contain zero exported user files.

GPT, extended/mixed partitions, other guest filesystems, attribute-list
continuations, reparse/provider data, compressed/encrypted NTFS files and
alternate streams are outside the current supported subset. Unsupported
guest mode fails; it does not silently return `disk.raw`.

## Newly translated handlers

| EXE entry | Native implementation | Verified scope and limits |
|---|---|---|
| Git Object | `XGitObjectArchive` | Blob zlib stream, declared size and optional object-name SHA1; commit/tree/tag and packs excluded |
| ALZ | `XAlzArchive` | STORE and raw DEFLATE with CRC32; method 1 and encryption excluded |
| PMA SFX | `XLHA`, `XSFX` | Exact PMS envelope and existing PM0/PM1/PM2 codecs; nonempty archives with bounded CP/M padding |
| RZIP | `XRzipArchive`, `XU3RzipDecoder` | v2.0/v2.1 STORE/BZIP2 chunks, overlapping copies and CRC; bounded input/output |
| CHM | `XChmArchive` | ITSF v2/v3, plain and LZXC v2 resources, U3 internal-resource filtering |
| HOG2 | `XHOG2` | Descent 3 adjacent table and STORE payloads; distinct from Descent 1/2 HOG |
| PBO | `XPboArchive` | STORE/Cprs, optional Vers properties and SHA1 footer; encryption and Elite trailer excluded |
| ARC | `XPakDecoder` | Corrected method 11 Distilled tree and match limits; method 10 already worked |
| SFX BZIP2 | `XBzip2SFX` | BZIP2 streams in supported executable carriers |
| NTFS | `XNTFSArchive` | Bounded user-file extraction and virtual-disk composition described above |

Earlier translated handlers include ARC methods 5/6/7, CP/M Crunch, CPMLZH,
Unix Compact, SQLite text export and VHDX allocation mapping. Source-level
address maps and format-specific restrictions accompany the readers in
`XArchive/U3_HANDLER_PORTS.md` and the individual `*.PROVENANCE.md` files.
These native handlers do not invoke the old U3 executable at runtime.

## Direct U3 sample compatibility (2026-09-05)

The direct U3 comparison led to native fixes for multipart and solid RAR,
Copy/BCJ2 and multipart 7z, empty 7z, DOS0 ADF geometry and classic ACE STORE.
FreeArc control blocks and the observed STORE/LZMA/REP/EXE/DELTA chains now
decode in process. The optional external FreeArc helper remains available for
unsupported method chains; malformed data and resource failures do not fall
back to it. See the format-specific source provenance for parameter limits.

To extract a UU/base64 wrapper's named payload, use:

```text
xfileunpackerc --transport-only --extractarchive output archive.uu
```

`--u3` commands enable this behavior automatically. Ordinary native operations
retain their existing nested-archive view; an empty nested archive keeps its
named outer payload. RAR link targets are exported as
text streams, not restored filesystem links. An incomplete later multipart
volume reports unavailable preceding content and extracts accessible members.

The full build, saved original-U3 byte comparisons and repeat corpus results
are retained in `F:/ownCloud/_build/xfileunpacker/u3-parity/u3-observed-port/RESULTS.md`.
