include(../../dep/build_tools/cmake/cpp_standart_setup.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/../../dep/FormatWidgets/XFileExplorer/xfileexplorer.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XAboutWidget/xaboutwidget.cmake)
if(NOT DEFINED XOPTIONS_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XOptions/xoptions.cmake)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/../../dep/XStyles/xstyles.cmake)

find_package(Threads REQUIRED)

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
endif()

set(XFILEUNPACKER_GUI_SOURCES
    ${XFILEEXPLORER_SOURCES}
    ${XABOUTWIDGET_SOURCES}
    ${XOPTIONS_SOURCES}
    ${XSTYLES_SOURCES}
    dialogabout.cpp
    dialogabout.h
    dialogabout.ui
    guimainwindow.cpp
    guimainwindow.h
    guimainwindow.ui
    main_gui.cpp
    "${CMAKE_CURRENT_LIST_DIR}/../../res/resources_gui.qrc"
)

if(WIN32 AND EXISTS "${X_WINDOWS_RESOURCE_ICON_RC}")
    list(APPEND XFILEUNPACKER_GUI_SOURCES "${X_WINDOWS_RESOURCE_ICON_RC}")
endif()

if(APPLE AND EXISTS "${X_MACOS_ICON}")
    set_source_files_properties("${X_MACOS_ICON}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    list(APPEND XFILEUNPACKER_GUI_SOURCES "${X_MACOS_ICON}")
endif()

add_executable(XFileUnpacker WIN32
    ${XFILEUNPACKER_GUI_SOURCES}
)

set_target_properties(XFileUnpacker PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE    TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "${MACOSX_BUNDLE_GUI_IDENTIFIER}"
    OUTPUT_NAME xfileunpacker
)

target_include_directories(XFileUnpacker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(XFileUnpacker PRIVATE
    Qt${XFILEUNPACKER_QT_MAJOR}::Widgets
    Qt${XFILEUNPACKER_QT_MAJOR}::Concurrent
    Qt${XFILEUNPACKER_QT_MAJOR}::Network
    Qt${XFILEUNPACKER_QT_MAJOR}::PrintSupport
    Qt${XFILEUNPACKER_QT_MAJOR}::Svg
    ${XFILEUNPACKER_SCRIPT_LIBS}
    Threads::Threads
)

if(WIN32)
    target_link_libraries(XFileUnpacker PRIVATE Wintrust Crypt32)
endif()

if(TARGET xfileunpacker_translations)
    add_dependencies(XFileUnpacker xfileunpacker_translations)
endif()

if(UNIX AND NOT APPLE)
    set(XFILEUNPACKER_RUNTIME_DEST "${CMAKE_INSTALL_BINDIR}")
    set(XFILEUNPACKER_DOC_DEST     "${CMAKE_INSTALL_DOCDIR}/xfileunpacker")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../../res/linux/xfileunpacker.desktop.in"
        "${CMAKE_CURRENT_BINARY_DIR}/xfileunpacker.desktop"
        @ONLY
    )
else()
    set(XFILEUNPACKER_RUNTIME_DEST ".")
    set(XFILEUNPACKER_DOC_DEST     ".")
endif()

install(TARGETS XFileUnpacker
    RUNTIME DESTINATION ${XFILEUNPACKER_RUNTIME_DEST}
    BUNDLE  DESTINATION .
)

if(UNIX AND NOT APPLE)
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../README.md"
        DESTINATION ${XFILEUNPACKER_DOC_DEST}
        OPTIONAL
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/xfileunpacker.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../res/linux/hicolor/"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor")
else()
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE"
        DESTINATION . RENAME LICENSE.txt OPTIONAL)
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../res/license.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../res/readme.txt"
        "${X_WINDOWS_ICON}"
        DESTINATION . OPTIONAL)
    install(CODE [[file(WRITE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/portable" "")]])
endif()

if(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/../../dep/build_tools/cmake/macdeployqt.cmake)
endif()

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/../../dep/build_tools/cmake/deploy_qt_windows.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/../../dep/build_tools/cmake/deploy_msvc.cmake)
endif()
