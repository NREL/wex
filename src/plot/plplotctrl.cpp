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

class wxDCOutputDevice : public wxPLOutputDevice
{
	wxDC &m_dc;
	wxPen m_pen;
	wxBrush m_brush;
	wxFont m_font0;
	double m_fontSize;
	bool m_fontBold;
#define CAST(x) ((int)x)
public:
	wxDCOutputDevice( wxDC &dc ) 
		: wxPLOutputDevice(), m_dc(dc), 
			m_pen( *wxBLACK_PEN ), m_brush( *wxBLACK_BRUSH ), 
			m_font0( m_dc.GetFont() )
	{
		m_fontSize = 0;
		m_fontBold = ( m_font0.GetWeight() == wxFONTWEIGHT_BOLD );
	}

	wxDC *GetDC() { return &m_dc; }
	
	virtual bool Equals( double a, double b ) const {
		return ((int)a) == ((int)b);
	}

	virtual void Clip( double x, double y, double width, double height ) { 
		m_dc.SetClippingRegion( CAST(x), CAST(y), CAST(width), CAST(height) );
	}

	virtual void Unclip() {
		m_dc.DestroyClippingRegion();
	}

	virtual void Brush( const wxColour &c, Style sty ) { 
		m_brush.SetColour( c );
		switch( sty )
		{
		case NONE: m_brush = *wxTRANSPARENT_BRUSH; break;
		case HATCH: m_brush.SetStyle(wxCROSSDIAG_HATCH); break;
		default: m_brush.SetStyle( wxSOLID ); break;
		}

		m_dc.SetBrush( m_brush );
	}

	virtual void Pen( const wxColour &c, double size, 
		Style line = SOLID, Style join = MITER, Style cap = BUTT ) {

		m_pen.SetColour( c );
		m_pen.SetWidth( size < 1.0 ? 1 : CAST(size) );
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
		m_dc.SetPen( m_pen );
	}

	virtual void Point( double x, double y ) {
		m_dc.DrawPoint( CAST(x), CAST(y) );
	}

	virtual void Line( double x1, double y1, double x2, double y2 ) {
		m_dc.DrawLine( CAST(x1), CAST(y1), CAST(x2), CAST(y2) );
	}

	virtual void Lines( size_t n, const wxRealPoint *pts ) {
		if ( n == 0 ) return;
		std::vector<wxPoint> ipt( n, wxPoint(0,0) );
		for( size_t i=0;i<n;i++ )
			ipt[i] = wxPoint( CAST(pts[i].x), CAST(pts[i].y) );

		m_dc.DrawLines( n, &ipt[0] );
	}

	virtual void Polygon( size_t n, const wxRealPoint *pts, Style sty ) {
		if ( n == 0 ) return;
		std::vector<wxPoint> ipt( n, wxPoint(0,0) );
		for( size_t i=0;i<n;i++ )
			ipt[i] = wxPoint( CAST(pts[i].x), CAST(pts[i].y) );

		m_dc.DrawPolygon( n, &ipt[0], 0, 0, sty==ODDEVEN ?  wxODDEVEN_RULE : wxWINDING_RULE );
	}
	virtual void Rect( double x, double y, double width, double height ) {
		m_dc.DrawRectangle( CAST(x), CAST(y), CAST(width), CAST(height) );
	}
	virtual void Circle( double x, double y, double radius ) {
		m_dc.DrawCircle( CAST(x), CAST(y), CAST(radius) );
	}
	
	virtual void Font( double relpt = 0, bool bold = false ) {
		wxFont font( m_font0 );
		if ( relpt != 0 ) font.SetPointSize( font.GetPointSize() + CAST(relpt) );
		font.SetWeight( bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL );
		m_dc.SetFont( font );
		m_fontSize = relpt;
		m_fontBold = bold;
	}
	
	virtual void Font( double *rel, bool *bld ) const {
		if ( rel ) *rel = m_fontSize;
		if ( bld ) *bld = m_fontBold;
	}

	virtual void Text( const wxString &text, double x, double y, double angle=0 ) {
		if ( angle != 0 ) m_dc.DrawRotatedText( text, CAST(x), CAST(y), angle );
		else m_dc.DrawText( text, CAST(x), CAST(y) );
	}
	virtual void Measure( const wxString &text, double *width, double *height ) {
		wxSize sz( m_dc.GetTextExtent( text ) );
		if ( width )  *width = (double)sz.x;
		if ( height ) *height = (double)sz.y;
	}
	virtual double CharHeight() {
		return (double)m_dc.GetCharHeight();
	}
};

DEFINE_EVENT_TYPE( wxEVT_PLOT_LEGEND )
DEFINE_EVENT_TYPE( wxEVT_PLOT_HIGHLIGHT )
DEFINE_EVENT_TYPE( wxEVT_PLOT_ZOOM )
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
	EVT_LEFT_DCLICK( wxPLPlotCtrl::OnLeftDClick )
	EVT_RIGHT_DOWN( wxPLPlotCtrl::OnRightDown )
	EVT_MOTION( wxPLPlotCtrl::OnMotion )	
	EVT_MOUSE_CAPTURE_LOST( wxPLPlotCtrl::OnMouseCaptureLost )
	
	EVT_MENU_RANGE( ID_COPY_DATA_CLIP, ID_EXPORT_SVG, wxPLPlotCtrl::OnPopupMenu )
END_EVENT_TABLE()

wxPLPlotCtrl::wxPLPlotCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size)
	: wxWindow(parent, id, pos, size), wxPLPlot()
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	SetFont( *wxNORMAL_FONT );

	m_scaleTextSize = false;
	m_includeLegendOnExport = false;
	
	m_anchorPoint = wxPoint(0, 0);
	m_currentPoint = wxPoint(0, 0);
	m_moveLegendMode = false;
	m_moveLegendErase = false;
	m_highlightMode = false;
	m_highlightErase = false;
	m_highlightLeftPercent = 0.0;
	m_highlightRightPercent = 0.0;
	m_highlightTopPercent = 0.0;
	m_highlightBottomPercent = 0.0;
	m_highlighting = HIGHLIGHT_DISABLE;
	
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
	// nothing to do
}

void wxPLPlotCtrl::GetHighlightBounds( double *left, double *right, double *top, double *bottom )
{
	*left = m_highlightLeftPercent;
	*right = m_highlightRightPercent;
	if (top) *top = m_highlightTopPercent;
	if (bottom) *bottom = m_highlightBottomPercent;
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
			
			if (menuid == ID_EXPORT_SCREEN ||
				menuid == ID_EXPORT_SMALL ||
				menuid == ID_EXPORT_NORMAL)
			{
				wxString filename = ShowExportDialog();
				if ( !filename.IsEmpty() )
					if ( !Export( filename, imgSize.x, imgSize.y ) )
						wxMessageBox("Error writing image file to:\n\n" + filename, "Export error", wxICON_ERROR|wxOK);
			}
			else if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxBitmapDataObject(GetBitmap(imgSize.x, imgSize.y)));
				wxTheClipboard->Close();
			}
		}
		break;
	}
}

wxString wxPLPlotCtrl::ShowExportDialog( )
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
			ext.MakeLower();

			int filt = fdlg.GetFilterIndex();
			wxLogStatus("FILTER IDX=%d\n", filt);
			switch( filt )
			{
			case 0: if (ext != "bmp") fn += ".bmp"; break;
			case 1: if (ext != "jpg") fn += ".jpg"; break;
			case 2: if (ext != "png") fn += ".png"; break;
			}

			return fn;
		}
	}

	return wxEmptyString;
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
			wxDCOutputDevice odev(dctemp);
			CalcLegendTextLayout( odev );
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

bool wxPLPlotCtrl::Export( const wxString &file, int width, int height )
{
	wxFileName fn(file);
	wxString ext( fn.GetExt().Lower() );
	if ( ext == "pdf" ) return ExportPdf( file );
	else if ( ext == "svg" ) return ExportSvg( file );
	else {
		wxBitmapType type( wxBITMAP_TYPE_INVALID );

		if ( ext == "bmp" ) type = wxBITMAP_TYPE_BMP;
		else if ( ext == "jpg" ) type = wxBITMAP_TYPE_JPEG;
		else if ( ext == "png" ) type = wxBITMAP_TYPE_PNG;
		else if ( ext == "xpm" ) type = wxBITMAP_TYPE_XPM;
		else if ( ext == "tiff" ) type = wxBITMAP_TYPE_TIFF;
		else return false;

		return GetBitmap(width,height).SaveFile(file, type);
	}
}

bool wxPLPlotCtrl::ExportPdf( const wxString &file )
{
	int width, height;
	GetClientSize( &width, &height );
	wxClientDC dc(this);
	double ppi = dc.GetPPI().x;
	return RenderPdf( file, width*72.0/ppi, height*72.0/ppi );
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
	wxFont font_normal( dc.GetFont() );
	if ( m_scaleTextSize )
	{
		// scale text according to geometry width, within limits
		double point_size = geom.width/1000.0 * 12.0;
		if ( point_size > 23 ) point_size = 23;
		if ( point_size < 7 ) point_size = 7;
		font_normal.SetPointSize( (int)point_size );
	}
	dc.SetFont( font_normal );

	gcdc_ref gdc;
	if ( wxMemoryDC *memdc = dynamic_cast<wxMemoryDC*>( &dc ) )
		gdc.m_ptr = new wxGCDC( *memdc );
	else if ( wxWindowDC *windc = dynamic_cast<wxWindowDC*>( &dc ) )
		gdc.m_ptr = new wxGCDC( *windc );
	else if ( wxPrinterDC *prindc = dynamic_cast<wxPrinterDC*>( &dc ) )
		gdc.m_ptr = new wxGCDC( *prindc );

	wxDC &aadc = gdc.m_ptr ? *gdc.m_ptr : dc;

	wxDCOutputDevice odev1(dc);
	wxDCOutputDevice odev2(aadc);

	wxPLPlot::Render( odev1, odev2, geom );
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

void wxPLPlotCtrl::OnSize( wxSizeEvent & )
{
	if ( m_scaleTextSize ) m_legendInvalidated = true;
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
	
	wxCoord highlight_y = m_currentPoint.y < m_anchorPoint.y ? m_currentPoint.y : m_anchorPoint.y;
	wxCoord highlight_height = abs( m_currentPoint.y - m_anchorPoint.y );

	int irect = 0;
	
	if ( m_highlighting == HIGHLIGHT_SPAN )
	{
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
		}
	}
	else
	{
		// rectangular (RECT or ZOOM) highlight on current plot
		for ( std::vector<wxRect>::const_iterator it = m_plotRects.begin();
			it != m_plotRects.end();
			++it )
		{
			if ( it->Contains( wxPoint(highlight_x, highlight_y) ) )
			{
				irect = it-m_plotRects.begin();

				wxRect hrct( highlight_x, highlight_y, highlight_width, highlight_height );
				dc.DrawRectangle( hrct );
				break;
			}
		}

	}

	m_highlightLeftPercent = 100.0*( ((double)(highlight_x - m_plotRects[0].x)) / ( (double)m_plotRects[0].width ) );
	m_highlightRightPercent = 100.0*( ((double)(highlight_x + highlight_width - m_plotRects[0].x )) / ((double)m_plotRects[0].width ) );
	m_highlightTopPercent = 100.0*( ((double)(highlight_y - m_plotRects[irect].y)) / ( (double) m_plotRects[irect].height) );
	m_highlightBottomPercent = 100.0*( ((double)(highlight_y + highlight_height - m_plotRects[irect].y)) / ( (double) m_plotRects[irect].height) );
}

void wxPLPlotCtrl::OnLeftDClick( wxMouseEvent &evt )
{
	if ( m_highlighting == HIGHLIGHT_ZOOM )
	{
		RescaleAxes();
		Invalidate();
		Refresh();
		
		wxCommandEvent e( wxEVT_PLOT_ZOOM, GetId() );
		e.SetEventObject( this );
		GetEventHandler()->ProcessEvent( e );
	}
	else
		evt.Skip();
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
	else if ( m_highlighting != HIGHLIGHT_DISABLE )
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
	else if ( m_highlighting != HIGHLIGHT_DISABLE && m_highlightMode )
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

		wxCoord diffx = abs( ClientToScreen(evt.GetPosition()).x - ClientToScreen(m_anchorPoint).x );
		wxCoord diffy = abs( ClientToScreen(evt.GetPosition()).y - ClientToScreen(m_anchorPoint).y );
		if ( (m_highlighting==HIGHLIGHT_SPAN && diffx > 10)
			|| m_highlighting==HIGHLIGHT_RECT && diffx > 1 && diffy > 1 )
		{
			wxCommandEvent e( wxEVT_PLOT_HIGHLIGHT, GetId() );
			e.SetEventObject( this );
			GetEventHandler()->ProcessEvent( e );
		}
		else if ( m_highlighting==HIGHLIGHT_ZOOM  && diffx > 1 && diffy > 1 )
		{
			wxPLAxis *ax = GetXAxis1();
			if ( !ax ) ax = GetXAxis2();
			wxPLAxis *ay = GetYAxis1();
			if ( !ay ) ay = GetYAxis2();

			if (ax && ay)
			{
				double left, right, top, bottom;
				GetHighlightBounds( &left, &right, &top, &bottom );

				double min, max;
				ax->GetWorld( &min, &max );
				ax->SetWorld( min+(max-min)*0.01*left, min+(max-min)*0.01*right );

				ay->GetWorld( &min, &max );
				ay->SetWorld( max-(max-min)*0.01*bottom, max-(max-min)*0.01*top );

				Invalidate();
				Refresh();
				
				wxCommandEvent e( wxEVT_PLOT_ZOOM, GetId() );
				e.SetEventObject( this );
				GetEventHandler()->ProcessEvent( e );
			}
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
