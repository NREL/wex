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

#ifndef __snaplayout_h
#define __snaplayout_h

#include <vector>

#include <wx/scrolwin.h>

class wxFrame;

class wxSnapLayout : public wxScrolledWindow
{
public:
	wxSnapLayout( wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize );
	virtual ~wxSnapLayout();

	void Add( wxWindow *win, int width = -1, int height = -1 );
	void Delete( wxWindow * );
	size_t Count();
	wxWindow *Get( size_t i );
	std::vector<wxWindow*> Windows();
	int Find( wxWindow *w );
	void DeleteAll();
	void ScrollTo( wxWindow * );
	void AutoLayout();
	void ClearHighlights();
	void Highlight( wxWindow * );
	void SetShowSizing( bool b ) { m_showSizing = b; }
	void SetBackgroundText( const wxString & );

private:

	struct layout_box {
		wxWindow *win;
		wxSize req;
		wxRect rect;
		wxRect active;
		wxRect size_nw, size_se, size_ne, size_sw, move_box[4];
		bool highlight;
	};

	std::vector<layout_box*> m_list;

	struct drop_target {
		int index;
		wxRect target;
	};
	std::vector<drop_target> m_targets;

	void OnLeftDown( wxMouseEvent & );
	void OnLeftUp( wxMouseEvent & );
	void OnMotion( wxMouseEvent & );
	void OnCaptureLost( wxMouseCaptureLostEvent & );
	void OnSize( wxSizeEvent & );
	void OnLeaveWindow( wxMouseEvent & );
	void OnPaint( wxPaintEvent & );
	void OnErase( wxEraseEvent & );

	enum { NW, NE, SW, SE };
	layout_box *CheckActive( const wxPoint &p, int *handle );

	struct wydata { int y; layout_box *lb; };
	std::vector<wydata> GetSortedYPositions( size_t imax );
	void AutoLayout2();
	bool CanPlace( size_t idx, int width, int height, int xplace, int yplace, int client_width );
	void Place( size_t i, int cwidth );

	int m_handle;
	layout_box *m_active;
	drop_target *m_curtarget;

	void ShowTransparency( wxRect r );
	void HideTransparency();

	class OverlayWindow;

	OverlayWindow *m_transp;
	wxRect m_sizerect;
	bool m_showSizing;

	int m_sizeHover;
	int m_moveHover;

	wxString m_backgroundText;

	int m_space;
	int m_scrollRate;

	wxPoint m_orig;

	DECLARE_EVENT_TABLE();
};

#endif