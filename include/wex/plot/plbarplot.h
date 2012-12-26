#ifndef __pl_barplot_h
#define __pl_barplot_h

#include "wex/plot/plplotctrl.h"

class wxPLBarPlot : public wxPLPlottable
{
public:
	wxPLBarPlot();
	wxPLBarPlot( const std::vector<wxRealPoint> &data, 
		const wxString &label = wxEmptyString, 
		const wxColour &col = *wxLIGHT_GREY );
	virtual ~wxPLBarPlot();
	
	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxDC &dc, const wxRect &rct);

	void SetColour( const wxColour &col ) { m_colour = col; }
	void SetThickness( int thick, bool scale = false ) { m_thickness = thick; m_scaleThickness = scale; }
	void SetData( const std::vector<wxRealPoint> &data ) { m_data = data; }

	void SetStackedOn( wxPLBarPlot *bp ) { m_stackedOn = bp; }
	void SetGroup( const std::vector<wxPLBarPlot*> &grp ) { m_group = grp; }

protected:

	double CalcYStart(double x);
	double CalcXStart(double x, const wxPLDeviceMapping &map, int dispwidth);
	int CalcDispBarWidth( const wxPLDeviceMapping &map );

	void Init();
	wxColour m_colour;
	int m_thickness;
	bool m_scaleThickness;
	std::vector< wxRealPoint > m_data;

	wxPLBarPlot *m_stackedOn;
	std::vector<wxPLBarPlot*> m_group;
};
#endif

