# WEX: Extensions Library for wxWidgets
[![TravisCI](https://travis-ci.org/NREL/wex.svg?branch=develop)](https://travis-ci.org/NREL/wex)

WEX is a library of extensions to the [wxWidgets](https://www.wxwidgets.org/) cross-platform user interface (UI) library. These extensions are for custom UI widgets developed for the National Renewable Energy Laboratory's [System Advisor Model (SAM)](https://sam.nrel.gov) and [DView](https://github.com/NREL/wex/wiki/DView) data visualization software.

WEX also includes two executable programs:

_lkscript_ is a code editor for the [LK](https://github.com/NREL/lk) scripting language.
_dview_ is the [DView](https://github.com/NREL/wex/wiki/DView) visualization software for time series data.

# Prerequisites

The WEX libraries require [LK](https://github.com/NREL/lk) for scripting funtionality. Before building the WEX libraries:

1. Build LK.

2. Create an environment variable called `LKDIR` that points to the folder containing the LK libraries.

# Windows

The [build_vs2017](build_vs2017) folder contains project files for Microsoft Visual Studio 2017 (VS 2017).

To build the WEX libraries, open /build_vs2017/wex_vc14.sln in VS 2017 and build debug and release configuration for both win32 and x64. If the builds are successful, you should see the following files:

```
wexvc13wx3.lib
wexvc13wx3d.lib
wexvc13wx3x64.lib
wexvc13wx3x64d.lib
```
You should also see versions of the following executable programs (lkscript may be in a separate subfolder):

```
dview
lkscript
sandbox
```

WEX requires libcurl and SSL libraries for HTTP and HTTPS. Those libraires are precompiled for Windows and included in [build_vs2017/libcurl_ssl_win32] and [build_vs2017/libcurl_ssl_x64]. If you want to rebuild those libraries to incorporate security patches, see [Building libcurl+ssl using VS 2017 For Windows](build_libcurl_ssl_for_windows.md).

# Mac

Makefiles for Mac OS are in the [build_osx](build_osx) folder. Minimum OS X version is 10.9. MacOS 10.12 is used with macosx-version-min=10.9 flag set for wex and SAM releases.

# Linux

Makefiles for Linux are in the [build_linux](build_linux) folder. Minimum requirements: gcc 4.8.5 and glibc 2.17. CentOS 7 is mimimum build platform used for wex and SAM releases.

# Contributing

If you have found an issue with WEX or would like to make a feature request, please let us know by adding a new issue on the [issues page](https://github.com/NREL/wex/issues).

Please see the [Contributing](https://github.com/NREL/wex/blob/develop/CONTRIBUTING.MD) page for the full contributing policy and to get instructions for getting started.  We must get your agreement on conforming to the terms of the license before your code can be accepted.

If you would like to submit code to fix an issue or add a feature, you can use GitHub to do so. The overall steps are to create a fork on GitHub.com using the link above, and then install GitHub on your computer and use it to clone your fork, create a branch for your changes, and then once you have made your changes, commit and push the changes to your fork. You can then create a pull request that we will review and merge into the repository if approved.  

# License

WEX is licensed under an MIT [license](LICENSE.md).
