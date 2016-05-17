#ifndef __pltext_h
#define __pltext_h

#include <vector>

#include <wx/string.h>
#include <wx/window.h>

#include <wex/plot/ploutdev.h>

class wxPLTextLayout
{
public:
	enum TextAlignment { LEFT, CENTER, RIGHT };	

	wxPLTextLayout( wxPLOutputDevice &dc, const wxString &text, TextAlignment ta = LEFT );
	
	void Align( TextAlignment ta );
	void Render( wxPLOutputDevice &dc, double x, double y, double rotationDegrees = 0.0, bool drawBounds = false ); 
	
	inline double Width() { return m_bounds.x; }
	inline double Height() { return m_bounds.y; }

protected:
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
	
	wxString Escape( const wxString &text );
	std::vector<text_piece> Parse( const wxString &text );
};



class wxPLTextLayoutDemo : public wxWindow
{
public:
	wxPLTextLayoutDemo( wxWindow *parent );

	std::vector<double> Draw( wxPLOutputDevice &dc, const wxPLRealRect &geom );

	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );

	DECLARE_EVENT_TABLE();
};

#endif
