#include <wx/simplebook.h>
#include <wx/dcbuffer.h>
#include <wx/scrolwin.h>

#include <algorithm>

#include "wex/ribbonbook.h"

class wxRibbonRenderer : public wxWindow
{
friend class wxRibbonNotebook;

private:
	
	wxRibbonNotebook *m_notebook;
	
	int m_xStart;
	std::vector<int> m_widths;
	int m_hoverIdx;
	int m_pressIdx;

	DECLARE_EVENT_TABLE();

	wxRibbonRenderer( wxRibbonNotebook *rn )
		: wxWindow( rn, wxID_ANY )
	{
		SetBackgroundStyle( wxBG_STYLE_CUSTOM );

		m_notebook = rn;
		m_hoverIdx = -1;
		m_pressIdx = -1;
	}

	void OnSize(wxSizeEvent &evt)
	{
		Refresh();
	}

	void OnPaint(wxPaintEvent &evt)
	{
		wxAutoBufferedPaintDC dc(this);

		int cwidth, cheight;
		GetClientSize(&cwidth, &cheight);
		
		int nwin = m_notebook->m_flipper->GetPageCount();
		m_widths.resize( nwin );
		
		int CharHeight = dc.GetCharHeight();

		dc.SetPen( wxPen( m_notebook->m_ribbonColour ) );
		dc.SetBrush( wxBrush( m_notebook->m_ribbonColour ) );
		dc.DrawRectangle( 0, 0, cwidth, cheight );

		int x = m_notebook->m_xOffset;
		
		if ( ! m_notebook->m_headerText.IsEmpty() )
		{
			wxFont font_normal_bold( *wxNORMAL_FONT );
			font_normal_bold.SetWeight( wxFONTWEIGHT_BOLD );
			dc.SetFont( font_normal_bold );
			
			wxSize ex = dc.GetTextExtent( m_notebook->m_headerText );
			dc.SetTextForeground( m_notebook->m_textColour );
			dc.DrawText( m_notebook->m_headerText, x + 5, cheight/2-dc.GetCharHeight()/2-1 );
			x += ex.x + 10;
		}

		m_xStart = x;

		dc.SetFont( *wxNORMAL_FONT );

		for (int i=0;i<nwin;i++)
		{
			int bitw, bith, txtw, txth;
			bitw = m_notebook->m_pageList[i].icon.GetWidth();
			bith = m_notebook->m_pageList[i].icon.GetHeight();

			dc.GetTextExtent( m_notebook->m_pageList[i].text, &txtw, &txth );

			if ( !m_notebook->m_textOnRight ) m_widths[i] = std::max(txtw, bitw) + 20;
			else m_widths[i] = bitw+txtw+20;

			if (i==m_notebook->m_flipper->GetSelection() || i == m_hoverIdx)
			{
				wxColour c = i==m_notebook->m_flipper->GetSelection()?
						m_notebook->m_selectColour:m_notebook->m_highlightColour;

				dc.SetPen( wxPen( c ) );
				dc.SetBrush( wxBrush( c ) );
				dc.DrawRectangle( x, 0, m_widths[i], cheight-2 );
			}

			dc.SetTextForeground( m_notebook->m_textColour );

			if (!m_notebook->m_textOnRight)
			{
				int h_left = cheight-7-CharHeight;
				wxRect rct( x+ (m_widths[i]-bitw)/2, (h_left-bith)/2, bitw, bith );
				dc.DrawBitmap( m_notebook->m_pageList[i].icon, rct.x, rct.y );		
				dc.DrawText( m_notebook->m_pageList[i].text, x+(m_widths[i]-txtw)/2, h_left+1 );
				
				DrawOverlay( dc , m_notebook->m_pageList[i].overlay, rct, m_notebook->m_pageList[i].overlay_pos );
			}
			else
			{
				wxRect rct( x+5, (cheight-bith)/2 , bitw, bith );
				dc.DrawBitmap( m_notebook->m_pageList[i].icon, rct.x, rct.y );
				dc.DrawText( m_notebook->m_pageList[i].text, x + bitw + 15, cheight/2-CharHeight/2-1 );

				DrawOverlay(dc,  m_notebook->m_pageList[i].overlay, rct, m_notebook->m_pageList[i].overlay_pos );
			}

			x += m_widths[i] + m_notebook->m_xSpacing;

		}

		dc.SetPen( wxPen( wxColour(120,120,120) ));
		dc.DrawLine( 1, cheight-2, cwidth-1, cheight-2);
		if (m_notebook->m_ctrlPanel != NULL)
			dc.DrawLine( 1, 0, 1, cheight-2 );
	}

	void DrawOverlay( wxDC &dc, wxBitmap &overlay, wxRect rct, unsigned int pos )
	{
		if ( !overlay.IsOk() ) return;

		int x, y;
		if (pos & wxLEFT) x = rct.x + 1;
		else x = rct.x + rct.width - overlay.GetWidth() - 1;

		if (pos & wxTOP ) y = rct.y + 1;
		else y = rct.y + rct.height - overlay.GetHeight() - 1;

		dc.DrawBitmap( overlay, x, y );
	}

	void OnLeftDown(wxMouseEvent &evt)
	{	
		int mouse_x = evt.GetX();
		int x = m_xStart;
		for (size_t i=0;i<m_widths.size();i++)
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
		m_hoverIdx = -1;
		int mouse_x = evt.GetX();
		int x = m_xStart;
		for (size_t i=0;i<m_widths.size();i++)
		{
			if (mouse_x >= x && mouse_x < x+m_widths[i])
			{
				if (m_hoverIdx != i)
					Refresh();
				m_hoverIdx = i;
				return;
			}
			x += m_widths[i] + m_notebook->m_xSpacing;
		}

		Refresh();

	}

	void OnLeftUp(wxMouseEvent &evt)
	{
		if (m_pressIdx >= 0 && this->HasCapture())
			this->ReleaseMouse();

		int mouse_x = evt.GetX();
		int x = m_xStart;
		for (size_t i=0;i<m_widths.size();i++)
		{
			if (mouse_x >= x && mouse_x < x+m_widths[i] && m_pressIdx == i)
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

	void OnLeave(wxMouseEvent &evt)
	{
		m_pressIdx = -1;
		m_hoverIdx = -1;
		Refresh();
	}
};

BEGIN_EVENT_TABLE(wxRibbonRenderer, wxWindow)
	EVT_LEFT_DCLICK( wxRibbonRenderer::OnLeftDown)
	EVT_LEFT_DOWN( wxRibbonRenderer::OnLeftDown )
	EVT_LEFT_UP( wxRibbonRenderer::OnLeftUp)
	EVT_MOTION(wxRibbonRenderer::OnMouseMove)
	EVT_PAINT( wxRibbonRenderer::OnPaint )
	EVT_SIZE( wxRibbonRenderer::OnSize )
	EVT_LEAVE_WINDOW( wxRibbonRenderer::OnLeave )
END_EVENT_TABLE()

/************* wxRibbonNotebook ************** */

BEGIN_EVENT_TABLE(wxRibbonNotebook, wxPanel)
	EVT_SIZE( wxRibbonNotebook::OnSize )
END_EVENT_TABLE()

wxRibbonNotebook::wxRibbonNotebook(wxWindow *parent, int id, const wxPoint &pos, const wxSize &sz)
	: wxPanel(parent, id, pos, sz, wxCLIP_CHILDREN)
{
	
	m_ribbonColour = *wxWHITE;
	m_textOnRight = false;
	m_highlightColour = wxColour(224,232,246);
	m_selectColour = wxColour(193,210,238);
	m_textColour = *wxBLACK;
	m_xSpacing = 4;
	m_xOffset = 6;
	m_border = 0;
	m_ribbonHeight = 52;

	m_ctrlPanel = 0;
	m_ctrlPanelWidth = 0;

	m_renderer = new wxRibbonRenderer(this);
	m_flipper = new wxSimplebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );

	Reposition();
}

void wxRibbonNotebook::AddControlPanel(wxPanel *panel, int width)
{
	m_ctrlPanel = panel;
	if (panel) panel->Reparent(this);
	m_ctrlPanelWidth = width;
	Reposition();
}

void wxRibbonNotebook::AddPage(wxWindow *win, const wxString &text, const wxBitmap &icon, bool active)
{
	win->Reparent( m_flipper );
	m_flipper->AddPage(win, text);

	page_info x;
	x.text = text;
	x.icon = icon;
	x.overlay = wxNullBitmap;
	x.overlay_pos = wxRIGHT|wxBOTTOM;
	x.scroll_win = 0;

	m_pageList.push_back( x );


	if (active)
		m_flipper->ChangeSelection( m_flipper->GetPageCount()-1 );

	Reposition();
}

int wxRibbonNotebook::GetPageIndex(wxWindow *win)
{
	if ( win == NULL ) return -1;
	int ndx = -1;
	for (int i=0; i<m_flipper->GetPageCount(); i++)
	{
		if ( m_flipper->GetPage(i) == win )
			ndx = i;
	}
	return ndx;
}


wxWindow *wxRibbonNotebook::RemovePage( int ndx )
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

void wxRibbonNotebook::DeletePage( int ndx )
{
	wxWindow *win = RemovePage( ndx );
	if ( win != 0 ) win->Destroy();
}

wxWindow *wxRibbonNotebook::GetPage( int index )
{
	return m_flipper->GetPage( index );
}

void wxRibbonNotebook::AddScrolledPage(wxWindow *win, const wxString &text, const wxBitmap &icon, bool active)
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
	x.icon = icon;
	x.overlay = wxNullBitmap;
	x.scroll_win = win;
	x.overlay_pos = wxRIGHT|wxBOTTOM;

	m_pageList.push_back( x );

	if (active)
		m_flipper->ChangeSelection( m_flipper->GetPageCount()-1 );

	Reposition();
}

int wxRibbonNotebook::GetSelection()
{
	return m_flipper->GetSelection();
}

void wxRibbonNotebook::SetIcon(int id, const wxBitmap &icon)
{
	if ((id < m_pageList.size()) && (id >= 0))
	{
		m_pageList[id].icon = icon;
		Reposition();
	}
}

void wxRibbonNotebook::SetOverlay(int id, const wxBitmap &overlay, unsigned int position )
{
	if ( id < m_pageList.size() && id >= 0 )
	{
		m_pageList[id].overlay = overlay;
		m_pageList[id].overlay_pos = position;
		
	}
}

void wxRibbonNotebook::SetText(int id, const wxString &text)
{
	if ((id < (int)m_pageList.size()) && (id >= 0))
	{
		m_pageList[id].text = text;
		Reposition();
	}
}

void wxRibbonNotebook::SetSelection(int id)
{
	m_flipper->SetSelection(id);
	Refresh();
}

int wxRibbonNotebook::GetPageCount()
{
	return m_flipper->GetPageCount();
}
void wxRibbonNotebook::Reposition()
{
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);
	
	int max_bit_height = 0;
	for (size_t i=0;i<m_pageList.size();i++)
		if (m_pageList[i].icon.GetHeight() > max_bit_height)
			max_bit_height = m_pageList[i].icon.GetHeight();

	wxClientDC cdc(this);
	cdc.SetFont(*wxNORMAL_FONT);

	if (!m_textOnRight)
	{
		if ((max_bit_height + cdc.GetCharHeight() + 19) > m_ribbonHeight)
			m_ribbonHeight = max_bit_height + cdc.GetCharHeight() + 19;
	}
	else
		m_ribbonHeight = max_bit_height + 10;

	int cpwidth = 0;

	if (m_ctrlPanel != 0)
	{
		cpwidth = m_ctrlPanelWidth;
		if (cpwidth < 0) m_ctrlPanel->GetBestSize(&cpwidth,NULL);
		m_ctrlPanel->SetSize(0,0, cpwidth, m_ribbonHeight);
	}

	m_renderer->SetSize(cpwidth,0, cwidth-cpwidth, m_ribbonHeight);
	m_flipper->SetSize(m_border, m_ribbonHeight+m_border, cwidth-m_border-m_border, cheight-m_border-m_border-m_ribbonHeight);

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

void wxRibbonNotebook::OnSize(wxSizeEvent &evt)
{
	Reposition();
}

