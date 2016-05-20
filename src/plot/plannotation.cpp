
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
		wxPLOutputDevice::Style style ) 
	: wxPLAnnotation( ), m_points( pts ), m_size(size), m_colour(c), m_style(style)
{
}

wxPLLineAnnotation::~wxPLLineAnnotation()
{
	// nothing to do
}

void wxPLLineAnnotation::Draw( wxPLOutputDevice &dc, const wxPLAnnotationMapping &map )
{
	if ( m_points.size() == 0 ) return;
	
	std::vector<wxRealPoint> mapped( m_points.size(), wxRealPoint() );
	for( size_t i=0;i<m_points.size();i++ )
		mapped[i] = map.ToDevice( m_points[i] );

	dc.Pen( m_colour, m_size, m_style );
	dc.Lines( mapped.size(), &mapped[0] );
}
