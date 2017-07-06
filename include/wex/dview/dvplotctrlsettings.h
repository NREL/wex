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

#ifndef __DVPlotCtrlSettings_h
#define __DVPlotCtrlSettings_h

/*
 * wxDVPlotCtrlSettings.h
 *
 * This class stores the configuration of wxDVPlotCtrl.
 * We keep track of things like current tab, axis position, etc so that they can be restored quickly.
 */

#include <wx/wx.h>

#include <unordered_map>
using std::unordered_map;
#pragma warning(disable: 4290)  // ignore warning: 'C++ exception specification ignored except to indicate a function is not __declspec(nothrow)'

class wxDVPlotCtrlSettings
{
public:
	wxDVPlotCtrlSettings();
	wxDVPlotCtrlSettings(const wxDVPlotCtrlSettings &cpy) { m_properties = cpy.m_properties; }
	virtual ~wxDVPlotCtrlSettings();

	wxDVPlotCtrlSettings &operator=(const wxDVPlotCtrlSettings &rhs)
	{
		m_properties = rhs.m_properties;
		return *this;
	}

	void SetProperty(const wxString &prop, const wxString &value);
	void SetProperty(const wxString &prop, int value);
	void SetProperty(const wxString &prop, double value);
	void SetProperty(const wxString &prop, bool Value);
	wxString GetProperty(const wxString &prop);

private:
	unordered_map<wxString, wxString, wxStringHash, wxStringEqual> m_properties;
};

#endif
