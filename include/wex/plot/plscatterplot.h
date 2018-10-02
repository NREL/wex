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

#ifndef __pl_scatterplot_h
#define __pl_scatterplot_h

#include "wex/plot/plplot.h"

class wxPLColourMap;

class wxPLScatterPlot : public wxPLPlottable
{
public:
	wxPLScatterPlot();
	wxPLScatterPlot(const std::vector<wxRealPoint> &data,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxBLUE,
		double size = 1,
		bool scale = false);

	virtual ~wxPLScatterPlot();

	void SetColourMap(wxPLColourMap *cmap); // does not take ownership of colour map
	void SetColours(const std::vector<double> &zv);
	void ClearColours();
	void SetSizes(const std::vector<double> &sv);
	void ClearSizes();

	virtual wxRealPoint At(size_t i) const;
	virtual size_t Len() const;
	virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);
	virtual void DrawInLegend(wxPLOutputDevice &dc, const wxPLRealRect &rct);

	void SetColour(const wxColour &col) { m_colour = col; }
	void SetSize(double radius) { m_radius = radius; }
	void SetLineOfPerfectAgreementFlag(bool flagValue);

protected:
	wxColour m_colour;
	double m_radius;
	bool m_scale;
	bool m_drawLineOfPerfectAgreement;
	bool m_isLineOfPerfectAgreementDrawn;
	std::vector<wxRealPoint> m_data;
	std::vector<double> m_colours, m_sizes;
	wxPLColourMap *m_cmap;
};

#endif
