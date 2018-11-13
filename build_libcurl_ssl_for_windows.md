# Building libcurl+ssl using VS 2017 Community Edition

Adapated from Aron P. Dobos, originally written in October 2013 and updated March 2017

# 32-bit Version

1.	Download software:

	a.	ActivePerl: http://www.activestate.com/activeperl/downloads
	
	b.	OpenSSL 1.0.1c 	http://www.openssl.org/source/openssl-1.0.1c.tar.gz
	
	c.	Libssl 1.4.2 http://www.libssh2.org/download/libssh2-1.4.2.tar.gz
	
	d.	Curl 7.28.0 http://curl.haxx.se/download/curl-7.28.0.zip
	
	e.	Netwide assembler http://www.nasm.us/pub/nasm/releasebuilds/2.13.03/win32/nasm-2.13.03-win32.zip

2.	Install ActivePerl if you don’t already have it.

3.	Create a folder c:\curl and un-tar openssl, libssl, and curl into this folder.

4.	Start menu -> run -> cmd.exe.

5.	`cd C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build`

6.	Enter the command and press enter: `vcvars32.bat`.

7.	Test by typing `cl.exe`.  Output should look like this:

	```
	C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build>cl
	Microsoft (R) C/C++ Optimizing Compiler Version 19.13.26129 for x86
	Copyright (C) Microsoft Corporation.  All rights reserved.

	```

8.	Unzip the netwide assembler binaries to c:\nasm-2.13.03.

9.	Add it to the path `path = %PATH%;C:\nasm-2.13.03`.

10.	cd `c:\curl\openssl-1.0.1c`

11.	`perl Configure VC-WIN32 –-prefix=c:/curl/openssl_dll`

12.	`ms\do_nasm`

13.	`nmake –f ms\ntdll.mak`

14.	`nmake –f ms\ntdll.mak test`

15.	`nmake –f ms\ntdll.mak install`

16.	Open VS 2017.

17.	Open c:\curl\libssh2-1.4.2\win32\libssh2.dsw.

18.	Allow VS2017 to upgrade the project file.

19.	On the toolbar, select the “OpenSSL DLL Release” "x86" configuration.

20.	Right click on ‘libssh2’ in the solution explorer, and select ‘Properties’.

	a.	Under ‘C/C++/General/Additional Include Directories’, add c:\curl\openssl_dll\include;
	
	b.	Under ‘Linker/General/Additional Library Directories’, add c:\curl\openssl_dll\lib;
	
	c.	Under ‘Linker/Input’, remove zlib.lib.

21.	In libssh2_config.h, comment out line 25 to prevent conflict with snprintf defined in stdio.h	

22. 	Build the solution  C:\curl\libssh2-1.4.2\win32\Release_dll should have the files libssh2.dll and libssh2.lib.  You might get errors about the “tests” project not finding libeay32.lib, but you can probably ignore this.

23.	Close the project. Open C:\curl\curl-7.28.0\lib\libcurl.vcproj – allow VS2017 to upgrade the project file.

24.	Select the ‘Release’ build.

25.	In solution explorer, select properties of ‘libcurl’.

26.	In General, select Configuration Type = ‘Dynamic Library (.dll)’ , then click “Apply” in the dialog.

27.	Under C/C++/General/Additional Include Directories, add C:\curl\openssl-1.0.1c\inc32\;C:\curl\libssh2-1.4.2\include; c:\curl\openssl_lib\include\; c:\curl\openssl_lib\include\openssl.

28.	Under C/C++/Preprocessor/Preprocessor Definitions/, add USE_OPENSSL;USE_LIBSSH2;CURL_DISABLE_LDAP;HAVE_LIBSSH2;HAVE_LIBSSH2_H;LIBSSH2_WIN32 ;LIBSSH2_LIBRARY;USE_SSLEAY

29.	Under Linker/Input/Additional Dependencies, add libssh2.lib;libeay32.lib;ssleay32.lib;ws2_32.lib

30.	Under Linker/General/Additional Library Directories, add C:\curl\libssh2-1.4.2\win32\Release_dll;c:\curl\openssl_dll\lib

31.	Build.  It should succeed.

32.	Now you have all the dlls, libs, and relevant header files to include in other VC2013 projects that use the v120 C runtime.

33.	Aggregate all the headers and binaries into a single “redistributable” with these commands:

	a.	`>> copy C:\curl\curl-7.28.0\lib\Release\libcurl.dll c:\curl\openssl_dll\bin`
	
	b.	`>> copy C:\curl\curl-7.28.0\lib\Release\libcurl.lib c:\curl\openssl_dll\lib`
	
	c.	`>> copy C:\curl\curl-7.28.0\lib\Release\libcurl.exp c:\curl\openssl_dll\lib`
	
	d.	`>> copy C:\curl\libssh2-1.4.2\win32\Release_dll\libssh2.dll c:\curl\openssl_dll\bin`
	
	e.	`>> copy C:\curl\libssh2-1.4.2\win32\Release_dll\libssh2.lib c:\curl\openssl_dll\lib`
	
	f.	`>> copy C:\curl\libssh2-1.4.2\win32\Release_dll\libssh2.exp c:\curl\openssl_dll\lib`
	
	g.	`>> mkdir c:\curl\openssl_dll\include\ssh`
	
	h.	`>> copy c:\curl\libssh2-1.4.2\include\* c:\curl\openssl_dll\include\ssh`
	
	i.	`>> mkdir c:\curl\openssl_dll\include\curl`
	
	j.	`>> copy c:\curl\curl-7.28.0\include\curl\* c:\curl\openssl_dll\include\curl`
	
34.	Rename the c:\curl\openssl_dll folder to libcurl_ssl_vs2017_win32.

# 64-bit Version

1.	If you’re doing this right after having finished up the 32 bit build above, create a subfolder in c:\curl called “win32-build” and move all the current contents of c:\curl (except the untouched tar.gz files) into c:\curl\win32-build.

2.	Close the command prompt and open a new one to refresh the environment variables.

3.	Change to c:\curl and untar the three libraries: curl-7.28.0, libssh2-1.4.2, openssl-1.0.1c.

4.	`cd C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build`

5.	Enter the command and press enter>> vcvarsall.bat x86_amd64.

6.	Test by typing `cl.exe`.  Output should look like this:

	```
	C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build>cl
	Microsoft (R) C/C++ Optimizing Compiler Version 19.13.26129 for x64
	Copyright (C) Microsoft Corporation.  All rights reserved.
	```

7.	Unzip the netwide assembler binaries to c:\nasm-2.13.03.

8.	Add it to the path path = %PATH%;C:\nasm-2.13.03

9.	`cd c:\curl\openssl-1.0.1c`

10.	`perl Configure VC-WIN64A –-prefix=c:/curl/openssl_dll`

11.	`ms\do_win64a`

12.	`nmake –f ms\ntdll.mak`

13.	`nmake –f ms\ntdll.mak test`

14.	`nmake –f ms\ntdll.mak install`

15.	Open VS 2017

16.	Open c:\curl\libssh2-1.4.2\win32\libssh2.dsw

17.	Allow VS2017 to upgrade the project file

18.	Click on the dropdown button next to “Win32” and open the “Configuration Manager”

19.	Under “Active solution platform”, select “<New…>” and click OK to create x64 configuration.

20.	Under “Active solution configuration”, select “DLL Release”

21.	Right click on ‘libssh2’ in the solution explorer, and select ‘Properties’

	a.	Under ‘C/C++/General/Additional Include Directories’, add c:\curl\openssl_dll\include;

	b.	Under ‘Linker/General/Additional Library Directories’, add c:\curl\openssl_dll\lib;

	c.	Under ‘Linker/Input’, remove zlib.lib

22.	In libssh2_config.h, comment out line 25 to prevent conflict with snprintf defined in stdio.h

23.	Build the solution  C:\curl\libssh2-1.4.2\win32\Release_dll should have the files libssh2.dll and libssh2.lib.  You might get errors about the “tests” project not finding libeay32.lib, but you can probably ignore this. 

24.	Close the project. Open C:\curl\curl-7.28.0\lib\libcurl.vcproj – allow VS2017 to upgrade the project file.

25.	In the configuration manager, create a new “x64” platform, and select the ‘Release’ build.

26.	In solution explorer, select properties of ‘libcurl’.

27.	In General, select Configuration Type = ‘Dynamic Library (.dll)’, then click “Apply” in the dialog

28.	Under C/C++/General/Additional Include Directories, add C:\curl\openssl-1.0.1c\inc32\;C:\curl\libssh2-1.4.2\include; c:\curl\openssl_lib\include\; c:\curl\openssl_lib\include\openssl

29.	Under C/C++/Preprocessor/Preprocessor Definitions/ add USE_OPENSSL;USE_LIBSSH2;CURL_DISABLE_LDAP;HAVE_LIBSSH2;HAVE_LIBSSH2_H;LIBSSH2_WIN32 ;LIBSSH2_LIBRARY;USE_SSLEAY

30.	Under Linker/Input/Additional Dependencies, add libssh2.lib;libeay32.lib;ssleay32.lib;ws2_32.lib

31.	Under Linker/General/Additional Library Directories, add C:\curl\libssh2-1.4.2\win32\Release_dll;c:\curl\openssl_dll\lib

32.	Build.  It should succeed.

33.	Now you have all the 64-bit dlls, libs, and relevant header files to include in other VS2014 projects that use the v140 C runtime.

34.	Aggregate all the headers and binaries into a single “redistributable” with these commands:

	a. `>> copy C:\curl\curl-7.28.0\lib\x64\Release\libcurl.dll c:\curl\openssl_dll\bin`
	
	b. `>> copy C:\curl\curl-7.28.0\lib\x64\Release\libcurl.lib c:\curl\openssl_dll\lib`
	
	c. `>> copy C:\curl\curl-7.28.0\lib\x64\Release\libcurl.exp c:\curl\openssl_dll\lib`
	
	d. `>> copy C:\curl\libssh2-1.4.2\win32\Release_dll\libssh2.dll c:\curl\openssl_dll\bin`
	
	e. `>> copy C:\curl\libssh2-1.4.2\win32\Release_dll\libssh2.lib c:\curl\openssl_dll\lib`
	
	f. `>> copy C:\curl\libssh2-1.4.2\win32\Release_dll\libssh2.exp c:\curl\openssl_dll\lib`
	
	g. `>> mkdir c:\curl\openssl_dll\include\ssh`
	
	h. `>> copy c:\curl\libssh2-1.4.2\include\* c:\curl\openssl_dll\include\ssh`
	
	i. `>> mkdir c:\curl\openssl_dll\include\curl`
	
	j. `>> copy c:\curl\curl-7.28.0\include\curl\* c:\curl\openssl_dll\include\curl`

33.	Rename the c:\curl\openssl_dll folder to libcurl_ssl_vs2017_x64.
