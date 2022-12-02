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

#ifndef __pltext_h
#define __pltext_h

#include <vector>

#include <wx/string.h>
#include <wx/window.h>

#include <wex/plot/ploutdev.h>

class wxPLTextLayout {
public:
    enum TextAlignment {
        LEFT, CENTER, RIGHT
    };

    wxPLTextLayout(wxPLOutputDevice &dc, const wxString &text, TextAlignment ta = LEFT);

    void Align(TextAlignment ta);

    void Render(wxPLOutputDevice &dc, double x, double y, double rotationDegrees = 0.0, bool drawBounds = false);

    inline double Width() { return m_bounds.x; }

    inline double Height() { return m_bounds.y; }

    static wxString Escape(const wxString &text);

protected:
    struct text_piece {
        text_piece() : text(wxEmptyString), state(NORMAL), origin(0, 0), size(0, 0), aligned_x(0) {}

        enum TextState {
            NORMAL, SUPERSCRIPT, SUBSCRIPT
        };
        wxString text;
        TextState state;
        wxRealPoint origin;
        wxRealPoint size;
        double aligned_x;
    };

    std::vector<std::vector<text_piece> > m_lines;
    wxRealPoint m_bounds;

    std::vector<text_piece> Parse(const wxString &text);
};

class wxPLTextLayoutDemo : public wxWindow {
public:
    wxPLTextLayoutDemo(wxWindow *parent);

    std::vector<double> Draw(wxPLOutputDevice &dc, const wxPLRealRect &geom);

    void OnPaint(wxPaintEvent &);

    void OnSize(wxSizeEvent &);

DECLARE_EVENT_TABLE();
};

int wxFreeTypeLoadAllFonts(const wxString &path = wxEmptyString);

int wxFreeTypeLoadFont(const wxString &font_file);

wxArrayString wxFreeTypeListFonts();

wxString wxFreeTypeFontFile(int ifnt);

wxString wxFreeTypeFontName(int ifnt);

bool wxFreeTypeFontStyle(int ifnt, bool *bold, bool *italic);

int wxFreeTypeFindFont(const wxString &font);

unsigned char *wxFreeTypeFontData(int ifnt, size_t *len);

wxSize wxFreeTypeMeasure(int ifnt, double points, unsigned int dpi, const wxString &text);

void wxFreeTypeDraw(wxImage *img, bool init_img, const wxPoint &pos,
                    int ifnt, double points, unsigned int dpi,
                    const wxString &text, const wxColour &c = *wxBLACK, double angle = 0.0);

wxImage wxFreeTypeDraw(wxRealPoint *pos, int ifnt, double points, unsigned int dpi,
                       const wxString &text, const wxColour &c = *wxBLACK, double angle = 0.0);

void wxFreeTypeDraw(wxDC &dc, const wxPoint &pos, int ifnt, double points, unsigned int dpi,
                    const wxString &text, const wxColour &c = *wxBLACK, double angle = 0.0);

void wxFreeTypeDraw(wxGraphicsContext &gc, const wxPoint &pos, int ifnt, double points, unsigned int dpi,
                    const wxString &text, const wxColour &c = *wxBLACK, double angle = 0.0);

class wxFreeTypeDemo : public wxWindow {
    std::vector<int> faces;

    int Face(int idx);

public:
    wxFreeTypeDemo(wxWindow *parent);

    static void GenerateTTFBinaryFontData();

    void OnPaint(wxPaintEvent &);

    void OnSize(wxSizeEvent &);

DECLARE_EVENT_TABLE();
};

#endif
