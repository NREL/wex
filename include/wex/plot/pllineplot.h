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

#ifndef __pl_lineplot_h
#define __pl_lineplot_h

#include "wex/plot/plplot.h"

class wxPLLinePlot : public wxPLPlottable {
public:
    enum Style {
        NO_LINE, SOLID, DOTTED, DASHED
    };
    enum Marker {
        NO_MARKER, CIRCLE, SQUARE, DIAMOND, HOURGLASS
    };

    wxPLLinePlot();

    wxPLLinePlot(const std::vector<wxRealPoint> &data,
                 const wxString &label = wxEmptyString,
                 const wxColour &col = *wxBLUE,
                 Style sty = SOLID,
                 double thick = 2.0,
                 Marker mkr = NO_MARKER);

    virtual ~wxPLLinePlot();

    virtual wxRealPoint At(size_t i) const;

    virtual size_t Len() const;

    virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);

    virtual void DrawInLegend(wxPLOutputDevice &dc, const wxPLRealRect &rct);

    bool GetIgnoreZeros();

    void SetIgnoreZeros(bool value = true);

    void SetColour(const wxColour &col) { m_colour = col; }

    void SetThickness(double thick) { m_thickness = thick; }

    void SetStyle(Style ss) { m_style = ss; }

    void SetMarker(Marker mm) { m_marker = mm; }

    void SetData(const std::vector<wxRealPoint> &data) { m_data = data; }

protected:
    void Init();

    wxColour m_colour;
    double m_thickness;
    Style m_style;
    Marker m_marker;
    std::vector<wxRealPoint> m_data;

    void DrawMarkers(wxPLOutputDevice &dc, std::vector<wxRealPoint> &points, double size);

private:
    bool m_ignoreZeros;
};

#endif
