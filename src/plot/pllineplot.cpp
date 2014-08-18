

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

void wxPLLinePlot::Draw( wxDC &dc, const wxPLDeviceMapping &map )
{
	size_t len = Len();
	if ( len < 2 ) return;

	wxRealPoint wmin( map.GetWorldMinimum() );
	wxRealPoint wmax( map.GetWorldMaximum() );

	dc.SetPen( wxPen( m_colour, m_thickness, GetWxLineStyle(m_style) ) );
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
			points.clear();
		}
	}
	
	if ( points.size() > 1 )
		dc.DrawLines( points.size(), &points[0] );
}

void wxPLLinePlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	dc.SetPen( wxPen( m_colour, m_thickness, GetWxLineStyle(m_style) ) );
	dc.DrawLine( rct.x, rct.y+rct.height/2, rct.x+rct.width, rct.y+rct.height/2 );
}
