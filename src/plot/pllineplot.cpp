

#include <wx/dc.h>
#include "wex/plot/pllineplot.h"

#ifdef __WXOSX__
#include <cmath>
#define wxIsNaN(a) std::isnan(a)
#endif

static int GetWxLineStyle( wxPLLinePlot::Style sty )
{
	int wxsty = wxPENSTYLE_SOLID;
	if ( sty == wxPLLinePlot::DASHED ) wxsty = wxPENSTYLE_LONG_DASH;
	if ( sty == wxPLLinePlot::DOTTED ) wxsty = wxPENSTYLE_DOT;
	return wxsty;
}

wxPLLinePlot::wxPLLinePlot()
{
	Init();
}

wxPLLinePlot::wxPLLinePlot( const std::vector< wxRealPoint > &data, 
						   const wxString &label, const wxColour &col,
						   Style sty,
						   int thick,
						   Marker mkr )
	: wxPLPlottable( label )
{
	Init();
	m_colour = col;
	m_data = data;
	m_style = sty;
	m_thickness = thick;
	m_marker = mkr;
}

wxPLLinePlot::~wxPLLinePlot()
{
	// nothing to do 
}

void wxPLLinePlot::Init()
{
	m_colour = "forest green";
	m_thickness = 2;
	m_style = SOLID;
	m_marker = NONE;
	m_ignoreZeros = false;
}

wxRealPoint wxPLLinePlot::At( size_t i ) const
{
	return m_data[i];
}

size_t wxPLLinePlot::Len() const
{
	return m_data.size();
}

bool wxPLLinePlot::GetIgnoreZeros()
{
	return m_ignoreZeros;
}

void wxPLLinePlot::SetIgnoreZeros(bool value)
{
	m_ignoreZeros = value;
}

void wxPLLinePlot::DrawMarkers( wxDC &dc, std::vector<wxPoint> &points, int size )
{
	if ( m_marker == NONE ) return;

	std::vector<wxPoint> mkr(4, wxPoint() );
		
	int radius = (size<=3) ? 3 : 4;

	for( size_t i=0;i<points.size();i++ )
	{
		wxPoint &p = points[i];
		
		if ( m_marker == HOURGLASS )
		{
			mkr[0] = wxPoint( p.x-radius, p.y-radius );
			mkr[1] = wxPoint( p.x+radius, p.y-radius );
			mkr[2] = wxPoint( p.x-radius, p.y+radius );
			mkr[3] = wxPoint( p.x+radius, p.y+radius );
			
			dc.DrawPolygon( 4, &mkr[0] );
		}
		else if ( m_marker == SQUARE )
		{
			mkr[0] = wxPoint( p.x-radius, p.y-radius );
			mkr[1] = wxPoint( p.x+radius, p.y-radius );
			mkr[2] = wxPoint( p.x+radius, p.y+radius );
			mkr[3] = wxPoint( p.x-radius, p.y+radius );
			dc.DrawPolygon( 4, &mkr[0] );
		}
		else if ( m_marker == DIAMOND )
		{
			mkr[0] = wxPoint( p.x, p.y-radius );
			mkr[1] = wxPoint( p.x+radius, p.y );
			mkr[2] = wxPoint( p.x, p.y+radius );
			mkr[3] = wxPoint( p.x-radius, p.y );
			dc.DrawPolygon( 4, &mkr[0] );
		}
		else
		{
			dc.DrawCircle( p, radius );
		}
	}
}

void wxPLLinePlot::Draw( wxDC &dc, const wxPLDeviceMapping &map )
{
	size_t len = Len();
	if ( len < 2 ) return;

	wxRealPoint wmin( map.GetWorldMinimum() );
	wxRealPoint wmax( map.GetWorldMaximum() );
	
	wxPen line_pen( m_colour, m_thickness, GetWxLineStyle(m_style) );
	line_pen.SetJoin( wxJOIN_ROUND );

	dc.SetPen( line_pen );
	dc.SetBrush( wxBrush( m_colour, wxSOLID ) );

	std::vector< wxPoint > points;
	points.reserve( len );

	for ( size_t i = 0; i<len; i++ )
	{
		wxRealPoint pt( At(i) );
		bool nanval = wxIsNaN( pt.x ) || wxIsNaN( pt.y );
		if ( !nanval && pt.x >= wmin.x && pt.x <= wmax.x )
			points.push_back(map.ToDevice( At(i) ));

		if ( nanval && points.size() > 1 )
		{
			// draw currently accumulated points and clear
			// accumulator - this will draw the contiguous
			// segments of data that don't have any NaN values
			dc.DrawLines( points.size(), &points[0] );
			DrawMarkers( dc, points, m_thickness );
			points.clear();
		}
	}
	
	if ( points.size() > 1 )
	{
		dc.DrawLines( points.size(), &points[0] );
		DrawMarkers( dc, points, m_thickness );
	}
}

void wxPLLinePlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	int thick = m_thickness;
	if ( thick > 3 ) thick = 3; // limit line thickness for legend display
	dc.SetPen( wxPen( m_colour, m_thickness, GetWxLineStyle(m_style) ) );
	dc.SetBrush( wxBrush( m_colour, wxSOLID ) );
	dc.DrawLine( rct.x, rct.y+rct.height/2, rct.x+rct.width, rct.y+rct.height/2 );
	if ( m_marker != NONE )
	{
		std::vector<wxPoint> mkr(1, wxPoint( rct.x + rct.width/2, rct.y + rct.height/2 ) );
		DrawMarkers( dc, mkr, thick );
	}
}
