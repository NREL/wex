#ifndef __DVCoarseRainbowColourMap_h
#define __DVCoarseRainbowColourMap_h

#include "dview/dvcolourmap.h"

#include <vector>

class wxDVCoarseRainbowColourMap : public wxDVColourMap
{
public:
	wxDVCoarseRainbowColourMap(double min = 0, double max = 1);
	virtual ~wxDVCoarseRainbowColourMap();

	wxString GetName();

	wxColour ColourForValue(double val);

	wxSize DrawIn(wxDC& dc, const wxRect& geom);
private:
	std::vector<wxColour> mColourList;
};

#endif

