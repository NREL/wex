#ifndef __pl_scatterplot_h
#define __pl_scatterplot_h

#include "plot/plplotctrl.h"

class wxPLScatterPlot : public wxPLPlottable
{
public:
	wxPLScatterPlot();
	wxPLScatterPlot( const std::vector<wxRealPoint> &data,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxBLUE,
		int size = 1,
		bool scale = false );

	virtual ~wxPLScatterPlot();
	
	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxDC &dc, const wxRect &rct);

	void SetColour( const wxColour &col ) { m_colour = col; }
	void SetSize( int radius ) { m_size = radius; }

protected:
	wxColour m_colour;
	int m_size;
	bool m_scale;
	std::vector<wxRealPoint> m_data;
};

#endif
