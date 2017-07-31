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

#include <wx/dcclient.h>
#include <wx/dcbuffer.h>

#include "wex/utils.h"
#include "wex/label.h"

BEGIN_EVENT_TABLE(wxLabel, wxWindow)
EVT_ERASE_BACKGROUND(wxLabel::OnErase)
EVT_PAINT(wxLabel::OnPaint)
EVT_SIZE(wxLabel::OnResize)
END_EVENT_TABLE()

wxLabel::wxLabel(wxWindow *parent, int id, const wxString &caption, const wxPoint &pos, const wxSize &size)
: wxWindow(parent, id, pos, size)
{
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	InvalidateBestSize();
	m_colour = *wxBLACK;
	m_text = caption;
	m_alignTop = false;
	m_alignRight = false;
	m_relSize = 0;
	m_bold = false;
	m_wordWrap = false;
}

wxSize wxLabel::DoGetBestSize() const
{
	wxClientDC dc(const_cast<wxLabel*>(this));
	wxFont font(GetFont());
	if (m_bold) font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(font);
	wxSize size = dc.GetTextExtent(m_text);
	int border = (int)(4.0 * wxGetScreenHDScale());
	return wxSize(size.x + border, size.y + border);
}

void wxLabel::AlignTop(bool b)
{
	m_alignTop = b;
}

void wxLabel::AlignRight(bool b)
{
	m_alignRight = b;
}

void wxLabel::SetText(const wxString &txt)
{
	InvalidateBestSize();
	m_text = txt;
	Refresh();
}

void wxLabel::SetColour(const wxColour &c)
{
	m_colour = c;
}

void wxLabel::SetBold(bool b)
{
	m_bold = b;
}

void wxLabel::SetWordWrap(bool b)
{
	m_wordWrap = b;
}

void wxLabel::SetRelativeSize(int sz)
{
	m_relSize = sz;
}

void wxLabel::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC pdc(this);
	PrepareDC(pdc);
	pdc.SetDeviceClippingRegion(this->GetUpdateRegion());

	pdc.SetBackground(wxBrush(GetBackgroundColour()));
	pdc.Clear();

	int width, height;
	GetClientSize(&width, &height);

	wxLabelDraw(pdc, wxRect(0, 0, width, height), GetFont(), m_text, m_colour,
		m_alignTop, m_alignRight, m_bold, m_wordWrap, m_relSize);

	pdc.DestroyClippingRegion();
}

void wxLabel::OnErase(wxEraseEvent &)
{
	// nothing to do
}

void wxLabel::OnResize(wxSizeEvent &)
{
	Refresh();
}

void wxLabelDraw(wxDC &dc, const wxRect &geom, const wxFont &font, const wxString &text, const wxColour &col,
	bool alignTop, bool alignRight, bool isBold, bool wordWrap, int relSize)
{
	dc.SetTextForeground(col);

	wxFont f(font);

	if (isBold)
		f.SetWeight(wxFONTWEIGHT_BOLD);
	else if (!isBold && f.GetWeight() != wxFONTWEIGHT_NORMAL)
		f.SetWeight(wxFONTWEIGHT_NORMAL);

	if (relSize != 0)
		f.SetPointSize(f.GetPointSize() + relSize);

	dc.SetFont(f);

	wxCoord width, height;
	dc.GetTextExtent(text, &width, &height);
	wxRect tbounds(geom.x + 1, geom.y + geom.height / 2 - height / 2 - 1,
		width, height + 3);

	if (alignTop) tbounds.y = geom.y + 1;
	if (alignRight) tbounds.x = geom.x + geom.width - width - 2;

	if (wordWrap && !alignRight && alignTop)
		wxDrawWordWrappedText(dc, text, geom.width - 4, true, tbounds.x, geom.y + 1);
	else
		dc.DrawText(text, tbounds.x, tbounds.y);
}