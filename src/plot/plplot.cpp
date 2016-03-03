#include <numeric>
#include <limits>

#include <wx/tokenzr.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>

#include "wex/pdf/pdfdoc.h"
#include "wex/pdf/pdffont.h"
#include "wex/plot/plhistplot.h"
#include "wex/plot/plplot.h"
#include "wex/plot/ploutdev.h"

#ifdef __WXOSX__
#include <cmath>
#define wxIsNaN(a) std::isnan(a)
#endif

static const double text_space = 3.0;
static const wxRealPoint legend_item_box(14.0, 14.0);

class wxPLAxisDeviceMapping : public wxPLDeviceMapping
{
private:
	wxPLAxis *m_xAxis;
	double m_xPhysMin, m_xPhysMax;
	wxPLAxis *m_yAxis;
	double m_yPhysMin, m_yPhysMax;
	double m_physicalConstraint;
	wxRealPoint m_ptCenter;
	bool m_primaryX, m_primaryY;

public:
	wxPLAxisDeviceMapping( wxPLAxis *x, double xmin, double xmax, bool primaryx,
		wxPLAxis *y, double ymin, double ymax, bool primaryy )
		: m_xAxis(x), m_xPhysMin(xmin), m_xPhysMax(xmax), m_primaryX(primaryx), 
		m_yAxis(y), m_yPhysMin(ymin), m_yPhysMax(ymax), m_primaryY(primaryy)
	{
		wxRealPoint pos, size;
		GetDeviceExtents( &pos, &size );
		m_ptCenter = wxRealPoint( 0.5*(pos.x+size.x), 0.5*(pos.y+size.y) );
		m_physicalConstraint = (size.x < size.y) ? size.x : size.y;
	}
	
	virtual wxRealPoint ToDevice( double x, double y ) const
	{
		if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(m_xAxis)) {
			// this is a polar plot, so translate the "point" as if it's a angle/radius combination

			// adjust for where zero degrees should be (straight up?) and units of angular measure
			double angle_in_rad = pa->AngleInRadians(x);

			// get radius in physical units
			double radius0 = m_yAxis->WorldToPhysical(y, 0, m_physicalConstraint / 2.0); // max radius has to be 1/2 of physical constraint

			// locate point relative to center of polar plot
			wxRealPoint pt(radius0*cos(angle_in_rad), radius0*sin(angle_in_rad));

			// return point on physical device
			return (m_ptCenter + pt);
		}
		else // Cartesian plot
			return wxRealPoint( m_xAxis->WorldToPhysical( x, m_xPhysMin, m_xPhysMax ),
				m_yAxis->WorldToPhysical( y, m_yPhysMin, m_yPhysMax ) );
	}
	
	virtual void GetDeviceExtents( wxRealPoint *pos, wxRealPoint *size ) const
	{
			if ( pos ) {
				pos->x = m_xPhysMin;
				pos->y = m_yPhysMin < m_yPhysMax ? m_yPhysMin : m_yPhysMax;
			}

			if ( size ) {
				size->x = m_xPhysMax - m_xPhysMin;
				size->y = fabs( m_yPhysMax - m_yPhysMin);
			}
	}

	virtual wxRealPoint GetWorldMinimum() const
	{
		return wxRealPoint(
			m_xAxis->PhysicalToWorld( m_xPhysMin, m_xPhysMin, m_xPhysMax ),
			m_yAxis->PhysicalToWorld( m_yPhysMin, m_yPhysMin, m_yPhysMax ) );
	}

	virtual wxRealPoint GetWorldMaximum() const
	{
		return wxRealPoint(
			m_xAxis->PhysicalToWorld( m_xPhysMax, m_xPhysMin, m_xPhysMax ),
			m_yAxis->PhysicalToWorld( m_yPhysMax, m_yPhysMin, m_yPhysMax ) );
	}

	virtual wxPLAxis *GetXAxis() const {
		return m_xAxis;
	}

	virtual wxPLAxis *GetYAxis() const {
		return m_yAxis;
	}

	virtual bool IsPrimaryXAxis() const { return m_primaryX; }
	virtual bool IsPrimaryYAxis() const { return m_primaryY; }
};


wxString wxPLPlottable::GetXDataLabel( wxPLPlot *plot ) const
{
	if ( !m_xLabel.IsEmpty() ) return m_xLabel;
	
	wxPLPlot::AxisPos xap, yap;
	wxPLPlot::PlotPos ppos;
	if ( plot && plot->GetPlotPosition( this, &xap, &yap, &ppos ) )
		if ( wxPLAxis *ax = plot->GetAxis( xap, ppos ) )
			return ax->GetLabel();
	
	return wxEmptyString;
}

wxString wxPLPlottable::GetYDataLabel( wxPLPlot *plot ) const
{
	if ( !m_yLabel.IsEmpty() ) return m_yLabel;
	if ( !m_label.IsEmpty() ) return m_label;
	
	wxPLPlot::AxisPos xap, yap;
	wxPLPlot::PlotPos ppos;
	if ( plot && plot->GetPlotPosition( this, &xap, &yap, &ppos ) )
		if ( wxPLAxis *ax = plot->GetAxis( yap, ppos ) )
			return ax->GetLabel();
	
	return wxEmptyString;
}

wxPLAxis *wxPLPlottable::SuggestXAxis() const 
{
	double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
	GetMinMax( &xmin, &xmax, &ymin, &ymax );
	return new wxPLLinearAxis( xmin, xmax );
}

wxPLAxis *wxPLPlottable::SuggestYAxis() const 
{
	double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
	GetMinMax( &xmin, &xmax, &ymin, &ymax );
	return new wxPLLinearAxis( ymin, ymax );
}

bool wxPLPlottable::GetMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax) const
{
	if (Len() == 0) return false;

	double myXMin = std::numeric_limits<double>::quiet_NaN();
	double myXMax = std::numeric_limits<double>::quiet_NaN();
	double myYMin = std::numeric_limits<double>::quiet_NaN();
	double myYMax = std::numeric_limits<double>::quiet_NaN();

	for (size_t i=0; i<Len(); i++)
	{
		wxRealPoint pt(At(i));

		bool nanval = wxIsNaN( pt.x ) || wxIsNaN( pt.y );
		if ( !nanval )
		{
			if ( wxIsNaN( myXMin ) ) 
				myXMin = pt.x;
			if ( wxIsNaN( myXMax ) ) 
				myXMax = pt.x;
			if ( wxIsNaN( myYMin ) ) 
				myYMin = pt.y;
			if ( wxIsNaN( myYMax ) ) 
				myYMax = pt.y;

			if ( pt.x < myXMin )
				myXMin = pt.x;
			if ( pt.x > myXMax )
				myXMax = pt.x;
			if ( pt.y < myYMin )
				myYMin = pt.y;
			if ( pt.y > myYMax )
				myYMax = pt.y;
		}
	}

	if (pxmin) *pxmin = myXMin;
	if (pxmax) *pxmax = myXMax;
	if (pymin) *pymin = myYMin;
	if (pymax) *pymax = myYMax;

	return !wxIsNaN( myXMin )
		&& !wxIsNaN( myXMax )
		&& !wxIsNaN( myYMin )
		&& !wxIsNaN( myYMax );
}

bool wxPLPlottable::ExtendMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax, bool extendToNice) const
{
	double xmin, xmax, ymin, ymax;
	if (!GetMinMax(&xmin, &xmax, &ymin, &ymax)) return false;
	double yminNice = ymin, ymaxNice = ymax;
	if (extendToNice) wxPLAxis::ExtendBoundsToNiceNumber(&ymaxNice, &yminNice);
	if (pxmin && xmin < *pxmin) *pxmin = xmin;
	if (pxmax && xmax > *pxmax) *pxmax = xmax;
	if (pymin && yminNice < *pymin) *pymin = yminNice;
	if (pymax && ymaxNice > *pymax) *pymax = ymaxNice;
	return true;
}

std::vector<wxString> wxPLPlottable::GetExportableDatasetHeaders( wxUniChar sep, wxPLPlot *plot ) const
{
	std::vector<wxString> tt;
	wxString xLabel = GetXDataLabel( plot );
	wxString yLabel = GetYDataLabel( plot );
			
	//Remove sep chars that we don't want
	while (xLabel.Find(sep) != wxNOT_FOUND)
	{
		xLabel = xLabel.BeforeFirst(sep) + xLabel.AfterFirst(sep);
	}

	while (yLabel.Find(sep) != wxNOT_FOUND)
	{
		yLabel = yLabel.BeforeFirst(sep) + yLabel.AfterFirst(sep);
	}

	tt.push_back(xLabel);
	tt.push_back(yLabel);

	return tt;
}

std::vector<wxRealPoint> wxPLPlottable::GetExportableDataset(double Xmin, double Xmax, bool visible_only) const
{
	std::vector<wxRealPoint> data;
	wxRealPoint pt;

	for (size_t i = 0; i < Len(); i++)
	{
		pt = At(i);
		if((pt.x >= Xmin && pt.x <= Xmax) || !visible_only) { data.push_back(At(i)); }
	}

	return data;
}

wxPLSideWidgetBase::wxPLSideWidgetBase()
{
	m_bestSize.x = m_bestSize.y = -1;
}

wxPLSideWidgetBase::~wxPLSideWidgetBase()
{
	// nothing to do
}

void wxPLSideWidgetBase::InvalidateBestSize()
{
	m_bestSize.x = m_bestSize.y = -1;
}

wxRealPoint wxPLSideWidgetBase::GetBestSize()
{
	if ( m_bestSize.x < 0 || m_bestSize.y < 0 )
		m_bestSize = CalculateBestSize();

	if ( m_bestSize.x <= 0 ) m_bestSize.x = 5;
	if ( m_bestSize.y <= 0 ) m_bestSize.y = 5;
	return m_bestSize;
}

struct escape_sequence
{
	wxString code;
	wxUniChar value;
};

static escape_sequence escape_codes[] = {

	{ "Alpha",   L'\x0391' },
	{ "Beta",    L'\x0392' },
	{ "Gamma",   L'\x0393' },
	{ "Delta",   L'\x0394' },
	{ "Epsilon", L'\x0395' },
	{ "Zeta",    L'\x0396' },
	{ "Eta",     L'\x0397' },
	{ "Theta",   L'\x0398' },
	{ "Iota",    L'\x0399' },
	{ "Kappa",   L'\x039a' },
	{ "Lambda",  L'\x039b' },
	{ "Mu",      L'\x039c' },
	{ "Nu",      L'\x039d' },
	{ "Xi",      L'\x039e' },
	{ "Omicron", L'\x039f' },
	{ "Pi",      L'\x03a0' },
	{ "Rho",     L'\x03a1' },
	{ "Sigma",   L'\x03a3' },
	{ "Tau",     L'\x03a4' },
	{ "Upsilon", L'\x03a5' },
	{ "Phi",     L'\x03a6' },
	{ "Chi",     L'\x03a7' },
	{ "Psi",     L'\x03a8' },
	{ "Omega",   L'\x03a9' },

	{ "alpha",   L'\x03b1' },
	{ "beta",    L'\x03b2' },
	{ "gamma",   L'\x03b3' },
	{ "delta",   L'\x03b4' },
	{ "epsilon", L'\x03b5' },
	{ "zeta",    L'\x03b6' },
	{ "eta",     L'\x03b7' },
	{ "theta",   L'\x03b8' },
	{ "iota",    L'\x03b9' },
	{ "kappa",   L'\x03ba' },
	{ "lambda",  L'\x03bb' },
	{ "mu",      L'\x03bc' },
	{ "nu",      L'\x03bd' },
	{ "xi",      L'\x03be' },
	{ "omicron", L'\x03bf' },
	{ "pi",      L'\x03c0' },
	{ "rho",     L'\x03c1' },
	{ "fsigma",  L'\x03c3' },
	{ "sigma",   L'\x03c3' },
	{ "tau",     L'\x03c4' },
	{ "upsilon", L'\x03c5' },
	{ "phi",     L'\x03c6' },
	{ "chi",     L'\x03c7' },
	{ "psi",     L'\x03c8' },
	{ "omega",   L'\x03c9' },

	{ "emph",    L'\x00a1' },
	{ "qmark",   L'\x00bf' },
	{ "cent",    L'\x00a2' },
	{ "pound",   L'\x00a3' },
	{ "euro",    L'\x20ac' },
	{ "section", L'\x00a7' },
	{ "dot",     L'\x00b7' },
	{ "mult",    L'\x00d7' },
	{ "copy",    L'\x00a9' },
	{ "reg",     L'\x00ae' },
	{ "deg",     L'\x00b0' },
	{ "pm",      L'\x00b1' },
	{ "ne",      L'\x2260' },
	{ "approx",  L'\x2248' },

	{ wxEmptyString, 0 },
};

class wxPLPlot::text_layout
{

	struct text_piece
	{
		text_piece() : text(wxEmptyString), state(NORMAL), origin(0, 0), size(0, 0), aligned_x(0) {  }

		enum TextState { NORMAL, SUPERSCRIPT, SUBSCRIPT }; 
		wxString text;
		TextState state;
		wxRealPoint origin;
		wxRealPoint size;
		double aligned_x;
	};

	std::vector< std::vector<text_piece> > m_lines;
	wxRealPoint m_bounds;

	static const int FontPointAdjust = 2;

public:
	enum TextAlignment { LEFT, CENTER, RIGHT };

	text_layout( wxPLOutputDevice &dc, const wxString &text, TextAlignment ta = LEFT )
		: m_bounds(0, 0)
	{
		if ( text.IsEmpty() ) return;

		double fontPoints = 0;
		bool fontBold = false;
		
		// get current font state for subsequent relative adjustments
		dc.Font( &fontPoints, &fontBold );

		// split text into lines, and parse each one into text pieces after resolving escape sequences
		wxArrayString lines = wxStringTokenize( text, "\r\n" );
		for (size_t i=0;i<lines.Count();i++)
			m_lines.push_back( parse( escape(lines[i]) ) );

		if ( m_lines.size() == 0 ) return;
		
		// compute extents of each text piece with the right font
		for ( size_t i=0;i<m_lines.size(); i++ )
		{
			for ( size_t j=0;j<m_lines[i].size(); j++ )
			{
				text_piece &tp = m_lines[i][j];
				double width, height;
				dc.Font( tp.state == text_piece::NORMAL ? fontPoints : fontPoints-FontPointAdjust, fontBold );
				dc.Measure( tp.text, &width, &height );
				tp.size.x = width;
				tp.size.y = height;
			}
		}

		// obtain the approximate heights for normal and small text
		dc.Font( fontPoints-FontPointAdjust, fontBold );
		double height_small = dc.CharHeight();
		dc.Font( fontPoints, fontBold );
		double height_normal = dc.CharHeight();

		// sequentially calculate the origins of each text piece
		
		const double offset = 0.25*height_small; // amount to raise/lower super/sub-scripts
		double y = 0;
		
		for ( size_t i=0;i<m_lines.size(); i++ )
		{
			double x = 0;
			bool has_sup = false, has_sub = false;
			// layout this line's X positions, keep track of whether it has super/subs
			for ( size_t j=0; j< m_lines[i].size(); j++ )
			{
				text_piece &tp = m_lines[i][j];
				if ( tp.state == text_piece::SUPERSCRIPT ) has_sup = true;
				else if (tp.state == text_piece::SUBSCRIPT ) has_sub = true;
				
				// save original relative alignment from x 
				// allows future realignment of text pieces if needed
				tp.aligned_x = x;

				tp.origin.x = x;
				x += tp.size.x;

			}

			// save the line width as the maximum bounds
			if ( x > m_bounds.x )
				m_bounds.x = x;

			// layout this line's Y positions
			if ( has_sup ) y += offset;
			for ( size_t j=0;j<m_lines[i].size(); j++ )
			{
				text_piece &tp = m_lines[i][j];
				if ( tp.state == text_piece::NORMAL )
					tp.origin.y = y;
				else if (tp.state == text_piece::SUPERSCRIPT )
					tp.origin.y = y - offset;
				else if (tp.state == text_piece::SUBSCRIPT )
					tp.origin.y = y + height_normal - 3*offset;
			}

			y += height_normal + offset/3;
		}

		if ( ta != LEFT )
			align( ta );

		// save the final y position as the maximum height
		m_bounds.y = y;
	}

	void align( int ta )
	{
		// realign x offsets for differently aligned text
		// now that we know the total bounds width
		// note: does not require recalculating text sizes since they are cached
		for ( size_t i=0;i<m_lines.size();i++ )
		{
			double line_width = 0;
			for ( size_t j=0;j<m_lines[i].size();j++ )
			{
				// restore original aligned positions for each text piece
				m_lines[i][j].origin.x = m_lines[i][j].aligned_x;
				line_width += m_lines[i][j].size.x;
			}

			double offset = 0;
			if ( ta == CENTER ) offset = 0.5*(m_bounds.x - line_width);
			else if ( ta == RIGHT ) offset = m_bounds.x - line_width;

			if ( offset != 0 ) // only do this for center/right alignments
				for ( size_t j=0;j<m_lines[i].size();j++ )
					m_lines[i][j].origin.x += offset;
		}
	}

	void render( wxPLOutputDevice &dc, double x, double y, double rotationDegrees = 0.0, bool draw_bounds = false )
	{
		if ( m_lines.size() == 0 ) return;

		double fontPoints = 0;
		bool fontBold = false;
		dc.Font( &fontPoints, &fontBold );

		if ( draw_bounds )
		{
			dc.Pen( *wxLIGHT_GREY, 0.5 );
			dc.Brush( *wxBLUE, wxPLOutputDevice::NONE );
		}

		// layout has already been calculated, assuming the same font.
		// render the text directly given the starting coordinates
		if ( rotationDegrees == 0.0 )
		{
			for ( size_t i=0;i<m_lines.size();i++ )
			{
				for ( size_t j=0;j<m_lines[i].size();j++ )
				{
					text_piece &tp = m_lines[i][j];
					dc.Font( tp.state == text_piece::NORMAL ? fontPoints : fontPoints-FontPointAdjust, fontBold );
					dc.Text( tp.text, x + tp.origin.x, y + tp.origin.y );				
					if ( draw_bounds )
						dc.Rect( x+tp.origin.x, y+tp.origin.y, tp.size.x, tp.size.y );
				}
			}
		}
		else
		{
			double theta = -M_PI/180*rotationDegrees;
			double sintheta = sin(theta);
			double costheta = cos(theta);
			for ( size_t i=0;i<m_lines.size();i++ )
			{
				for ( size_t j=0;j<m_lines[i].size();j++ )
				{
					text_piece &tp = m_lines[i][j];
					dc.Font( tp.state == text_piece::NORMAL ? fontPoints : fontPoints-FontPointAdjust, fontBold );
					double rotx = tp.origin.x*costheta - tp.origin.y*sintheta;
					double roty = tp.origin.x*sintheta + tp.origin.y*costheta;
					dc.Text( tp.text, x + rotx, y + roty, rotationDegrees );
				}
			}
		}

		// restore font state
		dc.Font( fontPoints, fontBold );
	}

	wxString escape( const wxString &text )
	{
		wxString result;
		result.Alloc( text.Length() ); 

		bool last_char_slash = false;
		wxString::const_iterator it = text.begin();
		while( it != text.end() )
		{
			if ( *it == '\\'  && !last_char_slash )
			{
				++it; // skip the slash
				wxString code;
				while ( it != text.end()
					&& (*it) != '\\'
					&& wxIsalpha( (*it) ) )
				{
					code += *it;
					++it;
				}

				last_char_slash = ( it != text.end() 
					&& code.Len() == 0 
					&& *it == '\\' );

				if ( it != text.end()  
					&& (*it == ' ' || *it == '\t' ) )
					it++; // skip next space if it exists.  assume is for delineating codes

								
				wxUniChar value( 0 );
				size_t idx = 0;
				while ( ::escape_codes[idx].value != 0 )
				{
					if ( ::escape_codes[idx].code == code )
					{
						value = ::escape_codes[idx].value;
						break;
					}
					else idx++;
				}

				if ( value != 0 ) result += value;
				else if ( !last_char_slash ) result << L'\x275a';
			}
			else
			{
				last_char_slash = false;
				result += *it;
				++it;
			}
		}

		return result;
	}

	std::vector<text_piece> parse( const wxString &text )
	{
		std::vector<text_piece> list;

		text_piece tp;

		wxString::const_iterator it = text.begin();
		while( it != text.end() )
		{
			if ( *it == '^' || *it == '_' )
			{
				wxUniChar modifier = *it;
				it++; // skip modifier to get the next character
				if ( it == text.end() || *it == modifier )
				{
					// if we have a double modifier,
					// simply add it as normal text
					tp.text += *it;
				}
				else 
				{
					if ( !tp.text.IsEmpty() )
					{
						tp.state = text_piece::NORMAL;
						list.push_back( tp );
						tp.text.clear();
					}
				
					tp.state = (modifier == '^') ? text_piece::SUPERSCRIPT : text_piece::SUBSCRIPT;
					if ( it != text.end() && *it == '{' )
					{
						it++; // skip the {
						while ( it != text.end() && *it != '}' )
						{
							tp.text += *it;
							it++;
						}
					
						// go until } encountered
						if ( it != text.end() && *it == '}' )
							it++;
					}
					else
					{
						while ( it != text.end() 
							&& *it != ' ' 
							&& *it != '/'
							&& *it != '\t'
							&& *it != '^'
							&& *it != '_'
							&& *it != '('
							&& *it != '{'
							&& *it != '['
							&& *it != '='
							&& *it != ','
							&& *it != ';' )
						{
							tp.text += *it;
							it++;
						}

						if ( it != text.end() && *it != ' ' )
							--it; // return the character that ended the current mode
					}

					if ( tp.text.Len() > 0 )
					{
						list.push_back( tp );
						tp.text.clear();
					}
				}
			}
			else
			{
				tp.text += *it;
			}

			if ( it != text.end() )
				++it;
		}

		// push back final text piece if needed		
		if ( !tp.text.IsEmpty() )
		{
			tp.state = text_piece::NORMAL;
			list.push_back( tp );
		}

		return list;
	}

	inline double width() { return m_bounds.x; }
	inline double height() { return m_bounds.y; }
};

class wxPLPlot::axis_layout
{
private:

	struct tick_layout
	{
		tick_layout( wxPLOutputDevice &dc, const wxPLAxis::TickData &td )
			: text( dc, td.label, text_layout::CENTER ), world( td.world ), 
			  tick_size( td.size ), 
			  text_angle(0.0) {  }
				
		text_layout text;
		double world;
		wxPLAxis::TickData::TickSize tick_size;
		double text_angle;
	};

	std::vector< tick_layout > m_tickList;
	int m_axisPos;
	wxSize m_bounds;

	void renderAngular( wxPLOutputDevice &dc, double radius, wxPLPolarAngularAxis *axis, double cntr_x, double cntr_y)
	{
		wxRealPoint cntr(cntr_x, cntr_y);

		// draw tick marks and tick labels
		for (size_t i = 0; i < m_tickList.size(); i++)
		{
			tick_layout &ti = m_tickList[i];
			double tick_length = ti.tick_size == wxPLAxis::TickData::LARGE ? 5 : 2;
			double angle = axis->AngleInRadians(ti.world);
			wxRealPoint pt0( cntr + wxRealPoint(radius*cos(angle), radius*sin(angle)) );
			wxRealPoint pt1( cntr + wxRealPoint((radius - tick_length)*cos(angle), (radius - tick_length)*sin(angle)) );
			dc.Line(pt0, pt1);

			if (ti.text.width() > 0)
			{
				// text is place via upper left corner of text box, which only works for the bottom right
				// quadrant of the polar plot.  so, text must be placed based on quadrant
				
				if (pt0.x < pt1.x && pt0.y >= pt1.y) // bottom left quadrant
					ti.text.render(dc, pt0.x - ti.text.width() - text_space, pt0.y, 0, false);
				else if (pt0.x < pt1.x && pt0.y < pt1.y) // top left quadrant
					ti.text.render(dc, pt0.x - ti.text.width() - text_space, pt0.y-ti.text.height(), 0, false);
				else if (pt0.x >= pt1.x && pt0.y < pt1.y) // top right quadrant
					ti.text.render(dc, pt0.x + text_space, pt0.y-ti.text.height(), 0, false);
				else
					ti.text.render(dc, pt0.x + text_space, pt0.y, 0, false);
			}

		}
	}


public:	
	static const int TextAxisOffset = 3;

	std::vector<double> ticks( wxPLAxis::TickData::TickSize size )
	{
		std::vector<double> values;
		for (size_t i=0;i<m_tickList.size();i++)
			if ( m_tickList[i].tick_size == size )
				values.push_back( m_tickList[i].world );
		return values;
	}

	axis_layout( int ap, wxPLOutputDevice &dc, wxPLAxis *axis, double phys_min, double phys_max )
	{
		m_axisPos = ap;

		// compute the 'best' geometry 
		std::vector< wxPLAxis::TickData > tl;
		axis->GetAxisTicks( phys_min, phys_max, tl );
		if ( tl.size() == 0 ) return;

		// calculate text size of each tick
		m_tickList.reserve( tl.size() );
		for ( size_t i=0;i<tl.size();i++ )
			m_tickList.push_back( tick_layout( dc, tl[i] ) );
		
		if ( ap == X_BOTTOM || ap == X_TOP )
		{
			// calculate axis bounds extents assuming
			// no text rotation
			double xmin = phys_min;
			double xmax = phys_max;
			double ymax = 0;

			for ( size_t i=0;i<m_tickList.size();i++ )
			{
				tick_layout &ti = m_tickList[i];
				double phys = axis->WorldToPhysical( ti.world, phys_min, phys_max );
				if ( phys-ti.text.width()/2 < xmin )
					xmin = phys-ti.text.width()/2;
				if ( phys+ti.text.width()/2 > xmax )
					xmax = phys+ti.text.width()/2;
				if ( ti.text.height() > ymax )
					ymax = ti.text.height();
			}

			m_bounds.x = xmax-xmin;
			m_bounds.y = ymax;

			// if this is a polar axis, we're done
			if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(axis))
				return;

			// required pixels of separation between adjacent tick texts
			const double tick_label_space = 4;

			// mediocre attempt for laying out axis tick text to avoid overlap
			// note: may need to adjust bounds accordingly
			
			std::vector< tick_layout* > labeledTicks;
			for ( size_t i=0; i < m_tickList.size(); i++ )
				if ( m_tickList[i].text.width() > 0 )
					labeledTicks.push_back( &m_tickList[i] );
			if ( labeledTicks.size() > 2 )
			{
				// find the largest tick mark label
				// and try to position it by rotation so
				// that it doesn't touch adjacent text
				double textAngle = 0.0; // default: no rotation

				size_t index = 0;
				double width = 0;
				for ( size_t j = 0; j < labeledTicks.size(); j++ )
				{
					if ( labeledTicks[j]->text.width() > width )
					{
						width = labeledTicks[j]->text.width();
						index = j;
					}
				}

				if ( index == 0 ) index++;
				else if ( index == labeledTicks.size()-1 ) index--;

				tick_layout &left = *labeledTicks[index-1];
				tick_layout &center = *labeledTicks[index];
				tick_layout &right = *labeledTicks[index+1];

				double phys_left = axis->WorldToPhysical( left.world, phys_min, phys_max );
				double phys = axis->WorldToPhysical( center.world, phys_min, phys_max );
				double phys_right = axis->WorldToPhysical( right.world, phys_min, phys_max );
				
				if ( (phys - center.text.width()/2 - tick_label_space < phys_left + left.text.width()/2)
					|| (phys + center.text.width()/2 + tick_label_space > phys_right - right.text.width()/2) )
				{
					// rotate all the ticks, not just the large ones

					textAngle = 45.0;
					for ( size_t j=0;j<m_tickList.size();j++ )
					{
						m_tickList[j].text_angle = textAngle;
						// calculate text offset positions so that 
						// text bounding rect corners line up with point on axis

						// realign text layout due to rotation (better look for multi-line tick labels)
						m_tickList[j].text.align( ap == X_TOP ? text_layout::LEFT : text_layout::RIGHT );
					}

					// recalculate bound height to rotated text
					m_bounds.y = labeledTicks[index]->text.height()
						+ fabs( labeledTicks[index]->text.width() * sin( M_PI/180*textAngle ) );
				}
			}
		}
		else
		{ // ap = Y_LEFT || Y_RIGHT
			double ymin = phys_max; // ymin is upper coordinate
			double ymax = phys_min; // ymax is lower coordinate
			double xmax = 0; // maximum text width
			for ( size_t i=0;i<m_tickList.size();i++ )
			{
				tick_layout &ti = m_tickList[i];
				double phys = axis->WorldToPhysical( ti.world, phys_min, phys_max );
				if ( phys-ti.text.height()/2 < ymin )
					ymin = phys-ti.text.height()/2;
				if ( phys+ti.text.height()/2 > ymax )
					ymax = phys+ti.text.height()/2;				
				if ( ti.text.width() > xmax )
					xmax = ti.text.width();
			}

			m_bounds.x = xmax;
			m_bounds.y = ymax-ymin;		
		}
	}

	wxSize bounds() { return m_bounds; }

	void render( wxPLOutputDevice &dc, double ordinate, wxPLAxis *axis, double phys_min, double phys_max, double ordinate_opposite = -1 )
	{
		// if this is a polar angular axis, render it and leave
		// radius passed in a 'ordinate', center.x as phys_min, center.y as phys_max
		if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(axis)) {
			renderAngular(dc, ordinate, pa, phys_min, phys_max);
			return;
		}

		// draw tick marks and tick labels
		for ( size_t i=0;i<m_tickList.size(); i++ )
		{
			tick_layout &ti = m_tickList[i];
			double physical = axis->WorldToPhysical( ti.world, phys_min, phys_max );

			if ( ti.tick_size != wxPLAxis::TickData::NONE )
			{
				wxRealPoint tickStart, tickEnd;
				double tick_length = ti.tick_size == wxPLAxis::TickData::LARGE ? 5 : 2;
				if ( m_axisPos == X_BOTTOM || m_axisPos == X_TOP )
				{
					tickStart.x = physical;
					tickStart.y = ordinate;
					tickEnd.x = physical;
					tickEnd.y = m_axisPos == X_BOTTOM ? ordinate-tick_length : ordinate+tick_length;
				}
				else
				{
					tickStart.x = ordinate;
					tickStart.y = physical;
					tickEnd.x = m_axisPos == Y_LEFT ? ordinate + tick_length : ordinate - tick_length;
					tickEnd.y = physical;
				}

				dc.Line( tickStart, tickEnd );

				// draw ticks on opposite ordinate line if needed
				if ( ordinate_opposite > 0 )
				{
					if ( m_axisPos == X_BOTTOM || m_axisPos == X_TOP )
					{
						tickStart.x = physical;
						tickStart.y = ordinate_opposite;
						tickEnd.x = physical;
						tickEnd.y = m_axisPos == X_BOTTOM ? ordinate_opposite+tick_length : ordinate_opposite-tick_length;
					}
					else
					{
						tickStart.x = ordinate_opposite;
						tickStart.y = physical;
						tickEnd.x = m_axisPos == Y_LEFT ? ordinate_opposite-tick_length : ordinate_opposite+tick_length;
						tickEnd.y = physical;
					}

					dc.Line( tickStart, tickEnd );
				}
			}

			if ( ti.text.width() > 0 )
			{
				double text_x = 0, text_y = 0;
				if ( m_axisPos ==  X_BOTTOM )
				{
					if ( ti.text_angle == 0.0 )
					{
						text_x = physical - ti.text.width()/2;
						text_y = ordinate + TextAxisOffset;
					}
					else
					{
						double angleRad = -M_PI/180*ti.text_angle;
						text_x = physical - ti.text.width()*cos(angleRad);
						text_y = ordinate + TextAxisOffset - ti.text.width()*sin(angleRad);
					}
				}
				else if ( m_axisPos == X_TOP )
				{
					if ( ti.text_angle == 0.0 )
					{
						text_x = physical - ti.text.width()/2;
						text_y = ordinate - TextAxisOffset - ti.text.height();
					}
					else
					{
						double angleRad = -M_PI/180*ti.text_angle;
						text_x = physical + ti.text.height()*sin(angleRad);
						text_y = ordinate - TextAxisOffset - ti.text.height()*cos(angleRad);
					}
				}
				else if ( m_axisPos == Y_LEFT )
				{
					text_x = ordinate - ti.text.width() - TextAxisOffset;
					text_y = physical - ti.text.height()/2;
				}
				else
				{ // Y_RIGHT
					text_x = ordinate + TextAxisOffset;
					text_y = physical - ti.text.height()/2;
				}
			
				ti.text.render( dc, 
					text_x, text_y, ti.text_angle, false );			
			}
		}

	}
};

wxPLPlot::axis_data::axis_data()
	: axis(0), layout(0), label(0)
{
}

wxPLPlot::axis_data::~axis_data()
{
	if (axis) delete axis;
	if (layout) delete layout;
	if (label) delete label;
}

void wxPLPlot::axis_data::set( wxPLAxis *a )
{
	if (axis == a) return;
	if (axis) delete axis;
	axis = a;
	invalidate();
}

void wxPLPlot::axis_data::invalidate()
{
	if (layout) delete layout;
	layout = 0;
	if (label) delete label;
	label = 0;
}


wxPLPlot::wxPLPlot()
{
	for ( size_t i=0;i<4;i++ )
		m_sideWidgets[i] = 0;

	m_showLegend = true;
	m_showCoarseGrid = true;
	m_showFineGrid = true;
	m_showTitle = true;
	m_titleLayout = 0;
	m_gridColour.Set( 225, 225, 225 );
	m_plotAreaColour = *wxWHITE;
	m_tickTextColour = *wxBLACK;
	m_axisColour = *wxBLACK;
	m_legendRect.x = 10;
	m_legendRect.y = 10;
	m_reverseLegend = false;
	m_legendInvalidated = true;
	m_legendPosPercent.x = 85.0;
	m_legendPosPercent.y = 4.0;
	m_legendPos = FLOATING;
	m_anchorPoint = wxPoint(0, 0);
	m_currentPoint = wxPoint(0, 0);
	m_moveLegendMode = false;
	m_moveLegendErase = false;
}

wxPLPlot::~wxPLPlot()
{
	if ( m_titleLayout != 0 )
		delete m_titleLayout;

	for ( std::vector<plot_data>::iterator it = m_plots.begin();
		it != m_plots.end();
		++it )
		delete it->plot;

	for ( size_t i=0;i<m_legendItems.size(); i++ )
		delete m_legendItems[i];

	for ( size_t i=0;i<4;i++ )
		if ( m_sideWidgets[i] != 0 )
			delete m_sideWidgets[i];
}

void wxPLPlot::AddPlot( wxPLPlottable *p, AxisPos xap, AxisPos yap, PlotPos ppos, bool update_axes )
{
	plot_data dd;
	dd.plot = p;
	dd.ppos = ppos;
	dd.xap = xap;
	dd.yap = yap;

	m_plots.push_back( dd );

	if ( GetAxis( xap ) == 0 )
		SetAxis( p->SuggestXAxis(), xap );

	if ( GetAxis( yap, ppos ) == 0 )
		SetAxis( p->SuggestYAxis(), yap, ppos );

	if ( p->IsShownInLegend() )		
		m_legendInvalidated = true;

	if ( update_axes )
		UpdateAxes( false );
}

wxPLPlottable *wxPLPlot::RemovePlot( wxPLPlottable *p, PlotPos plotPosition )
{
	for ( std::vector<plot_data>::iterator it = m_plots.begin();
		it != m_plots.end();
		++it )
	{
		if ( it->plot == p && (it->ppos == plotPosition || plotPosition == NPLOTPOS) )
		{
			m_plots.erase( it );
			if ( p->IsShownInLegend() )
				m_legendInvalidated = true;

			return p;
		}
	}

	return 0;
}

bool wxPLPlot::ContainsPlot(wxPLPlottable *p, PlotPos plotPosition)
{
	for (std::vector<plot_data>::iterator it = m_plots.begin();
		it != m_plots.end();
		++it)
	{
		if (it->plot == p && (it->ppos == plotPosition || plotPosition == NPLOTPOS))
		{
			return true;
		}
	}

	return false;
}

void wxPLPlot::DeleteAllPlots()
{
	for ( std::vector<plot_data>::iterator it = m_plots.begin();
		it != m_plots.end();
		++it )
		delete it->plot;

	m_plots.clear();	
	m_legendInvalidated = true;

	UpdateAxes( true );
}

size_t wxPLPlot::GetPlotCount()
{
	return m_plots.size();
}

wxPLPlottable *wxPLPlot::GetPlot( size_t i )
{
	if ( i >= m_plots.size() ) return 0;
	
	return m_plots[i].plot;
}

wxPLPlottable *wxPLPlot::GetPlotByLabel( const wxString &series )
{
	for( size_t i=0;i<m_plots.size();i++ )
		if ( m_plots[i].plot->GetLabel() == series )
			return m_plots[i].plot;

	return 0;
}

bool wxPLPlot::GetPlotPosition( const wxPLPlottable *p, 
	AxisPos *xap, AxisPos *yap, PlotPos *ppos )
{
	for ( std::vector<plot_data>::iterator it = m_plots.begin();
		it != m_plots.end();
		++it )
	{
		if ( it->plot == p )
		{
			if ( xap != 0 ) *xap = it->xap;
			if ( yap != 0 ) *yap = it->yap;
			if ( ppos != 0 ) *ppos = it->ppos;
			return true;
		}
	}

	return false;
}

wxPLAxis *wxPLPlot::GetAxis( AxisPos axispos, PlotPos ppos )
{
	switch( axispos )
	{
	case X_BOTTOM: return m_x1.axis;
	case X_TOP: return m_x2.axis;
	case Y_LEFT: return m_y1[ppos].axis;
	case Y_RIGHT: return m_y2[ppos].axis;
	default: return 0;
	}
}

wxPLAxis &wxPLPlot::Axis( AxisPos axispos, PlotPos ppos )
{
static wxPLLinearAxis s_nullAxis(0, 1, "<null-axis>");
	switch( axispos )
	{
	case X_BOTTOM: if ( m_x1.axis ) return *m_x1.axis;
	case X_TOP: if ( m_x2.axis ) return *m_x2.axis;
	case Y_LEFT: if ( m_y1[ppos].axis ) return *m_y1[ppos].axis;
	case Y_RIGHT: if ( m_y2[ppos].axis ) return *m_y2[ppos].axis;
	default: return s_nullAxis;
	}
}

void wxPLPlot::SetAxis( wxPLAxis *a, AxisPos axispos, PlotPos ppos )
{
	switch( axispos )
	{
	case X_BOTTOM: SetXAxis1( a ); break;
	case X_TOP: SetXAxis2( a ); break;
	case Y_LEFT: SetYAxis1( a, ppos ); break;
	case Y_RIGHT: SetYAxis2( a, ppos ); break;
	}
}

void wxPLPlot::SetTitle( const wxString &title )
{
	if ( title != m_title && m_titleLayout != 0 )
	{
		delete m_titleLayout;
		m_titleLayout = 0;
	}
		
	m_title = title;
}

void wxPLPlot::SetLegendLocation( LegendPos pos, double xpercent, double ypercent )
{
	if ( xpercent > -998.0 )
	{
		if ( xpercent < -10 ) xpercent = -10;
		if ( xpercent > 90 ) xpercent = 90;
		m_legendPosPercent.x = xpercent;
	}

	if ( ypercent > -998.0 )
	{
		if ( ypercent < -10 ) ypercent = -10;
		if ( ypercent > 90 ) ypercent = 90;
		m_legendPosPercent.y = ypercent;
	}

	m_legendPos = pos;
}

bool wxPLPlot::SetLegendLocation( const wxString &spos )
{
	wxString llpos = spos.Lower();
	
	if ( llpos == "northwest" ) { SetLegendLocation( NORTHWEST ); return true; }
	if ( llpos == "northeast" ) { SetLegendLocation( NORTHEAST ); return true; }
	if ( llpos == "southwest" ) { SetLegendLocation( SOUTHWEST ); return true; }
	if ( llpos == "southeast" ) { SetLegendLocation( SOUTHEAST ); return true; }
	
	if ( llpos == "north" ) { SetLegendLocation( NORTH ); return true; }
	if ( llpos == "south" ) { SetLegendLocation( SOUTH ); return true; }
	if ( llpos == "east" ) { SetLegendLocation( EAST ); return true; }
	if ( llpos == "west" ) { SetLegendLocation( WEST ); return true; }

	if ( llpos == "bottom" ) { SetLegendLocation( BOTTOM ); return true; }
	if ( llpos == "right" ) { SetLegendLocation( RIGHT ); return true; }

	return false;
}

void wxPLPlot::SetSideWidget( wxPLSideWidgetBase *sw, AxisPos pos )
{
	if ( m_sideWidgets[pos] != 0 
		&& m_sideWidgets[pos] != sw )
		delete m_sideWidgets[pos];

	m_sideWidgets[pos] = sw;
}

wxPLSideWidgetBase *wxPLPlot::GetSideWidget( AxisPos pos )
{
	return m_sideWidgets[pos];
}

wxPLSideWidgetBase *wxPLPlot::ReleaseSideWidget( AxisPos pos )
{
	wxPLSideWidgetBase *w = m_sideWidgets[pos];
	m_sideWidgets[pos] = 0;
	return w;
}

void wxPLPlot::WriteDataAsText( wxUniChar sep, wxOutputStream &os, bool visible_only, bool include_x )
{
	if ( m_plots.size() == 0 ) { return; }

	wxTextOutputStream tt(os);
	wxString sepstr(sep);
	wxString xDataLabel = "";
	wxPLAxis *xaxis;
	wxPLPlottable *plot;
	double worldMin;
	double worldMax;
	wxPLHistogramPlot* histPlot;
	std::vector<bool> includeXForPlot(m_plots.size(), false);
	std::vector<wxString> Headers;
	std::vector< std::vector<wxRealPoint> > data;
	size_t maxLength = 0;
	bool keepGoing; //Used to stop early if all columns are no longer visible.

	//Add column headers
	for ( size_t i = 0; i < m_plots.size(); i++ )
	{
		plot = m_plots[i].plot;

		worldMin = 0.0;
		worldMax = plot->At(plot->Len() - 1).x;

		//We only include the x column on a plot if we are including X and if its x header is different than the previous column's.
		if(i == 0)
		{
			includeXForPlot[i] = true;
		}
		else if(histPlot = dynamic_cast<wxPLHistogramPlot*>( plot )) 
		{ 
			includeXForPlot[i] = true;

			//For CDF plots there is no X data label. The closest useful label is the Y label of the companion PDF histogram plot, so we need to store for use by the CDF plot.
			if(xDataLabel == "") { xDataLabel = m_plots[i].plot->GetYDataLabel(this); }
		}
		else if(histPlot = dynamic_cast<wxPLHistogramPlot*>( m_plots[i - 1].plot ))
		{
			includeXForPlot[i] = true;
		}
		else
		{
			includeXForPlot[i] = (m_plots[i].plot->GetXDataLabel( this ) != m_plots[i-1].plot->GetXDataLabel( this ));
		}

		if(histPlot = dynamic_cast<wxPLHistogramPlot*>( m_plots[i].plot ))
		{
			//Do nothing
		}
		else
		{
			xaxis = GetAxis( m_plots[i].xap, m_plots[i].ppos );
			if ( xaxis )
			{
				worldMin = xaxis->GetWorldMin();
				worldMax = xaxis->GetWorldMax();
			}
		}

		Headers = plot->GetExportableDatasetHeaders(sep, this);
		data.push_back(plot->GetExportableDataset(worldMin, worldMax, visible_only));

		if (include_x && includeXForPlot[i])
		{
			if (i > 0) { tt << sepstr;}	//Extra column since we have a new set of x values.
			tt << (Headers[0].IsEmpty() ? xDataLabel : Headers[0]);
			tt << sepstr;
		}

		tt << Headers[1];

		if (Headers.size() > 2)
		{
			for (size_t j = 2; j < Headers.size(); j++)
			{
				tt << sepstr;
				tt << Headers[j];
			}
		}
		if ( i < m_plots.size() - 1 ) { tt << sepstr; }
		if ( data[i].size() > maxLength ) { maxLength = data[i].size(); }
	}

	tt << "\n";

	//Add data
	for (size_t RowNum = 0; RowNum < maxLength; RowNum++)
	{
		keepGoing = false;

		for (size_t PlotNum = 0; PlotNum < m_plots.size(); PlotNum++)
		{
			if ( RowNum < data[PlotNum].size() )
			{
				keepGoing = true;

				if ( include_x && includeXForPlot[PlotNum] )
				{
					if (PlotNum > 0) tt << sepstr; //extra sep before to add blank column before new x values, as in header.
					tt << wxString::Format("%lg", data[PlotNum][RowNum].x );
					tt << sepstr; 
				}

				tt << wxString::Format("%lg", data[PlotNum][RowNum].y);
			}
			else
			{
				if (PlotNum > 0) tt << sepstr; //extra sep before to add blank column before new x values, as in header.
				tt << sepstr; 
			}

			if ( PlotNum < m_plots.size() - 1 ) { tt << sepstr; }
		}

		if (!keepGoing) break;

		tt << "\n";
	}
}


void wxPLPlot::Render( wxPLOutputDevice &dc, wxPLRealRect geom )
{
#define NORMAL_FONT(dc)  dc.Font( 0, false )
#define TITLE_FONT(dc)   dc.Font( +1, false )
#define LEGEND_FONT(dc)  dc.Font( -1, false )
#define AXIS_FONT(dc)    dc.Font( 0, false )

	// ensure plots have the axes they need to be rendered
	for ( size_t i = 0; i< m_plots.size(); i++ )
	{
		if ( GetAxis( m_plots[i].xap ) == 0 )
			SetAxis( m_plots[i].plot->SuggestXAxis(), m_plots[i].xap );

		if ( GetAxis( m_plots[i].yap, m_plots[i].ppos ) == 0 )
			SetAxis( m_plots[i].plot->SuggestYAxis(), m_plots[i].yap, m_plots[i].ppos );
	}

	// draw any side widgets first and remove the space from the total plot area
	if ( m_sideWidgets[Y_LEFT] != 0 )
	{
		wxRealPoint sz = m_sideWidgets[Y_LEFT]->GetBestSize();
		m_sideWidgets[Y_LEFT]->Render( dc, 
			wxPLRealRect( geom.x, geom.y, 
				sz.x, geom.height ) );
		geom.width -= sz.x;
		geom.x += sz.x;
	}

	if ( m_sideWidgets[Y_RIGHT] != 0 )
	{
		wxRealPoint sz = m_sideWidgets[Y_RIGHT]->GetBestSize();		
		m_sideWidgets[Y_RIGHT]->Render( dc, 
			wxPLRealRect( geom.x+geom.width-sz.x, geom.y, 
				sz.x, geom.height ) );

		geom.width -= sz.x;
	}


	wxPLRealRect box( geom.x+text_space, 
		geom.y+text_space, 
		geom.width-text_space-text_space, 
		geom.height-text_space-text_space );

	bool legend_bottom = false;
	bool legend_right = false;
	if ( m_showLegend )
	{
		LEGEND_FONT(dc);
		CalcLegendTextLayout( dc );

		if ( m_legendPos == BOTTOM )
		{
			double height = 0;
			for ( size_t i=0;i<m_legendItems.size();i++ )
				if (m_legendItems[i]->layout->height() > height )
					height = m_legendItems[i]->layout->height();

			if ( height > 0 ) 
				legend_bottom = true;

			box.height -= height + text_space;
		}

		if ( m_legendPos == RIGHT )
		{
			double width = 0;
			for ( size_t i=0;i<m_legendItems.size();i++ )
				if (m_legendItems[i]->layout->width() > width )
					width = m_legendItems[i]->layout->width();

			if (width > 0)
				legend_right = true;

			box.width -= width + 5*text_space + legend_item_box.x;
		}
	}

	// position and render title using cached layout if possible
	if ( m_showTitle && !m_title.IsEmpty() )
	{
		TITLE_FONT(dc);
		if ( m_titleLayout == 0 )
			m_titleLayout = new text_layout( dc, m_title, text_layout::CENTER );

		m_titleLayout->render( dc, box.x+box.width/2-m_titleLayout->width()/2, box.y, 0, false );
		box.y += m_titleLayout->height() + text_space;
		box.height -= m_titleLayout->height() + text_space;
	}
	else
	{
		// if no title, leave a few extra pixels at the top for Y axes labels at the edges
		int topmargin = (int)(0.5*dc.CharHeight() ) + text_space;
		box.y += topmargin;
		box.height -= topmargin;
	}

	NORMAL_FONT(dc);

	wxPLRealRect plotbox = box; // save current box for where to draw the axis labels

	// determine sizes of axis labels

	if ( m_x2.axis && m_x2.axis->IsLabelVisible() && !m_x2.axis->GetLabel().IsEmpty() )
	{
		if ( m_x2.label == 0 )
			m_x2.label = new text_layout( dc, m_x2.axis->GetLabel(), text_layout::CENTER );

		box.y += m_x2.label->height() + text_space;
		box.height -= m_x2.label->height() + text_space;
	}

	if ( m_x1.axis && m_x1.axis->IsLabelVisible() && !m_x1.axis->GetLabel().IsEmpty() )
	{
		if ( m_x1.label == 0 )
			m_x1.label = new text_layout( dc, m_x1.axis->GetLabel(), text_layout::CENTER );

		box.height -= m_x1.label->height() + 2*text_space;
	}

	double yleft_max_label_width = 0, yright_max_label_width = 0;
	for ( size_t pp = 0; pp < NPLOTPOS; pp++ )
	{
		if (m_y1[pp].axis && m_y1[pp].axis->IsLabelVisible() && !m_y1[pp].axis->GetLabel().IsEmpty() )
		{
			if ( m_y1[pp].label == 0 )
				m_y1[pp].label = new text_layout( dc, m_y1[pp].axis->GetLabel(), text_layout::CENTER );

			if ( m_y1[pp].label->height() > yleft_max_label_width )
				yleft_max_label_width = m_y1[pp].label->height();
		}

		if (m_y2[pp].axis && m_y2[pp].axis->IsLabelVisible() && !m_y2[pp].axis->GetLabel().IsEmpty() )
		{
			if ( m_y2[pp].label == 0 )
				m_y2[pp].label = new text_layout( dc, m_y2[pp].axis->GetLabel(), text_layout::CENTER );

			if ( m_y2[pp].label->height() > yright_max_label_width )
				yright_max_label_width = m_y2[pp].label->height();
		}
	}

	box.x += yleft_max_label_width + 2*text_space;
	box.width -= yleft_max_label_width + 2*text_space;
	box.width -= yright_max_label_width + 2*text_space;
	
	if ( box.width < 50 || box.height < 50 )
		return; // nothing useful to do at this scale

	// now that we have the labels approximately sized for the various axes
	// and the size of the bounding box is what is left for the graphing area,
	// estimate the space required by the axis tick labels and reduce the 
	// effective box accordingly which defines how much space is left for the
	// actual plot

	// note that the axis_layouts are cached and are only
	// invalidated on a resize even.t, or when an axis is changed

	AXIS_FONT(dc);	
	if ( m_x2.axis != 0 )
	{
		if ( m_x2.layout == 0 )
			m_x2.layout = new axis_layout( X_TOP, dc, m_x2.axis, box.x, box.x+box.width );

		if ( m_x2.layout->bounds().x > box.width )
		{   // this adjusts for really wide tick text at the ends of the axis
			double diff = m_x2.layout->bounds().x - box.width;
			double adj = 2*diff/3; // actual adjustment needed is diff/2, but leave a little extra space

			if (box.x + box.width + diff > plotbox.x + plotbox.width )
				box.width -= adj;

			if ( box.x - diff < plotbox.x )
			{
				box.x += adj;
				box.width -= adj;
			}
		}

		box.y += m_x2.layout->bounds().y;
		box.height -= m_x2.layout->bounds().y;
	}

	// create an indicator as to whether or not this is a cartesion plot
	bool is_cartesian = true;
	if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(m_x1.axis))
		is_cartesian = false;


	if ( m_x1.axis != 0 )
	{
		if (is_cartesian) {
			if (m_x1.layout == 0)
				m_x1.layout = new axis_layout(X_BOTTOM, dc, m_x1.axis, box.x, box.x + box.width);
			
			box.height -= m_x1.layout->bounds().y;
		}
		else {
			if (m_x1.layout == 0) {
				if (box.width < box.height)
					m_x1.layout = new axis_layout(X_BOTTOM, dc, m_x1.axis, box.x, box.x + box.width);
				else
					m_x1.layout = new axis_layout(X_BOTTOM, dc, m_x1.axis, box.y, box.y + box.height);
			}

			box.y += m_x1.layout->bounds().y;
			box.height -= m_x1.layout->bounds().y*2;
		}
	}
	
	double yleft_max_axis_width = 0, yright_max_axis_width = 0;
	for ( size_t pp=0;pp<NPLOTPOS;pp++)
	{
		if ( m_y1[pp].axis != 0 )
		{
			if (m_y1[pp].layout == 0) {
				if (is_cartesian)
					m_y1[pp].layout = new axis_layout( Y_LEFT, dc, m_y1[pp].axis, box.y+box.height, box.y );
				else {
					if (box.width < box.height)
						m_y1[pp].layout = new axis_layout(Y_LEFT, dc, m_y1[pp].axis, box.x, box.x + box.width);
					else
						m_y1[pp].layout = new axis_layout(Y_LEFT, dc, m_y1[pp].axis, box.y + box.height, box.y);
				}
			}

			if ( m_y1[pp].layout->bounds().x > yleft_max_axis_width )
				yleft_max_axis_width = m_y1[pp].layout->bounds().x;
		}

		if ( m_y2[pp].axis != 0 )
		{
			if ( m_y2[pp].layout == 0 )
				m_y2[pp].layout = new axis_layout( Y_RIGHT, dc, m_y2[pp].axis, box.y+box.height, box.y );

			if ( m_y2[pp].layout->bounds().x > yright_max_axis_width )
				yright_max_axis_width = m_y2[pp].layout->bounds().x;
				
		}
	}

	box.x += yleft_max_axis_width;
	box.width -= yleft_max_axis_width;
	box.width -= yright_max_axis_width;
		
	if ( box.width < 30 || box.height < 30 )
		return; // nothing useful to do at this scale

	size_t nyaxes = 0; // number of Y axes (plots) that need to be shown
	for ( size_t pp=0;pp<NPLOTPOS;pp++ )
		if ( m_y1[pp].axis || m_y2[pp].axis )
			if ( pp+1 > nyaxes )
				nyaxes = pp+1;

	if ( nyaxes == 0 ) nyaxes = 1;
	const double plot_space = 16;
	double single_plot_height = box.height/nyaxes - (nyaxes-1)*(plot_space/2);
	if ( single_plot_height < 50 ) return;

	// compute box dimensions for each plot/subplot
	// and fill plot area with background color
	dc.Brush(  m_plotAreaColour );
	dc.Pen( *wxWHITE, 1.0, wxPLOutputDevice::NONE );

	double cur_plot_y_start = box.y;
	m_plotRects.clear();
	for ( size_t pp=0;pp<nyaxes; pp++ )
	{
		wxPLRealRect rect( box.x, cur_plot_y_start, box.width, single_plot_height );
		if (is_cartesian) 
			dc.Rect(rect);
		else {
			double radius = (box.width < box.height) ? box.width / 2.0 : box.height / 2.0;
			wxRealPoint cntr(box.x + box.width / 2.0, box.y + box.height / 2.0);
			dc.Circle(cntr, radius);
		}
		m_plotRects.push_back(rect);
		cur_plot_y_start += single_plot_height + plot_space;
	}

	// render grid lines
	if ( m_showCoarseGrid )
	{
		dc.Pen( m_gridColour, 0.5, 
			wxPLOutputDevice::SOLID, wxPLOutputDevice::MITER, wxPLOutputDevice::BUTT );

		if (is_cartesian)	DrawGrid( dc, wxPLAxis::TickData::LARGE );
		else DrawPolarGrid(dc, wxPLAxis::TickData::LARGE);
	}

	if ( m_showFineGrid )
	{		
		dc.Pen( m_gridColour, 0.5, 
			wxPLOutputDevice::DOT, wxPLOutputDevice::MITER, wxPLOutputDevice::BUTT );

		if (is_cartesian) DrawGrid( dc, wxPLAxis::TickData::SMALL );
		else DrawPolarGrid(dc, wxPLAxis::TickData::SMALL);
	}

	// render plots
	for ( size_t i = 0; i< m_plots.size(); i++ )
	{
		wxPLAxis *xaxis = GetAxis( m_plots[i].xap );
		wxPLAxis *yaxis = GetAxis( m_plots[i].yap, m_plots[i].ppos );
		if ( xaxis == 0 || yaxis == 0 ) continue; // this should never be encountered
		wxPLRealRect &bb = m_plotRects[ m_plots[i].ppos ];
		wxPLAxisDeviceMapping map( xaxis, bb.x, bb.x+bb.width, xaxis == GetXAxis1(),
			yaxis, bb.y+bb.height, bb.y, yaxis == GetYAxis1() );

		dc.SetAntiAliasing( m_plots[i].plot->GetAntiAliasing() );

		dc.Clip( bb.x, bb.y, bb.width, bb.height );
		m_plots[i].plot->Draw( dc, map );
		dc.Unclip();
	}

	dc.SetAntiAliasing( false );

	// draw some axes
	AXIS_FONT(dc);
	dc.Pen( m_axisColour, 0.5 );	
	if ( m_x2.axis )
		m_x2.layout->render( dc, m_plotRects[0].y, m_x2.axis, 
			box.x, box.x+box.width, 
			m_x1.axis == 0 ? m_plotRects[nyaxes-1].y+m_plotRects[nyaxes-1].height : -1 );
	
	// set up some polar plot values
	wxPLRealRect rect1 = m_plotRects[0];
	double pp_radius = (rect1.width < rect1.height) ? rect1.width / 2.0 : rect1.height / 2.0;
	wxRealPoint pp_center(rect1.x + rect1.width / 2.0, rect1.y + rect1.height / 2.0);

	// render y axes
	for ( size_t pp=0;pp<nyaxes; pp++ )
	{
		if (m_y1[pp].axis != 0) {
			if (is_cartesian)
				m_y1[pp].layout->render( dc, box.x, m_y1[pp].axis, 
					m_plotRects[pp].y + m_plotRects[pp].height,  m_plotRects[pp].y,
					m_y2[pp].axis == 0 ? box.x+box.width : -1 );
			else
				m_y1[pp].layout->render( dc, pp_center.x, m_y1[pp].axis, 
					pp_center.y,  pp_center.y-pp_radius,
					-1 );
		}
		
		if ( m_y2[pp].axis != 0 )
			m_y2[pp].layout->render( dc, box.x+box.width, m_y2[pp].axis, 
				m_plotRects[pp].y + m_plotRects[pp].height,  m_plotRects[pp].y,
				-1 /* m_y1[pp].axis == 0 ? box.x : -1 */ );
	}

	// render x1 axis (or angular axis on polar plots)
	if (m_x1.axis) {
		if (is_cartesian)
			m_x1.layout->render(dc, m_plotRects[nyaxes - 1].y + m_plotRects[nyaxes - 1].height, m_x1.axis,
				box.x, box.x + box.width,
				-1 /*m_x2.axis == 0 ? m_plotRects[0].y : -1*/);
		else
			m_x1.layout->render(dc, pp_radius, m_x1.axis, pp_center.x, pp_center.y, -1);
	}

	// draw boundaries around plots
	if (is_cartesian) {
		for (size_t pp = 0; pp < nyaxes; pp++)
		{
			dc.Line(m_plotRects[pp].x, m_plotRects[pp].y, m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y);
			dc.Line(m_plotRects[pp].x, m_plotRects[pp].y, m_plotRects[pp].x, m_plotRects[pp].y + m_plotRects[pp].height);
			dc.Line(m_plotRects[pp].x, m_plotRects[pp].y + m_plotRects[pp].height, m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y + m_plotRects[pp].height);
			dc.Line(m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y, m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y + m_plotRects[pp].height);
		}
	}
	else {
		dc.Line(pp_center.x-pp_radius,pp_center.y,pp_center.x+pp_radius,pp_center.y);
		dc.Line(pp_center.x, pp_center.y + pp_radius, pp_center.x, pp_center.y - pp_radius);

		dc.Brush( *wxWHITE, wxPLOutputDevice::NONE ); // otherwise, circle is filled in
		dc.Circle(pp_center, pp_radius);
	}


	// draw axis labels
	AXIS_FONT(dc);
	if (m_x1.axis && m_x1.axis->IsLabelVisible() && !m_x1.axis->GetLabel().IsEmpty()) {
		if (is_cartesian)
			m_x1.label->render(dc, box.x + box.width / 2 - m_x1.label->width() / 2, plotbox.y + plotbox.height - m_x1.label->height() - text_space, 0, false);
		else {
			double dist = sqrt(pow(pp_radius, 2) / 2.0);
			m_x1.label->render(dc, pp_center.x + dist, pp_center.y - dist - m_x1.layout->bounds().y - m_x1.label->height() - text_space, 0, false);
		}
	}
	
	if ( m_x2.axis && m_x2.axis->IsLabelVisible() && !m_x2.axis->GetLabel().IsEmpty() )
		m_x2.label->render( dc, box.x+box.width/2-m_x2.label->width()/2, plotbox.y, 0, false );

	for ( size_t pp = 0; pp<nyaxes; pp++ )
	{
		if (m_y1[pp].axis && m_y1[pp].axis->IsLabelVisible() && !m_y1[pp].axis->GetLabel().IsEmpty()) {
			if (is_cartesian) {
				m_y1[pp].label->render(dc, plotbox.x + yleft_max_label_width - m_y1[pp].label->height(),
					m_plotRects[pp].y + m_plotRects[pp].height / 2 + m_y1[pp].label->width() / 2, 90, false);
			}
			else {
				m_y1[pp].label->render(dc, pp_center.x, pp_center.y - pp_radius + m_y1[pp].label->width()+text_space, 90, false);
			}
		}

		if (m_y2[pp].axis && m_y2[pp].axis->IsLabelVisible() && !m_y2[pp].axis->GetLabel().IsEmpty() )
			m_y2[pp].label->render( dc, plotbox.x+plotbox.width - yright_max_label_width + m_y2[pp].label->height(),
				m_plotRects[pp].y + m_plotRects[pp].height/2 - m_y2[pp].label->width()/2, -90, false );
	}

	LEGEND_FONT(dc);

	wxPLRealRect plotarea( m_plotRects[0].x, m_plotRects[0].y, m_plotRects[0].width, m_plotRects[0].height*nyaxes + (nyaxes-1)*plot_space );
	DrawLegend( dc, (m_legendPos==FLOATING||legend_bottom||legend_right) ? geom : plotarea  );
}

void wxPLPlot::DrawGrid( wxPLOutputDevice &dc, wxPLAxis::TickData::TickSize size )
{
	if ( m_plotRects.size() < 1 ) return;
	
	axis_data *xgrid_axis = 0;
	if ( m_x1.axis != 0 ) xgrid_axis = &m_x1;
	else if (m_x2.axis != 0 ) xgrid_axis = &m_x2;

	if ( xgrid_axis != 0 )
	{
		std::vector<double> ticks = xgrid_axis->layout->ticks( size );
		for ( size_t i=0;i<ticks.size();i++ )
		{
			double xpos = xgrid_axis->axis->WorldToPhysical( ticks[i], m_plotRects[0].x, m_plotRects[0].x+m_plotRects[0].width );

			for ( size_t pp = 0; pp < m_plotRects.size(); pp++ )
			{
				dc.Line( xpos, m_plotRects[pp].y, 
					xpos, m_plotRects[pp].y + m_plotRects[pp].height );
			}				
		}
	}

	for (size_t pp = 0; pp < m_plotRects.size(); pp++ )
	{
		axis_data *ygrid_axis = 0;
		if (m_y1[pp].axis != 0 ) ygrid_axis = &m_y1[pp];
		else if ( m_y2[pp].axis != 0 ) ygrid_axis = &m_y2[pp];

		if ( ygrid_axis != 0 )
		{
			std::vector<double> ticks = ygrid_axis->layout->ticks( size );
			for ( size_t j=0;j<ticks.size();j++)
			{
				double ypos = ygrid_axis->axis->WorldToPhysical( ticks[j], m_plotRects[pp].y+m_plotRects[pp].height, m_plotRects[pp].y );

				dc.Line( m_plotRects[0].x, ypos, 
					m_plotRects[0].x + m_plotRects[0].width, ypos );
			}
		}
	}
}

void wxPLPlot::DrawPolarGrid( wxPLOutputDevice &dc, wxPLAxis::TickData::TickSize size)
{
	if (m_plotRects.size() < 1) return;

	dc.Brush( *wxWHITE, wxPLOutputDevice::NONE );
	wxRealPoint cntr(m_plotRects[0].x + m_plotRects[0].width / 2.0, m_plotRects[0].y + m_plotRects[0].height / 2.0);
	double max_radius = (m_plotRects[0].width < m_plotRects[0].height) ? m_plotRects[0].width / 2.0 : m_plotRects[0].height / 2.0;

	// circles from center
	axis_data *radial_grid = 0;
	if (m_y1[0].axis != 0) radial_grid = &m_y1[0];

	if (radial_grid != 0)
	{
		std::vector<double> ticks = radial_grid->layout->ticks(size);
		for (size_t j = 0; j<ticks.size(); j++)
		{
			double radius = radial_grid->axis->WorldToPhysical(ticks[j], 0, max_radius);
			dc.Circle(cntr, radius);
		}
	}

	// rays from center out to edge of plot
	axis_data *angular_grid = 0;
	if (m_x1.axis != 0) angular_grid = &m_x1;

	if (angular_grid != 0) {
		if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(m_x1.axis)) {
			std::vector<double> ticks = angular_grid->layout->ticks(size);
			for (size_t i = 0; i<ticks.size(); i++)
			{
				double angle = pa->AngleInRadians(ticks[i]);
				wxRealPoint pt(max_radius*cos(angle), max_radius*sin(angle));
				dc.Line(cntr, cntr + pt);
			}
		}
	}

}



wxPLPlot::legend_item::legend_item( wxPLOutputDevice &dc, wxPLPlottable *p )
{
	plot = p;
	text = p->GetLabel();
	layout = new text_layout( dc, text );
}

wxPLPlot::legend_item::~legend_item()
{
	if ( layout ) delete layout;
}

void wxPLPlot::CalcLegendTextLayout( wxPLOutputDevice &dc )
{
	if ( !m_showLegend ) return;

	if ( m_legendInvalidated )
	{
		LEGEND_FONT(dc);

		// rebuild legend items to show in plot
		for ( size_t i=0;i<m_legendItems.size(); i++ )
			delete m_legendItems[i];
		m_legendItems.clear();

		for ( size_t i=0; i<m_plots.size(); i++ )
		{
			if ( !m_plots[i].plot->IsShownInLegend() ) continue;

			m_legendItems.push_back( new legend_item( dc, m_plots[i].plot ) );
		}
	
		
		// realculate legend bounds based on text layouts
		if ( m_legendPos == BOTTOM )
		{
			m_legendRect.width = 0;
			m_legendRect.height = 0;
			
			for ( size_t i=0; i<m_legendItems.size(); i++ )
			{
				if ( !m_legendItems[i]->text.IsEmpty() )
				{
					double height = m_legendItems[i]->layout->height();
					if ( height > m_legendRect.height )
						m_legendRect.height = height;

					double width = m_legendItems[i]->layout->width();
					m_legendRect.width += width + 5*text_space + legend_item_box.x;
				}
			}
		}
		else
		{
			m_legendRect.width = 0;	
			m_legendRect.height = text_space;
			for ( size_t i=0; i<m_legendItems.size(); i++ )
			{
				if ( !m_legendItems[i]->text.IsEmpty() )
				{
					double width = m_legendItems[i]->layout->width();
					if ( width > m_legendRect.width )
						m_legendRect.width = width;

					double height = m_legendItems[i]->layout->height();
					if ( height < legend_item_box.y )
						height = legend_item_box.y;

					m_legendRect.height += height + text_space;
				}
			}
		}
		
		m_legendRect.width += legend_item_box.x + 5*text_space;
		m_legendInvalidated = false;
	}
}

void wxPLPlot::DrawLegend( wxPLOutputDevice &dc, const wxPLRealRect& geom )
{
	if ( !m_showLegend )
		return;

	int layout = wxVERTICAL;
	double max_item_height = 0;
	double max_item_width = 0;

	// offset by LegendXYPercents
	if ( m_legendPos == FLOATING )
	{
		if (m_legendPosPercent.x < -10) m_legendPosPercent.x = -10;
		if (m_legendPosPercent.x > 90) m_legendPosPercent.x = 90;
		if (m_legendPosPercent.y < -10) m_legendPosPercent.y = -10;
		if (m_legendPosPercent.y > 90) m_legendPosPercent.y = 90;

		m_legendRect.x = (int)(geom.x + m_legendPosPercent.x / 100.0 * geom.width);
		m_legendRect.y = (int)(geom.y + m_legendPosPercent.y / 100.0 * geom.height);
	}
	else
	{
		switch ( m_legendPos )
		{
		case NORTHWEST:
			m_legendRect.x = geom.x + text_space*2;
			m_legendRect.y = geom.y + text_space*2;
			break;
		case SOUTHWEST:
			m_legendRect.x = geom.x + text_space*2;
			m_legendRect.y = geom.y + geom.height - m_legendRect.height - text_space*2;
			break;
		case NORTHEAST:
			m_legendRect.x = geom.x + geom.width - m_legendRect.width - text_space*2;
			m_legendRect.y = geom.y + text_space*2;
			break;
		case SOUTHEAST:
			m_legendRect.x = geom.x + geom.width - m_legendRect.width - text_space*2;
			m_legendRect.y = geom.y + geom.height - m_legendRect.height - text_space*2;
			break;
		case NORTH:
			m_legendRect.x = geom.x + geom.width/2 - m_legendRect.width/2;
			m_legendRect.y = geom.y + text_space*2;
			break;
		case SOUTH:
			m_legendRect.x = geom.x + geom.width/2 - m_legendRect.width/2;
			m_legendRect.y = geom.y + geom.height - m_legendRect.height - text_space*2;
			break;
		case EAST:
			m_legendRect.x = geom.x + geom.width - m_legendRect.width - text_space*2;
			m_legendRect.y = geom.y + geom.height/2 - m_legendRect.height/2;
			break;
		case WEST:
			m_legendRect.x = geom.x + text_space*2;
			m_legendRect.y = geom.y + geom.height/2 - m_legendRect.height/2;
			break;
		case BOTTOM:
			m_legendRect.x = geom.x + text_space;
			for (size_t i=0;i<m_legendItems.size();i++)
				if ( m_legendItems[i]->layout->height() > max_item_height )
					max_item_height = m_legendItems[i]->layout->height();
			m_legendRect.y = geom.y + geom.height - max_item_height - text_space;
			layout = wxHORIZONTAL;
			break;
		case RIGHT:
			m_legendRect.y = geom.y + text_space;
			for (size_t i=0;i<m_legendItems.size();i++)
				if ( m_legendItems[i]->layout->width() > max_item_width )
					max_item_width = m_legendItems[i]->layout->width();
			m_legendRect.x = geom.x + geom.width - max_item_width - 5*text_space-legend_item_box.x;
			break;
		}
	}
	
	dc.SetAntiAliasing( false );
	
	if ( m_legendPos != BOTTOM && m_legendPos != RIGHT )
	{
		dc.Brush( *wxWHITE );
		dc.Pen( *wxLIGHT_GREY, 0.5 );
		dc.Rect( m_legendRect );
	}
	
	double x = m_legendRect.x;
	double y = m_legendRect.y + text_space;
	for ( size_t i = 0; i < m_legendItems.size(); i++ )
	{
		legend_item &li = *m_legendItems[ m_reverseLegend ? m_legendItems.size() - i - 1 : i ];

		if ( li.text.IsEmpty() ) continue;
		
		double yoff_text = 0;
		if ( layout == wxHORIZONTAL )
			yoff_text = (max_item_height - li.layout->height())/2;

		li.layout->render( dc, x + legend_item_box.x + 2*text_space + text_space, y + yoff_text );

		double yoff_box = li.layout->height()/2 - legend_item_box.y/2;
		if ( layout == wxHORIZONTAL )
			yoff_box = max_item_height/2 - legend_item_box.y/2;

		dc.SetAntiAliasing( li.plot->GetAntiAliasing() );

		li.plot->DrawInLegend( dc, wxPLRealRect( x + 2*text_space, y + yoff_box , 
			legend_item_box.x, legend_item_box.y ) );

		if ( layout == wxHORIZONTAL )
			x += 5*text_space + legend_item_box.x + li.layout->width();
		else
			y += li.layout->height() + text_space;
	}
	
	dc.SetAntiAliasing( false );

	//dc.SetPen( *wxBLACK_PEN );
	//dc.SetBrush( *wxTRANSPARENT_BRUSH );
	//dc.DrawRectangle( m_legendRect );
}

void wxPLPlot::Invalidate()
{
	if ( m_titleLayout != 0 )
	{
		delete m_titleLayout;
		m_titleLayout = 0;
	}

	m_legendInvalidated = true;
	m_x1.invalidate();
	m_x2.invalidate();
	for ( size_t i=0;i<NPLOTPOS;i++ )
	{
		m_y1[i].invalidate();
		m_y2[i].invalidate();
	}
}

void wxPLPlot::DeleteAxes()
{
	SetXAxis1( 0 );
	SetXAxis2( 0 );
	for ( size_t pp = 0; pp<NPLOTPOS; pp++ )
	{
		SetYAxis1( 0, (wxPLPlot::PlotPos)pp );
		SetYAxis2( 0, (wxPLPlot::PlotPos)pp );
	}

	Invalidate();
}

void wxPLPlot::RescaleAxes()
{
	//This does not set axes to null, or change anything other than their bounds.
	bool xAxis1Set = false, xAxis2Set = false;
	std::vector<bool> yAxis1Set(NPLOTPOS, false), yAxis2Set(NPLOTPOS, false);

	for (size_t i = 0; i < m_plots.size(); ++i)
    {
        wxPLPlottable *p = m_plots[i].plot;
        wxPLPlot::AxisPos xap = m_plots[i].xap;
		wxPLPlot::AxisPos yap = m_plots[i].yap;
		wxPLPlot::PlotPos ppos = m_plots[i].ppos;

        if (xap == X_BOTTOM)
        {
            if ( !xAxis1Set && GetXAxis1() )
            {
				double xMin, xMax;
				p->GetMinMax(&xMin, &xMax, NULL, NULL);
				GetXAxis1()->SetWorld(xMin, xMax);
            }
            else
            {
				wxPLAxis *pnew = p->SuggestXAxis();
				if (pnew)
				{
					if (GetXAxis1()) GetXAxis1()->ExtendBound( pnew );

					delete pnew;
				}
            }
			xAxis1Set = true;
        }

        if (xap == X_TOP)
        {
            if (!xAxis2Set && GetXAxis2())
            {
				double xMin, xMax;
				p->GetMinMax(&xMin, &xMax, NULL, NULL);
				GetXAxis2()->SetWorld(xMin, xMax);              
            }
            else
            {
				wxPLAxis *pnew = p->SuggestXAxis();
				if (pnew)
				{
					if (GetXAxis2()) GetXAxis2()->ExtendBound( pnew );
					delete pnew;
				}
            }
			xAxis2Set = true;
        }

        if (yap == Y_LEFT)
        {
			if (!yAxis1Set[ppos] && GetYAxis1(ppos))
            {
				double yMin, yMax;
				p->GetMinMax(NULL, NULL, &yMin, &yMax);
				GetYAxis1(ppos)->SetWorld(yMin, yMax);
            }
            else
            {
				wxPLAxis *pnew = p->SuggestYAxis();
				if (pnew)
				{
					if (GetYAxis1( ppos ) )
						GetYAxis1(ppos)->ExtendBound(pnew);
					delete pnew;
				}
            }
			yAxis1Set[ppos] = true;
        }

        if (yap == Y_RIGHT)
        {
			if (!yAxis2Set[ppos] && GetYAxis2( ppos ) )
            {
				double yMin, yMax;
				p->GetMinMax(NULL, NULL, &yMin, &yMax);
				GetYAxis2(ppos)->SetWorld(yMin, yMax);
            }
            else
            {
				wxPLAxis *pnew = p->SuggestYAxis();
				if (pnew)
				{
					if (GetYAxis2(ppos)) GetYAxis2(ppos)->ExtendBound(pnew);
					delete pnew;
				}
            }
			yAxis2Set[ppos] = true;
        }
    }
}

void wxPLPlot::UpdateAxes( bool recalc_all )
{
	int position = 0;

    // if we're not recalculating axes using all iplots then set
    // position to last one in list.
    if ( !recalc_all )
    {
        position =  m_plots.size()-1;
        if (position < 0) 
			position = 0;
    }

    if ( recalc_all )
    {
		SetXAxis1( NULL );
		SetXAxis2( NULL );
		
		for (int i=0; i<NPLOTPOS; i++)
		{
			SetYAxis1( NULL, (PlotPos)i );
			SetYAxis2( NULL, (PlotPos)i );
		}
    }

    for (int i = position; i < m_plots.size(); i++ )
    {
        wxPLPlottable *p = m_plots[i].plot;
        AxisPos xap = m_plots[i].xap;
		AxisPos yap = m_plots[i].yap;
		PlotPos ppos = m_plots[i].ppos;

        if( xap == X_BOTTOM )
        {
            if ( GetXAxis1() == NULL )
            {
                SetXAxis1( p->SuggestXAxis() );
            }
            else
            {
				if ( wxPLAxis *pnew = p->SuggestXAxis() )
				{
					GetXAxis1()->ExtendBound( pnew );
					delete pnew;
				}
            }
        }

        if( xap == X_TOP )
        {
            if( GetXAxis2() == NULL )
            {
                SetXAxis2( p->SuggestXAxis() );                
            }
            else
            {
				if ( wxPLAxis *pnew = p->SuggestXAxis() )
				{
					GetXAxis2()->ExtendBound( pnew );
					delete pnew;
				}
            }
        }

        if( yap == Y_LEFT )
        {
			if ( GetYAxis1( ppos ) == NULL )
            {
                SetYAxis1( p->SuggestYAxis(), ppos );
            }
            else
            {
				if ( wxPLAxis *pnew = p->SuggestYAxis() )
				{
					GetYAxis1( ppos )->ExtendBound( pnew );
					delete pnew;
				}
            }
        }

        if ( yap == Y_RIGHT )
        {
            if ( GetYAxis2( ppos ) == NULL )
            {
                SetYAxis2(p->SuggestYAxis(), ppos);
            }
            else
            {
				if ( wxPLAxis *pnew = p->SuggestYAxis() )
				{
					GetYAxis2( ppos )->ExtendBound(pnew);
					delete pnew;
				}
            }
			GetYAxis2(ppos)->ShowTickText( true );
        }
    }
}

static wxPathList s_pdfFontDirs;

bool wxPLPlot::AddPdfFontDir( const wxString &path )
{
	wxFileName fn(path);
	fn.Normalize();
	wxString folder( fn.GetFullPath() );
	if (s_pdfFontDirs.Index( folder ) == wxNOT_FOUND )
	{
		if ( wxDirExists( folder ) )
		{
			s_pdfFontDirs.Add( folder );
			return true;
		}
	}

	return false;
}

wxString wxPLPlot::LocatePdfFontInfoXml( const wxString &face )
{
	return s_pdfFontDirs.FindAbsoluteValidPath( face + ".xml" );
}

wxArrayString wxPLPlot::ListAvailablePdfFonts()
{
	wxArrayString faces;
	for( size_t i=0;i<s_pdfFontDirs.size();i++ )
	{
		wxArrayString files;
		wxDir::GetAllFiles( s_pdfFontDirs[i], &files, "*.xml", wxDIR_FILES );
		for( size_t k=0;k<files.size();k++ )
		{
			wxFileName file( files[k] );
			faces.Add( file.GetName() );
		}
	}
	return faces;
}

static wxString s_pdfDefaultFontFace("Helvetica");
static double s_pdfDefaultFontPoints = 12.0;

static void EnsureStandardPdfFontPaths()
{
	// make sure we have the standard locations for pdf font data
	wxPLPlot::AddPdfFontDir( wxPathOnly(wxStandardPaths::Get().GetExecutablePath() ) + "/pdffonts" );
}

static bool IsBuiltinPdfFont( const wxString &face )
{
	return face == "Helvetica"
		|| face == "Courier"
		|| face == "Times"
		|| face == "Arial"
		|| face == "ZapfDingbats"
		|| face == "Symbol";
}

bool wxPLPlot::SetPdfDefaultFont( const wxString &face, double points )
{
	EnsureStandardPdfFontPaths();

	if ( points > 0 ) s_pdfDefaultFontPoints = points; // negative point size retains current size

	if ( face.IsEmpty() ) return true; // no changes to the face

	if (  !IsBuiltinPdfFont(face) )
	{
		wxString xml( LocatePdfFontInfoXml( face ) );
		if ( !xml.IsEmpty() )
		{
			s_pdfDefaultFontFace = face;
			return true;
		}
		else
			return false;
	}
	else
	{
		s_pdfDefaultFontFace = face;
		return true;
	}
}

bool wxPLPlot::RenderPdf( const wxString &file, double width, double height )
{
	EnsureStandardPdfFontPaths();

	wxPdfDocument doc( wxPORTRAIT, "pt", wxPAPER_A5 );
	doc.AddPage( wxPORTRAIT, width, height );

	if ( !IsBuiltinPdfFont( s_pdfDefaultFontFace ) )
	{
		wxString xml( LocatePdfFontInfoXml( s_pdfDefaultFontFace ) );
		if ( !doc.AddFont( s_pdfDefaultFontFace, wxEmptyString, xml ) 
			|| !doc.SetFont( s_pdfDefaultFontFace, wxPDF_FONTSTYLE_REGULAR, s_pdfDefaultFontPoints ) )
			doc.SetFont( "Helvetica", wxPDF_FONTSTYLE_REGULAR, s_pdfDefaultFontPoints );
	}
	else
		doc.SetFont( s_pdfDefaultFontFace, wxPDF_FONTSTYLE_REGULAR, s_pdfDefaultFontPoints );
	
	Invalidate();

	wxPLPdfOutputDevice dc(doc);
	Render( dc, wxPLRealRect( 3, 3, width-6, height-6 ) );
		
	Invalidate();
		
	const wxMemoryOutputStream &data = doc.CloseAndGetBuffer();
	wxFileOutputStream fp( file );
	if (!fp.IsOk()) return false;

	wxMemoryInputStream tmpis( data );
	fp.Write( tmpis );
	return fp.Close();
}

/*
wxPostScriptDC dc(wxT("output.ps"), true, wxGetApp().GetTopWindow());

if (dc.Ok())
{
    // Tell it where to find the AFM files
    dc.GetPrintData().SetFontMetricPath(wxGetApp().GetFontPath());

    // Set the resolution in points per inch (the default is 720)
    dc.SetResolution(1440);

    // Draw on the device context
    ...
}
*/

#ifdef TEXTLAYOUT_DEMO

#include <wx/dc.h>

BEGIN_EVENT_TABLE( TextLayoutDemo, wxWindow )
	EVT_PAINT( TextLayoutDemo::OnPaint )
	EVT_SIZE( TextLayoutDemo::OnSize )
END_EVENT_TABLE()


TextLayoutDemo::TextLayoutDemo( wxWindow *parent )
	: wxWindow( parent, wxID_ANY )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
}

std::vector<double> TextLayoutDemo::Draw( wxDC &scdc, const wxPLRealRect &geom )
{
	std::vector<double> vlines;
	scdc.SetClippingRegion( geom );
	
	// test 1
	{
		scdc.SetFont( *wxNORMAL_FONT );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t1( dc, "basic text, nothing special" );
		t1.render( dc, geom.x+20, geom.y+20, 0.0, true );
		vlines.push_back( geom.x+20 );
		vlines.push_back( geom.x+20+t1.width() );
	}

	// test 2
	{
		scdc.SetFont( wxFont(18, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Century Schoolbook") );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t2( dc, "escape^sup_sub \\\\  \\  \\euro \\badcode \\Omega~\\Phi\\ne\\delta, but\n\\Kappa\\approx\\zeta \\emph\\qmark, and this is the end of the text!. Cost was 10 \\pound, or 13.2\\cent" );
		t2.render( dc, geom.x+20, geom.y+120, 0.0, true );
	}
	
	// test 3
	{
		scdc.SetFont( wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Consolas") );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t3( dc, "super^2 ,not_\\rho\\gamma f hing_{special,great,best}\n\\alpha^^\\beta c^\\delta  efjhijkl__mnO^25 pq_0 r\\Sigma tuvwxyz\nABCDEFGHIJKL^^MNOPQRSTUVWXZY" );
		t3.render( dc, geom.x+20, geom.y+420, 90, true );
		t3.render( dc, geom.x+200, geom.y+350, 45.0, true );
		t3.render( dc, geom.x+400, geom.y+300, 0.0, true );
		vlines.push_back( geom.x+400 );
		vlines.push_back( geom.x+400+t3.width() );
	}
	
	// test 4
	{
		scdc.SetFont( wxFont(16, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t4( dc, "x_1^2_3 abc=y_2^4" );
		t4.render( dc, geom.x+200, geom.y+70, 0, false );
	}

	// test 5
	{
		scdc.SetFont( wxFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t5( dc, "small (7): x_1^2_3 abc=y_2^4" );
		t5.render( dc, geom.x+500, geom.y+60, 0, false );
	}
	
	// test 6
	{
		scdc.SetFont( wxFont(8, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t6( dc, "small (8): x_1^2_3 abc=y_2^4" );
		t6.render( dc, geom.x+500, geom.y+80, 0, false );
	}
	
	// test 7
	{
		scdc.SetFont( wxFont(9, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t7( dc, "small (9): x_1^2_3 abc=y_2^4" );
		t7.render( dc, geom.x+500, geom.y+100, 0, false );
	}
	
	// test 8
	{
		scdc.SetFont( wxFont(10, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		wxDCOutputDevice dc(scdc);
		wxPLPlot::text_layout t8( dc, "small (10): x_1^2_3 abc=y_2^4" );
		t8.render( dc, geom.x+500, geom.y+120, 0, false );
	}

	vlines.push_back( geom.x+500 );

	scdc.DestroyClippingRegion();
	scdc.SetPen( *wxBLUE_PEN );
	scdc.SetBrush( *wxTRANSPARENT_BRUSH );
	scdc.DrawRectangle( geom );

	return vlines;
}

void TextLayoutDemo::OnPaint( wxPaintEvent & )
{
	wxAutoBufferedPaintDC pdc( this );
	
	int width, height;
	GetClientSize( &width, &height );

	pdc.SetBackground( *wxWHITE_BRUSH );
	pdc.Clear();

	wxGCDC gdc1(pdc);

#ifdef __WXMSW__
//	gdc1.GetGraphicsContext()->MSWSetStringFormat( wxGraphicsContext::Format_Normal );
#endif

	Draw( gdc1, wxPLRealRect( 0, 0, width, height/2 ) );

	wxGCDC gdc2(pdc);

#ifdef __WXMSW__
//	gdc2.GetGraphicsContext()->MSWSetStringFormat( wxGraphicsContext::Format_OptimizeForSmallFont );
#endif
	
	Draw( gdc2, wxPLRealRect( 0, height/2, width, height/2 ) );
	/*
	std::vector<int> vlines = Draw(pdc, wxPLRealRect( 0, 0, width, height/2 ) );

	
	wxGCDC gdc2(pdc);
	Draw( gdc2, wxPLRealRect( 0, height/2, width, height/2 ) );

	pdc.SetPen( *wxGREEN_PEN );
	pdc.SetBrush( *wxTRANSPARENT_BRUSH );

	for ( size_t i=0;i<vlines.size();i++ )
		pdc.DrawLine( vlines[i], 0, vlines[i], height );

	pdc.SetFont( wxFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	pdc.SetTextForeground( *wxRED );
	pdc.DrawText( wxT("Using wxAutoBufferedPaintDC"), width/3, 10 );
	pdc.DrawText( wxT("green lines for showing text extent issues (compare top & bottom)"), width/3, 10+pdc.GetCharHeight() );
	pdc.DrawText( wxT("CowxPLRealRected using wxGCDC. Modified wxWidgets:graphics.cpp"), width/3, height/2+10 );
	pdc.DrawText( wxT("This time using StringFormat::GenericTypographic()"), width/3, height/2+10+pdc.GetCharHeight() );
	*/
}

void TextLayoutDemo::OnSize( wxSizeEvent & )
{
	Refresh();
}
#endif
