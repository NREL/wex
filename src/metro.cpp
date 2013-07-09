#include <wx/settings.h>
#include <wx/fontenum.h>
#include <wx/dcbuffer.h>
#include <wx/simplebook.h>
#include <wx/scrolwin.h>

#include <algorithm>

#include "wex/metro.h"

wxFont wxMetroTheme::NormalFont( int size )
{
	wxFont font( wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

	if ( size > 1 )
		font.SetPointSize( size );

	if ( wxFontEnumerator::IsValidFacename( "Segoe UI" ) )
		font.SetFaceName( "Segoe UI" );

	return font;
}

wxFont wxMetroTheme::LightFont( int size )
{
	wxFont font( wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

	if ( size > 1 )
		font.SetPointSize( size );

	if ( wxFontEnumerator::IsValidFacename( "Segoe UI Light" ) )
		font.SetFaceName( "Segoe UI Light" );

	return font;
}

wxColour wxMetroTheme::Background()
{
	return *wxWHITE;
}

wxColour wxMetroTheme::Foreground()
{
	return wxColour( 0, 114, 198 ); // "outlook.com" blue colour
}

wxColour wxMetroTheme::HoverColour()
{
	return wxColour( 0, 88, 153 ); // "outlook.com" blue hover colour
}

wxColour wxMetroTheme::AccentColour()
{
	return wxColour( 255, 148, 0 ); // "nrel orange" colour
}

wxColour wxMetroTheme::TextColour()
{
	return wxColour( 140, 140, 140 ); // "medium gray"
}

wxColour wxMetroTheme::HighlightColour()
{
	return wxColour(224,232,246);
}

wxColour wxMetroTheme::SelectColour()
{
	return wxColour(193,210,238);
}



BEGIN_EVENT_TABLE(wxMetroButton, wxWindow)
	EVT_PAINT(wxMetroButton::OnPaint)
	EVT_SIZE(wxMetroButton::OnResize)
	EVT_LEFT_DOWN(wxMetroButton::OnLeftDown)
	EVT_LEFT_DCLICK( wxMetroButton::OnLeftDown )
	EVT_LEFT_UP(wxMetroButton::OnLeftUp)
	EVT_ENTER_WINDOW(wxMetroButton::OnEnter)
	EVT_LEAVE_WINDOW(wxMetroButton::OnLeave)
END_EVENT_TABLE()


wxMetroButton::wxMetroButton(wxWindow *parent, int id, const wxString &label, const wxPoint &pos, const wxSize &sz)
	: wxWindow(parent, id, pos, sz)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	m_label = label;
	m_state = 0;  // state: 0=normal, 1=hover, 2=click
	m_pressed = false;

	SetBackgroundColour( wxMetroTheme::Background() );
	SetForegroundColour( wxMetroTheme::Foreground() );
	SetFont( wxMetroTheme::NormalFont() );
}

wxSize wxMetroButton::DoGetBestSize() const
{
	wxClientDC dc( const_cast<wxMetroButton*>( this ) );
	dc.SetFont( GetFont() );
	int tw, th;
	dc.GetTextExtent( m_label, &tw, &th );
	return wxSize( tw + 20, th + 10 );
}

void wxMetroButton::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC dc(this);
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);

	dc.SetBrush( wxBrush( m_state == 1 ? wxMetroTheme::HighlightColour() : GetBackgroundColour() ) ) ;
	dc.SetPen( wxPen( GetForegroundColour(), 3) );
	dc.DrawRectangle(0,0,cwidth,cheight);

	int tw, th;
	dc.GetTextExtent( m_label, &tw, &th );
	dc.SetFont( GetFont() );
	dc.SetTextForeground( GetForegroundColour() );

	int offset = 0;
	if ( m_state == 2 ) offset = 1;

	dc.DrawText( m_label, cwidth/2 - tw/2+offset, cheight/2 - th/2+offset );
}

void wxMetroButton::OnResize(wxSizeEvent &)
{
	Refresh();
}

void wxMetroButton::OnLeftDown(wxMouseEvent &)
{
	SetFocus();
	CaptureMouse();
	m_pressed = true;
	m_state = 2;
	Refresh();
}

void wxMetroButton::OnLeftUp(wxMouseEvent &evt)
{
	if (HasCapture())
	{
		m_pressed = false;
		ReleaseMouse();
		m_state = 0;
		Refresh();

		wxSize sz = GetClientSize();
		int x = evt.GetX();
		int y = evt.GetY();
		if (x>=0&&y>=0 && x<=sz.GetWidth() && y<=sz.GetHeight())
		{
			wxCommandEvent press(wxEVT_COMMAND_BUTTON_CLICKED, this->GetId() );
			press.SetEventObject(this);
			GetEventHandler()->ProcessEvent(press);
		}
	}

}

void wxMetroButton::OnEnter(wxMouseEvent &)
{
	m_state = m_pressed?2:1;
	Refresh();
}

void wxMetroButton::OnLeave(wxMouseEvent &)
{
	m_state = 0;
	Refresh();
}






class wxMetroNotebookRenderer : public wxWindow
{
friend class wxMetroNotebook;

private:
	
	wxMetroNotebook *m_notebook;
	
	int m_xStart;
	std::vector<int> m_widths;
	int m_hoverIdx;
	int m_pressIdx;
	int m_nTabsShown;
	int m_dotdotWidth;
	bool m_dotdotHover;

	DECLARE_EVENT_TABLE();

	wxMetroNotebookRenderer( wxMetroNotebook *rn )
		: wxWindow( rn, wxID_ANY )
	{
		SetBackgroundStyle( wxBG_STYLE_CUSTOM );

		m_nTabsShown = 0;
		m_notebook = rn;
		m_hoverIdx = -1;
		m_pressIdx = -1;
		m_dotdotWidth = 0;
		m_dotdotHover = false;
	}

	void OnSize(wxSizeEvent &)
	{
		Refresh();
	}

	void OnPaint(wxPaintEvent &)
	{
		wxAutoBufferedPaintDC dc(this);

		int cwidth, cheight;
		GetClientSize(&cwidth, &cheight);
		
		int nwin = m_notebook->m_flipper->GetPageCount();
		m_widths.resize( nwin );
		
		bool dark = (m_notebook->m_style & wxMNB_REVERSED);

		wxColour bg = m_notebook->GetBackgroundColour();
		wxColour fg = m_notebook->GetForegroundColour();

		wxColour col_bg = dark ? fg : bg;
		wxColour col_fg = dark ? bg : fg;
		wxColour col_text = dark ? *wxWHITE : wxMetroTheme::TextColour();
		wxColour col_sel = dark ? *wxWHITE : fg;
		wxColour col_hover = dark ? wxMetroTheme::HoverColour() : wxMetroTheme::SelectColour();

		dc.SetFont( m_notebook->GetFont() );
		int CharHeight = dc.GetCharHeight();

		dc.SetBackground( col_bg );
		dc.Clear();

		int selIdx = m_notebook->m_flipper->GetSelection();
		int x = m_notebook->m_xOffset;	
		int padding = m_notebook->m_xPadding;

		m_xStart = x;

		m_dotdotWidth = 0;
		m_nTabsShown = nwin;
		for (int i=0;i<nwin;i++)
		{
			int txtw, txth;
			dc.GetTextExtent( m_notebook->m_pageList[i].text, &txtw, &txth );
			m_widths[i] = txtw+padding+padding;

			if ( i > 0 && x+m_widths[i] > cwidth - 30 ) // 30 approximates width of '...'
			{
				m_dotdotWidth = 1;
				m_nTabsShown = i;
				break;
			}

			if ( dark && ( i == m_hoverIdx || i == selIdx ) )
			{
				dc.SetPen( wxPen( col_hover, 1 ) );
				dc.SetBrush( wxBrush( col_hover ) );
				dc.DrawRectangle( x, 0, m_widths[i], cheight );
			}

			if ( dark )
				dc.SetTextForeground( i == selIdx ? col_sel : col_text );
			else
				dc.SetTextForeground( i == selIdx ? col_sel : ( i==m_hoverIdx ? col_hover : col_text ) );
			
			dc.DrawText( m_notebook->m_pageList[i].text, x + padding, cheight/2-CharHeight/2-1 );
			x += m_widths[i] + m_notebook->m_xSpacing;

		}

		if ( m_dotdotWidth > 0 )
		{
			wxFont font( wxMetroTheme::NormalFont(14) );
			font.SetWeight( wxFONTWEIGHT_BOLD );
			dc.SetFont( font );
			int txtw, txth;
			dc.GetTextExtent( "...", &txtw, &txth );

			m_dotdotWidth = txtw;

			if ( dark && m_dotdotHover )
			{
				dc.SetPen( wxPen( col_hover, 1 ) );
				dc.SetBrush( wxBrush( col_hover ) );
				dc.DrawRectangle( cwidth - m_dotdotWidth - padding, 0, m_dotdotWidth + padding, cheight );
			}

			if ( dark )
				dc.SetTextForeground( col_text );
			else
				dc.SetTextForeground( m_dotdotHover ? col_hover : col_text );

			dc.DrawText( "...", cwidth - m_dotdotWidth - padding/2, cheight/2 - txth/2-1 );

		}

		if ( !dark )
		{
			dc.SetPen( wxPen( wxMetroTheme::TextColour() ));
			dc.DrawLine( 0, cheight-2, cwidth, cheight-2);
		}
	}
	
	void OnLeftDown(wxMouseEvent &evt)
	{	
		int mouse_x = evt.GetX();
		int x = m_xStart;
		for (size_t i=0;i<m_widths.size() && i < m_nTabsShown;i++)
		{
			if (mouse_x >= x && mouse_x < x+m_widths[i])
			{
				m_pressIdx = i;
				if (this->HasCapture())
					this->ReleaseMouse();

				this->CaptureMouse();
				return;
			}
			x += m_widths[i] + m_notebook->m_xSpacing;
		}
	}

	void OnMouseMove(wxMouseEvent &evt)
	{
		int cwidth, cheight;
		GetClientSize(&cwidth, &cheight);
		m_hoverIdx = -1;
		int mouse_x = evt.GetX();

		bool was_hovering = m_dotdotHover;
		m_dotdotHover = ( m_dotdotWidth > 0 && mouse_x > cwidth - m_dotdotWidth - m_notebook->m_xPadding );

		int x = m_xStart;
		for (size_t i=0;i<m_widths.size();i++)
		{
			if (mouse_x >= x && mouse_x < x+m_widths[i])
			{
				if (m_hoverIdx != (int)i)
					Refresh();
				m_hoverIdx = i;
				return;
			}
			x += m_widths[i] + m_notebook->m_xSpacing;
		}

		if ( m_dotdotHover != was_hovering )
			Refresh();
			

	}

	void OnLeftUp(wxMouseEvent &evt)
	{
		if (m_pressIdx >= 0 && this->HasCapture())
			this->ReleaseMouse();

		int mouse_x = evt.GetX();
		int x = m_xStart;
		for (size_t i=0;i<m_widths.size() && i < m_nTabsShown;i++)
		{
			if (mouse_x >= x && mouse_x < x+m_widths[i] && m_pressIdx == (int)i)
			{
				wxNotebookEvent evt(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING, m_notebook->GetId() );
				evt.SetEventObject(m_notebook);
				evt.SetOldSelection( m_notebook->GetSelection() );
				evt.SetSelection(i);
				m_notebook->ProcessEvent(evt);

				m_notebook->m_flipper->ChangeSelection(i);
				Refresh();

				// fire EVT_NOTEBOOK_PAGE_CHANGED
				wxNotebookEvent evt2(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, m_notebook->GetId() );
				evt2.SetEventObject(m_notebook);
				evt2.SetSelection(i);
				m_notebook->ProcessEvent(evt2);

				return;
			}
			x += m_widths[i] + m_notebook->m_xSpacing;
		}
	}

	void OnLeave(wxMouseEvent &)
	{
		m_pressIdx = -1;
		m_hoverIdx = -1;
		m_dotdotHover = false;
		Refresh();
	}
};

BEGIN_EVENT_TABLE(wxMetroNotebookRenderer, wxWindow)
	EVT_LEFT_DCLICK( wxMetroNotebookRenderer::OnLeftDown)
	EVT_LEFT_DOWN( wxMetroNotebookRenderer::OnLeftDown )
	EVT_LEFT_UP( wxMetroNotebookRenderer::OnLeftUp)
	EVT_MOTION(wxMetroNotebookRenderer::OnMouseMove)
	EVT_PAINT( wxMetroNotebookRenderer::OnPaint )
	EVT_SIZE( wxMetroNotebookRenderer::OnSize )
	EVT_LEAVE_WINDOW( wxMetroNotebookRenderer::OnLeave )
END_EVENT_TABLE()

/************* wxMetroNotebook ************** */

BEGIN_EVENT_TABLE(wxMetroNotebook, wxPanel)
	EVT_SIZE( wxMetroNotebook::OnSize )
END_EVENT_TABLE()

wxMetroNotebook::wxMetroNotebook(wxWindow *parent, int id, const wxPoint &pos, const wxSize &sz, long style)
	: wxPanel(parent, id, pos, sz, wxCLIP_CHILDREN)
{
	m_style = style;
	m_xSpacing = 2;
	m_xOffset = 2;
	m_xPadding = 10;
	m_yPadding = 12;


	m_renderer = new wxMetroNotebookRenderer(this);
	m_flipper = new wxSimplebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );

	SetBackgroundColour( wxMetroTheme::Background() );
	SetForegroundColour( wxMetroTheme::Foreground() );
	SetFont( wxMetroTheme::LightFont(14) );
	
	Reposition();
}

void wxMetroNotebook::AddPage(wxWindow *win, const wxString &text, bool active)
{
	win->Reparent( m_flipper );
	m_flipper->AddPage(win, text);

	page_info x;
	x.text = text;
	x.scroll_win = 0;

	m_pageList.push_back( x );


	if (active)
		m_flipper->ChangeSelection( m_flipper->GetPageCount()-1 );

	Reposition();
}

int wxMetroNotebook::GetPageIndex(wxWindow *win)
{
	if ( win == NULL ) return -1;
	int ndx = -1;
	for (size_t i=0; i<m_flipper->GetPageCount(); i++)
	{
		if ( m_flipper->GetPage(i) == win )
			ndx = i;
	}
	return ndx;
}


wxWindow *wxMetroNotebook::RemovePage( int ndx )
{
	if ( ndx < 0 || ndx >= (int)m_pageList.size() )
		return 0;
	
	wxWindow *win = m_flipper->GetPage( ndx );

	int cursel = m_flipper->GetSelection();
	if ( cursel >= ndx )
	{
		cursel--;
		if ( cursel < 0 )
			cursel = 0;
	}

	if ( m_pageList[ndx].scroll_win != 0
		&& win->GetParent() == m_pageList[ndx].scroll_win )
	{
		win->SetParent( this );
		m_pageList[ndx].scroll_win->Destroy();
		m_pageList[ndx].scroll_win = 0;
	}

	m_flipper->RemovePage(ndx);
	m_pageList.erase( m_pageList.begin() + ndx );
		
	if ( m_flipper->GetSelection() != cursel )
		m_flipper->ChangeSelection(cursel);

	Reposition();

	return win;
}

void wxMetroNotebook::DeletePage( int ndx )
{
	wxWindow *win = RemovePage( ndx );
	if ( win != 0 ) win->Destroy();
}

wxWindow *wxMetroNotebook::GetPage( int index )
{
	return m_flipper->GetPage( index );
}

void wxMetroNotebook::AddScrolledPage(wxWindow *win, const wxString &text, bool active)
{
	wxScrolledWindow *scrollwin = new wxScrolledWindow( m_flipper );
	win->Reparent( scrollwin );

	int cw, ch;
	win->GetClientSize(&cw,&ch);

	scrollwin->SetScrollbars( 1, 1, cw, ch, 0, 0 );
	scrollwin->SetScrollRate(25,25);
	
	win->Move(0,0);


	m_flipper->AddPage( scrollwin, text );

	page_info x;
	x.text = text;
	x.scroll_win = win;

	m_pageList.push_back( x );

	if (active)
		m_flipper->ChangeSelection( m_flipper->GetPageCount()-1 );

	Reposition();
}

int wxMetroNotebook::GetSelection()
{
	return m_flipper->GetSelection();
}

void wxMetroNotebook::SetText(int id, const wxString &text)
{
	if ((id < (int)m_pageList.size()) && (id >= 0))
	{
		m_pageList[id].text = text;
		Reposition();
	}
}

void wxMetroNotebook::SetSelection(int id)
{
	m_flipper->SetSelection(id);
	Refresh();
}

int wxMetroNotebook::GetPageCount()
{
	return m_flipper->GetPageCount();
}
void wxMetroNotebook::Reposition()
{
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);
	
	wxClientDC cdc(this);
	cdc.SetFont( GetFont() );
	m_tabHeight = cdc.GetCharHeight() + m_yPadding;

	m_renderer->SetSize(0, 0, cwidth, m_tabHeight);
	m_flipper->SetSize(0, m_tabHeight, cwidth, cheight-m_tabHeight);

	for (size_t i=0;i<m_pageList.size();i++)
	{
		if ( m_pageList[i].scroll_win != 0 )
		{
			if (wxScrolledWindow *parent = dynamic_cast<wxScrolledWindow*>( m_flipper->GetPage(i) ))
			{
				int cw, ch;
				m_pageList[i].scroll_win->GetClientSize(&cw,&ch);
				int vw, vh;
				parent->GetVirtualSize(&vw,&vh);

				if (vw != cw || vh != ch)
				{
					parent->SetScrollbars(1,1, cw, ch);
					parent->SetScrollRate(25,25);
				}

				int x,y;
				m_pageList[i].scroll_win->GetPosition(&x,&y);
				if (vw > cw && vh > ch 
					&& (x != 0 || y != 0))
					m_pageList[i].scroll_win->Move(0,0);
		
			}
		}
	}
}

void wxMetroNotebook::OnSize(wxSizeEvent &)
{
	Reposition();
}
