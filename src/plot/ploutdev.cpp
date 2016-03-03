#include <vector>
#include <wex/pdf/pdfdoc.h>
#include <wex/pdf/pdffont.h>
#include <wex/plot/ploutdev.h>

void wxPLOutputDevice::SetAntiAliasing( bool )
{
	// nothing to do in base class...
}

wxPLPdfOutputDevice::wxPLPdfOutputDevice( wxPdfDocument &doc ) 
	: wxPLOutputDevice(), m_pdf(doc)	
{
	m_fontPoint = 0;
	m_fontPoint0 = m_pdf.GetFontSize();
	m_fontBold = (m_pdf.GetFontStyles() & wxPDF_FONTSTYLE_BOLD) ? true : false;
	m_pen = m_brush = true;
}

	// Pure virtuals, to be implemented
bool wxPLPdfOutputDevice::Equals( double a, double b ) const
{
	return ( wxRound(10.0*a) == wxRound(10.0*b) );
}

void wxPLPdfOutputDevice::Clip( double x, double y, double width, double height ) {
	m_pdf.ClippingRect( x, y, width, height );
}

void wxPLPdfOutputDevice::Unclip() {
	m_pdf.UnsetClipping();
}

void wxPLPdfOutputDevice::Pen( const wxColour &c, double size, 
	Style line, Style join, Style cap )
{
	if ( line == NONE ) {
		m_pen = false;
		return;
	}

	m_pen = true;

	wxPdfArrayDouble dash;
	wxPdfLineStyle style;
		
	style.SetColour( c );
		
	style.SetWidth( size );

	double dsize = size;
	if ( dsize < 1.5 ) dsize = 1.5;
		
	switch( line )
	{
	case DOT:
		dash.Add( dsize );
		dash.Add( dsize );
		break;
	case DASH:
		dash.Add( 2.0*dsize );
		dash.Add( dsize );
		break;
	case DOTDASH:
		dash.Add( dsize );
		dash.Add( dsize );
		dash.Add( 2.0*dsize );
		dash.Add( dsize );
		break;
	}
	style.SetDash( dash );

	switch( join )
	{
	case ROUND: style.SetLineJoin( wxPDF_LINEJOIN_ROUND ); break;
	case BEVEL: style.SetLineJoin( wxPDF_LINEJOIN_BEVEL ); break;
	default: style.SetLineJoin( wxPDF_LINEJOIN_MITER ); break;
	}
		
	switch( cap )
	{
	case ROUND: style.SetLineCap( wxPDF_LINECAP_ROUND ); break;
	default: style.SetLineCap( wxPDF_LINECAP_BUTT ); break;
	}

	m_pdf.SetLineStyle( style );
}

void wxPLPdfOutputDevice::Brush( const wxColour &c, Style sty ) {
	if ( sty == NONE )
	{
		m_brush = false;
		return;
	}

	// currently, hatch and other patterns not supported
	m_pdf.SetFillColour( c );
}
	
void wxPLPdfOutputDevice::Point( double x, double y ){
	m_pdf.Circle( x, y, 0.5 );
}

void wxPLPdfOutputDevice::Line( double x1, double y1, double x2, double y2 )	{
	m_pdf.Line( x1, y1, x2, y2 );
}

void wxPLPdfOutputDevice::Lines( size_t n, const wxRealPoint *pts ) {
	for (size_t i = 0; i < n; ++i)
	{			
		if (i == 0) m_pdf.MoveTo( pts[i].x, pts[i].y );
		else m_pdf.LineTo( pts[i].x, pts[i].y );
	}
	m_pdf.EndPath(wxPDF_STYLE_DRAW);
}

int wxPLPdfOutputDevice::GetDrawingStyle()
{
	int style = wxPDF_STYLE_NOOP;
	if ( m_brush && m_pen ) style = wxPDF_STYLE_FILLDRAW;
	else if (m_pen) style = wxPDF_STYLE_DRAW;
	else if (m_brush) style = wxPDF_STYLE_FILL;
	return style;
}

void wxPLPdfOutputDevice::Polygon( size_t n, const wxRealPoint *pts, Style winding ) {		
	if ( n == 0 ) return;
	int saveFillingRule = m_pdf.GetFillingRule();
	m_pdf.SetFillingRule( winding==ODDEVEN ? wxODDEVEN_RULE : wxWINDING_RULE );
	wxPdfArrayDouble xp(n, 0.0), yp(n, 0.0);
	for( size_t i=0;i<n;i++ )
	{
		xp[i] = pts[i].x;
		yp[i] = pts[i].y;
	}
	m_pdf.Polygon(xp, yp, GetDrawingStyle() );
	m_pdf.SetFillingRule(saveFillingRule);
}
void wxPLPdfOutputDevice::Rect( double x, double y, double width, double height ) {
	m_pdf.Rect( x, y, width, height, GetDrawingStyle() );
}

void wxPLPdfOutputDevice::Circle( double x, double y, double radius ) {
	m_pdf.Circle( x, y, radius, 0.0, 360.0, GetDrawingStyle() );
}
	
void wxPLPdfOutputDevice::Font( double relpt, bool bold ) {
	m_pdf.SetFontSize( m_fontPoint0 + relpt );
	m_fontPoint = relpt;
	m_fontBold = bold;
}

void wxPLPdfOutputDevice::Font( double *rel, bool *bld ) const {
	if ( rel ) *rel = m_fontPoint;
	if ( bld ) *bld = m_fontBold;
}

void wxPLPdfOutputDevice::Text( const wxString &text, double x, double y,  double angle ) {		
	double points = m_pdf.GetFontSize();
	double asc = (double)abs(m_pdf.GetFontDescription().GetAscent());
	double des = (double)abs(m_pdf.GetFontDescription().GetDescent());
	double em = asc+des;
	double ascfrac = asc/em;
	double ybase = points*ascfrac;

	if ( fabs(angle) < 0.5)
	{
		m_pdf.Text( x, y + ybase, text );
	}
	else
	{
		double xx = x + ybase*sin( angle*M_PI/180 );
		double yy = y + ybase*cos( angle*M_PI/180 );
		m_pdf.RotatedText( xx, yy, text, angle );
	}
}

void wxPLPdfOutputDevice::Measure( const wxString &text, double *width, double *height ) {
	*width = m_pdf.GetStringWidth(text);
	*height = m_pdf.GetFontSize();
}

double wxPLPdfOutputDevice::CharHeight() {
	return m_pdf.GetFontSize();
}


#define CAST(x) ((int)(m_scale*(x)))

wxPLDCOutputDevice::wxPLDCOutputDevice( wxDC *dc, wxDC *aadc, double scale ) 
	: wxPLOutputDevice(), m_dc(dc), m_aadc(aadc), m_curdc(dc),
		m_pen( *wxBLACK_PEN ), m_brush( *wxBLACK_BRUSH ), 
		m_font0( m_dc->GetFont() ), m_scale( scale )
{
	m_fontSize = 0;
	m_fontBold = ( m_font0.GetWeight() == wxFONTWEIGHT_BOLD );
}


void wxPLDCOutputDevice::SetAntiAliasing( bool on )
{
	if ( 0 != m_aadc )
	{
		if ( on )
		{
			m_aadc->SetPen( m_curdc->GetPen() );
			m_aadc->SetBrush( m_curdc->GetBrush() );
			m_aadc->SetFont( m_curdc->GetFont() );
			m_curdc = m_aadc;
		}
		else
		{
			m_dc->SetPen( m_curdc->GetPen() );
			m_dc->SetBrush( m_curdc->GetBrush() );
			m_dc->SetFont( m_curdc->GetFont() );
			m_curdc = m_dc;
		}
	}
}

wxDC *wxPLDCOutputDevice::GetDC() { return m_curdc; }
	
bool wxPLDCOutputDevice::Equals( double a, double b ) const {
	return ((int)a) == ((int)b);
}

void wxPLDCOutputDevice::Clip( double x, double y, double width, double height ) { 
	m_curdc->SetClippingRegion( CAST(x), CAST(y), CAST(width), CAST(height) );
}

void wxPLDCOutputDevice::Unclip() {
	m_curdc->DestroyClippingRegion();
}

void wxPLDCOutputDevice::Brush( const wxColour &c, Style sty ) { 
	m_brush.SetColour( c );
	switch( sty )
	{
	case NONE: m_brush = *wxTRANSPARENT_BRUSH; break;
	case HATCH: m_brush.SetStyle(wxCROSSDIAG_HATCH); break;
	default: m_brush.SetStyle( wxSOLID ); break;
	}

	m_curdc->SetBrush( m_brush );
}

void wxPLDCOutputDevice::Pen( const wxColour &c, double size, 
	Style line, Style join, Style cap ) {

	m_pen.SetColour( c );
	size *= m_scale;
	m_pen.SetWidth( size < 1.0 ? 1 : ((int)size) );
	switch( line )
	{
	case NONE: m_pen = *wxTRANSPARENT_PEN; break; // if transparent skip everything else
	case DOT: m_pen.SetStyle( wxDOT ); break;
	case DASH: m_pen.SetStyle( wxSHORT_DASH ); break;
	case DOTDASH: m_pen.SetStyle( wxDOT_DASH ); break;
	default: m_pen.SetStyle( wxSOLID ); break;
	}
	switch( join )
	{
	case MITER: m_pen.SetJoin( wxJOIN_MITER ); break;
	case BEVEL: m_pen.SetJoin( wxJOIN_BEVEL ); break;
	default: m_pen.SetJoin( wxJOIN_ROUND ); break;
	}
	switch( cap )
	{
	case ROUND: m_pen.SetCap( wxCAP_ROUND ); break;
	case MITER: m_pen.SetCap( wxCAP_PROJECTING ); break;
	default: m_pen.SetCap( wxCAP_BUTT );
	}
	m_curdc->SetPen( m_pen );
}

void wxPLDCOutputDevice::Point( double x, double y ) {
	m_curdc->DrawPoint( CAST(x), CAST(y) );
}

void wxPLDCOutputDevice::Line( double x1, double y1, double x2, double y2 ) {
	m_curdc->DrawLine( CAST(x1), CAST(y1), CAST(x2), CAST(y2) );
}

void wxPLDCOutputDevice::Lines( size_t n, const wxRealPoint *pts ) {
	if ( n == 0 ) return;
	std::vector<wxPoint> ipt( n, wxPoint(0,0) );
	for( size_t i=0;i<n;i++ )
		ipt[i] = wxPoint( CAST(pts[i].x), CAST(pts[i].y) );

	m_curdc->DrawLines( n, &ipt[0] );
}

void wxPLDCOutputDevice::Polygon( size_t n, const wxRealPoint *pts, Style sty ) {
	if ( n == 0 ) return;
	std::vector<wxPoint> ipt( n, wxPoint(0,0) );
	for( size_t i=0;i<n;i++ )
		ipt[i] = wxPoint( CAST(pts[i].x), CAST(pts[i].y) );

	m_curdc->DrawPolygon( n, &ipt[0], 0, 0, sty==ODDEVEN ?  wxODDEVEN_RULE : wxWINDING_RULE );
}
void wxPLDCOutputDevice::Rect( double x, double y, double width, double height ) {
	m_curdc->DrawRectangle( CAST(x), CAST(y), CAST(width), CAST(height) );
}
void wxPLDCOutputDevice::Circle( double x, double y, double radius ) {
	m_curdc->DrawCircle( CAST(x), CAST(y), CAST(radius) );
}
	
void wxPLDCOutputDevice::Font( double relpt, bool bold ) {
	wxFont font( m_font0 );
	if ( relpt != 0 ) font.SetPointSize( font.GetPointSize() + CAST(relpt) );
	font.SetWeight( bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL );
	m_curdc->SetFont( font );
	m_fontSize = relpt;
	m_fontBold = bold;
}
	
void wxPLDCOutputDevice::Font( double *rel, bool *bld ) const {
	if ( rel ) *rel = m_fontSize;
	if ( bld ) *bld = m_fontBold;
}

void wxPLDCOutputDevice::Text( const wxString &text, double x, double y, double angle ) {
	if ( angle != 0 ) m_curdc->DrawRotatedText( text, CAST(x), CAST(y), angle );
	else m_curdc->DrawText( text, CAST(x), CAST(y) );
}
void wxPLDCOutputDevice::Measure( const wxString &text, double *width, double *height ) {
	wxSize sz( m_curdc->GetTextExtent( text ) );
	if ( width )  *width = (double)sz.x/m_scale;
	if ( height ) *height = (double)sz.y/m_scale;
}
double wxPLDCOutputDevice::CharHeight() {
	return (double)m_curdc->GetCharHeight()/m_scale;
}
