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
	virtual void Render( wxPLOutputDevice &dc, const wxRect& geom);
	
protected:
	wxString m_format;
	double m_min;
	double m_max;
	std::vector<wxColour> m_colourList;
};

class wxPLCoarseRainbowColourMap : public wxPLColourMap
{
public:
	wxPLCoarseRainbowColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxPLFineRainbowColourMap : public wxPLColourMap
{
public:
	wxPLFineRainbowColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxPLJetColourMap : public wxPLColourMap
{
public:
	wxPLJetColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

class wxPLGrayscaleColourMap : public wxPLColourMap
{
public:
	wxPLGrayscaleColourMap( double min=0, double max=1 );
	virtual wxString GetName();
};

#endif
