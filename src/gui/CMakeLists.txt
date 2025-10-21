# GUI Application CMakeLists.txt
set(TARGET_NAME XFileUnpacker-GUI)

set(SOURCES
    main_gui.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

set(FORMS
    mainwindow.ui
)

set(RESOURCES
    resources.qrc
)

add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

if(Qt6_FOUND)
    target_link_libraries(${TARGET_NAME}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )
else()
    target_link_libraries(${TARGET_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )
endif()

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Installation
install(TARGETS ${TARGET_NAME} DESTINATION bin)
