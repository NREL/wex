#ifndef __DVColourMap_h
#define __DVColourMap_h

#include <vector>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/dc.h>

#include "wex/plot/plplotctrl.h"

class wxPLColourMap : public wxPLSideWidgetBase
{
public:
	wxPLColourMap(double min = 0, double max = 1);
	wxPLColourMap( const wxPLColourMap &cpy );
	virtual ~wxPLColourMap();
	void Copy( const wxPLColourMap &cpy );
	wxPLColourMap &operator=( const wxPLColourMap &cpy );

	virtual wxString GetName() = 0;

	virtual void SetScaleMinMax(double min, double max);
	virtual void SetScaleMin(double min);
	virtual void SetScaleMax(double max);
	virtual double GetScaleMin();
	virtual double GetScaleMax();
	void ExtendScaleToNiceNumbers();
	void SetFormat( const wxString &fmt ) { m_format = fmt; }

	//If true, + dir.  If false, - dir.
	void ExtendToNiceInPosDir(double* d, bool posDir, bool useFineScale = false);

	virtual wxColour ColourForValue(double val);
	
	// side widget renderer
	virtual wxSize CalculateBestSize();
	virtual void Render(wxDC& dc, const wxRect& geom);
	
protected:
	wxString m_format;
	double m_min;
	double m_max;
	std::vector<wxColour> m_colourList;
};

class wxDVCoarseRainbowColourMap : public wxPLColourMap
{
public:
	wxDVCoarseRainbowColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxDVFineRainbowColourMap : public wxPLColourMap
{
public:
	wxDVFineRainbowColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxDVJetColourMap : public wxPLColourMap
{
public:
	wxDVJetColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxDVGrayscaleColourMap : public wxPLColourMap
{
public:
	wxDVGrayscaleColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

#endif
