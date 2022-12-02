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

#ifndef __pl_histplot_h
#define __pl_histplot_h

#include "wex/plot/plplot.h"

class wxPLHistogramPlot : public wxPLPlottable {
public:
    wxPLHistogramPlot();

    wxPLHistogramPlot(const std::vector<wxRealPoint> &data,
                      const wxString &label);

    void Init();

    enum NormalizeType {
        NO_NORMALIZE = 0, NORMALIZE, NORMALIZE_PDF
    };

    void SetData(const std::vector<wxRealPoint> &data);

    //Getters and Setters
    void SetLineStyle(const wxColour &c, double width);

    void SetFillColour(const wxColour &c);

    void SetNumberOfBins(size_t n);

    void SetNormalize(NormalizeType n);

    NormalizeType GetNormalize() const;

    int GetNumberOfBins() const;

    double GetNiceYMax();

    double HistAt(size_t i) const;

    wxRealPoint HistBinAt(size_t i) const;

    bool GetIgnoreZeros();

    void SetIgnoreZeros(bool value = true);

    static int GetSturgesBinsFor(int nDataPoints);

    static int GetFreedmanDiaconisBinsFor(int nDataPoints);

    static int GetSqrtBinsFor(int nDataPoints);

    virtual wxRealPoint At(size_t i) const;

    virtual size_t Len() const;

    virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);

    virtual void DrawInLegend(wxPLOutputDevice &dc, const wxPLRealRect &rct);

    virtual wxPLAxis *SuggestXAxis();

    virtual wxPLAxis *SuggestYAxis();

    virtual bool GetMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax) const;

    virtual std::vector<wxString> GetExportableDatasetHeaders(wxUniChar sep, wxPLPlot *plot) const;

    virtual std::vector<wxRealPoint> GetExportableDataset(double Xmin, double Xmax, bool visible_only) const;

private:
    bool m_normalize;
    bool m_normalizeToPdf;
    bool m_ignoreZeros;

    wxColour m_lineColour;
    double m_lineThickness;
    wxColour m_fillColour;

    size_t m_numberOfBins;

    std::vector<double> m_histData;
    std::vector<wxRealPoint> m_histDataBinRanges;

    void RecalculateHistogram();

    double m_niceMax;
    double m_dataMin, m_dataMax;

    std::vector<wxRealPoint> m_data;
};

#endif
