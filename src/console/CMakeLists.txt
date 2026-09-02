set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DUSE_ARCHIVE)
add_definitions(-DUSE_XSIMD)
# No USE_DIE here: the console front end is XArchiveConsole and runs no
# signature scan. The GUI keeps its Detect It Easy integration.

if(WIN32)
    add_definitions(-DNOMINMAX)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options(/FS)
    add_compile_options($<$<CONFIG:Debug>:/Z7> $<$<CONFIG:RelWithDebInfo>:/Z7>)
endif()

# The console front end is XArchiveConsole, not XScanEngineConsole: this
# executable is an unpacker, and the scan-engine console would drag in the
# signature engine and its database handling for commands it never runs.
# XArchiveConsole carries the archive commands plus the XFormats-backed
# viewers (-i, -e, -S, -w), which need no scan engine.
include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XArchive/xarchiveconsole.cmake)

# Static unpackers (Inno Setup, NSIS, UPX, ...). Included after Formats/XArchive so its guards
# skip re-adding those; defines USE_STATICUNPACKER, enabling the packer/installer branches in
# XFormats::createClass / _getFileTypes.
if(NOT DEFINED XSTATICUNPACKER_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XStaticUnpacker/xstaticunpacker.cmake)
endif()

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
endif()

add_executable(xfileunpackerc
    ${XARCHIVECONSOLE_SOURCES}
    ${XSTATICUNPACKER_SOURCES}
    main_console.cpp
)

target_include_directories(xfileunpackerc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../dep/Controls
)

target_link_libraries(xfileunpackerc PRIVATE
    xsimd
    capstone_x86
    bzip2
    lzma
    zlib
    ppmd
    Qt${XFILEUNPACKER_QT_MAJOR}::Core
    Qt${XFILEUNPACKER_QT_MAJOR}::Concurrent
    ${XFILEUNPACKER_COMPAT_LIBS}
)

if(WIN32)
    target_link_libraries(xfileunpackerc PRIVATE Wintrust Crypt32)
endif()

if(TARGET xfileunpacker_translations)
    add_dependencies(xfileunpackerc xfileunpacker_translations)
endif()

if(UNIX AND NOT APPLE)
    install(TARGETS xfileunpackerc RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
else()
    install(TARGETS xfileunpackerc RUNTIME DESTINATION ".")
endif()
