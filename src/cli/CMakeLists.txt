# CLI Application CMakeLists.txt
set(TARGET_NAME XFileUnpacker-CLI)

set(SOURCES
    main_cli.cpp
)

set(HEADERS
)

add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
)

if(Qt6_FOUND)
    target_link_libraries(${TARGET_NAME}
        Qt6::Core
    )
else()
    target_link_libraries(${TARGET_NAME}
        Qt5::Core
    )
endif()

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Installation
install(TARGETS ${TARGET_NAME} DESTINATION bin)
