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

set(XFILEUNPACKER_7ZIP_NOTICE_DIR
    # The vendored 7-Zip tree was folded into the Algos amalgamations; its three
# shipped license texts were relocated so the deployed licenses/7-Zip folder
# stays byte-identical.
    "${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive/Algos/licenses/7-Zip")
set(XFILEUNPACKER_LIBARCHIVE_DERIVED_NOTICE
    "${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive/Algos/licenses/libarchive-derived/NOTICE.txt")
set(XFILEUNPACKER_LZ4_NOTICE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive/Algos/licenses/LZ4")
# The zstd sources are Algos amalgamations now, but the decoder is still shipped,
# so its BSD/GPLv2 texts moved alongside the others and keep deploying unchanged.
set(XFILEUNPACKER_ZSTD_NOTICE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive/Algos/licenses/zstd")
# bzip2's license was the one that lived in a plain LICENSE file rather than a
# source header, so it was not carried across with the amalgamation and shipped
# nowhere for a while. brotli (Algos/brotlideclib.cpp, ~886 KB, compiled
# unconditionally) had never been attributed at all -- the amalgamator strips
# comments, which removed the upstream MIT header.
set(XFILEUNPACKER_BZIP2_NOTICE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive/Algos/licenses/bzip2")
set(XFILEUNPACKER_BROTLI_NOTICE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive/Algos/licenses/brotli")
# The 7-Zip ZS native decoder sources are now one amalgamated translation unit
# (_mylibs/XArchive/Algos/lz5lizarddeclib.cpp) with no license directory of its
# own; the LZ5, Lizard, xxHash and Zstandard texts are reproduced inline in
# THIRD_PARTY_NOTICES.md, which is deployed and installed below.
set(XFILEUNPACKER_THIRD_PARTY_NOTICES
    "${CMAKE_CURRENT_LIST_DIR}/../THIRD_PARTY_NOTICES.md")

function(xfileunpacker_deploy_third_party_notices target_name)
    if(EXISTS "${XFILEUNPACKER_THIRD_PARTY_NOTICES}")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_THIRD_PARTY_NOTICES}"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/THIRD_PARTY_NOTICES.md"
            VERBATIM
        )
    endif()

    if(EXISTS "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/License.txt"
       AND EXISTS "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/copying.txt"
       AND EXISTS "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/unRarLicense.txt")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses/7-Zip"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/License.txt"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/7-Zip/License.txt"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/copying.txt"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/7-Zip/copying.txt"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/unRarLicense.txt"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/7-Zip/unRarLicense.txt"
            VERBATIM
        )
    endif()

    if(EXISTS "${XFILEUNPACKER_LIBARCHIVE_DERIVED_NOTICE}")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses/libarchive-derived"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_LIBARCHIVE_DERIVED_NOTICE}"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/libarchive-derived/NOTICE.txt"
            VERBATIM
        )
    endif()

    if(EXISTS "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE"
       AND EXISTS "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE-lib")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses/LZ4"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/LZ4/LICENSE"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE-lib"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/LZ4/LICENSE-lib"
            VERBATIM
        )
    endif()

    if(EXISTS "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/LICENSE"
       AND EXISTS "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/COPYING")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses/zstd"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/LICENSE"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/zstd/LICENSE"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/COPYING"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/zstd/COPYING"
            VERBATIM
        )
    endif()

    if(EXISTS "${XFILEUNPACKER_BZIP2_NOTICE_DIR}/LICENSE")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses/bzip2"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_BZIP2_NOTICE_DIR}/LICENSE"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/bzip2/LICENSE"
            VERBATIM
        )
    endif()

    if(EXISTS "${XFILEUNPACKER_BROTLI_NOTICE_DIR}/LICENSE")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/licenses/brotli"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XFILEUNPACKER_BROTLI_NOTICE_DIR}/LICENSE"
                "$<TARGET_FILE_DIR:${target_name}>/licenses/brotli/LICENSE"
            VERBATIM
        )
    endif()

    # The LZ5 / Lizard / xxHash / Zstandard texts for the amalgamated 7-Zip ZS
    # decoder are carried inline in THIRD_PARTY_NOTICES.md, deployed above.
endfunction()

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
        "${CMAKE_CURRENT_LIST_DIR}/../translations/xfileunpacker_sq_AL.ts"
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
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dep/XArchive" XArchive)

option(BUILD_GUI "Build the GUI application" ON)
if(BUILD_GUI)
    add_subdirectory(gui)
endif()
add_subdirectory(console)

if(EXISTS "${XFILEUNPACKER_THIRD_PARTY_NOTICES}")
    install(FILES "${XFILEUNPACKER_THIRD_PARTY_NOTICES}"
        DESTINATION licenses
    )
endif()

if(EXISTS "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/License.txt"
   AND EXISTS "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/copying.txt"
   AND EXISTS "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/unRarLicense.txt")
    install(FILES
        "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/License.txt"
        "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/copying.txt"
        "${XFILEUNPACKER_7ZIP_NOTICE_DIR}/unRarLicense.txt"
        DESTINATION licenses/7-Zip
    )
endif()

if(EXISTS "${XFILEUNPACKER_LIBARCHIVE_DERIVED_NOTICE}")
    install(FILES "${XFILEUNPACKER_LIBARCHIVE_DERIVED_NOTICE}"
        DESTINATION licenses/libarchive-derived
    )
endif()

if(EXISTS "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE"
   AND EXISTS "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE-lib")
    install(FILES "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE"
        DESTINATION licenses/LZ4
    )
    install(FILES "${XFILEUNPACKER_LZ4_NOTICE_DIR}/LICENSE-lib"
        DESTINATION licenses/LZ4
        RENAME LICENSE-lib
    )
endif()

if(EXISTS "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/LICENSE"
   AND EXISTS "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/COPYING")
    install(FILES
        "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/LICENSE"
        "${XFILEUNPACKER_ZSTD_NOTICE_DIR}/COPYING"
        DESTINATION licenses/zstd
    )
endif()

if(EXISTS "${XFILEUNPACKER_BZIP2_NOTICE_DIR}/LICENSE")
    install(FILES "${XFILEUNPACKER_BZIP2_NOTICE_DIR}/LICENSE"
        DESTINATION licenses/bzip2
    )
endif()

if(EXISTS "${XFILEUNPACKER_BROTLI_NOTICE_DIR}/LICENSE")
    install(FILES "${XFILEUNPACKER_BROTLI_NOTICE_DIR}/LICENSE"
        DESTINATION licenses/brotli
    )
endif()

# No separate licenses/7-Zip-ZS-native install: the LZ5, Lizard, xxHash and
# Zstandard texts for the amalgamated decoder are reproduced inline in
# THIRD_PARTY_NOTICES.md, installed above.
