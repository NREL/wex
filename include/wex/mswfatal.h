/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

#ifndef __wx_mswfatal_h
#define __wx_mswfatal_h

#ifdef __WXMSW__

#include <wx/string.h>

// notes on using this:
//  - must link application to psapi.lib
//  - include dbghelp.dll in your application distribution
//  - ship stripped PDBs with your application

// call this in the constructor of your wxApp
// to set up exception handling
void wxMSWSetupExceptionHandler(const wxString &appname, const wxString &version, const wxString &email);

// call this function from your wxApp::OnFatalException() function
// to generate a stack trace and show an exception dialog box
void wxMSWHandleApplicationFatalException();

// create a segmentation fault.  a normal application should never call this,
// but is useful for testing.  Note this should be issued by a wxButton event
// handler, since a menu event behaves as a timer
//     https://social.msdn.microsoft.com/Forums/vstudio/en-US/0caf88f7-e22b-49be-a7e9-8504c0312cb8/exception-no-more-propagated-within-few-win32-function-call?forum=vcgeneral
//     https://forums.wxwidgets.org/viewtopic.php?t=18742&p=81165
void wxMSWSegmentationFault();

#endif // __WXMSW__

#endif // __wx_mswfatal_h