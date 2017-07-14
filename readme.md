# WEX: Extensions Library for wxWidgets

[WEX](/wiki) is a library of extensions to the [wxWidgets](https://www.wxwidgets.org/) cross-platform user interface (UI) library. These extensions are for custom UI widgets developed for the National Renewable Energy Laboratory's [System Advisor Model (SAM)](https://sam.nrel.gov) and [DView](https://github.com/NREL/wex/wiki/DView) data visualization software.

WEX also includes two executable programs:

_lkscript_ is a code editor for the [LK](https://github.com/NREL/lk) scripting language.
_dview_ is the [DView](https://github.com/NREL/wex/wiki/DView) visualization software for time series data.

# Prerequisites

The WEX libraries require [LK](https://github.com/NREL/lk) for scripting funtionality. Before building the WEX libraries:

1. Build LK.

2. Create an environment variable called `LKDIR` that points to the folder containing the LK libraries.

# Windows

The [build_vc2013](build_vc2013) folder contains project files for Microsoft Visual Studio 2013 (VS 2013).

To build the WEX libraries, open /build_vc2013/wexvc13wx3.sln in VS 2013 and build debug and release configuration for both win32 and x64. If the builds are successful, you should see the following files:

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

WEX requires libcurl and SSL libraries for HTTP and HTTPS. Those libraires are precompiled for Windows and included in [build_vc2013/libcurl_ssl_win32] and [build_vc2013/libcurl_ssl_x64]. If you want to rebuild those libraries to incorporate security patches, see [Building libcurl+ssl using VS Express 2013 For Windows Desktop](build_libcurl_ssl_for_windows.md).

# Mac

Makefiles for Mac OS are in the [build_osx](build_osx) folder.

# Linux

Makefiles for Linux are in the [build_linux](build_linux) folder.

# Contributing

If you have found an issue with WEX or would like to make a feature request, please let us know by adding a new issue on the [issues page](https://github.com/NREL/wex/issues).

If you would like to submit code to fix an issue or add a feature, you can use GitHub to do so. The overall steps are to create a fork on GitHub.com using the link above, and then install GitHub on your computer and use it to clone your fork, create a branch for your changes, and then once you have made your changes, commit and push the changes to your fork. You can then create a pull request that we will review and merge into the repository if approved.

# License

WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

(1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

(2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
following disclaimer in the documentation and/or other materials provided with the distribution.

(3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
products derived from this software without specific prior written permission from the respective party.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
