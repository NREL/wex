#ifndef __DVColourMap_h
#define __DVColourMap_h

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/dc.h>

class wxDVColourMap
{
public:
	wxDVColourMap(double min = 0, double max = 1);
	virtual ~wxDVColourMap();

	virtual wxString GetName() = 0;
	virtual void SetScaleMinMax(double min, double max);
	virtual void SetScaleMin(double min);
	virtual void SetScaleMax(double max);
	virtual double GetScaleMin();
	virtual double GetScaleMax();
	void ExtendScaleToNiceNumbers();
	//If true, + dir.  If false, - dir.
	void ExtendToNiceInPosDir(double* d, bool posDir);

	virtual wxColour ColourForValue(double val) = 0;

	//Draw in upper right corner of rect and return space used.
	virtual wxSize DrawIn(wxDC &dc, const wxRect& geom) = 0;

protected:
	double mMinVal;
	double mMaxVal;
};

#endif
