
#include "wex/plot/plannotation.h"

wxPLAnnotation::wxPLAnnotation()
{
}

wxPLAnnotation::~wxPLAnnotation()
{
}

wxPLAnnotationMapping::wxPLAnnotationMapping()
{
}

wxPLAnnotationMapping::~wxPLAnnotationMapping()
{
}


wxPLTextAnnotation::wxPLTextAnnotation( const wxString &text,
	const wxRealPoint &pos, 
	double size,
	double angle,
	const wxColour &c, 
	wxPLTextLayout::TextAlignment align )

	: wxPLAnnotation( ),
		m_text(text), m_pos(pos), m_size(size), m_angle(angle), m_colour(c), m_align(align), m_layout(0)
{
}

wxPLTextAnnotation::~wxPLTextAnnotation()
{
	if ( m_layout ) delete m_layout;
}
	
void wxPLTextAnnotation::Draw( wxPLOutputDevice &dc, const wxPLAnnotationMapping &map )
{
	if ( m_text.IsEmpty() ) return;

	dc.TextPoints( m_size );

	if ( m_layout ) delete m_layout;
	m_layout = new wxPLTextLayout( dc, m_text, m_align );
	
	wxRealPoint pos( map.ToDevice( m_pos ) );	
	if ( m_align == wxPLTextLayout::CENTER )
		pos.x -= m_layout->Width()/2;
	else if ( m_align == wxPLTextLayout::RIGHT )
		pos.x -= m_layout->Width();
	
	pos.y -= m_layout->Height()/2;
	
	dc.TextColour( m_colour );
	m_layout->Render( dc, pos.x, pos.y, m_angle, false );
}


wxPLLineAnnotation::wxPLLineAnnotation( const std::vector<wxRealPoint> &pts,
		double size,
		const wxColour &c,
		wxPLOutputDevice::Style style,
		ArrowType arrow ) 
	: wxPLAnnotation( ), m_points( pts ), m_size(size), m_colour(c), m_style(style), m_arrow(arrow)
{
}

wxPLLineAnnotation::~wxPLLineAnnotation()
{
	// nothing to do
}

static wxRealPoint rotate2d(
	const wxRealPoint &P, 
	double angle )
{
	double rad = angle*M_PI/180.0;
	return wxRealPoint(
		cos(rad)*P.x - sin(rad)*P.y,
		sin(rad)*P.x + cos(rad)*P.y );
}

void wxPLLineAnnotation::Draw( wxPLOutputDevice &dc, const wxPLAnnotationMapping &map )
{
	if ( m_points.size() < 2 ) return;

	dc.SetAntiAliasing( true );
	
	std::vector<wxRealPoint> mapped( m_points.size(), wxRealPoint() );
	for( size_t i=0;i<m_points.size();i++ )
		mapped[i] = map.ToDevice( m_points[i] );

	dc.Pen( m_colour, m_size, m_style );
	dc.Lines( mapped.size(), &mapped[0] );

	if ( m_arrow != NO_ARROW )
	{
		size_t len = mapped.size();
		wxRealPoint pt = mapped[len-1];
		wxRealPoint pt2 = mapped[len-2];

		wxRealPoint d( pt2.x - pt.x, pt2.y - pt.y );
		double sc = sqrt( d.x*d.x + d.y*d.y );
		double arrow_size = 3+m_size; // points
		d.x *= arrow_size/sc;
		d.y *= arrow_size/sc;

		const double angle = 30;

		wxRealPoint p1 = pt + rotate2d( d, -angle );
		wxRealPoint p2 = pt + rotate2d( d, angle );
		wxRealPoint avg( 0.5*(p1.x+p2.x), 0.5*(p1.y+p2.y) );
		wxRealPoint pts[5] = { avg, p1, pt, p2, avg };

		dc.Pen( m_colour, m_size, wxPLOutputDevice::SOLID );
		dc.Brush( m_colour );

		if ( m_arrow == FILLED_ARROW )
			dc.Polygon( 5, pts, wxPLOutputDevice::WINDING_RULE );
		else
			dc.Lines( 3, pts+1 );
	}
}
