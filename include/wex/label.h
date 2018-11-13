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

#ifndef __wexlabel_h
#define __wexlabel_h

#include <wx/window.h>

void wxLabelDraw(wxDC &dc, const wxRect &r, const wxFont &font, const wxString &text, const wxColour &col,
	bool alignTop, bool alignRight, bool isBold, bool wordWrap, int relSize);

class wxLabel : public wxWindow
{
public:
	wxLabel(wxWindow *parent, int id, const wxString &caption,
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

	virtual wxSize DoGetBestSize() const;

	void AlignRight(bool b = true);
	void AlignTop(bool b = true);

	void SetText(const wxString &txt);
	wxString GetText() { return m_text; }

	void SetColour(const wxColour &c);
	wxColour GetColour() { return m_colour; }

	void SetBold(bool b);
	bool IsBold() { return m_bold; }

	void SetWordWrap(bool b);
	bool IsWordWrapped() { return m_wordWrap; }

	void SetRelativeSize(int sz = 0);
	int GetRelativeSize() { return m_relSize; }

private:
	bool m_bold;
	int m_relSize;
	bool m_alignTop;
	bool m_alignRight;
	bool m_wordWrap;
	wxString m_text;
	wxColour m_colour;

	void OnErase(wxEraseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnResize(wxSizeEvent &evt);

	DECLARE_EVENT_TABLE()
};

#endif