set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DUSE_DEX)
add_definitions(-DUSE_PDF)
add_definitions(-DUSE_ARCHIVE)
add_definitions(-DUSE_XSIMD)

if(WIN32)
    add_definitions(-DNOMINMAX)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options(/FS)
    add_compile_options($<$<CONFIG:Debug>:/Z7> $<$<CONFIG:RelWithDebInfo>:/Z7>)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XScanEngine/xscanengineconsole.cmake)

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
endif()

add_executable(xfileunpackerc
    ${XSCANENGINECONSOLE_SOURCES}
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
