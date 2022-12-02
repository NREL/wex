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

#ifndef __pl_plotctrl_h
#define __pl_plotctrl_h

#include <vector>
#include <wx/window.h>
#include <wx/menu.h>
#include <wx/stream.h>
#include <wx/graphics.h>

#ifdef __WXOSX__
#define PL_USE_OVERLAY 1
#include <wx/overlay.h>
#endif

#include "wex/plot/plplot.h"

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_PLOT_LEGEND, 0)
DECLARE_EVENT_TYPE(wxEVT_PLOT_ZOOM, 0)
DECLARE_EVENT_TYPE(wxEVT_PLOT_HIGHLIGHT, 0)
DECLARE_EVENT_TYPE(wxEVT_PLOT_DRAGGING, 0)
DECLARE_EVENT_TYPE(wxEVT_PLOT_DRAG_START, 0)
DECLARE_EVENT_TYPE(wxEVT_PLOT_DRAG_END, 0)
END_DECLARE_EVENT_TYPES()

#define EVT_PLOT_LEGEND(id, func) EVT_COMMAND(id, wxEVT_PLOT_LEGEND, func)
#define EVT_PLOT_ZOOM(id, func) EVT_COMMAND(id, wxEVT_PLOT_ZOOM, func)
#define EVT_PLOT_HIGHLIGHT(id, func) EVT_COMMAND(id, wxEVT_PLOT_HIGHLIGHT, func)
#define EVT_PLOT_DRAGGING(id, func) EVT_COMMAND(id, wxEVT_PLOT_DRAGGING, func)
#define EVT_PLOT_DRAG_START(id, func) EVT_COMMAND(id, wxEVT_PLOT_DRAG_START, func)
#define EVT_PLOT_DRAG_END(id, func) EVT_COMMAND(id, wxEVT_PLOT_DRAG_END, func)

class wxPLPlotCtrl : public wxWindow, public wxPLPlot {
public:
    wxPLPlotCtrl(wxWindow *parent, int id,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize);

    virtual ~wxPLPlotCtrl();

    void SetScaleTextSize(bool scale) { m_scaleTextSize = scale; }

    void SetIncludeLegendOnExport(bool b) { m_includeLegendOnExport = b; }

    enum HighlightMode {
        HIGHLIGHT_DISABLE,
        HIGHLIGHT_RECT,
        HIGHLIGHT_ZOOM,
        HIGHLIGHT_SPAN
    };

    void SetHighlightMode(HighlightMode highlighting) { m_highlighting = highlighting; }

    void GetHighlightBounds(double *left, double *right, double *top = 0, double *bottom = 0);

    wxMenu &GetContextMenu() { return m_contextMenu; }

    wxString ShowExportDialog();

    // handle pdf,svg,bmp,jpg,png,xpm,tiff
    bool Export(const wxString &file, int width = -1, int height = -1);

    bool ExportPdf(const wxString &file);

    wxBitmap GetBitmap(int width = -1, int height = -1);

    void Render(wxGraphicsContext &gc, wxRect geom, double fontpoints = -1);

protected:
    virtual wxSize DoGetBestSize() const;

    void OnPopupMenu(wxCommandEvent &);

    void OnPaint(wxPaintEvent &);

    void OnSize(wxSizeEvent &);

    void OnLeftDown(wxMouseEvent &);

    void OnLeftUp(wxMouseEvent &);

    void OnLeftDClick(wxMouseEvent &);

    void OnRightDown(wxMouseEvent &);

    void OnMotion(wxMouseEvent &);

    void OnMouseCaptureLost(wxMouseCaptureLostEvent &);

    void UpdateHighlightRegion();

    void DrawLegendOutline();

private:
    bool m_scaleTextSize;
    bool m_includeLegendOnExport;

    bool m_moveLegendMode;
    bool m_moveLegendErase;
    wxPoint m_anchorPoint;
    wxPoint m_currentPoint;
    wxMenu m_contextMenu;
    bool m_highlightMode;

#ifdef PL_USE_OVERLAY
    wxOverlay m_overlay;
#endif
    bool m_highlightErase;
    double m_highlightLeftPercent;
    double m_highlightRightPercent;
    double m_highlightTopPercent;
    double m_highlightBottomPercent;
    HighlightMode m_highlighting;

DECLARE_EVENT_TABLE();
};

#endif
