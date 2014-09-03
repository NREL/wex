#include <numeric>
#include <limits>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/dcprint.h>
#include <wx/dcclient.h>
#include <wx/log.h>
#include <wx/tokenzr.h>
#include <wx/menu.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/sstream.h>
#include <wx/dcsvg.h>
#include <wx/tipwin.h>

#include "wex/dview/dvplothelper.h"

#include "wex/plot/plhistplot.h"
#include "wex/plot/plplotctrl.h"
#include "wex/pdf/pdfdc.h"

#ifdef __WXMSW__
#include "wex/ole/excelauto.h"
#endif

#ifdef __WXOSX__
#include <cmath>
#define wxIsNaN(a) std::isnan(a)
#endif

static const int text_space = 3;
static const wxSize legend_item_box(13, 13);

class wxPLAxisDeviceMapping : public wxPLDeviceMapping
{
private:
	wxPLAxis *m_xAxis;
	wxCoord m_xPhysMin, m_xPhysMax;
	wxPLAxis *m_yAxis;
	wxCoord m_yPhysMin, m_yPhysMax;
	wxCoord m_physical_constraint;
	wxPoint m_ptCenter;

public:
	wxPLAxisDeviceMapping( wxPLAxis *x, wxCoord xmin, wxCoord xmax,
		wxPLAxis *y, wxCoord ymin, wxCoord ymax )
		: m_xAxis(x), m_xPhysMin(xmin), m_xPhysMax(xmax), 
		m_yAxis(y), m_yPhysMin(ymin), m_yPhysMax(ymax)
	{
		wxRect box = GetDeviceExtents();
		m_ptCenter = wxPoint(box.x + box.width / 2.0, box.y + box.height / 2.0);
		m_physical_constraint = (box.width < box.height) ? box.width : box.height;
	}
	
	virtual wxPoint ToDevice( double x, double y ) const
	{
		if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(m_xAxis)) {
			// this is a polar plot, so translate the "point" as if it's a angle/radius combination

			// adjust for where zero degrees should be (straight up?) and units of angular measure
			double angle_in_rad = pa->AngleInRadians(x);

			// get radius in physical units
			double radius0 = m_yAxis->WorldToPhysical(y, 0, m_physical_constraint / 2.0); // max radius has to be 1/2 of physical constraint

			// locate point relative to center of polar plot
			wxPoint pt = wxPoint(radius0*cos(angle_in_rad), radius0*sin(angle_in_rad));

			// return point on physical device
			return (m_ptCenter + pt);
		}
		else // Cartesian plot
			return wxPoint( m_xAxis->WorldToPhysical( x, m_xPhysMin, m_xPhysMax ),
				m_yAxis->WorldToPhysical( y, m_yPhysMin, m_yPhysMax ) );
	}
	
	virtual wxRect GetDeviceExtents( ) const
	{
		return wxRect( m_xPhysMin, 
			m_yPhysMin < m_yPhysMax ? m_yPhysMin : m_yPhysMax,
			m_xPhysMax - m_xPhysMin,
			abs( m_yPhysMax - m_yPhysMin ) );
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
};


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
	wxDVPlotHelper::ExtendBoundsToNiceNumber(&ymaxNice, &yminNice);
	if (pxmin && xmin < *pxmin) *pxmin = xmin;
	if (pxmax && xmax > *pxmax) *pxmax = xmax;
	if (pymin && yminNice < *pymin) *pymin = yminNice;
	if (pymax && ymaxNice > *pymax) *pymax = ymaxNice;
	return true;
}

std::vector<wxString> wxPLPlottable::GetExportableDatasetHeaders( wxUniChar sep ) const
{
	std::vector<wxString> tt;
	wxString xLabel = GetXDataLabel();
	wxString yLabel = GetYDataLabel();
			
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

wxSize wxPLSideWidgetBase::GetBestSize()
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

class wxPLPlotCtrl::text_layout
{

	struct text_piece
	{
		text_piece() : text(wxEmptyString), state(NORMAL), origin(0, 0), size(0, 0), aligned_x(0) {  }

		enum TextState { NORMAL, SUPERSCRIPT, SUBSCRIPT }; 
		wxString text;
		TextState state;
		wxPoint origin;
		wxSize size;
		wxCoord aligned_x;
	};

	std::vector< std::vector<text_piece> > m_lines;
	wxSize m_bounds;

	static const int FontPointAdjust = 2;

public:
	enum TextAlignment { LEFT, CENTER, RIGHT };

	text_layout( wxDC &dc, const wxString &text, TextAlignment ta = LEFT )
		: m_bounds(0, 0)
	{
		if ( text.IsEmpty() ) return;

		// split text into lines, and parse each one into text pieces after resolving escape sequences
		wxArrayString lines = wxStringTokenize( text, "\r\n" );
		for (size_t i=0;i<lines.Count();i++)
			m_lines.push_back( parse( escape(lines[i]) ) );

		if ( m_lines.size() == 0 ) return;
		
		// set up fonts for text layout
		wxFont font_normal = dc.GetFont();
		wxFont font_small( font_normal );
		font_small.SetPointSize( font_small.GetPointSize() - FontPointAdjust );

		// compute extents of each text piece with the right font
		for ( size_t i=0;i<m_lines.size(); i++ )
		{
			for ( size_t j=0;j<m_lines[i].size(); j++ )
			{
				text_piece &tp = m_lines[i][j];
				wxCoord width, height;
				dc.GetTextExtent( tp.text, &width, &height, 0, 0, 
					tp.state == text_piece::NORMAL ? &font_normal : &font_small);
				tp.size.Set( width, height );
			}
		}

		// obtain the approximate heights for normal and small text
		dc.SetFont( font_small );
		wxCoord height_small = dc.GetCharHeight();
		dc.SetFont( font_normal );
		wxCoord height_normal = dc.GetCharHeight();

		// sequentially calculate the origins of each text piece
		
		const wxCoord offset = height_small/4; // amount to raise/lower super/sub-scripts
		wxCoord y = 0;
		
		for ( size_t i=0;i<m_lines.size(); i++ )
		{
			wxCoord x = 0;
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
			wxCoord line_width = 0;
			for ( size_t j=0;j<m_lines[i].size();j++ )
			{
				// restore original aligned positions for each text piece
				m_lines[i][j].origin.x = m_lines[i][j].aligned_x;
				line_width += m_lines[i][j].size.x;
			}

			wxCoord offset = 0;
			if ( ta == CENTER ) offset = (m_bounds.x - line_width)/2;
			else if ( ta == RIGHT ) offset = m_bounds.x - line_width;

			if ( offset != 0 ) // only do this for center/right alignments
				for ( size_t j=0;j<m_lines[i].size();j++ )
					m_lines[i][j].origin.x += offset;
		}
	}

	void render( wxDC &dc, wxCoord x, wxCoord y, double rotationDegrees = 0.0, bool draw_bounds = false )
	{
		if ( m_lines.size() == 0 ) return;

		wxFont font_normal = dc.GetFont();
		wxFont font_small( font_normal );
		font_small.SetPointSize( font_small.GetPointSize() - FontPointAdjust );

		if ( draw_bounds )
		{
			dc.SetPen( *wxLIGHT_GREY_PEN );
			dc.SetBrush( *wxTRANSPARENT_BRUSH );
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
					dc.SetFont( tp.state == text_piece::NORMAL ? font_normal : font_small );
					dc.DrawText( tp.text, x + tp.origin.x, y + tp.origin.y );				
					if ( draw_bounds )
						dc.DrawRectangle( x+tp.origin.x, y+tp.origin.y, tp.size.x, tp.size.y );
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
					dc.SetFont( tp.state == text_piece::NORMAL ? font_normal : font_small );
					double rotx = tp.origin.x*costheta - tp.origin.y*sintheta;
					double roty = tp.origin.x*sintheta + tp.origin.y*costheta;
					dc.DrawRotatedText( tp.text, x + rotx, y + roty, rotationDegrees );
				}
			}
		}

		dc.SetFont( font_normal );
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

	inline wxSize bounds() { return m_bounds; }
	inline wxCoord width() { return m_bounds.x; }
	inline wxCoord height() { return m_bounds.y; }
};

class wxPLPlotCtrl::axis_layout
{
private:

	struct tick_layout
	{
		tick_layout( wxDC &dc, const wxPLAxis::TickData &td )
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

	void renderAngular(wxDC &dc, wxCoord radius, wxPLPolarAngularAxis *axis, wxCoord cntr_x, wxCoord cntr_y)
	{
		wxPoint cntr = wxPoint(cntr_x, cntr_y);

		// draw tick marks and tick labels
		for (size_t i = 0; i < m_tickList.size(); i++)
		{
			tick_layout &ti = m_tickList[i];
			wxCoord tick_length = ti.tick_size == wxPLAxis::TickData::LARGE ? 5 : 2;
			double angle = axis->AngleInRadians(ti.world);
			wxPoint pt0 = cntr + wxPoint(radius*cos(angle), radius*sin(angle));
			wxPoint pt1 = cntr + wxPoint((radius - tick_length)*cos(angle), (radius - tick_length)*sin(angle));
			dc.DrawLine(pt0, pt1);

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

	axis_layout( int ap, wxDC &dc, wxPLAxis *axis, wxCoord phys_min, wxCoord phys_max )
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
			wxCoord xmin = phys_min;
			wxCoord xmax = phys_max;
			wxCoord ymax = 0;

			for ( size_t i=0;i<m_tickList.size();i++ )
			{
				tick_layout &ti = m_tickList[i];
				wxCoord phys = axis->WorldToPhysical( ti.world, phys_min, phys_max );
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
			const int tick_label_space = 4;

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
				wxCoord width = 0;
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

				wxCoord phys_left = axis->WorldToPhysical( left.world, phys_min, phys_max );
				wxCoord phys = axis->WorldToPhysical( center.world, phys_min, phys_max );
				wxCoord phys_right = axis->WorldToPhysical( right.world, phys_min, phys_max );
				
				if ( (phys - center.text.width()/2 - tick_label_space < phys_left + left.text.width()/2)
					|| (phys + center.text.width()/2 + tick_label_space > phys_right - right.text.width()/2) )
				{
					// rotate all the ticks, not just the large ones

					textAngle = 45;
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
			wxCoord ymin = phys_max; // ymin is upper coordinate
			wxCoord ymax = phys_min; // ymax is lower coordinate
			wxCoord xmax = 0; // maximum text width
			for ( size_t i=0;i<m_tickList.size();i++ )
			{
				tick_layout &ti = m_tickList[i];
				wxCoord phys = axis->WorldToPhysical( ti.world, phys_min, phys_max );
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

	void render( wxDC &dc, wxCoord ordinate, wxPLAxis *axis, wxCoord phys_min, wxCoord phys_max, wxCoord ordinate_opposite = -1 )
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
			wxCoord physical = axis->WorldToPhysical( ti.world, phys_min, phys_max );

			if ( ti.tick_size != wxPLAxis::TickData::NONE )
			{
				wxPoint tickStart, tickEnd;
				wxCoord tick_length = ti.tick_size == wxPLAxis::TickData::LARGE ? 5 : 2;
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

				dc.DrawLine( tickStart, tickEnd );

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

					dc.DrawLine( tickStart, tickEnd );
				}
			}

			if ( ti.text.width() > 0 )
			{
				wxCoord text_x = 0, text_y = 0;
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

wxPLPlotCtrl::axis_data::axis_data()
	: axis(0), layout(0), label(0)
{
}

wxPLPlotCtrl::axis_data::~axis_data()
{
	if (axis) delete axis;
	if (layout) delete layout;
	if (label) delete label;
}

void wxPLPlotCtrl::axis_data::set( wxPLAxis *a )
{
	if (axis == a) return;
	if (axis) delete axis;
	axis = a;
	invalidate();
}

void wxPLPlotCtrl::axis_data::invalidate()
{
	if (layout) delete layout;
	layout = 0;
	if (label) delete label;
	label = 0;
}


DEFINE_EVENT_TYPE( wxEVT_PLOT_LEGEND )
DEFINE_EVENT_TYPE( wxEVT_PLOT_HIGHLIGHT )
DEFINE_EVENT_TYPE( wxEVT_PLOT_DRAGGING )
DEFINE_EVENT_TYPE( wxEVT_PLOT_DRAG_START )
DEFINE_EVENT_TYPE( wxEVT_PLOT_DRAG_END )


enum { ID_COPY_DATA_CLIP = wxID_HIGHEST + 1251, 
		ID_SAVE_DATA_CSV, ID_SEND_EXCEL, 
		ID_TO_CLIP_SCREEN, ID_TO_CLIP_SMALL, ID_TO_CLIP_NORMAL, 
		ID_EXPORT_SCREEN, ID_EXPORT_SMALL, ID_EXPORT_NORMAL, ID_EXPORT_PDF,
		ID_EXPORT_SVG };

BEGIN_EVENT_TABLE( wxPLPlotCtrl, wxWindow )
	EVT_PAINT( wxPLPlotCtrl::OnPaint )
	EVT_SIZE( wxPLPlotCtrl::OnSize )
	EVT_LEFT_DOWN( wxPLPlotCtrl::OnLeftDown )
	EVT_LEFT_UP( wxPLPlotCtrl::OnLeftUp )
	EVT_RIGHT_DOWN( wxPLPlotCtrl::OnRightDown )
	EVT_MOTION( wxPLPlotCtrl::OnMotion )	
	EVT_MOUSE_CAPTURE_LOST( wxPLPlotCtrl::OnMouseCaptureLost )
	
	EVT_MENU_RANGE( ID_COPY_DATA_CLIP, ID_EXPORT_SVG, wxPLPlotCtrl::OnPopupMenu )
END_EVENT_TABLE()

wxPLPlotCtrl::wxPLPlotCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size)
	: wxWindow(parent, id, pos, size)
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );

	for ( size_t i=0;i<4;i++ )
		m_sideWidgets[i] = 0;

	m_scaleTextSize = false;
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
	m_includeLegendOnExport = false;
	m_legendInvalidated = true;
	m_legendPosPercent.x = 85.0;
	m_legendPosPercent.y = 4.0;
	m_legendPos = FLOATING;
	m_anchorPoint = wxPoint(0, 0);
	m_currentPoint = wxPoint(0, 0);
	m_moveLegendMode = false;
	m_moveLegendErase = false;
	m_highlightMode = false;
	m_highlightErase = false;
	m_highlightLeftPercent = 0.0;
	m_highlightRightPercent = 0.0;
	m_allowHighlighting = false;
	
	m_contextMenu.Append( ID_COPY_DATA_CLIP, "Copy data to clipboard" );
	m_contextMenu.Append( ID_SAVE_DATA_CSV, "Save data to CSV..." );
#ifdef __WXMSW__
	m_contextMenu.Append( ID_SEND_EXCEL, "Send data to Excel..." );
#endif
	m_contextMenu.AppendSeparator();
	m_contextMenu.Append( ID_TO_CLIP_SCREEN, "To clipboard (as shown)" );
	m_contextMenu.Append( ID_TO_CLIP_SMALL, "To clipboard (400x300)" );
	m_contextMenu.Append( ID_TO_CLIP_NORMAL, "To clipboard (800x600)" );
	m_contextMenu.AppendSeparator();
	m_contextMenu.Append( ID_EXPORT_SCREEN, "Export (as shown)..." );
	m_contextMenu.Append( ID_EXPORT_SMALL, "Export (400x300)..." );
	m_contextMenu.Append( ID_EXPORT_NORMAL, "Export (800x600)..." );
	m_contextMenu.AppendSeparator();
	m_contextMenu.Append( ID_EXPORT_PDF, "Export as PDF..." );
	m_contextMenu.Append( ID_EXPORT_SVG, "Export as SVG..." );
}

wxPLPlotCtrl::~wxPLPlotCtrl()
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

void wxPLPlotCtrl::AddPlot( wxPLPlottable *p, AxisPos xap, AxisPos yap, PlotPos ppos, bool update_axes )
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

wxPLPlottable *wxPLPlotCtrl::RemovePlot( wxPLPlottable *p, PlotPos plotPosition )
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

bool wxPLPlotCtrl::ContainsPlot(wxPLPlottable *p, PlotPos plotPosition)
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

void wxPLPlotCtrl::DeleteAllPlots()
{
	for ( std::vector<plot_data>::iterator it = m_plots.begin();
		it != m_plots.end();
		++it )
		delete it->plot;

	m_plots.clear();	
	m_legendInvalidated = true;

	UpdateAxes( true );
}

size_t wxPLPlotCtrl::GetPlotCount()
{
	return m_plots.size();
}

wxPLPlottable *wxPLPlotCtrl::GetPlot( size_t i )
{
	if ( i >= m_plots.size() ) return 0;
	
	return m_plots[i].plot;
}

wxPLPlottable *wxPLPlotCtrl::GetPlotByLabel( const wxString &series )
{
	for( size_t i=0;i<m_plots.size();i++ )
		if ( m_plots[i].plot->GetLabel() == series )
			return m_plots[i].plot;

	return 0;
}

bool wxPLPlotCtrl::GetPlotPosition( wxPLPlottable *p, 
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

wxPLAxis *wxPLPlotCtrl::GetAxis( AxisPos axispos, PlotPos ppos )
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

void wxPLPlotCtrl::SetAxis( wxPLAxis *a, AxisPos axispos, PlotPos ppos )
{
	switch( axispos )
	{
	case X_BOTTOM: SetXAxis1( a ); break;
	case X_TOP: SetXAxis2( a ); break;
	case Y_LEFT: SetYAxis1( a, ppos ); break;
	case Y_RIGHT: SetYAxis2( a, ppos ); break;
	}
}

void wxPLPlotCtrl::SetTitle( const wxString &title )
{
	if ( title != m_title && m_titleLayout != 0 )
	{
		delete m_titleLayout;
		m_titleLayout = 0;
	}
		
	m_title = title;
}

void wxPLPlotCtrl::SetLegendLocation( LegendPos pos, double xpercent, double ypercent )
{
	if ( xpercent < -10 ) xpercent = -10;
	if ( xpercent > 90 ) xpercent = 90;
	if ( ypercent < -10 ) ypercent = -10;
	if ( ypercent > 90 ) ypercent = 90;

	m_legendPosPercent.x = xpercent;
	m_legendPosPercent.y = ypercent;
	m_legendPos = pos;
}

bool wxPLPlotCtrl::SetLegendLocation( const wxString &spos )
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

void wxPLPlotCtrl::GetHighlightBounds( double *left, double *right )
{
	*left = m_highlightLeftPercent;
	*right = m_highlightRightPercent;
}

void wxPLPlotCtrl::SetSideWidget( wxPLSideWidgetBase *sw, AxisPos pos )
{
	if ( m_sideWidgets[pos] != 0 
		&& m_sideWidgets[pos] != sw )
		delete m_sideWidgets[pos];

	m_sideWidgets[pos] = sw;
}

wxPLSideWidgetBase *wxPLPlotCtrl::GetSideWidget( AxisPos pos )
{
	return m_sideWidgets[pos];
}

wxPLSideWidgetBase *wxPLPlotCtrl::ReleaseSideWidget( AxisPos pos )
{
	wxPLSideWidgetBase *w = m_sideWidgets[pos];
	m_sideWidgets[pos] = 0;
	return w;
}

void wxPLPlotCtrl::WriteDataAsText( wxUniChar sep, wxOutputStream &os, bool visible_only, bool include_x )
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
			if(xDataLabel == "") { xDataLabel = m_plots[i].plot->GetYDataLabel(); }
		}
		else if(histPlot = dynamic_cast<wxPLHistogramPlot*>( m_plots[i - 1].plot ))
		{
			includeXForPlot[i] = true;
		}
		else
		{
			includeXForPlot[i] = (m_plots[i].plot->GetXDataLabel() != m_plots[i-1].plot->GetXDataLabel());
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

		Headers = plot->GetExportableDatasetHeaders(sep);
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

void wxPLPlotCtrl::OnPopupMenu( wxCommandEvent &evt )
{
	int menuid = evt.GetId();
	switch(menuid)
	{
	case ID_COPY_DATA_CLIP:
		if (wxTheClipboard->Open())
		{
			wxString text;
			wxStringOutputStream sstrm( &text );
			WriteDataAsText( '\t', sstrm );
			wxTheClipboard->SetData(new wxTextDataObject(text));
			wxTheClipboard->Close();
		}
		break;
#ifdef __WXMSW__
	case ID_SEND_EXCEL:
		{
			wxExcelAutomation xl;
			if (!xl.StartExcel())
			{
				wxMessageBox("Could not start Excel.");
				return;
			}

			xl.Show( true );

			if (!xl.NewWorkbook())
			{
				wxMessageBox("Could not create a new Excel worksheet.");
				return;
			}
			
			wxString text;
			wxStringOutputStream sstrm( &text );
			WriteDataAsText( '\t', sstrm );

			xl.PasteNewWorksheet( "Plot Data", text );
			xl.AutoFitColumns();
		}
		break;
#endif

	case ID_SAVE_DATA_CSV:
		{
			wxFileDialog fdlg(this, "Save Graph Data", "", "graphdata", "CSV Data Files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (fdlg.ShowModal() == wxID_OK)
			{
				wxString fn = fdlg.GetPath();
				if (fn != "")
				{
					//Make sure we have an extension
					wxString ext;
					wxFileName::SplitPath(fn, NULL, NULL, NULL, &ext);
					if (ext.Lower() != "csv")
						fn += ".csv";

					wxFFileOutputStream out( fn );
					if ( out.IsOk() )
						WriteDataAsText( ',', out );
					else
						wxMessageBox("Could not write to file: \n\n" + fn, "Save Error", wxICON_ERROR);
				}
			}
		}
		break;
	case ID_EXPORT_PDF:
		{
			wxFileDialog fdlg(this, "Export as PDF", wxEmptyString, "graph",
				"PDF Document (*.pdf)|*.pdf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
			if ( fdlg.ShowModal() == wxID_OK )
				if( !ExportPdf( fdlg.GetPath() ) )
					wxMessageBox("PDF encountered an error: \n" + fdlg.GetPath());
		}
		break;
	case ID_EXPORT_SVG:
		{
			wxFileDialog fdlg(this, "Export as SVG", wxEmptyString, "graph",
				"SVG File (*.svg)|*.svg", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
			if ( fdlg.ShowModal() == wxID_OK )
				if ( !ExportSvg( fdlg.GetPath() ) )
					wxMessageBox("Failed to write SVG: " + fdlg.GetPath());
		}
		break;
	case ID_TO_CLIP_SCREEN:
	case ID_TO_CLIP_SMALL:
	case ID_TO_CLIP_NORMAL:
	case ID_EXPORT_SCREEN:
	case ID_EXPORT_SMALL:
	case ID_EXPORT_NORMAL:
		{
			wxSize imgSize = this->GetClientSize();

			if (menuid == ID_TO_CLIP_SMALL || menuid == ID_EXPORT_SMALL)
				imgSize.Set(400, 300);
			else if (menuid == ID_TO_CLIP_NORMAL || menuid == ID_EXPORT_NORMAL)
				imgSize.Set(800, 600);

			wxBitmap img = GetBitmap( imgSize.x, imgSize.y );

			if (menuid == ID_EXPORT_SCREEN ||
				menuid == ID_EXPORT_SMALL ||
				menuid == ID_EXPORT_NORMAL)
			{
				wxString filename;
				wxBitmapType bitmap_type;
				if ( ShowExportDialog(filename, bitmap_type) )
				{
					if (!img.SaveFile(filename, (wxBitmapType)bitmap_type))
						wxMessageBox("Error writing image file to:\n\n" + filename, "Export error", wxICON_ERROR|wxOK);
				}
			}
			else if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxBitmapDataObject(img));
				wxTheClipboard->Close();
			}
		}
		break;
	}
}

bool wxPLPlotCtrl::ShowExportDialog( wxString &exp_file_name, wxBitmapType &exp_bitmap_type )
{
	wxString fn;	
	wxFileDialog fdlg(this, "Export as image file", wxEmptyString, "plot",
		"BMP Image (*.bmp)|*.bmp|JPEG Image (*.jpg)|*.jpg|PNG Image (*.png)|*.png", 
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	
	if (fdlg.ShowModal() == wxID_OK)
	{
		fn = fdlg.GetPath();
		if ( !fn.IsEmpty() )
		{
			// ensure the extension is attached
			wxString ext;
			wxFileName::SplitPath( fn, NULL, NULL, NULL, &ext);

			int filt = fdlg.GetFilterIndex();
			wxLogStatus("FILTER IDX=%d\n", filt);
			if (filt == 0) // BMP
			{
				exp_bitmap_type = wxBITMAP_TYPE_BMP;
				if (ext.Lower() != "bmp")
					fn += ".bmp";
			}
			else if (filt == 1) // JPG
			{
				exp_bitmap_type = wxBITMAP_TYPE_JPEG;
				if (ext.Lower() != "jpg")
					fn += ".jpg";
			}
			else if (filt == 2) // PNG
			{
				exp_bitmap_type = wxBITMAP_TYPE_PNG;
				if (ext.Lower() != "png")
					fn += ".png";
			}

			exp_file_name = fn;

			return true;
		}
	}

	return false;
}

wxBitmap wxPLPlotCtrl::GetBitmap( int width, int height )
{
	bool legend_shown = m_showLegend;
	LegendPos legend_pos = m_legendPos;
	wxSize imgSize( GetClientSize() );
	
	bool invalidated = false;
	if ( (width > 10 && width < 10000 && height > 10 && height < 10000)
		|| (m_includeLegendOnExport && !m_showLegend ))
	{
		if ( m_includeLegendOnExport && !m_showLegend )
		{
			m_showLegend = true;
			m_legendPos = RIGHT;

			// estimate legend size and add it to exported bitmap size
			wxBitmap bittemp( 10, 10 );
			wxMemoryDC dctemp( bittemp );
			dctemp.SetFont( GetFont() );
			CalcLegendTextLayout( dctemp );
			m_legendInvalidated = true; // keep legend invalidated for subsequent render

			width += m_legendRect.width;
		}

		Invalidate(); // force recalc of layout
		invalidated = true;
		imgSize.Set(width, height);
	}

	wxBitmap bitmap( imgSize.GetWidth(), imgSize.GetHeight(), 32 );
	wxMemoryDC memdc( bitmap );
	wxGCDC gdc( memdc );

	// initialize font and background
	gdc.SetFont( GetFont() );
	gdc.SetBackground( *wxWHITE_BRUSH );
	gdc.Clear();
	
	wxRect rect(0, 0, imgSize.GetWidth(), imgSize.GetHeight());
	Render( gdc, rect );

	memdc.SelectObject( wxNullBitmap );

	if ( invalidated )
	{
		m_showLegend = legend_shown;
		m_legendPos = legend_pos;
		Invalidate(); // invalidate layout cache again for next time it is draw on screen
		Refresh(); // issue redraw to on-screen to recalculate layout right away.
	}

	return bitmap;
}

bool wxPLPlotCtrl::Export( const wxString &file, wxBitmapType type, int width, int height )
{
	return GetBitmap(width,height).SaveFile(file, type);
}

bool wxPLPlotCtrl::ExportPdf( const wxString &file )
{
	wxPrintData prnDat;
	prnDat.SetFilename( file );
	prnDat.SetOrientation( wxLANDSCAPE );
	prnDat.SetPaperId( wxPAPER_A5 );
	wxPdfDC  dc( prnDat );			
	if ( !dc.StartDoc(_("Exporting to pdf...")) )
		return false;

	dc.StartPage();
	dc.SetFont( *wxNORMAL_FONT );
	dc.SetBackground( *wxWHITE_BRUSH );
	dc.Clear();
					
	wxSize size = dc.GetSize();
	const double frac = 0.01;
	wxPoint margin( (int)(frac*((double)size.x)), (int)(frac*((double)size.y)) );
	wxRect geom( margin, wxSize( size.x - 2*margin.x, size.y - 2*margin.y ) );

	bool save = m_scaleTextSize ;
	m_scaleTextSize = false;
	Invalidate();
	Render( dc, geom );
	m_scaleTextSize = save;
	Invalidate();
	Refresh();

	dc.EndPage();
	dc.EndDoc();
	
	return true;
}

bool wxPLPlotCtrl::ExportSvg( const wxString &file )
{
	wxSize imgSize = GetClientSize();			
	wxSVGFileDC svgdc( file, imgSize.GetWidth(), imgSize.GetHeight() );
	if ( !svgdc.IsOk() ) return false;
				
	// initialize font and background
	svgdc.SetFont( GetFont() );
	svgdc.SetBackground( *wxWHITE_BRUSH );
	svgdc.Clear();
	wxRect rect(0, 0, imgSize.GetWidth(), imgSize.GetHeight());
	Invalidate();
	Render( svgdc, rect );
	Invalidate();
	Refresh(); // redraw on-screen version to recompute layout
	return true;
}

class gcdc_ref 
{
public:
	gcdc_ref() { m_ptr = 0; }
	~gcdc_ref() { if (m_ptr != 0) delete m_ptr; }
	wxGCDC *m_ptr;
};

void wxPLPlotCtrl::Render( wxDC &dc, wxRect geom )
{
	gcdc_ref gdc;
	if ( wxMemoryDC *memdc = dynamic_cast<wxMemoryDC*>( &dc ) )
		gdc.m_ptr = new wxGCDC( *memdc );
	else if ( wxWindowDC *windc = dynamic_cast<wxWindowDC*>( &dc ) )
		gdc.m_ptr = new wxGCDC( *windc );
	else if ( wxPrinterDC *prindc = dynamic_cast<wxPrinterDC*>( &dc ) )
		gdc.m_ptr = new wxGCDC( *prindc );
	
	wxDC &aadc = gdc.m_ptr != 0 ? *gdc.m_ptr : dc;
	
	// ensure plots have the axes they need to be rendered
	for ( size_t i = 0; i< m_plots.size(); i++ )
	{
		if ( GetAxis( m_plots[i].xap ) == 0 )
			SetAxis( m_plots[i].plot->SuggestXAxis(), m_plots[i].xap );

		if ( GetAxis( m_plots[i].yap, m_plots[i].ppos ) == 0 )
			SetAxis( m_plots[i].plot->SuggestYAxis(), m_plots[i].yap, m_plots[i].ppos );
	}

	// Configure fonts
	wxFont font_normal( dc.GetFont() );
	if ( m_scaleTextSize )
	{
		// scale text according to geometry width, within limits
		double point_size = geom.width/1000.0 * 12.0;
		if ( point_size > 23 ) point_size = 23;
		if ( point_size < 7 ) point_size = 7;
		font_normal.SetPointSize( (int)point_size );
	}

	wxFont font_bold( font_normal );
	font_bold.SetWeight( wxFONTWEIGHT_BOLD );
	//font_bold.SetPointSize( font_bold.GetPointSize()+1 );

	wxFont font_axis( font_normal );
	font_axis.SetPointSize( font_axis.GetPointSize()-2 );


	// draw any side widgets first and remove the space from the total plot area
	if ( m_sideWidgets[Y_LEFT] != 0 )
	{
		wxSize sz = m_sideWidgets[Y_LEFT]->GetBestSize();
		m_sideWidgets[Y_LEFT]->Render( aadc, 
			wxRect( geom.x, geom.y, 
				sz.x, geom.height ) );
		geom.width -= sz.x;
		geom.x += sz.x;
	}

	if ( m_sideWidgets[Y_RIGHT] != 0 )
	{
		wxSize sz = m_sideWidgets[Y_RIGHT]->GetBestSize();		
		m_sideWidgets[Y_RIGHT]->Render( aadc, 
			wxRect( geom.x+geom.width-sz.x, geom.y, 
				sz.x, geom.height ) );

		geom.width -= sz.x;
	}


	wxRect box( geom.x+text_space, 
		geom.y+text_space, 
		geom.width-text_space-text_space, 
		geom.height-text_space-text_space );

	bool legend_bottom = false;
	bool legend_right = false;
	if ( m_showLegend )
	{
		dc.SetFont( font_normal );
		CalcLegendTextLayout( dc );

		if ( m_legendPos == BOTTOM )
		{
			wxCoord height = 0;
			for ( size_t i=0;i<m_legendItems.size();i++ )
				if (m_legendItems[i]->layout->height() > height )
					height = m_legendItems[i]->layout->height();

			if ( height > 0 ) 
				legend_bottom = true;

			box.height -= height + text_space;
		}

		if ( m_legendPos == RIGHT )
		{
			wxCoord width = 0;
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
		dc.SetFont( font_bold );
		if ( m_titleLayout == 0 )
			m_titleLayout = new text_layout( dc, m_title, text_layout::CENTER );

		m_titleLayout->render( dc, box.x+box.width/2-m_titleLayout->width()/2, box.y, 0, false );
		box.y += m_titleLayout->height() + text_space;
		box.height -= m_titleLayout->height() + text_space;
	}

	dc.SetFont( font_normal );

	wxRect plotbox = box; // save current box for where to draw the axis labels

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

	wxCoord yleft_max_label_width = 0, yright_max_label_width = 0;
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
	// invalidated on a resize event, or when an axis is changed
	dc.SetFont( font_axis );
	
	if ( m_x2.axis != 0 )
	{
		if ( m_x2.layout == 0 )
			m_x2.layout = new axis_layout( X_TOP, dc, m_x2.axis, box.x, box.x+box.width );

		if ( m_x2.layout->bounds().x > box.width )
		{   // this adjusts for really wide tick text at the ends of the axis
			wxCoord diff = m_x2.layout->bounds().x - box.width;
			wxCoord adj = 2*diff/3; // actual adjustment needed is diff/2, but leave a little extra space

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
	bool cartesion_plot = true;
	if (wxPLPolarAngularAxis *pa = dynamic_cast<wxPLPolarAngularAxis *>(m_x1.axis))
		cartesion_plot = false;


	if ( m_x1.axis != 0 )
	{
		if (cartesion_plot) {
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
	
	wxCoord yleft_max_axis_width = 0, yright_max_axis_width = 0;
	for ( size_t pp=0;pp<NPLOTPOS;pp++)
	{
		if ( m_y1[pp].axis != 0 )
		{
			if (m_y1[pp].layout == 0) {
				if (cartesion_plot)
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
	const wxCoord plot_space = 16;
	wxCoord single_plot_height = box.height/nyaxes - (nyaxes-1)*(plot_space/2);
	if ( single_plot_height < 40 ) return;

	// compute box dimensions for each plot/subplot
	// and fill plot area with background color
	dc.SetBrush( wxBrush( m_plotAreaColour ) );
	dc.SetPen( *wxTRANSPARENT_PEN );

	wxCoord cur_plot_y_start = box.y;
	m_plotRects.clear();
	for ( size_t pp=0;pp<nyaxes; pp++ )
	{
		wxRect rect( box.x, cur_plot_y_start, box.width, single_plot_height );
		if (cartesion_plot) 
			dc.DrawRectangle(rect);
		else {
			wxCoord radius = (box.width < box.height) ? box.width / 2.0 : box.height / 2.0;
			wxPoint cntr = wxPoint(box.x + box.width / 2.0, box.y + box.height / 2.0);
			dc.DrawCircle(cntr, radius);
		}
		m_plotRects.push_back(rect);
		cur_plot_y_start += single_plot_height + plot_space;
	}

	// render grid lines
	if ( m_showCoarseGrid )
	{
		dc.SetPen( wxPen( m_gridColour, 1, wxSOLID ) );
		if (cartesion_plot)
			DrawGrid( dc, wxPLAxis::TickData::LARGE );
		else
			DrawPolarGrid(dc, wxPLAxis::TickData::LARGE);
	}

	if ( m_showFineGrid )
	{
		wxPen pen( m_gridColour, 1, wxDOT );
		pen.SetCap( wxCAP_BUTT );
		pen.SetJoin( wxJOIN_MITER );
		dc.SetPen( pen );
		if (cartesion_plot)
			DrawGrid( dc, wxPLAxis::TickData::SMALL );
		else
			DrawPolarGrid(dc, wxPLAxis::TickData::SMALL);
	}

	// render plots
	for ( size_t i = 0; i< m_plots.size(); i++ )
	{
		wxPLAxis *xaxis = GetAxis( m_plots[i].xap );
		wxPLAxis *yaxis = GetAxis( m_plots[i].yap, m_plots[i].ppos );
		if ( xaxis == 0 || yaxis == 0 ) continue; // this should never be encountered
		wxRect &bb = m_plotRects[ m_plots[i].ppos ];
		wxPLAxisDeviceMapping map( xaxis, bb.x, bb.x+bb.width,
			yaxis, bb.y+bb.height, bb.y );

		if ( m_plots[i].plot->GetAntiAliasing() )
		{
			aadc.SetClippingRegion( bb );
			m_plots[i].plot->Draw( aadc, map );
			aadc.DestroyClippingRegion();
		}
		else
		{
			dc.SetClippingRegion( bb );
			m_plots[i].plot->Draw( dc, map );
			dc.DestroyClippingRegion();

		}
	}

	// draw some axes
	dc.SetPen( m_axisColour );
	dc.SetTextForeground(  m_tickTextColour );
	
	if ( m_x2.axis )
		m_x2.layout->render( dc, m_plotRects[0].y, m_x2.axis, 
			box.x, box.x+box.width, 
			m_x1.axis == 0 ? m_plotRects[nyaxes-1].y+m_plotRects[nyaxes-1].height : -1 );
	
	// set up some polar plot values
	wxRect rect1 = m_plotRects[0];
	wxCoord pp_radius = (rect1.width < rect1.height) ? rect1.width / 2.0 : rect1.height / 2.0;
	wxPoint pp_center = wxPoint(rect1.x + rect1.width / 2.0, rect1.y + rect1.height / 2.0);

	// render y axes
	for ( size_t pp=0;pp<nyaxes; pp++ )
	{
		if (m_y1[pp].axis != 0) {
			if (cartesion_plot)
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
				m_y1[pp].axis == 0 ? box.x : -1 );
	}

	// render x1 axis (or angular axis on polar plots)
	if (m_x1.axis) {
		if (cartesion_plot)
			m_x1.layout->render(dc, m_plotRects[nyaxes - 1].y + m_plotRects[nyaxes - 1].height, m_x1.axis,
				box.x, box.x + box.width,
				m_x2.axis == 0 ? m_plotRects[0].y : -1);
		else
			m_x1.layout->render(dc, pp_radius, m_x1.axis, pp_center.x, pp_center.y, -1);
	}

	// draw boundaries around plots
	if (cartesion_plot) {
		for (size_t pp = 0; pp < nyaxes; pp++)
		{
			dc.DrawLine(m_plotRects[pp].x, m_plotRects[pp].y, m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y);
			dc.DrawLine(m_plotRects[pp].x, m_plotRects[pp].y, m_plotRects[pp].x, m_plotRects[pp].y + m_plotRects[pp].height);
			dc.DrawLine(m_plotRects[pp].x, m_plotRects[pp].y + m_plotRects[pp].height, m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y + m_plotRects[pp].height);
			dc.DrawLine(m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y, m_plotRects[pp].x + m_plotRects[pp].width, m_plotRects[pp].y + m_plotRects[pp].height);
		}
	}
	else {
		dc.DrawLine(pp_center.x-pp_radius,pp_center.y,pp_center.x+pp_radius,pp_center.y);
		dc.DrawLine(pp_center.x, pp_center.y + pp_radius, pp_center.x, pp_center.y - pp_radius);

		dc.SetBrush(*wxTRANSPARENT_BRUSH); // otherwise, circle is filled in
		dc.DrawCircle(pp_center, pp_radius);
	}

	// draw axis labels
	dc.SetTextForeground( *wxBLACK );
	dc.SetFont( font_normal );

	if (m_x1.axis && m_x1.axis->IsLabelVisible() && !m_x1.axis->GetLabel().IsEmpty()) {
		if (cartesion_plot)
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
			if (cartesion_plot) {
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

	/*
	dc.SetPen( wxPen( *wxLIGHT_GREY, 1 ) );
	dc.SetBrush( *wxTRANSPARENT_BRUSH );
	dc.DrawRectangle( box );
	*/

	dc.SetFont( font_normal );
	wxRect plotarea( m_plotRects[0].x, m_plotRects[0].y, m_plotRects[0].width, m_plotRects[0].height*nyaxes + (nyaxes-1)*plot_space );
	DrawLegend( dc, aadc, (m_legendPos==FLOATING||legend_bottom||legend_right) ? geom : plotarea  );
}

void wxPLPlotCtrl::DrawGrid( wxDC &dc, wxPLAxis::TickData::TickSize size )
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
			wxCoord xpos = xgrid_axis->axis->WorldToPhysical( ticks[i], m_plotRects[0].x, m_plotRects[0].x+m_plotRects[0].width );

			for ( size_t pp = 0; pp < m_plotRects.size(); pp++ )
				dc.DrawLine( xpos, m_plotRects[pp].y, xpos, m_plotRects[pp].y + m_plotRects[pp].height );
				
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
				wxCoord ypos = ygrid_axis->axis->WorldToPhysical( ticks[j], m_plotRects[pp].y+m_plotRects[pp].height, m_plotRects[pp].y );
				dc.DrawLine( m_plotRects[0].x, ypos, m_plotRects[0].x + m_plotRects[0].width, ypos );
			}
		}
	}
}

void wxPLPlotCtrl::DrawPolarGrid(wxDC &dc, wxPLAxis::TickData::TickSize size)
{
	if (m_plotRects.size() < 1) return;
	dc.SetBrush(*wxTRANSPARENT_BRUSH); // otherwise, grid circles are filled in
	wxPoint cntr = wxPoint(m_plotRects[0].x + m_plotRects[0].width / 2.0, m_plotRects[0].y + m_plotRects[0].height / 2.0);
	wxCoord max_radius = (m_plotRects[0].width < m_plotRects[0].height) ? m_plotRects[0].width / 2.0 : m_plotRects[0].height / 2.0;

	// circles from center
	axis_data *radial_grid = 0;
	if (m_y1[0].axis != 0) radial_grid = &m_y1[0];

	if (radial_grid != 0)
	{
		std::vector<double> ticks = radial_grid->layout->ticks(size);
		for (size_t j = 0; j<ticks.size(); j++)
		{
			wxCoord radius = radial_grid->axis->WorldToPhysical(ticks[j], 0, max_radius);
			dc.DrawCircle(cntr, radius);
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
				wxPoint pt = wxPoint(max_radius*cos(angle), max_radius*sin(angle));
				dc.DrawLine(cntr, cntr + pt);
			}
		}
	}

}



wxPLPlotCtrl::legend_item::legend_item( wxDC &dc, wxPLPlottable *p )
{
	plot = p;
	text = p->GetLabel();
	layout = new text_layout( dc, text );
}

wxPLPlotCtrl::legend_item::~legend_item()
{
	if ( layout ) delete layout;
}

void wxPLPlotCtrl::CalcLegendTextLayout( wxDC &dc )
{
	if ( !m_showLegend ) return;

	if ( m_legendInvalidated || m_scaleTextSize )
	{
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
				wxCoord height = m_legendItems[i]->layout->height();
				if ( height > m_legendRect.height )
					m_legendRect.height = height;

				wxCoord width = m_legendItems[i]->layout->width();
				m_legendRect.width += width + 5*text_space + legend_item_box.x;
			}
		}
		else
		{
			m_legendRect.width = 0;	
			m_legendRect.height = text_space;
			for ( size_t i=0; i<m_legendItems.size(); i++ )
			{
				wxCoord width = m_legendItems[i]->layout->width();
				if ( width > m_legendRect.width )
					m_legendRect.width = width;

				wxCoord height = m_legendItems[i]->layout->height();
				if ( height < legend_item_box.y )
					height = legend_item_box.y;
				m_legendRect.height += height + text_space;
			}
		}
		
		m_legendRect.width += legend_item_box.x + 5*text_space;
		m_legendInvalidated = false;
	}
}

void wxPLPlotCtrl::DrawLegend( wxDC &dc, wxDC &aadc, const wxRect& geom )
{
	if ( !m_showLegend )
		return;

	int layout = wxVERTICAL;
	wxCoord max_item_height = 0;
	wxCoord max_item_width = 0;

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
	
	if ( m_legendPos != BOTTOM && m_legendPos != RIGHT )
	{
		dc.SetBrush( *wxWHITE_BRUSH );
		dc.SetPen( *wxLIGHT_GREY_PEN );
	}
	else
	{
		dc.SetBrush( dc.GetBackground() );
		dc.SetPen(*wxTRANSPARENT_PEN);
	}
	dc.DrawRectangle( m_legendRect );
	
	wxCoord x = m_legendRect.x;
	wxCoord y = m_legendRect.y + text_space;
	for ( size_t i = 0; i < m_legendItems.size(); i++ )
	{
		legend_item &li = *m_legendItems[ m_reverseLegend ? m_legendItems.size() - i - 1 : i ];
		
		wxCoord yoff_text = 0;
		if ( layout == wxHORIZONTAL )
			yoff_text = (max_item_height - li.layout->height())/2;

		li.layout->render( dc, x + legend_item_box.x + 2*text_space + text_space, y + yoff_text );

		wxCoord yoff_box = li.layout->height()/2 - legend_item_box.y/2;
		if ( layout == wxHORIZONTAL )
			yoff_box = max_item_height/2 - legend_item_box.y/2;

		li.plot->DrawInLegend( aadc, wxRect( x + 2*text_space, y + yoff_box , 
			legend_item_box.x, legend_item_box.y ) );

		if ( layout == wxHORIZONTAL )
			x += 5*text_space + legend_item_box.x + li.layout->width();
		else
			y += li.layout->height() + text_space;
	}
	
	//dc.SetPen( *wxBLACK_PEN );
	//dc.SetBrush( *wxTRANSPARENT_BRUSH );
	//dc.DrawRectangle( m_legendRect );
}

void wxPLPlotCtrl::OnPaint( wxPaintEvent & )
{
	wxAutoBufferedPaintDC pdc( this );
	
	pdc.SetFont( GetFont() ); // initialze font and background
	pdc.SetBackground( wxBrush( GetBackgroundColour(), wxBRUSHSTYLE_SOLID ) );
	pdc.Clear();

	int width, height;
	GetClientSize( &width, &height );
	Render( pdc, wxRect(0, 0, width, height) );
}

void wxPLPlotCtrl::Invalidate()
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

void wxPLPlotCtrl::OnSize( wxSizeEvent & )
{
	Invalidate();
	Refresh();
}

void wxPLPlotCtrl::DrawLegendOutline()
{
	wxClientDC dc(this);
#ifdef PL_USE_OVERLAY
	wxDCOverlay overlaydc( m_overlay, &dc );
	overlaydc.Clear();
	dc.SetPen( wxColour( 100, 100, 100 ) );
	dc.SetBrush( wxColour( 150, 150, 150, 150 ) );
#else
	dc.SetLogicalFunction( wxINVERT );
	dc.SetPen( wxPen( *wxBLACK, 2 ) );
	dc.SetBrush( *wxTRANSPARENT_BRUSH );
#endif

	wxPoint diff = ClientToScreen(m_currentPoint) - ClientToScreen(m_anchorPoint);
    dc.DrawRectangle( wxRect( m_legendRect.x + diff.x, m_legendRect.y + diff.y, 
		m_legendRect.width, m_legendRect.height) );

	wxSize client = GetClientSize();
	if ( m_currentPoint.x > client.x - 10 )
	{
		dc.SetBrush( *wxBLACK_BRUSH );
		dc.DrawRectangle( client.x - 10, 0, 10, client.y );
	}
	else if ( m_currentPoint.y > client.y - 10 )
	{
		dc.SetBrush( *wxBLACK_BRUSH );
		dc.DrawRectangle( 0, client.y - 10, client.x, 10 );
	}
}

void wxPLPlotCtrl::UpdateHighlightRegion()
{
	wxClientDC dc(this);

#ifdef PL_USE_OVERLAY
	wxDCOverlay overlaydc( m_overlay, &dc );
	overlaydc.Clear();
	dc.SetPen( wxColour( 100, 100, 100 ) );
	dc.SetBrush( wxColour( 150, 150, 150, 150 ) );
#else
	dc.SetLogicalFunction( wxINVERT );
	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.SetBrush( *wxBLACK_BRUSH );
#endif


	wxCoord highlight_x = m_currentPoint.x < m_anchorPoint.x ? m_currentPoint.x : m_anchorPoint.x;
	wxCoord highlight_width = abs( m_currentPoint.x - m_anchorPoint.x );

	const wxCoord arrow_size = 10;
	
	for ( std::vector<wxRect>::const_iterator it = m_plotRects.begin();
		it != m_plotRects.end();
		++it )
	{
		if ( highlight_x < it->x )
		{
			highlight_width -= it->x - highlight_x;
			highlight_x = it->x;
		}
		else
		{
			if ( highlight_x + highlight_width > it->x + it->width )
				highlight_width -= highlight_x + highlight_width - it->x - it->width;
		}
		
		dc.DrawRectangle( wxRect( highlight_x, it->y, highlight_width, it->height) );
/*
		dc.DrawLine( highlight_x, it->y, highlight_x, it->y+it->height );
		dc.DrawLine( highlight_x+highlight_width, it->y, highlight_x+highlight_width, it->y+it->height );
		dc.DrawLine( highlight_x, it->y+it->height/2, highlight_x+highlight_width, it->y+it->height/2 );
		wxPoint al[3];
		al[0] = wxPoint( highlight_x, it->y+it->height/2 );
		al[1] = wxPoint( highlight_x+arrow_size, it->y+it->height/2-arrow_size/2 );
		al[2] = wxPoint( highlight_x+arrow_size, it->y+it->height/2+arrow_size/2 );
		dc.DrawPolygon( 3, al );
		
		wxPoint ar[3];
		ar[0] = wxPoint( highlight_x+highlight_width, it->y+it->height/2 );
		ar[1] = wxPoint( highlight_x+highlight_width-arrow_size, it->y+it->height/2-arrow_size/2 );
		ar[2] = wxPoint( highlight_x+highlight_width-arrow_size, it->y+it->height/2+arrow_size/2 );
		dc.DrawPolygon( 3, ar );
*/
	}

	m_highlightLeftPercent = 100.0*( ((double)(highlight_x - m_plotRects[0].x)) / ( (double)m_plotRects[0].width ) );
	m_highlightRightPercent = 100.0*( ((double)(highlight_x + highlight_width - m_plotRects[0].x )) / ((double)m_plotRects[0].width ) );

}

void wxPLPlotCtrl::OnLeftDown( wxMouseEvent &evt )
{
	if ( m_showLegend 
		&& m_legendRect.Contains( evt.GetPosition() ) )
	{
		m_moveLegendMode = true;
		m_moveLegendErase = false;
		m_anchorPoint = evt.GetPosition();		
		CaptureMouse();
	}
	else if ( m_allowHighlighting )
	{
		std::vector<wxRect>::const_iterator it;
		for ( it = m_plotRects.begin();
			it != m_plotRects.end();
			++it )
			if ( it->Contains( evt.GetPosition() ) )
				break;

		if ( it != m_plotRects.end() )
		{
			m_highlightMode = true;
			m_highlightErase = false;
			m_anchorPoint = evt.GetPosition();
			CaptureMouse();
		}
	}
}

void wxPLPlotCtrl::OnLeftUp( wxMouseEvent &evt )
{
	if ( HasCapture() )
		ReleaseMouse();


	if ( m_moveLegendMode )
	{
		m_moveLegendMode = false;
        
#ifdef PL_USE_OVERLAY		
		wxClientDC dc( this );
		wxDCOverlay overlaydc( m_overlay, &dc );
		overlaydc.Clear();      
        m_overlay.Reset();
#else
		if ( m_moveLegendErase )
			DrawLegendOutline();
#endif
		
		wxSize client = GetClientSize();
		wxPoint point = evt.GetPosition();
		wxPoint diff = ClientToScreen(point) - ClientToScreen(m_anchorPoint);
		m_legendPosPercent.x = 100.0*((double)(m_legendRect.x+diff.x) / (double)client.x);
		m_legendPosPercent.y = 100.0*((double)(m_legendRect.y+diff.y) / (double)client.y);

		LegendPos old = m_legendPos;
		
		// undock legend if it's currently docked
		if ( m_legendPos == RIGHT && point.x < client.x - 10 )
			m_legendPos = FLOATING;
		else if ( m_legendPos == BOTTOM && point.y < client.y - 10 )
			m_legendPos = FLOATING;

		// redock legend if applicable
		if ( m_legendPos == FLOATING )
		{
			if ( point.x  > client.x - 10 )
				m_legendPos = RIGHT;
			else if ( point.y > client.y - 10 )
				m_legendPos = BOTTOM;
		}

		if ( old != m_legendPos )
		{
			m_legendInvalidated = true; // also invalidate legend text layouts to recalculate shape
			Invalidate(); // recalculate all plot positions if legend snap changed.
		}

		Refresh(); // redraw with the legend in the new spot
		
		// issue event regarding the move of the legend
		wxCommandEvent e( wxEVT_PLOT_LEGEND, GetId() );
		e.SetEventObject( this );
		GetEventHandler()->ProcessEvent( e );
	}
	else if ( m_allowHighlighting && m_highlightMode )
	{
#ifdef PL_USE_OVERLAY		
		wxClientDC dc( this );
		wxDCOverlay overlaydc( m_overlay, &dc );
		overlaydc.Clear();      
        m_overlay.Reset();
#else
		if ( m_highlightErase )
			UpdateHighlightRegion();
#endif

		m_highlightMode = false;

		wxCoord diff = abs( ClientToScreen(evt.GetPosition()).x - ClientToScreen(m_anchorPoint).x );
		if ( diff > 10 )
		{
			wxCommandEvent e( wxEVT_PLOT_HIGHLIGHT, GetId() );
			e.SetEventObject( this );
			GetEventHandler()->ProcessEvent( e );
		}
	}
}

void wxPLPlotCtrl::OnRightDown( wxMouseEvent & )
{
	PopupMenu( &m_contextMenu );
}

void wxPLPlotCtrl::OnMotion( wxMouseEvent &evt )
{
	if ( m_moveLegendMode )
	{
#ifndef PL_USE_OVERLAY
		if ( m_moveLegendErase )
			DrawLegendOutline();
#endif

		m_currentPoint = evt.GetPosition();

		DrawLegendOutline();
		m_moveLegendErase = true;
	}
	else if ( m_highlightMode )
	{
#ifndef PL_USE_OVERLAY
		if ( m_highlightErase )
			UpdateHighlightRegion();
#endif

		m_currentPoint = evt.GetPosition();

		UpdateHighlightRegion();
		m_highlightErase = true;
	}

	//TODO:  see if we can get the below functionality to display point coordinates in a tool tip working correctly.
	//The code properly retrieves the X and Y values but the tooltip does not display.
	/*
	wxRealPoint rpt;
	wxPLPlottable *ds;
	wxPoint MousePos;
	size_t radius = 10;	//radius around a point that will trigger showing tooltip
	wxString tipText = "";
	wxTipWindow *tipwindow = NULL;
	wxPLAxis *xaxis;
	wxPLAxis *yaxis;
	double min = 0;
	double max = 0;
	wxPoint DataPos;
	int Xvar = 0;
	int Yvar = 0;

	if (tipwindow != NULL)
	{
		tipwindow->Destroy();
		tipwindow = NULL;
	}

	if (!m_x1.axis || m_plots.size() == 0 || evt.Dragging()) { return; }

	MousePos = evt.GetPosition();

	for (size_t PlotNum = 0; PlotNum < m_plots.size(); PlotNum++)
	{
		ds = m_plots[PlotNum].plot;
		xaxis = GetAxis(m_plots[PlotNum].xap);
		yaxis = GetAxis(m_plots[PlotNum].yap, m_plots[PlotNum].ppos);

		if (xaxis == 0 || yaxis == 0) continue; // this should never be encountered

		min = xaxis->GetWorldMin();
		max = xaxis->GetWorldMax();
		wxRect &plotSurface = m_plotRects[m_plots[PlotNum].ppos];
		wxPLAxisDeviceMapping map(xaxis, plotSurface.x, plotSurface.x + plotSurface.width, yaxis, plotSurface.y + plotSurface.height, plotSurface.y);

		for (size_t i = 0; i < ds->Len(); i++)
		{
			rpt = ds->At(i);

			if (rpt.x >= min && rpt.x <= max)
			{
				DataPos = map.ToDevice(rpt.x, rpt.y);
				Xvar = MousePos.x - DataPos.x;
				Yvar = MousePos.y - DataPos.y;

				if ((Xvar * Xvar) + (Yvar * Yvar) <= (radius * radius))
				{
					tipText = "X: " + (wxString)std::to_string(rpt.x) + "\nY: " + (wxString)std::to_string(rpt.y);
					break;
				}
			}
		}

		if (tipText != "") { break; }
	}

	if (tipText != "")
	{
		tipwindow = new wxTipWindow(this, tipText);	//TODO:  this line causing error that seems to be in wxWidgets itself and I can't track down why:  ..\..\src\msw\window.cpp(576): 'SetFocus' failed with error 0x00000057 (the parameter is incorrect.).
		wxRect &rect = wxRect(DataPos.x - radius, DataPos.y - radius, 2 * radius, 2 * radius);
		tipwindow->SetBoundingRect(rect);
	}
	*/

	evt.Skip();
}

void wxPLPlotCtrl::OnMouseCaptureLost( wxMouseCaptureLostEvent & )
{
	if ( m_moveLegendMode )
	{
		m_moveLegendMode = false;
		Refresh();
	}
}

void wxPLPlotCtrl::DeleteAxes()
{
	SetXAxis1( 0 );
	SetXAxis2( 0 );
	for ( size_t pp = 0; pp<NPLOTPOS; pp++ )
	{
		SetYAxis1( 0, (wxPLPlotCtrl::PlotPos)pp );
		SetYAxis2( 0, (wxPLPlotCtrl::PlotPos)pp );
	}

	Invalidate();
}

void wxPLPlotCtrl::RescaleAxes()
{
	//This does not set axes to null, or change anything other than their bounds.
	bool xAxis1Set = false, xAxis2Set = false;
	std::vector<bool> yAxis1Set(NPLOTPOS, false), yAxis2Set(NPLOTPOS, false);

	for (size_t i = 0; i < m_plots.size(); ++i)
    {
        wxPLPlottable *p = m_plots[i].plot;
        wxPLPlotCtrl::AxisPos xap = m_plots[i].xap;
		wxPLPlotCtrl::AxisPos yap = m_plots[i].yap;
		wxPLPlotCtrl::PlotPos ppos = m_plots[i].ppos;

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

void wxPLPlotCtrl::UpdateAxes( bool recalc_all )
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



BEGIN_EVENT_TABLE( TextLayoutDemo, wxWindow )
	EVT_PAINT( TextLayoutDemo::OnPaint )
	EVT_SIZE( TextLayoutDemo::OnSize )
END_EVENT_TABLE()


TextLayoutDemo::TextLayoutDemo( wxWindow *parent )
	: wxWindow( parent, wxID_ANY )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
}

std::vector<wxCoord> TextLayoutDemo::Draw( wxDC &dc, const wxRect &geom )
{
	std::vector<wxCoord> vlines;
	dc.SetClippingRegion( geom );
	
	dc.SetFont( *wxNORMAL_FONT );
	wxPLPlotCtrl::text_layout t1( dc, "basic text, nothing special" );
	t1.render( dc, geom.x+20, geom.y+20, 0.0, true );

	vlines.push_back( geom.x+20 );
	vlines.push_back( geom.x+20+t1.width() );

	dc.SetFont( wxFont(18, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Century Schoolbook") );
	wxPLPlotCtrl::text_layout t2( dc, "escape^sup_sub \\\\  \\  \\euro \\badcode \\Omega~\\Phi\\ne\\delta, but\n\\Kappa\\approx\\zeta \\emph\\qmark, and this is the end of the text!. Cost was 10 \\pound, or 13.2\\cent" );
	t2.render( dc, geom.x+20, geom.y+120, 0.0, true );

	dc.SetFont( wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Consolas") );
	wxPLPlotCtrl::text_layout t3( dc, "super^2 ,not_\\rho\\gamma f hing_{special,great,best}\n\\alpha^^\\beta c^\\delta  efjhijkl__mnO^25 pq_0 r\\Sigma tuvwxyz\nABCDEFGHIJKL^^MNOPQRSTUVWXZY" );
	t3.render( dc, geom.x+20, geom.y+420, 90, true );
	t3.render( dc, geom.x+200, geom.y+350, 45.0, true );
	t3.render( dc, geom.x+400, geom.y+300, 0.0, true );
	vlines.push_back( geom.x+400 );
	vlines.push_back( geom.x+400+t3.width() );

	dc.SetFont( wxFont(16, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	wxPLPlotCtrl::text_layout t4( dc, "x_1^2_3 abc=y_2^4" );
	t4.render( dc, geom.x+200, geom.y+70, 0, false );

	
	dc.SetFont( wxFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	wxPLPlotCtrl::text_layout t5( dc, "small (7): x_1^2_3 abc=y_2^4" );
	t5.render( dc, geom.x+500, geom.y+60, 0, false );
	
	dc.SetFont( wxFont(8, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	wxPLPlotCtrl::text_layout t6( dc, "small (8): x_1^2_3 abc=y_2^4" );
	t6.render( dc, geom.x+500, geom.y+80, 0, false );
	
	dc.SetFont( wxFont(9, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	wxPLPlotCtrl::text_layout t7( dc, "small (9): x_1^2_3 abc=y_2^4" );
	t7.render( dc, geom.x+500, geom.y+100, 0, false );
	
	dc.SetFont( wxFont(10, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	wxPLPlotCtrl::text_layout t8( dc, "small (10): x_1^2_3 abc=y_2^4" );
	t8.render( dc, geom.x+500, geom.y+120, 0, false );

	vlines.push_back( geom.x+500 );

	dc.DestroyClippingRegion();
	dc.SetPen( *wxBLUE_PEN );
	dc.SetBrush( *wxTRANSPARENT_BRUSH );
	dc.DrawRectangle( geom );

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

	Draw( gdc1, wxRect( 0, 0, width, height/2 ) );

	wxGCDC gdc2(pdc);

#ifdef __WXMSW__
//	gdc2.GetGraphicsContext()->MSWSetStringFormat( wxGraphicsContext::Format_OptimizeForSmallFont );
#endif
	
	Draw( gdc2, wxRect( 0, height/2, width, height/2 ) );
	/*
	std::vector<int> vlines = Draw(pdc, wxRect( 0, 0, width, height/2 ) );

	
	wxGCDC gdc2(pdc);
	Draw( gdc2, wxRect( 0, height/2, width, height/2 ) );

	pdc.SetPen( *wxGREEN_PEN );
	pdc.SetBrush( *wxTRANSPARENT_BRUSH );

	for ( size_t i=0;i<vlines.size();i++ )
		pdc.DrawLine( vlines[i], 0, vlines[i], height );

	pdc.SetFont( wxFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
	pdc.SetTextForeground( *wxRED );
	pdc.DrawText( wxT("Using wxAutoBufferedPaintDC"), width/3, 10 );
	pdc.DrawText( wxT("green lines for showing text extent issues (compare top & bottom)"), width/3, 10+pdc.GetCharHeight() );
	pdc.DrawText( wxT("Corrected using wxGCDC. Modified wxWidgets:graphics.cpp"), width/3, height/2+10 );
	pdc.DrawText( wxT("This time using StringFormat::GenericTypographic()"), width/3, height/2+10+pdc.GetCharHeight() );
	*/
}

void TextLayoutDemo::OnSize( wxSizeEvent & )
{
	Refresh();
}
