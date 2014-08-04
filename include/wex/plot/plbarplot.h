#ifndef __pl_barplot_h
#define __pl_barplot_h

#include <vector>
#include "wex/plot/plplotctrl.h"

#define wxPL_BAR_AUTOSIZE -1

class wxPLBarPlotBase : public wxPLPlottable
{
public:
	wxPLBarPlotBase();
	wxPLBarPlotBase( const std::vector<wxRealPoint> &data, 
		const wxString &label = wxEmptyString, 
		const wxColour &col = *wxLIGHT_GREY );
	virtual ~wxPLBarPlotBase();
	
	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	
	void SetColour( const wxColour &col ) { m_colour = col; }
	void SetThickness( int thick = wxPL_BAR_AUTOSIZE ) { m_thickness = thick; }
	void SetData( const std::vector<wxRealPoint> &data ) { m_data = data; }

	virtual void DrawInLegend( wxDC &dc, const wxRect &rct);

protected:
	
	void Init();
	wxColour m_colour;
	int m_thickness;
	std::vector< wxRealPoint > m_data;

};

class wxPLBarPlot : public wxPLBarPlotBase
{
public:
	wxPLBarPlot();
	wxPLBarPlot( const std::vector<wxRealPoint> &data, 
		const wxString &label = wxEmptyString, 
		const wxColour &col = *wxLIGHT_GREY );
	virtual ~wxPLBarPlot();
	
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	
	void SetStackedOn( wxPLBarPlot *bp ) { m_stackedOn = bp; }
	void SetGroup( const std::vector<wxPLBarPlot*> &grp ) { m_group = grp; }

protected:

	double CalcYStart(double x);
	double CalcXStart(double x, const wxPLDeviceMapping &map, int dispwidth);
	int CalcDispBarWidth( const wxPLDeviceMapping &map );
	
	wxPLBarPlot *m_stackedOn;
	std::vector<wxPLBarPlot*> m_group;
};

class wxPLHBarPlot : public wxPLBarPlotBase
{
public:
	wxPLHBarPlot();
	wxPLHBarPlot( const std::vector<wxRealPoint> &data, double baseline_x = 0.0,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxLIGHT_GREY );
	virtual ~wxPLHBarPlot();
		
	void SetStackedOn( wxPLHBarPlot *bp ) { m_stackedOn = bp; }
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	
protected:
	double CalcXStart(double y);
	int CalcDispBarWidth( const wxPLDeviceMapping &map );

	wxPLHBarPlot *m_stackedOn;
	double m_baselineX;
};

#endif

