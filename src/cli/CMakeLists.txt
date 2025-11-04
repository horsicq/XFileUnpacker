# CLI Application CMakeLists.txt
set(TARGET_NAME xfuc)

add_definitions(-DUSE_DEX)
add_definitions(-DUSE_PDF)
add_definitions(-DUSE_ARCHIVE)

if(WIN32)
    add_definitions(-DNOMINMAX)
endif()

# Add 3rd party libraries
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../dep/XArchive/3rdparty/bzip2 ${CMAKE_BINARY_DIR}/3rdparty/bzip2)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../dep/XArchive/3rdparty/lzma ${CMAKE_BINARY_DIR}/3rdparty/lzma)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../dep/XArchive/3rdparty/zlib ${CMAKE_BINARY_DIR}/3rdparty/zlib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../dep/XArchive/3rdparty/ppmd ${CMAKE_BINARY_DIR}/3rdparty/ppmd)

# Include XFormats and XArchive
include(${CMAKE_CURRENT_LIST_DIR}/../../dep/Formats/xformats.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XArchive/xarchives.cmake)

set(SOURCES
    main_cli.cpp
    ${XFORMATS_SOURCES}
    ${XARCHIVES_SOURCES}
)

set(HEADERS
)

add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Link Qt5
if(Qt6_FOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core)
else()
    target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE bzip2)
target_link_libraries(${TARGET_NAME} PRIVATE lzma)
target_link_libraries(${TARGET_NAME} PRIVATE zlib)
target_link_libraries(${TARGET_NAME} PRIVATE ppmd)

if(WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE Wintrust)
    target_link_libraries(${TARGET_NAME} PRIVATE Crypt32)
endif()

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Installation
install(TARGETS ${TARGET_NAME} DESTINATION bin)
