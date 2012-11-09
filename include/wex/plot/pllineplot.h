#ifndef __pl_lineplot_h
#define __pl_lineplot_h

#include "wex/plot/plplotctrl.h"

class wxPLLinePlot : public wxPLPlottable
{
public:
	enum Style { SOLID, DOTTED, DASHED };

	wxPLLinePlot();
	wxPLLinePlot( const std::vector<wxRealPoint> &data, 
		const wxString &label = wxEmptyString, 
		const wxColour &col = *wxBLUE,
		Style sty = SOLID,
		int thick = 2,
		bool scale = false );
	virtual ~wxPLLinePlot();


	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxDC &dc, const wxRect &rct);

	void SetColour( const wxColour &col ) { m_colour = col; }
	void SetThickness( int thick, bool scale = false ) { m_thickness = thick; m_scaleThickness = scale; }
	void SetStyle( Style ss ) { m_style = ss; }
	void SetData( const std::vector<wxRealPoint> &data ) { m_data = data; }

protected:
	void Init();
	wxColour m_colour;
	int m_thickness;
	Style m_style;
	bool m_scaleThickness;
	std::vector< wxRealPoint > m_data;
};
#endif

