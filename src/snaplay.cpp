#include <wx/dcbuffer.h>

#include <wex/snaplay.h>


BEGIN_EVENT_TABLE( wxSnapLayout, wxScrolledWindow )
	EVT_SIZE( wxSnapLayout::OnSize )
	EVT_ERASE_BACKGROUND( wxSnapLayout::OnErase )
	EVT_PAINT( wxSnapLayout::OnPaint )
END_EVENT_TABLE()

wxSnapLayout::wxSnapLayout( wxWindow *parent, int id, const wxPoint &pos, const wxSize &size )
	: wxScrolledWindow( parent, id, pos, size, wxHSCROLL|wxVSCROLL|wxCLIP_CHILDREN )
{
	SetBackgroundStyle( wxBG_STYLE_PAINT );
	m_space = 20;
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

		icol++;

		if ( row_shift )
			x += l.active.width;
		
		idx++;
	}
	
	
	SetScrollbars( 1, 1, client.x, y+row_max_height, vs.x, vs.y );
	SetScrollRate( 30, 30 );
}


void wxSnapLayout::OnLeftDown( wxMouseEvent & )
{
}

void wxSnapLayout::OnLeftUp( wxMouseEvent & )
{
}

void wxSnapLayout::OnMotion( wxMouseEvent & )
{
}

void wxSnapLayout::OnCaptureLost( wxMouseCaptureLostEvent & )
{
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
	dc.SetFont( *wxSMALL_FONT );
	dc.SetTextForeground( *wxRED );

	for( size_t i=0;i<m_list.size();i++)
	{
		dc.DrawRectangle( m_list[i]->active );
		dc.DrawText( wxString::Format( "(%d) row %d col %d size: %d %d", i, 
			m_list[i]->row, m_list[i]->col,
			m_list[i]->rect.width, m_list[i]->rect.height ), 
			m_list[i]->active.x+1, m_list[i]->active.y+1);
	}

	*/
	dc.SetBrush( *wxLIGHT_GREY_BRUSH );
	dc.SetPen( *wxLIGHT_GREY_PEN );
	for( size_t i=0;i<m_list.size();i++)
		if (m_list[i]->highlight ) dc.DrawRectangle( m_list[i]->active );
}