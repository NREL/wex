/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

#ifndef __DVColourMap_h
#define __DVColourMap_h

#include <vector>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/dc.h>

#include "wex/plot/plplot.h"

class wxPLColourMap : public wxPLSideWidgetBase
{
public:
	wxPLColourMap(double min = 0, double max = 1);
	wxPLColourMap(const wxPLColourMap &cpy);
	virtual ~wxPLColourMap();
	void Copy(const wxPLColourMap &cpy);
	wxPLColourMap &operator=(const wxPLColourMap &cpy);

	virtual wxString GetName() = 0;

	virtual void SetScaleMinMax(double min, double max);
	virtual void SetScaleMin(double min);
	virtual void SetScaleMax(double max);
	virtual double GetScaleMin();
	virtual double GetScaleMax();
	void ExtendScaleToNiceNumbers();
	void SetFormat(const wxString &fmt);
	void SetLabels(const wxArrayString &l);
	wxArrayString GetLabels() const { return m_labels; }

	//If true, + dir.  If false, - dir.
	void ExtendToNiceInPosDir(double* d, bool posDir, bool useFineScale = false);

	virtual wxColour ColourForValue(double val);

	void SetReversed(bool r = true) { m_reversed = r; }
	bool IsReversed();

	// side widget renderer
	virtual wxRealPoint CalculateBestSize(wxPLOutputDevice &dc);
	virtual void Render(wxPLOutputDevice &dc, const wxPLRealRect& geom);

protected:
	bool m_reversed;
	wxString m_format;
	double m_min;
	double m_max;
	wxArrayString m_labels;
	std::vector<wxColour> m_colourList;
};

class wxPLCoarseRainbowColourMap : public wxPLColourMap
{
public:
	wxPLCoarseRainbowColourMap(double min = 0, double max = 1);
	virtual wxString GetName();
};

class wxPLFineRainbowColourMap : public wxPLColourMap
{
public:
	wxPLFineRainbowColourMap(double min = 0, double max = 1);
	virtual wxString GetName();
};

class wxPLJetColourMap : public wxPLColourMap
{
public:
	wxPLJetColourMap(double min = 0, double max = 1);
	virtual wxString GetName();
};

class wxPLParulaColourMap : public wxPLColourMap
{
public:
	wxPLParulaColourMap(double min = 0, double max = 1);
	virtual wxString GetName();
};

class wxPLGrayscaleColourMap : public wxPLColourMap
{
public:
	wxPLGrayscaleColourMap(double min = 0, double max = 1);
	virtual wxString GetName();
};

#endif
