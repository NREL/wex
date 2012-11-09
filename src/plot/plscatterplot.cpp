#include <wx/dc.h>
#include "wex/plot/plscatterplot.h"


wxPLScatterPlot::wxPLScatterPlot()
{
	m_colour = *wxBLUE;
	m_size = 1;
	m_scale = false;
	m_antiAliasing = false;
}

wxPLScatterPlot::wxPLScatterPlot( const std::vector<wxRealPoint> &data,
	const wxString &label,
	const wxColour &col,
	int size,
	bool scale )
	: wxPLPlottable( label )
{
	m_data = data;
	m_colour = col;
	m_size = size;
	m_scale = scale;
	m_antiAliasing = false;
}


wxPLScatterPlot::~wxPLScatterPlot()
{
	// nothing to do currently
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
	dc.SetPen( wxPen( m_colour, m_size ) );
	dc.SetBrush( wxBrush( m_colour ) );

	wxRealPoint min = map.GetWorldMinimum();
	wxRealPoint max = map.GetWorldMaximum();

	for ( size_t i=0; i<Len(); i++ )
	{
		const wxRealPoint p = At(i);
		if ( p.x >= min.x && p.x <= max.x
			&& p.y >= min.y && p.y <= max.y )
			if ( m_size < 2 )
				dc.DrawPoint( map.ToDevice( p ) );			
			else
				dc.DrawCircle( map.ToDevice(p), m_size );
	}
}

void wxPLScatterPlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	dc.SetPen( *wxBLACK_PEN );
	dc.SetBrush( wxBrush( m_colour ) );
	dc.DrawRectangle( rct );
}
