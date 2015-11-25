#ifndef __DVColourMap_h
#define __DVColourMap_h

#include <vector>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/dc.h>

#include "wex/plot/plplotctrl.h"

class wxDVColourMap : public wxPLSideWidgetBase
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
	void ExtendScaleToNiceNumbers(bool useFineScale = false);

	//If true, + dir.  If false, - dir.
	void ExtendToNiceInPosDir(double* d, bool posDir, bool useFineScale = false);

	virtual wxColour ColourForValue(double val);
	
	// side widget renderer
	virtual wxSize CalculateBestSize();
	virtual void Render(wxDC& dc, const wxRect& geom);
	
protected:
	double m_min;
	double m_max;
	std::vector<wxColour> m_colourList;
};

class wxDVCoarseRainbowColourMap : public wxDVColourMap
{
public:
	wxDVCoarseRainbowColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxDVFineRainbowColourMap : public wxDVColourMap
{
public:
	wxDVFineRainbowColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxDVJetColourMap : public wxDVColourMap
{
public:
	wxDVJetColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxDVGrayscaleColourMap : public wxDVColourMap
{
public:
	wxDVGrayscaleColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

#endif
