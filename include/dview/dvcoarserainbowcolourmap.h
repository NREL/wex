#ifndef __DVCoarseRainbowColourMap_h
#define __DVCoarseRainbowColourMap_h

#include <vector>

#include "plot/plplotctrl.h"
#include "dview/dvcolourmap.h"

class wxDVCoarseRainbowColourMap : 
	public wxDVColourMap, public wxPLSideWidgetBase
{
public:
	wxDVCoarseRainbowColourMap(double min = 0, double max = 1);
	virtual ~wxDVCoarseRainbowColourMap();

	wxString GetName();

	virtual wxColour ColourForValue(double val);

	virtual wxSize CalculateBestSize();
	virtual void Render(wxDC& dc, const wxRect& geom);
private:
	std::vector<wxColour> mColourList;
};

#endif

