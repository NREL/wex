/*
BSD 3-Clause License

Copyright (c) Alliance for Sustainable Energy, LLC. See also https://github.com/NREL/wex/blob/develop/LICENSE
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __pl_sectorplot_h
#define __pl_sectorplot_h

#include "wex/plot/pltext.h"
#include "wex/plot/plplot.h"
#include "wex/numeric.h"

class wxPLSectorPlot : public wxPLPlottable {
public:
    wxPLSectorPlot();

    void AddSector(double value, const wxString &label);

    void AddInnerSector(double value, const wxString &label);

    void SetColours(const std::vector<wxColour> &cl) {
        m_colourList = cl;
    }

    void SetFormat(wxNumericMode m, int deci, bool thousep,
                   const wxString &pre, const wxString &post) {
        m_fmtMode = m;
        m_fmtDeci = deci;
        m_fmtThouSep = thousep;
        m_fmtPre = pre;
        m_fmtPost = post;
        Invalidate();
    }

    void ShowSegmentValues(bool b) { m_labelValues = b; }

    void SetCalloutSize(double len) { m_calloutSize = len; }

    void SetCenterHoleSize(double fraction) { m_holeFraction = fraction; }

    void SetTextSpace(double sp) { m_textSpace = sp; }

    void SetBorder(double br) { m_border = br; }

    virtual wxRealPoint At(size_t i) const;

    virtual size_t Len() const;

    virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);

    virtual void DrawInLegend(wxPLOutputDevice &dc, const wxPLRealRect &rct);

    virtual wxPLAxis *SuggestXAxis();

    virtual wxPLAxis *SuggestYAxis();

protected:

    bool m_labelValues;
    double m_holeFraction;
    double m_calloutSize;
    double m_textSpace;
    double m_border;

    wxNumericMode m_fmtMode;
    int m_fmtDeci;
    bool m_fmtThouSep;
    wxString m_fmtPre, m_fmtPost;

    struct sector {
        sector(double v, const wxString &l) : value(v), label(l), layout(0) {}

        ~sector() { if (layout) delete layout; }

        sector(const sector &s) {
            layout = 0;
            copy(s);
        }

        void copy(const sector &s) {
            value = s.value;
            label = s.label;
            if (layout) delete layout;
            layout = 0;
        }

        sector &operator=(const sector &rhs) {
            copy(rhs);
            return *this;
        }

        double value;
        wxString label;

        wxPLTextLayout *layout;
        double start, angle;
        wxRealPoint textpos;
        wxRealPoint textsize;
        wxRealPoint callout_start, callout_end;
    };

    void Invalidate();

    void Invalidate(std::vector<sector> &list);

    void Layout(double radius, wxRealPoint *TL, wxRealPoint *BR);

    void CalculateAngles(std::vector<sector> &list);

    std::vector<sector> m_sectors;
    std::vector<sector> m_inner;
    std::vector<wxColour> m_colourList;
};

#endif
