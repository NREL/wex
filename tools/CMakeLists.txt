#####################################################################################################################
#
# Project Settings
#
#####################################################################################################################

include_directories($ENV{SSCDIR})

set(wexsandbox_SRC
        sandbox/sandbox.cpp)

file(TO_CMAKE_PATH "$ENV{LKDIR}" LKDIR)

set(DVIEW_SRC
        dview/dview.cpp
        ${LKDIR}/src/sqlite3.c
        )

#####################################################################################################################
#
# CMake Targets
#
#####################################################################################################################

# wexsandbox
add_executable(wexsandbox ${WEX_SRC} ${wexsandbox_SRC})
set_target_properties(wexsandbox
        PROPERTIES
        DEBUG_POSTFIX "d"
        )
if (MSVC)
    set_target_properties(wexsandbox
            PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS)
endif ()

target_include_directories(wexsandbox SYSTEM PRIVATE ../include ../src/freetype/include ${LKDIR}/include ${CURL_DIR}/include)

if (APPLE)
    # dview bundle
    set(DVIEW_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../build_resources/DView.icns)
    set_source_files_properties(${DVIEW_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(APPS "\${CMAKE_INSTALL_PREFIX}/DView.app")
elseif (WIN32)
    set(DVIEW_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../build_resources/dview.rc)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/DView.exe")
endif ()

add_executable(DView
        WIN32
        MACOSX_BUNDLE
        ${DVIEW_SRC}
        ${DVIEW_ICON})

target_include_directories(DView SYSTEM PRIVATE ../include ../src/freetype/include ${LKDIR}/include ${CURL_DIR}/include)

set_target_properties(DView
        PROPERTIES
        DEBUG_POSTFIX "d"
        )

#####################################################################################################################
#
# Link Libraries and Options
#
#####################################################################################################################

# DView
target_link_libraries(DView wex)
if (MSVC)
    target_link_libraries(DView Winhttp)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(DView ${CMAKE_DL_LIBS} ${FONTCONFIG_LIBRARY})
endif ()

# wex sandbox
if (${CMAKE_PROJECT_NAME} STREQUAL system_advisor_model)
    target_link_libraries(wexsandbox lk)
else ()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR MSVC)
        unset(LKD_LIB CACHE)
        find_library(LKD_LIB
                NAMES lkd.a lkd.lib
                PATHS $ENV{LKD_LIB} $ENV{LKDIR}/build $ENV{LKDIR}/build/Debug)
        target_link_libraries(wexsandbox debug ${LKD_LIB})
    endif ()
    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR MSVC)
        unset(LK_LIB CACHE)
        find_library(LK_LIB
                NAMES lk.a lk.lib
                PATHS $ENV{LK_LIB} $ENV{LKDIR}/build $ENV{LKDIR}/build/Release)
        target_link_libraries(wexsandbox optimized ${LK_LIB})
    endif()
endif ()
target_link_libraries(wexsandbox wex ${wxWidgets_LIBRARIES})

if (UNIX)
    target_link_libraries(wexsandbox -ldl -lcurl)
elseif (MSVC)
    find_library(CURL_LIB
            NAMES libcurl.lib
            PATHS ../build_resources/libcurl_ssl_x64/lib)
    target_link_libraries(wexsandbox ${CURL_LIB} Winhttp)
endif ()


#####################################################################################################################
#
# Target Installation
#
#####################################################################################################################

if (APPLE)
    install(TARGETS DView DESTINATION .)
    install(CODE "
		include(BundleUtilities)
		set(BU_CHMOD_BUNDLE_ITEMS true)
		fixup_bundle(\"${APPS}\" \"\" \"${wxWidgets_LIBRARY_DIRS}\")
	")
endif ()

if (MSVC)
    set(LIBCURL_FILES
            dbghelp.dll
            libcurl.dll
            libeay32.dll
            libssh2.dll
            msvcp140.dll
            openssl.exe
            ssleay32.dll
            vcruntime140.dll
            )
    set(WEXSB_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)

    foreach (file_i ${LIBCURL_FILES})
        file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build_resources/libcurl_ssl_x64/bin/${file_i} LIB_PATH)
        add_custom_command(
                TARGET wexsandbox
                POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                ARGS -E copy ${LIB_PATH} ${WEXSB_DIR}
        )
    endforeach (file_i)
endif ()
