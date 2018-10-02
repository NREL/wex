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

#include <wx/wx.h>
#include <wx/frame.h>
#include <wx/stc/stc.h>
#include <wx/webview.h>
#include <wx/dynlib.h>

#include <wx/app.h>

#include <vector>
#include <wx/frame.h>
#include <wx/stc/stc.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/busyinfo.h>
#include <wx/stdpaths.h>

#include <wex/plot/plplot.h>
#include <wex/lkscript.h>
#include <wex/metro.h>
#include <wex/utils.h>

class MyApp : public wxApp
{
public:
	bool OnInit()
	{
#ifdef __WXMSW__
		typedef BOOL(WINAPI *SetProcessDPIAware_t)(void);
		wxDynamicLibrary dllUser32(wxT("user32.dll"));
		SetProcessDPIAware_t pfnSetProcessDPIAware =
			(SetProcessDPIAware_t)dllUser32.RawGetSymbol(wxT("SetProcessDPIAware"));
		if (pfnSetProcessDPIAware)
			pfnSetProcessDPIAware();
#endif

		wxArrayString args;
		for (int i = 0; i < argc; i++)
			args.Add(argv[i]);

		bool run = args.Index("-run") >= 0;

		wxInitAllImageHandlers();

		wxString wexdir;
		if (wxGetEnv("WEXDIR", &wexdir))
		{
			if (!wxPLPlot::AddPdfFontDir(wexdir + "/pdffonts"))
				wxMessageBox("Could not add font dir: " + wexdir + "/pdffonts");
			if (!wxPLPlot::SetPdfDefaultFont("ComputerModernSansSerif"))
				wxMessageBox("Could not set default pdf font to Computer Modern Sans Serif");
		}

		if (args.size() > 1)
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				if (args[i] == "-run") continue;

				if (!wxFileExists(args[i]))
				{
					wxMessageBox("The script does not exist:\n\n" + args[i]);
					continue;
				}

				wxLKScriptWindow *sw = wxLKScriptWindow::CreateNewWindow(!run);
				if (sw->Load(args[i]))
				{
					if (run)
					{
						if (sw->RunScript()) // if run OK, close window
							sw->Destroy();
						else
						{
							wxMessageBox("Script error.");
							sw->Show(); // otherwise, show script & errors
						}
					}
					else
						sw->Show();
				}
				else
				{
					wxMessageBox("Error loading script file:\n\n" + args[i]);
					sw->Destroy();
				}
			}
		}
		else
			wxLKScriptWindow::CreateNewWindow();

		return wxTopLevelWindows.size() > 0;
	}
};

IMPLEMENT_APP(MyApp)