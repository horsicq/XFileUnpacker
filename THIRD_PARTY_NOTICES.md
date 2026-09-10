# Third-party notices

## Public-domain LZMA SDK codecs

XFileUnpacker retains the LzmaDec, Lzma2Dec, Ppmd7, and Ppmd8 decoder units
from LZMA SDK 26.01 for its own native archive readers. These units are public
domain and carry their upstream Igor Pavlov, Dmitry Shkarin, and Dmitry
Subbotin notices directly in the amalgamated source files. The 7-Zip
application, archive-handler backend, and LGPL/unRAR-restricted units are not
included.

## 7-Zip ZS-derived native compatibility

XFileUnpacker includes selected decoder sources and compatibility behavior
derived from [7-Zip ZS](https://github.com/mcmilk/7-Zip-zstd):

- Upstream commit: `bde380e6ca6fa783adf9462da955f6fab2fabcb2`
- XArchive C++ decoder source: `_mylibs/XArchive/Algos/lz5lizarddeclib.cpp`
  (single amalgamated translation unit; section markers inside name the
  original upstream file each block came from)

The source-derived additions provide native XArchive decoding for LZ5 1.5 and
Lizard 2.1 frames, including the 7-Zip ZS multithread frame envelope and every
valid frame block-size ID from 1 through 7. Large frame buffers are allocated
only when an actual data block needs them. They also add compatibility for the
fork's Zstandard, Brotli, LZ4, LZ5, and Lizard 7z coder IDs, its Brotli/LZ4
frame envelopes, Brotli large-window streams produced with `long` greater than
24, concatenated and skippable Zstandard frames, Zstandard v0.4-v0.7 legacy
streams, Zstandard-compressed NSIS payloads, and LZ4-compressed SquashFS data.
The legacy support compiles the versioned
`zstd_v04.c` through `zstd_v07.c` decoders and Zstandard-namespaced xxHash from
the matching vendored Zstandard 1.5.7 source. The fork recognizes v0.1-v0.3
magic values, but the `ZSTD_decompressStream()` adapter actually used by its
archive decoder explicitly reports those versions as unsupported; XFileUnpacker
therefore does not claim v0.1-v0.3 decoding. Only the behavior needed by
XArchive was imported and rewritten as C++ translation units under `Algos`;
the 7-Zip ZS application and backend are not included.

The LZ5 and Lizard libraries and xxHash 0.8.3 are licensed under the BSD
2-Clause License. xxHash is Copyright (C) 2012-2023 Yann Collet. The supporting
Zstandard entropy sources used by the Lizard decoder are dual-licensed under the
BSD 3-Clause License or GPL-2.0; XFileUnpacker elects the BSD option, so only
that text is reproduced here. The GPL-2.0 alternative is available upstream at
<https://github.com/facebook/zstd/blob/dev/COPYING>.

The exact upstream license texts follow.

These decoder sources are compiled directly into XArchive. XFileUnpacker does
not ship, execute, or dynamically load a 7-Zip ZS executable or DLL.

#### LZ5

```
LZ4 Library
Copyright (c) 2011-2015, Yann Collet
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

#### Lizard

```
Lizard Library
Copyright (C) 2011-2016, Yann Collet.
Copyright (C) 2016-2017, Przemyslaw Skibinski <inikep@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

#### xxHash

```
xxHash - Extremely Fast Hash algorithm
Header File
Copyright (C) 2012-2023 Yann Collet

BSD 2-Clause License (https://www.opensource.org/licenses/bsd-license.php)

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following disclaimer
     in the documentation and/or other materials provided with the
     distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You can contact the author at:
  - xxHash homepage: https://www.xxhash.com
  - xxHash source repository: https://github.com/Cyan4973/xxHash
```

#### Zstandard (entropy sources used by the Lizard decoder)

```
BSD License

For Zstandard software

Copyright (c) Meta Platforms, Inc. and affiliates. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name Facebook, nor Meta, nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## Focused libarchive-derived readers

XFileUnpacker does not build or link libarchive.  The previously generic
libarchive backend was removed.  Only capabilities missing from XArchive were
implemented as native XArchive code, based on
the reader behavior in [libarchive](https://www.libarchive.org/) 3.8.9:

- WARC records
- safe, self-contained mtree manifests
- UU and begin-base64 transport decoding
- AFIO large-ASCII CPIO headers
- native compression-filter stacking around existing archive readers,
  including compressed CPIO/WARC/XAR inputs and RPM-to-CPIO delegation
- LZ4-filtered archives and TAR.LZ4 (decoded by the separately licensed LZ4
  source described below)

The applicable BSD notices are installed as
`licenses/libarchive-derived/NOTICE.txt`.

## LZ4

XFileUnpacker compiles the decoder-capable library sources from
[LZ4](https://github.com/lz4/lz4) directly into both applications.

- Version: 1.10.0
- Vendored source: `_mylibs/XArchive/Algos/lz4declib.cpp`

The LZ4 library is licensed under the BSD 2-Clause License. The project-level
licensing summary and library license are installed under `licenses/LZ4`:

- `LICENSE`
- `LICENSE-lib`

The GPL-licensed LZ4 command-line program is not built.

## Zstandard

XFileUnpacker compiles the decoder-only single-file Zstandard implementation
`_mylibs/XArchive/Algos/zstddeclib.cpp` directly into both applications.  Its
embedded version macro is 1.6.0.

The Zstandard library is dual-licensed under the BSD 3-Clause License or
GPL-2.0. XFileUnpacker uses the BSD-licensed option. Both upstream license
alternatives are installed under `licenses/zstd`:

- `LICENSE`
- `COPYING`

No Zstandard library target, DLL, command-line program, tests, or contributed
tools are built.

## bzip2

XFileUnpacker compiles the bzip2 decompression sources
(`_mylibs/XArchive/Algos/bzip2declib.cpp`) directly into both applications.

- Upstream: [bzip2](https://sourceware.org/bzip2/)
- Vendored source: `_mylibs/XArchive/Algos/bzip2declib.cpp`

bzip2 is licensed under a BSD-style license. The upstream license text is
installed under `licenses/bzip2`:

- `LICENSE`

## Brotli

XFileUnpacker compiles the Brotli decoder
(`_mylibs/XArchive/Algos/brotlideclib.cpp`, included by
`_mylibs/XArchive/Algos/xbrotlidecoder.cpp`) directly into both applications.

- Upstream: [google/brotli](https://github.com/google/brotli)
- Vendored source: `_mylibs/XArchive/Algos/brotlideclib.cpp`

Brotli is licensed under the MIT License. The upstream license text is
installed under `licenses/brotli`:

- `LICENSE`

The Brotli encoder, command-line program and tests are not built.
