if(TARGET Qt6::Core OR TARGET Qt6::Widgets)
    set(XFILEUNPACKER_QT_MAJOR 6)
elseif(TARGET Qt5::Core OR TARGET Qt5::Widgets)
    set(XFILEUNPACKER_QT_MAJOR 5)
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
    set(XFILEUNPACKER_QT_MAJOR ${QT_VERSION_MAJOR})
endif()

if(XFILEUNPACKER_QT_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS Widgets Concurrent Network PrintSupport Script ScriptTools LinguistTools OpenGL Svg)
    set(XFILEUNPACKER_SCRIPT_LIBS Qt5::Script Qt5::ScriptTools)
elseif(XFILEUNPACKER_QT_MAJOR GREATER_EQUAL 6)
    find_package(Qt${XFILEUNPACKER_QT_MAJOR} REQUIRED COMPONENTS Widgets Concurrent Network PrintSupport Qml LinguistTools OpenGL Svg)
    set(XFILEUNPACKER_SCRIPT_LIBS Qt${XFILEUNPACKER_QT_MAJOR}::Qml)

    if(NOT TARGET Qt5::Widgets)
        add_library(Qt5::Widgets INTERFACE IMPORTED)
        set_property(TARGET Qt5::Widgets PROPERTY
            INTERFACE_LINK_LIBRARIES Qt${XFILEUNPACKER_QT_MAJOR}::Widgets)
    endif()
endif()

file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/../release_version.txt X_PROJECT_VERSION)
project(XFileUnpacker VERSION ${X_PROJECT_VERSION} LANGUAGES CXX)

set(X_COMPANYNAME    "ntinfo")
set(X_PROJECTNAME    "XFileUnpacker")
set(X_MAINTAINER     "horsicq@gmail.com")
set(X_DESCRIPTION    "XFileUnpacker is a file unpacker")
set(X_HOMEPAGE       "http://ntinfo.biz")
set(X_ORIGINAL_FILENAME "xfileunpacker")

if(NOT DEFINED MACOSX_BUNDLE_GUI_IDENTIFIER OR "${MACOSX_BUNDLE_GUI_IDENTIFIER}" STREQUAL "")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.ntinfo.xfileunpacker")
endif()
set(BUNDLE_ID_OPTION "${MACOSX_BUNDLE_GUI_IDENTIFIER}")

set(X_RESOURCE_DIR               "${CMAKE_CURRENT_LIST_DIR}/../res")
set(X_WINDOWS_ICON               "${X_RESOURCE_DIR}/windows/main.ico")
set(X_WINDOWS_RESOURCE_RC_IN     "${X_RESOURCE_DIR}/windows/resource.rc.in")
set(X_WINDOWS_RESOURCE_ICON_RC_IN "${X_RESOURCE_DIR}/windows/resource_icon.rc.in")
set(X_WINDOWS_RESOURCE_RC        "${X_RESOURCE_DIR}/windows/resource.rc")
set(X_WINDOWS_RESOURCE_ICON_RC   "${X_RESOURCE_DIR}/windows/resource_icon.rc")
set(X_MACOS_ICON                 "${X_RESOURCE_DIR}/macos/main.icns")
set(X_MACOS_INFO_PLIST_IN        "${X_RESOURCE_DIR}/macos/Info.plist.in")
set(X_MACOS_INFO_PLIST           "${X_RESOURCE_DIR}/macos/Info.plist")

set(XFILEUNPACKER_DEPLOY_INIT "${CMAKE_CURRENT_LIST_DIR}/../dep/build_tools/cmake/deploy_init.cmake")
if(EXISTS "${XFILEUNPACKER_DEPLOY_INIT}"
   AND EXISTS "${X_WINDOWS_RESOURCE_RC_IN}"
   AND EXISTS "${X_WINDOWS_RESOURCE_ICON_RC_IN}"
   AND EXISTS "${X_MACOS_ICON}"
   AND EXISTS "${X_MACOS_INFO_PLIST_IN}")
    include("${XFILEUNPACKER_DEPLOY_INIT}")
else()
    message(STATUS "Skipping deploy_init.cmake — packaging templates missing")
endif()

set(XFILEUNPACKER_TRANSLATION_HELPER "${CMAKE_CURRENT_LIST_DIR}/../dep/build_tools/cmake/deploy_translation.cmake")
if(EXISTS "${XFILEUNPACKER_TRANSLATION_HELPER}")
    include("${XFILEUNPACKER_TRANSLATION_HELPER}")

    set(XFILEUNPACKER_TS_FILES
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_ar_AR.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_bn_BN.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_de_DE.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_es_ES.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_fa_FA.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_fr_FR.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_he_IL.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_id_ID.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_it_IT.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_ja_JP.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_ko_KR.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_pl_PL.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_pt_BR.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_pt_PT.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_ru_RU.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_sv_SE.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_tr_TR.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_uk_UA.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_vi_VN.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_zh_CN.ts"
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_zh_TW.ts"
    )

    deploy_add_translations(
        TARGET_NAME xfileunpacker_translations
        ADD_TO_ALL
        SOURCE_DIRS "${CMAKE_CURRENT_LIST_DIR}"
        TS_FILES ${XFILEUNPACKER_TS_FILES}
        INSTALL_DESTINATION lang
        WINDOWS_APPDATA_SUBDIR "${X_PROJECTNAME}"
    )
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dep/Formats/xsimd" XSIMD)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dep/XCapstone" XCapstone)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dep/XCapstone/x86" XCapstone_86)

option(BUILD_GUI "Build the GUI application" ON)
if(BUILD_GUI)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dep/XCppfilt" XCppfilt)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dep/XYara/3rdparty/yara" yara)
    add_subdirectory(gui)
endif()
add_subdirectory(console)
