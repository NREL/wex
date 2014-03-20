#include <wx/dcbuffer.h>
#include <wx/frame.h>

#include <wex/snaplay.h>
#include <wex/metro.h>


BEGIN_EVENT_TABLE( wxSnapLayout, wxScrolledWindow )
	EVT_SIZE( wxSnapLayout::OnSize )
	EVT_ERASE_BACKGROUND( wxSnapLayout::OnErase )
	EVT_PAINT( wxSnapLayout::OnPaint )
	EVT_LEFT_DOWN( wxSnapLayout::OnLeftDown )
	EVT_LEFT_UP( wxSnapLayout::OnLeftUp )
	EVT_MOTION( wxSnapLayout::OnMotion )
	EVT_MOUSE_CAPTURE_LOST( wxSnapLayout::OnCaptureLost )
END_EVENT_TABLE()

wxSnapLayout::wxSnapLayout( wxWindow *parent, int id, const wxPoint &pos, const wxSize &size )
	: wxScrolledWindow( parent, id, pos, size, wxHSCROLL|wxVSCROLL|wxCLIP_CHILDREN )
{
	SetBackgroundStyle( wxBG_STYLE_PAINT );
	m_space = 20;
	m_active = 0;
	m_handle = -1;
	m_curtarget = 0;
	m_transp = 0;
}

wxSnapLayout::~wxSnapLayout()
{
	for( size_t i=0;i<m_list.size();i++ )
		delete m_list[i];
}
	
void wxSnapLayout::Add( wxWindow *win, int width, int height )
{
	if ( Find( win ) >= 0 ) return;

	if ( win->GetParent() != this )
		win->Reparent( this );

	layout_box *l = new layout_box;
	l->win = win;
	l->req.x = width;
	l->req.y = height;
	l->rect = wxRect( 0, 0, 500, 300 );
	l->row = -1;
	l->col = -1;
	l->highlight = false;
	m_list.push_back( l );

	AutoLayout();
}

void wxSnapLayout::ClearHighlights()
{
	for( size_t i=0;i<m_list.size();i++ )
		m_list[i]->highlight = false;

	Refresh();
}

void wxSnapLayout::Highlight( wxWindow *w )
{
	int i = Find( w );
	if ( i >= 0 )
	{
		m_list[i]->highlight = true;
		Refresh();
	}
}

void wxSnapLayout::Delete( wxWindow *w )
{
	int i = Find( w );
	if ( i >= 0 )
	{
		w->Destroy();
		delete m_list[i];
		m_list.erase( m_list.begin()+i );
		AutoLayout();
	}
}

int wxSnapLayout::Find( wxWindow *w )
{
	for( size_t i=0;i<m_list.size();i++ )
		if ( m_list[i]->win == w )
			return i;

	return -1;
}

void wxSnapLayout::DeleteAll()
{
	for( size_t i=0;i<m_list.size();i++ )
	{
		m_list[i]->win->Destroy();
		delete m_list[i];
	}
	m_list.clear();
	Refresh();
}
void wxSnapLayout::ScrollTo( wxWindow *w )
{
	int i = Find( w );
	if ( i >= 0 )
		Scroll( m_list[i]->rect.x, m_list[i]->rect.y );
}

#define LAYOUT(r,c) layout[m_list.size()*r+c]

void wxSnapLayout::AutoLayout()
{
	m_targets.clear();

	if ( m_list.size() == 0 ) return;

	wxPoint vs( GetViewStart() );
	wxSize client( GetClientSize() );

	int x = 0;
	int y = 0;

	int row_min_height = 10000;
	int row_max_height = 0;
	
	size_t nn = m_list.size();
	
	int irow = 0;
	int icol = 0;
	int idx = 0;
	while( idx < m_list.size() )
	{
		layout_box &l = *m_list[idx];
		wxSize sz( l.req.x <= 0 || l.req.y <= 0 ? l.win->GetBestSize() : l.req );

		// determine if this widget goes in this row

		l.active.width = sz.x + m_space + m_space;
		l.active.height = sz.y + m_space + m_space;

		bool row_shift = false;

		if ( x + l.active.width < client.x || idx == 0 )
		{
			// widget fits in this row
			l.active.x = x;
			l.active.y = y;
			x += l.active.width;
		}
		else
		{
			irow++;
			icol = 0;
			row_shift = true;

			// try to find a place in the next row

			// shift down to the next row
			y += row_max_height + m_space;
			x = 0;

			l.active.x = x;
			l.active.y = y;

			// reset row height trackers
			row_max_height = 0;
			row_min_height = 10000;
		}

		if ( l.active.height < row_min_height ) row_min_height = l.active.height;
		if ( l.active.height > row_max_height ) row_max_height = l.active.height;
		
		l.rect.x = l.active.x+m_space;
		l.rect.y = l.active.y+m_space;
		l.rect.width = sz.x;
		l.rect.height = sz.y;
		
		l.win->Move( CalcScrolledPosition( wxPoint( l.rect.x, l.rect.y ) ) );
		l.win->SetClientSize( l.rect.width, l.rect.height );
		l.row = irow;
		l.col = icol;

		drop_target dt;
		dt.index = idx;
		dt.target.x = l.active.x - m_space;
		dt.target.y = l.active.y;
		dt.target.width = m_space+m_space;
		dt.target.height = l.active.height;
		m_targets.push_back( dt );


		icol++;

		if ( row_shift )
			x += l.active.width;
		
		idx++;
	}
	
	
	SetScrollbars( 1, 1, client.x, y+row_max_height, vs.x, vs.y );
	SetScrollRate( 30, 30 );
}

wxSnapLayout::layout_box *wxSnapLayout::CheckActive( const wxPoint &p, int *handle )
{
	*handle = -1;
	for( size_t i=0;i<m_list.size();i++ )
	{
		if ( m_list[i]->active.Contains( p ) )
		{
			wxRect &r = m_list[i]->active;
			if ( p.x < r.x+m_space && p.y < r.y+m_space )
				*handle = NW;
			else if ( p.x < r.x+m_space && p.y > r.y+r.height-m_space )
				*handle = SW;
			else if ( p.x > r.x+r.width-m_space && p.y < r.y+m_space )
				*handle = NE;
			else if ( p.x > r.x+r.width-m_space && p.y > r.y+r.height-m_space )
				*handle = SE;

			return m_list[i];
		}
	}

	return 0;
}

void wxSnapLayout::OnLeftDown( wxMouseEvent &evt )
{
	wxPoint p = CalcScrolledPosition( evt.GetPosition() );		
	m_active = CheckActive( p, &m_handle );
	Refresh();
	
	if ( m_active )
	{
		switch( m_handle )
		{
		case SE:
		case NW:
			SetCursor( wxCURSOR_SIZENWSE ); break;
		case SW:
		case NE:
			SetCursor( wxCURSOR_SIZENESW ); break;
		default:
			SetCursor( wxCURSOR_SIZING );
		}
	}
	else SetCursor( wxCURSOR_DEFAULT );

	m_orig = ClientToScreen( evt.GetPosition() );

	if( m_active && m_handle >= 0 )
		ShowTransparency( m_active->rect );

	if ( m_active != 0 && !HasCapture() )
		CaptureMouse();
}

void wxSnapLayout::OnLeftUp( wxMouseEvent &evt )
{
	if ( HasCapture() )
		ReleaseMouse();

	if ( m_active != 0 && m_handle >= 0 )
	{

		m_active->req.x = m_sizerect.width;
		m_active->req.y = m_sizerect.height;

		AutoLayout();
	}
	else if ( m_active != 0 && m_handle < 0 && m_curtarget != 0 )
	{
		std::vector<layout_box*>::iterator it = std::find( m_list.begin(), m_list.end(), m_active );
		if ( it != m_list.end() )
		{
			m_list.erase( it );
			m_list.insert( m_list.begin() + m_curtarget->index, m_active );
			AutoLayout();
		}
	}


	m_active = 0;
	m_curtarget = 0;

	SetCursor( wxCURSOR_DEFAULT );
	Refresh();
	HideTransparency();
}

void wxSnapLayout::OnMotion( wxMouseEvent &evt )
{
	wxPoint p = CalcScrolledPosition( evt.GetPosition() );
	wxPoint diff = ClientToScreen( evt.GetPosition() ) - m_orig;
	
	if ( m_active && m_handle < 0 )
	{
		drop_target *last = m_curtarget;
		m_curtarget = 0;
		for( size_t i=0;i<m_targets.size();i++ )
			if ( m_targets[i].target.Contains( p ) )
				m_curtarget = &m_targets[i];

		wxRect r( m_active->rect );
		r.x += diff.x;
		r.y += diff.y;
		ShowTransparency( r );

		if ( last != m_curtarget )
			Refresh();
	}
	else if ( m_active && m_handle >= 0 )
	{		

		wxRect r( m_active->rect );
		switch( m_handle )
		{
		case NW:
			r.x += diff.x;
			r.y += diff.y;
			r.width -= diff.x;
			r.height -= diff.y;
			break;
		case NE:
			r.y += diff.y;
			r.width += diff.x;
			r.height -= diff.y;
			break;
		case SW:
			r.x += diff.x;
			r.width -= diff.x;
			r.height += diff.y;
			break;
		case SE:
			r.width += diff.x;
			r.height += diff.y;
			break;
		}


		if( r.width < 10 ) r.width = 10;
		if( r.height < 10 ) r.height = 10;

		m_sizerect = r;

		ShowTransparency( r );
	}
}

void wxSnapLayout::OnCaptureLost( wxMouseCaptureLostEvent & )
{
	m_active = 0;
	m_handle = -1;
	Refresh();
	HideTransparency();
}

void wxSnapLayout::OnSize( wxSizeEvent & )
{
	AutoLayout();
}

void wxSnapLayout::OnErase( wxEraseEvent & )
{
	/* nothing to do */
}

void wxSnapLayout::OnPaint( wxPaintEvent & )
{
	wxAutoBufferedPaintDC dc( this );
	DoPrepareDC(dc);

	dc.SetBackground( *wxWHITE_BRUSH );
	dc.Clear();
	
	/*
	dc.SetBrush( *wxTRANSPARENT_BRUSH );
	dc.SetPen( *wxBLUE_PEN );

	for( size_t i=0;i<m_list.size();i++)
	{
		dc.DrawRectangle( m_list[i]->active );
		dc.DrawText( wxString::Format( "(%d) row %d col %d size: %d %d", i, 
			m_list[i]->row, m_list[i]->col,
			m_list[i]->rect.width, m_list[i]->rect.height ), 
			m_list[i]->active.x+1, m_list[i]->active.y+1);
	}
	*/

	
	dc.SetPen( *wxTRANSPARENT_PEN );

	dc.SetBrush( wxMetroTheme::Colour( wxMT_SELECT ) );
	for( size_t i=0;i<m_list.size();i++)
		if (m_list[i]->highlight ) dc.DrawRectangle( m_list[i]->active );

	if ( m_active != 0 && m_handle < 0 )
	{
		dc.SetBrush( wxBrush( wxMetroTheme::Colour( wxMT_HIGHLIGHT ), wxSOLID ) );
		for( size_t i=0;i<m_targets.size();i++ )
			dc.DrawRectangle( m_targets[i].target );
	}

	if ( m_curtarget )
	{
		dc.SetBrush( wxBrush( wxMetroTheme::Colour( wxMT_DIMHOVER ) ) );
		dc.DrawRectangle( m_curtarget->target );
	}
}

void wxSnapLayout::ShowTransparency( wxRect r )
{
	wxPoint pos = ClientToScreen( wxPoint(r.x, r.y) );
	wxSize size(r.width, r.height);

	if ( m_transp == 0 )
	{
		m_transp = new wxFrame( this, wxID_ANY, wxEmptyString,  pos, size, 
			wxBORDER_NONE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR );
		m_transp->SetBackgroundColour( *wxLIGHT_GREY );
		m_transp->SetTransparent( 200 );
	}
	else
	{
		m_transp->Move( pos );
		m_transp->SetClientSize( size );
	}


	m_transp->ShowWithoutActivating();
}

void wxSnapLayout::HideTransparency()
{
	if ( m_transp )
	{
		m_transp->Hide();
		m_transp = 0;
	}
}