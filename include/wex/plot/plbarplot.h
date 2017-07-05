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

#ifndef __pl_barplot_h
#define __pl_barplot_h

#include <vector>
#include "wex/plot/plplot.h"

#define wxPL_BAR_AUTOSIZE -1

class wxPLBarPlotBase : public wxPLPlottable
{
public:
	wxPLBarPlotBase();
	wxPLBarPlotBase(const std::vector<wxRealPoint> &data, double baseline,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxLIGHT_GREY);
	virtual ~wxPLBarPlotBase();

	virtual wxRealPoint At(size_t i) const;
	virtual size_t Len() const;

	void SetColour(const wxColour &col) { m_colour = col; }
	void SetThickness(double thick = wxPL_BAR_AUTOSIZE) { m_thickness = thick; }
	void SetData(const std::vector<wxRealPoint> &data) { m_data = data; }

	virtual void DrawInLegend(wxPLOutputDevice &dc, const wxPLRealRect &rct);

protected:

	void Init();
	double m_baseline;
	wxColour m_colour;
	double m_thickness;
	std::vector< wxRealPoint > m_data;
};

class wxPLBarPlot : public wxPLBarPlotBase
{
public:
	wxPLBarPlot();
	wxPLBarPlot(const std::vector<wxRealPoint> &data, double baseline_y = 0.0,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxLIGHT_GREY);
	virtual ~wxPLBarPlot();

	virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);

	void SetStackedOn(wxPLBarPlot *bp) { m_stackedOn = bp; }
	void SetGroup(const std::vector<wxPLBarPlot*> &grp) { m_group = grp; }

	virtual wxPLAxis *SuggestYAxis() const;

protected:

	double CalcYPos(double x) const;
	double CalcXPos(double x, const wxPLDeviceMapping &map, double dispwidth);
	double CalcDispBarWidth(const wxPLDeviceMapping &map);

	wxPLBarPlot *m_stackedOn;
	std::vector<wxPLBarPlot*> m_group;
};

class wxPLHBarPlot : public wxPLBarPlotBase
{
public:
	wxPLHBarPlot();
	wxPLHBarPlot(const std::vector<wxRealPoint> &data, double baseline_x = 0.0,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxLIGHT_GREY);
	virtual ~wxPLHBarPlot();

	void SetStackedOn(wxPLHBarPlot *bp) { m_stackedOn = bp; }
	virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);

	virtual wxPLAxis *SuggestXAxis() const;
protected:
	double CalcXPos(double y) const;
	double CalcDispBarWidth(const wxPLDeviceMapping &map);

	wxPLHBarPlot *m_stackedOn;
};

#endif
