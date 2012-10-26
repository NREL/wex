#ifndef __DVFineRainbowColourMap_h
#define __DVFineRainbowColourMap_h

/*
 * wxDVFineRainbowColourMap
 * 
 * Rainbow colour map with a lot of color values.
 */

#include <vector>

#include "dview/dvcolourmap.h"

class wxDVFineRainbowColourMap : public wxDVColourMap
{
public:
	wxDVFineRainbowColourMap(double min = 0, double max = 1);
	virtual ~wxDVFineRainbowColourMap();

	wxString GetName();
	wxColour ColourForValue(double val);
	wxSize DrawIn(wxDC& dc, const wxRect& geom);

private:
	std::vector<wxColour> mColourList;
};

#endif

