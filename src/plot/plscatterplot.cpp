#include <algorithm>

#include <wx/dc.h>

#include "wex/plot/plcolourmap.h"
#include "wex/plot/plscatterplot.h"

wxPLScatterPlot::wxPLScatterPlot()
{
	m_cmap = 0;
	m_colour = *wxBLUE;
	m_radius = 3;
	m_scale = false;
	m_antiAliasing = false;
	m_drawLineOfPerfectAgreement = false;
}

wxPLScatterPlot::wxPLScatterPlot( const std::vector<wxRealPoint> &data,
	const wxString &label,
	const wxColour &col,
	int size,
	bool scale )
	: wxPLPlottable( label )
{
	m_cmap = 0;
	m_data = data;
	m_colour = col;
	m_radius = size;
	m_scale = scale;
	m_antiAliasing = false;
	m_drawLineOfPerfectAgreement = false;
}


wxPLScatterPlot::~wxPLScatterPlot()
{
	// nothing to do currently
}

void wxPLScatterPlot::SetColourMap( wxPLColourMap *cmap )
{
	m_cmap = cmap;
}

void wxPLScatterPlot::SetColours( const std::vector<double> &zv )
{
	m_colours = zv;
}

void wxPLScatterPlot::ClearColours()
{
	m_colours.clear();
}

void wxPLScatterPlot::SetSizes( const std::vector<double> &sv )
{
	m_sizes = sv;
}

void wxPLScatterPlot::ClearSizes()
{
	m_sizes.clear();
}
	
wxRealPoint wxPLScatterPlot::At( size_t i ) const
{
	return m_data[i];
}

size_t wxPLScatterPlot::Len() const
{
	return m_data.size();
}

void wxPLScatterPlot::Draw( wxDC &dc, const wxPLDeviceMapping &map )
{
	dc.SetPen( wxPen( m_colour, 1 ) );
	dc.SetBrush( wxBrush( m_colour ) );

	wxRealPoint min = map.GetWorldMinimum();
	wxRealPoint max = map.GetWorldMaximum();

	wxPLColourMap *zcmap = 0;
	if ( m_cmap && m_colours.size() == m_data.size() )
		zcmap = m_cmap;

	bool has_sizes = ( m_sizes.size() == m_data.size() );

	for ( size_t i=0; i<Len(); i++ )
	{
		const wxRealPoint p = At(i);
		if ( p.x >= min.x && p.x <= max.x
			&& p.y >= min.y && p.y <= max.y )
		{
			int rad = m_radius;

			if ( has_sizes )
			{
				rad = (int) m_sizes[i];
				if ( rad < 1 ) rad = 1;
			}
			
			if ( zcmap )
			{
				wxColour C( zcmap->ColourForValue( m_colours[i] ) );
				dc.SetPen( wxPen( C, 1 ) );
				dc.SetBrush( wxBrush( C ) );
			}

			if ( rad < 2 )
				dc.DrawPoint( map.ToDevice( p ) );			
			else
				dc.DrawCircle( map.ToDevice(p), rad );
		}
	}

	if ( m_drawLineOfPerfectAgreement 
		&& !m_isLineOfPerfectAgreementDrawn )
	{
		m_isLineOfPerfectAgreementDrawn = true;		
		dc.SetPen(wxPen(*wxBLACK, m_radius));
		dc.SetBrush(wxBrush(*wxBLACK));

		wxRealPoint pstart, pend;
		if (min.x <= min.y) pstart = wxRealPoint(min.y, min.y);
		else pstart = wxRealPoint(min.x, min.x);

		if (max.x <= max.y) pend = wxRealPoint(max.x, max.x);
		else pend = wxRealPoint(max.y, max.y);

		dc.DrawLine(map.ToDevice(pstart), map.ToDevice(pend));
	}
}

void wxPLScatterPlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	dc.SetPen( wxPen( m_colour, 1 ) );
	dc.SetBrush( wxBrush( m_colour ) );
	wxCoord rad = std::min( rct.width, rct.height );
	rad = rad/2 - 2;
	if ( rad < 2 ) rad = 2;
	dc.DrawCircle( rct.x+rct.width/2, rct.y+rct.height/2, rad );
}
void wxPLScatterPlot::SetLineOfPerfectAgreementFlag(bool flagValue)
{
	m_drawLineOfPerfectAgreement = flagValue;
	m_isLineOfPerfectAgreementDrawn = false;
}
