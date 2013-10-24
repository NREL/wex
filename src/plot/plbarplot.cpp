
#include <wx/dc.h>
#include "wex/plot/plbarplot.h"


wxPLBarPlot::wxPLBarPlot()
{
	Init();
}

wxPLBarPlot::wxPLBarPlot( const std::vector<wxRealPoint> &data, 
	const wxString &label, 
	const wxColour &col )
	: wxPLPlottable( label )
{
	Init();
	m_colour = col;
	m_data = data;
}

wxPLBarPlot::~wxPLBarPlot()
{
	// nothing to do
}

void wxPLBarPlot::Init()
{
	m_stackedOn = 0;
	m_colour = *wxLIGHT_GREY;
	m_scaleThickness = true;
	m_thickness = 10;
}
	
wxRealPoint wxPLBarPlot::At( size_t i ) const
{
	return m_data[i];
}

size_t wxPLBarPlot::Len() const
{
	return m_data.size();
}

void wxPLBarPlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.SetBrush( wxBrush( m_colour ) );
	dc.DrawRectangle( rct );
}


double wxPLBarPlot::CalcYStart(double x)
{
	double y_start = 0;

	if (m_stackedOn && m_stackedOn != this)
		y_start += m_stackedOn->CalcYStart(x);

	for (size_t i=0; i<Len(); i++)
	{
		if (At(i).x == x)
		{
			y_start += At(i).y;
			break;
		}
	}

	return y_start;
}

double wxPLBarPlot::CalcXStart(double x, const wxPLDeviceMapping &map, int dispwidth)
{
	if (m_stackedOn && m_stackedOn != this)
		return m_stackedOn->CalcXStart(x, map, dispwidth);

	double x_start = map.ToDevice( x, 0 ).x;

	size_t g, idx;
	std::vector<wxPLBarPlot*>::iterator iit = std::find( m_group.begin(), m_group.end(), this );
	if ( m_group.size() > 0 
		&&  iit != m_group.end() )
	{
		idx = iit-m_group.begin();

		std::vector<size_t> barwidths;
		for (g=0;g<m_group.size();g++)
			barwidths.push_back( m_group[g]->CalcDispBarWidth(map) );

		double group_width = 0;
		for (g=0;g<m_group.size();g++)
			group_width += barwidths[g];

		double bar_start = x_start - group_width/2;
		for (g=0;g<idx;g++)
			bar_start += barwidths[g];

		x_start = bar_start + dispwidth/2;
	}

	return x_start;
}

int wxPLBarPlot::CalcDispBarWidth( const wxPLDeviceMapping &map )
{
	if ( m_scaleThickness )
	{
		wxRect rct = map.GetDeviceExtents();
		double xmin = map.GetWorldMinimum().x;
		double xmax = map.GetWorldMaximum().x;

		int bars_in_view = 0;
		for ( size_t i=0;i<Len();i++ )
		{
			double x = At(i).x;
			if ( x >= xmin && x <= xmax )
				bars_in_view++;
		}

		return (int)( ((double)rct.GetWidth()) / ((double)( bars_in_view + 3 )) );
	}
	else return m_thickness;
}

void wxPLBarPlot::Draw( wxDC &dc, const wxPLDeviceMapping &map )
{
	if ( Len() == 0 ) return;

	int dispbar_w = CalcDispBarWidth( map );
		
	dc.SetBrush(wxBrush(m_colour));
	dc.SetPen( *wxTRANSPARENT_PEN );
	
	for (size_t i=0; i<Len(); i++)
	{
		wxRealPoint pt = At(i);
		int pbottom=0, ptop=0;
		double y_start=0;
		double x_start = CalcXStart( pt.x, map, dispbar_w );

		if (m_stackedOn != NULL && m_stackedOn != this)
		{
			y_start = m_stackedOn->CalcYStart( pt.x );	
			x_start = m_stackedOn->CalcXStart( pt.x, map, dispbar_w);
		}

		wxRect prct;
		prct.x = x_start - dispbar_w/2;
		prct.width = dispbar_w;

		pbottom = map.ToDevice( 0, y_start ).y;
		ptop = map.ToDevice( 0, pt.y + y_start ).y;

		prct.y = pbottom < ptop ? pbottom : ptop;
		prct.height = abs( pbottom-ptop );

		dc.DrawRectangle(prct.x, prct.y, prct.width, prct.height);
	}
}

