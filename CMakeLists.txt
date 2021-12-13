#####################################################################################################################
#
# CMake and System Settings
#
#####################################################################################################################

cmake_minimum_required(VERSION 3.11)

option(SAM_SKIP_TOOLS "Skips the wex sandbox and Dview builds" OFF)

#
# If project isn't system_advisor_model and SAM_SKIP_TOOLS=1,
#   environment vars LK_LIB and LKD_LIB can be used to specify where to find those libraries
#

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version")
endif ()

if (UNIX AND NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT APPLE)
    set(CURL_DIR build_resources/libcurl_ssl_x64)
endif ()

Project(wex C CXX)


#####################################################################################################################
#
# Compile Options per Platform
#
#####################################################################################################################

if (MSVC)
    add_compile_options(/W3 /MP)
    add_compile_definitions(LK_USE_WXWIDGETS _UNICODE _MBCS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NON_CONFORMING_SWPRINTFS=1 _SCL_SECURE_NO_WARNINGS=1 __WXMSW__ NOPCH FT2_BUILD_LIBRARY OPJ_STATIC FT_CONFIG_MODULES_H=\"slimftmodules.h\" FT_CONFIG_OPTIONS_H=\"slimftoptions.h\")
    foreach (flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
        set(${flag_var} "${${flag_var}} /D_DEBUG" CACHE STRING "compile flags" FORCE)
    endforeach ()
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    if (APPLE)
        add_compile_options(-arch x86_64)
    endif ()
    add_compile_options(-Wall -Wno-unknown-pragmas -Wno-strict-aliasing)
    add_compile_options(-fno-common ${wxWidgets_CXX_FLAGS})
    add_definitions(-DLK_USE_WXWIDGETS -D_CRT_SECURE_NO_WARNINGS -D_MBCS -DFT2_BUILD_LIBRARY -DOPJ_STATIC -DFT_CONFIG_MODULES_H=\"slimftmodules.h\" -DFT_CONFIG_OPTIONS_H=\"slimftoptions.h\")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_DEBUG)
    else ()
        add_compile_options(-O3)
    endif ()
endif ()


#####################################################################################################################
#
# Packages
#
#####################################################################################################################

# setup wxWidgets
if (UNIX)
    if (EXISTS /usr/local/bin/wx-config-3)
        set(wxWidgets_CONFIG_EXECUTABLE /usr/local/bin/wx-config-3)
        find_package(wxWidgets REQUIRED xrc stc richtext ribbon propgrid aui gl html qa adv core xml net base)
    else ()
        set(wxWidgets_CONFIG_EXECUTABLE $ENV{WXMSW3}/bin/wx-config)
        find_package(wxWidgets REQUIRED xrc stc richtext ribbon propgrid aui gl html qa adv core xml net base)
    endif ()
else ()
    set(wxWidgets_ROOT_DIR $ENV{WXMSW3})
    find_package(wxWidgets REQUIRED xrc stc richtext ribbon propgrid aui gl html qa adv core xml net base scintilla)
endif ()

include(${wxWidgets_USE_FILE})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")

if (UNIX AND NOT APPLE)
    find_library(FONTCONFIG_LIBRARY NAMES fontconfig)
endif ()

if (MSVC)
    find_package(OpenGL REQUIRED)
endif ()


#####################################################################################################################
#
# Project Settings
#
#####################################################################################################################

add_subdirectory(src)
if (NOT SAM_SKIP_TOOLS)
    add_subdirectory(tools)
endif()
