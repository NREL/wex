#include <wx/dc.h>
#include "plot/pllineplot.h"

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
						   bool scale )
	: wxPLPlottable( label )
{
	Init();
	m_colour = col;
	m_data = data;
	m_style = sty;
	m_thickness = thick;
	m_scaleThickness = scale;
}

wxPLLinePlot::~wxPLLinePlot()
{
	// nothing to do 
}

void wxPLLinePlot::Init()
{
	m_colour = "forest green";
	m_thickness = 2;
	m_scaleThickness = false;
	m_style = SOLID;
}

wxRealPoint wxPLLinePlot::At( size_t i ) const
{
	return m_data[i];
}

size_t wxPLLinePlot::Len() const
{
	return m_data.size();
}


void wxPLLinePlot::Draw( wxDC &dc, const wxPLDeviceMapping &map )
{
	size_t len = Len();
	if ( len < 2 ) return;

	dc.SetPen( wxPen( m_colour, m_thickness, GetWxLineStyle(m_style) ) );
	wxPoint start = map.ToDevice( At(0).x, At(0).y );

	std::vector< wxPoint > points( len );
	for ( size_t i = 0; i<len; i++ )
		points[i] = map.ToDevice( At(i) );
	
	dc.DrawLines( points.size(), &points[0] );
	
	/*
	// example code to fill the plot below the line to the bottom of the 
	// device extents.  uses alpha channel for color transparency
	// someday this could be fully implemented...
	
	wxCoord phys_left = points[0].x;
	wxCoord phys_right = points[ points.size()-1 ].x;
	wxCoord phys_ymax = map.GetDeviceExtents().y + map.GetDeviceExtents().height;

	points.push_back( wxPoint( phys_right, phys_ymax ) );
	points.push_back( wxPoint( phys_left, phys_ymax ) );
	
	dc.SetPen( *wxTRANSPARENT_PEN );
	wxColour transcol( m_colour.Red(), m_colour.Green(), m_colour.Blue(), 100 );
	dc.SetBrush( wxBrush( transcol, wxSOLID ) );
	dc.DrawPolygon( points.size(), &points[0] );
	*/
}

void wxPLLinePlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	dc.SetPen( wxPen( m_colour, m_thickness, GetWxLineStyle(m_style) ) );
	dc.DrawLine( rct.x, rct.y+rct.height/2, rct.x+rct.width, rct.y+rct.height/2 );
}
