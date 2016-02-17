
#include <wx/dc.h>
#include "wex/plot/plbarplot.h"
#include <algorithm>


wxPLBarPlotBase::wxPLBarPlotBase()
{
	Init();
}

wxPLBarPlotBase::wxPLBarPlotBase( const std::vector<wxRealPoint> &data, 
	const wxString &label, 
	const wxColour &col )
	: wxPLPlottable( label )
{
	Init();
	m_colour = col;
	m_data = data;
}

wxPLBarPlotBase::~wxPLBarPlotBase()
{
	// nothing to do
}

void wxPLBarPlotBase::Init()
{
	m_colour = *wxLIGHT_GREY;
	m_thickness = wxPL_BAR_AUTOSIZE;
}
	
wxRealPoint wxPLBarPlotBase::At( size_t i ) const
{
	return m_data[i];
}

size_t wxPLBarPlotBase::Len() const
{
	return m_data.size();
}

void wxPLBarPlotBase::DrawInLegend( wxPLOutputDevice &dc, const wxRect &rct)
{
	dc.Pen( m_colour );
	dc.Brush( m_colour );
	dc.Rect( rct );
}

////////// wxPLBarPlot ///////////


wxPLBarPlot::wxPLBarPlot()
	: wxPLBarPlotBase()
{
	m_stackedOn = 0;
}

wxPLBarPlot::wxPLBarPlot( const std::vector<wxRealPoint> &data, 
	const wxString &label, 
	const wxColour &col )
	: wxPLBarPlotBase( data, label, col )
{
	m_stackedOn = 0;
}

wxPLBarPlot::~wxPLBarPlot()
{
	// nothing to do
}

wxPLAxis *wxPLBarPlot::SuggestYAxis() const
{
	if ( Len() < 1 ) return new wxPLLinearAxis( 0, 1 );

	double ymin = CalcYPos( At(0).x );
	double ymax = ymin;

	for (size_t i=1; i<Len(); i++)
	{
		double y = CalcYPos( At(i).x );
		if ( y < ymin ) ymin = y;
		if ( y > ymax ) ymax = y;
	}

	if ( ymin > 0 ) ymin = 0;
	if ( ymax < 0 ) ymax = 0;

	return new wxPLLinearAxis( ymin, ymax );
}

double wxPLBarPlot::CalcYPos(double x) const
{
	double y_start = 0;

	if (m_stackedOn && m_stackedOn != this)
		y_start += m_stackedOn->CalcYPos(x);

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

double wxPLBarPlot::CalcXPos(double x, const wxPLDeviceMapping &map, int dispwidth)
{
	if (m_stackedOn && m_stackedOn != this)
		return m_stackedOn->CalcXPos(x, map, dispwidth);

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
	if ( m_thickness <= 1 )
	{
		wxRealPoint pos, size;
		map.GetDeviceExtents( &pos, &size );
		double xmin = map.GetWorldMinimum().x;
		double xmax = map.GetWorldMaximum().x;

		int bars_in_view = 0;
		for ( size_t i=0;i<Len();i++ )
		{
			double x = At(i).x;
			if ( x >= xmin && x <= xmax )
				bars_in_view++;
		}

		if ( m_group.size() > 0 ) bars_in_view *= m_group.size();

		return (int)( ((double)size.x) / ((double)( bars_in_view + 4 )) );
	}
	else return m_thickness;
}

void wxPLBarPlot::Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map )
{
	if ( Len() == 0 ) return;

	int dispbar_w = CalcDispBarWidth( map );
	
	dc.Pen( m_colour );
	dc.Brush( m_colour );
	
	for (size_t i=0; i<Len(); i++)
	{
		wxRealPoint pt = At(i);
		int pbottom=0, ptop=0;
		double y_start=0;
		double x_start = CalcXPos( pt.x, map, dispbar_w );

		if (m_stackedOn != NULL && m_stackedOn != this)
		{
			y_start = m_stackedOn->CalcYPos( pt.x );	
			x_start = m_stackedOn->CalcXPos( pt.x, map, dispbar_w);
		}

		wxRect prct;
		prct.x = x_start - dispbar_w/2;
		prct.width = dispbar_w;

		pbottom = map.ToDevice( 0, y_start ).y;
		ptop = map.ToDevice( 0, pt.y + y_start ).y;

		prct.y = pbottom < ptop ? pbottom : ptop;
		prct.height = abs( pbottom-ptop );

		dc.Rect(prct.x, prct.y, prct.width, prct.height);
	}
}


/////////// wxPLHBarPlot ////////////

wxPLHBarPlot::wxPLHBarPlot() : wxPLBarPlotBase() { /* nothing to do */ }

wxPLHBarPlot::wxPLHBarPlot( const std::vector<wxRealPoint> &data, double baseline_x,
	const wxString &label,
	const wxColour &col)
	: wxPLBarPlotBase( data, label, col ), m_baselineX( baseline_x )
{
	m_stackedOn = 0;
}

wxPLHBarPlot:: ~wxPLHBarPlot()
{
	m_stackedOn = 0;
}

void wxPLHBarPlot::Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map )
{
	if( Len() == 0 ) return;

	int bar_width = CalcDispBarWidth( map );
	dc.Pen( m_colour );
	dc.Brush( m_colour );
	for (size_t i=0; i<Len(); i++)
	{
		wxRealPoint pt(  At(i) );
		int pleft=0, pright=0;
		double x_start=m_baselineX;
			
		if ( m_stackedOn != NULL && m_stackedOn != this )
			x_start = m_stackedOn->CalcXPos( pt.y );	

		wxRect prct;

		pleft = map.ToDevice( x_start, 0 ).x;
		pright = map.ToDevice( pt.x, 0 ).x;

		if (pleft < pright)
		{
			prct.x = pleft;
			prct.width = pright-pleft;
		}
		else
		{
			prct.x = pright;
			prct.width = pleft-pright;
		}
						
		prct.y = map.ToDevice( 0, pt.y ).y - bar_width/2;
		prct.height = bar_width;
		prct.width = abs(pleft - pright);
		if ( prct.width > 0 && prct.height > 0 )
			dc.Rect(prct.x, prct.y, prct.width, prct.height);
	}
	
	wxRealPoint start,end;
	end.x = start.x = map.ToDevice( m_baselineX, 0 ).x;
	
	wxRealPoint pos, size;
	map.GetDeviceExtents( &pos, &size );

	start.y = pos.y+1;
	end.y = start.y + size.y-1;
	
	dc.Pen( *wxBLACK, 1 );
	dc.Line(start.x, start.y, end.x, end.y);
}

	
double wxPLHBarPlot::CalcXPos(double y) const
{
	double x_start = m_baselineX;

	if (m_stackedOn && m_stackedOn != this)
		x_start += m_stackedOn->CalcXPos(y);

	for (size_t i=0; i<Len(); i++)
	{
		if (At(i).y == y)
		{
			x_start += At(i).x;
			break;
		}
	}

	return x_start;

}

int wxPLHBarPlot::CalcDispBarWidth( const wxPLDeviceMapping &map )
{
	if ( m_thickness <= 1 )
	{
		wxRealPoint pos, size;
		map.GetDeviceExtents( &pos, &size );
		double ymin = map.GetWorldMinimum().y;
		double ymax = map.GetWorldMaximum().y;

		int bars_in_view = 0;
		for ( size_t i=0;i<Len();i++ )
		{
			double y = At(i).y;
			if ( y >= ymin && y <= ymax )
				bars_in_view++;
		}


		return (int)( ((double)size.y) / ((double)( bars_in_view + 4 )) );
	}
	else return m_thickness;
}

wxPLAxis *wxPLHBarPlot::SuggestXAxis() const
{
	if ( Len() < 1 ) return new wxPLLinearAxis( 0, 1 );

	double xmin = CalcXPos( At(0).y );
	double xmax = xmin;

	for (size_t i=1; i<Len(); i++)
	{
		double x = CalcXPos( At(i).y );
		if ( x < xmin ) xmin = x;
		if ( x > xmax ) xmax = x;
	}

	if ( xmin > m_baselineX ) xmin = m_baselineX;
	if ( xmax < m_baselineX ) xmax = m_baselineX;

	return new wxPLLinearAxis( xmin, xmax );
}
