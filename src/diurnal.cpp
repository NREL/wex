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

#include <algorithm>

#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/tokenzr.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "wex/utils.h"
#include "wex/diurnal.h"

BEGIN_EVENT_TABLE(wxDiurnalPeriodCtrl, wxWindow)
                EVT_PAINT(wxDiurnalPeriodCtrl::OnPaint)
                EVT_ERASE_BACKGROUND(wxDiurnalPeriodCtrl::OnErase)
                EVT_SIZE(wxDiurnalPeriodCtrl::OnResize)
                EVT_CHAR(wxDiurnalPeriodCtrl::OnChar)
                EVT_KEY_DOWN(wxDiurnalPeriodCtrl::OnKeyDown)
                EVT_LEFT_DOWN(wxDiurnalPeriodCtrl::OnMouseDown)
                EVT_LEFT_UP(wxDiurnalPeriodCtrl::OnMouseUp)
                EVT_MOTION(wxDiurnalPeriodCtrl::OnMouseMove)
                EVT_KILL_FOCUS(wxDiurnalPeriodCtrl::OnLostFocus)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(wxEVT_DIURNALPERIODCTRL_CHANGE)

#ifndef MIN
#define MIN(a, b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif

#define SCHED_FONT wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD)

#define VALUE(r, c) m_data.at(m_ncols*(r)+(c))

wxDiurnalPeriodCtrl::wxDiurnalPeriodCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &sz,
    size_t nrows, size_t ncols, size_t cellWidth, size_t cellHeight)
        : wxWindow(parent, id, pos, sz, wxWANTS_CHARS) {
    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    m_mouseDown = false;
    m_rowHeaderSize = 30;
    m_colHeaderSize = 21;
    m_cellWidth = m_cellWidthNom = cellWidth;
    m_cellHeight = m_cellHeightNom = cellHeight;
    m_nrows = nrows;
    m_ncols = ncols;
    m_selStartR = m_selStartC = m_selEndR = m_selEndC = -1;
    m_min = 0;
    m_max = 9; // can be set to higher value in SetMinMax
    m_data.resize(nrows * ncols);

    SetupTOUGrid();
}

wxDiurnalPeriodCtrl::~wxDiurnalPeriodCtrl() {
    /* nothing to do */
}

bool wxDiurnalPeriodCtrl::Enable(bool enable) {
    bool ret_val = wxWindow::Enable(enable);
    Refresh();
    return ret_val;
}

void wxDiurnalPeriodCtrl::AddColour(const wxColour &c) {
    m_colours.push_back(c);
}

bool wxDiurnalPeriodCtrl::GetColour(int i, wxColour &c) {
    if (i >= 0 && i < (int) m_colours.size()) {
        c = m_colours[i];
        return true;
    } else
        return false;
}

void wxDiurnalPeriodCtrl::SetColours(const std::vector<wxColour>& colors) {
    SetMinMax(0, colors.size() - 1, true);

    m_colours.clear();
    for (std::vector<int>::size_type i = 0; i != colors.size(); i++) {
        AddColour(colors[i]);
    }
}

void wxDiurnalPeriodCtrl::Set(size_t r, size_t c, int val) {
    if (r < m_nrows && c < m_ncols)
        VALUE(r, c) = val;
}

void wxDiurnalPeriodCtrl::SetMin(int min) {
    SetMinMax(min, m_max);
}

int wxDiurnalPeriodCtrl::GetMin() {
    return m_min;
}

void wxDiurnalPeriodCtrl::SetMax(int max) {
    SetMinMax(m_min, max);
}

int wxDiurnalPeriodCtrl::GetMax() {
    return m_max;
}

void wxDiurnalPeriodCtrl::SetMinMax(int min, int max, bool clamp) {
    m_min = min;
    m_max = max;

    if (!clamp)
        return;

    for (size_t r = 0; r < m_nrows; r++)
        for (size_t c = 0; c < m_ncols; c++) {
            if (VALUE(r, c) < min) VALUE(r, c) = min;
            if (VALUE(r, c) > max) VALUE(r, c) = max;
        }
}

int wxDiurnalPeriodCtrl::Get(size_t r, size_t c) const {
    if (r < m_nrows && c < m_ncols)
        return VALUE(r, c);
    else
        return -1;
}

void wxDiurnalPeriodCtrl::Set(int val) {
    for (size_t r = 0; r < m_nrows; r++)
        for (size_t c = 0; c < m_ncols; c++)
            VALUE(r, c) = val;
    Refresh();
}

void wxDiurnalPeriodCtrl::AddRowLabel(const wxString &s) {
    m_rowLabels.Add(s);
}

void wxDiurnalPeriodCtrl::AddColLabel(const wxString &s) {
    m_colLabels.Add(s);
}

void wxDiurnalPeriodCtrl::ClearLabels() {
    m_rowLabels.Clear();
    m_colLabels.Clear();
}

void wxDiurnalPeriodCtrl::ClearRowLabels() {
    m_rowLabels.Clear();
}

void wxDiurnalPeriodCtrl::ClearColLabels() {
    m_colLabels.Clear();
}

void wxDiurnalPeriodCtrl::OnErase(wxEraseEvent &) {
    /* nothing to do */
}

void wxDiurnalPeriodCtrl::OnPaint(wxPaintEvent &) {
    wxAutoBufferedPaintDC dc(this);
    wxSize sz(GetClientSize());
    wxRect geom(0, 0, sz.GetWidth(), sz.GetHeight());

    dc.SetBackground(GetBackgroundColour());
    dc.Clear();

    int r, c;
    int rows = m_nrows;
    int cols = m_ncols;

    dc.SetFont(SCHED_FONT);

    dc.SetPen(*wxTRANSPARENT_PEN);
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            int x = m_rowHeaderSize + c * m_cellWidth;
            int y = m_colHeaderSize + r * m_cellHeight;

            int selrs = MIN(m_selStartR, m_selEndR);
            int selcs = MIN(m_selStartC, m_selEndC);
            int selre = MAX(m_selStartR, m_selEndR);
            int selce = MAX(m_selStartC, m_selEndC);

            bool sel = (r >= selrs && r <= selre && c >= selcs && c <= selce);

            if (x >= geom.width || y >= geom.height)
                break;

            int val = VALUE(r, c);
            if ((val >= 0 && val - 1 < (int) m_colours.size()) || sel) {
                if (IsThisEnabled())
                    dc.SetBrush(wxBrush(sel ? wxColour(0, 114, 198) : m_colours[val]));
                else
                    dc.SetBrush(wxBrush(*wxLIGHT_GREY));

                dc.DrawRectangle(geom.x + x, geom.y + y, m_cellWidth, m_cellHeight);
            }

            wxString buf;
            buf << val;
            int textW, textH;
            dc.GetTextExtent(buf, &textW, &textH);
            x += m_cellWidth / 2 - textW / 2;
            y += m_cellHeight / 2 - textH / 2;

            wxColour unselColour;
            if (m_colours[val] == wxColour(0, 51, 0)
             || m_colours[val] == wxColour(81, 60, 0)
             || m_colours[val] == wxColour(106, 0, 28)
             || m_colours[val] == wxColour(38, 0, 126)
             || m_colours[val] == wxColour(0, 111, 142)) {
                unselColour = *wxWHITE;
            }
            else {
                unselColour = *wxBLACK;
            }
            dc.SetTextForeground(sel ? *wxWHITE : unselColour);
            dc.DrawText(buf, geom.x + x, geom.y + y);
        }
    }

    dc.SetPen(*wxWHITE_PEN);
    dc.SetTextForeground(wxColour(160, 160, 160));

    for (r = 0; r <= rows; r++) {
        dc.DrawLine(geom.x, geom.y + m_colHeaderSize + r * m_cellHeight,
                    geom.x + m_rowHeaderSize + cols * m_cellWidth, geom.y + m_colHeaderSize + r * m_cellHeight);

        if (r < (int) m_rowLabels.Count() && r < rows) {
            int yoff = m_cellHeight / 2 - dc.GetCharHeight() / 2;
            dc.DrawText(m_rowLabels[r], geom.x + 2, geom.y + m_colHeaderSize + r * m_cellHeight + yoff);
        }
    }

    for (c = 0; c <= cols; c++) {
        dc.DrawLine(geom.x + m_rowHeaderSize + c * m_cellWidth, geom.y,
                    geom.x + m_rowHeaderSize + c * m_cellWidth, geom.y + m_colHeaderSize + rows * m_cellHeight);

        if (c < (int) m_colLabels.Count() && c < cols) {
            int xoff = m_cellWidth / 2 - dc.GetCharHeight() / 2;
            dc.DrawRotatedText(m_colLabels[c], geom.x + m_rowHeaderSize + c * m_cellWidth + xoff,
                               geom.y + m_colHeaderSize - 2, 90);
        }
    }
}

void wxDiurnalPeriodCtrl::OnResize(wxSizeEvent &) {
    Refresh();
}

void wxDiurnalPeriodCtrl::UpdateLayout() {
    wxClientDC dc(const_cast<wxDiurnalPeriodCtrl *>(this));
    dc.SetFont(SCHED_FONT);

    int r, c;
    int rows = m_nrows;
    int cols = m_ncols;

    m_rowHeaderSize = 0;
    for (r = 0; r < rows && r < (int) m_rowLabels.Count(); r++) {
        wxSize tsz(dc.GetTextExtent(m_rowLabels[r]));
        if (tsz.x > m_rowHeaderSize)
            m_rowHeaderSize = tsz.x;
    }

    m_colHeaderSize = 0;
    for (c = 0; c < cols && c < (int) m_colLabels.Count(); c++) {
        wxSize tsz(dc.GetTextExtent(m_colLabels[c]));
        if (tsz.x > m_colHeaderSize)
            m_colHeaderSize = tsz.x;
    }

    double xScale, yScale;
    wxDevicePPIToScale(dc.GetPPI(), &xScale, &yScale);

    m_cellWidth = (int)(m_cellWidthNom * xScale);
    m_cellHeight = (int)(m_cellHeightNom * yScale);
    m_rowHeaderSize += (int) (6 * yScale);
    m_colHeaderSize += (int) (6 * xScale);
}

wxSize wxDiurnalPeriodCtrl::DoGetBestSize() const {
    const_cast<wxDiurnalPeriodCtrl *>(this)->UpdateLayout();
    return wxSize(m_rowHeaderSize + m_ncols * m_cellWidth,
                  m_colHeaderSize + m_nrows * m_cellHeight);
}

int wxDiurnalPeriodCtrl::ScheduleCharToInt(char c) {
    int ret = 0;
    switch (c) {
        case '0':
            ret = 0;
            break;
        case '1':
            ret = 1;
            break;
        case '2':
            ret = 2;
            break;
        case '3':
            ret = 3;
            break;
        case '4':
            ret = 4;
            break;
        case '5':
            ret = 5;
            break;
        case '6':
            ret = 6;
            break;
        case '7':
            ret = 7;
            break;
        case '8':
            ret = 8;
            break;
        case '9':
            ret = 9;
            break;
        case 'A':
        case 'a':
        case ':':
            ret = 10;
            break;
        case 'B':
        case 'b':
        case '=':
            ret = 11;
            break;
        case 'C':
        case 'c':
        case '<':
            ret = 12;
            break;
    }
    return ret;
}

char wxDiurnalPeriodCtrl::ScheduleIntToChar(int d) {
    char ret = '0';
    switch (d) {
        case 0:
            ret = '0';
            break;
        case 1:
            ret = '1';
            break;
        case 2:
            ret = '2';
            break;
        case 3:
            ret = '3';
            break;
        case 4:
            ret = '4';
            break;
        case 5:
            ret = '5';
            break;
        case 6:
            ret = '6';
            break;
        case 7:
            ret = '7';
            break;
        case 8:
            ret = '8';
            break;
        case 9:
            ret = '9';
            break;
        case 10:
            ret = 'A';
            break;
        case 11:
            ret = 'B';
            break;
        case 12:
            ret = 'C';
            break;
    }
    return ret;
}

bool wxDiurnalPeriodCtrl::Schedule(const wxString &sched) {
    if (sched.Len() != m_nrows * m_ncols)
        return false;

    for (size_t r = 0; r < m_nrows; r++)
        for (size_t c = 0; c < m_ncols; c++)
            VALUE(r, c) = ScheduleCharToInt(sched[r * m_ncols + c]);

    Refresh();

    return true;
}

wxString wxDiurnalPeriodCtrl::Schedule() const {
    wxString buf;
    for (size_t r = 0; r < m_nrows; r++)
        for (size_t c = 0; c < m_ncols; c++)
            buf << ScheduleIntToChar(VALUE(r, c));

    return buf;
}

void wxDiurnalPeriodCtrl::OnKeyDown(wxKeyEvent &evt) {
    if (evt.GetModifiers() == wxMOD_CONTROL) {
        switch (evt.GetKeyCode()) {
            case 'C':
                Copy();
                return;
            case 'V':
                Paste();
                return;
            default:
                break;
        }
    }
    evt.Skip();
}

void wxDiurnalPeriodCtrl::Copy() {
    if (wxTheClipboard->Open()) {
        // This data objects are held by the clipboard,
        // so do not delete them in the app.
        wxString tsv;
        for (size_t r = 0; r < m_nrows; r++) {
            for (size_t c = 0; c < m_ncols; c++) {
                tsv += wxString::Format("%d", VALUE(r, c));
                if (c < m_ncols - 1)
                    tsv += '\t';
            }
            tsv += '\n';
        }

        wxTheClipboard->SetData(new wxTextDataObject(tsv));
        wxTheClipboard->Close();
    }
}

void wxDiurnalPeriodCtrl::Paste() {
    if (wxTheClipboard->Open()) {
        wxTextDataObject tobj;
        wxTheClipboard->GetData(tobj);
        wxString sched = tobj.GetText();
        wxArrayString as = wxStringTokenize(sched, "\n\t");
        if (as.Count() >= (m_nrows * m_ncols)) {
            int as_ndx = 0;
            for (size_t r = 0; r < m_nrows; r++) {
                for (size_t c = 0; c < m_ncols; c++) {
                    long val = 0;
                    if (as_ndx < (int) as.Count())
                        as[as_ndx].ToLong(&val);
                    if ((val <= m_max) && (val >= m_min))
                        VALUE(r, c) = val;
                    as_ndx++;
                }
            }
        }
        wxCommandEvent change(wxEVT_DIURNALPERIODCTRL_CHANGE, this->GetId());
        change.SetEventObject(this);
        GetEventHandler()->ProcessEvent(change);
        Refresh();
    }
}

#ifdef __WXMSW__
bool wxDiurnalPeriodCtrl::MSWShouldPreProcessMessage(WXMSG* msg)
{
    // windows processing of ctrl+c and ctrl+v for copy and paste - from textctrl
    // check for our special keys here: if we don't do it and the parent frame
    // uses them as accelerators, they wouldn't work at all, so we disable
    // usual preprocessing for them
    if (msg->message == WM_KEYDOWN)
    {
        const WPARAM vkey = msg->wParam;
        if (HIWORD(msg->lParam) & KF_ALTDOWN)
        {
            // Alt-Backspace is accelerator for "Undo"
            if (vkey == VK_BACK)
                return false;
        }
        else // no Alt
        {
            // we want to process some Ctrl-foo and Shift-bar but no key
            // combinations without either Ctrl or Shift nor with both of them
            // pressed
            const int ctrl = wxIsCtrlDown();
            if (ctrl)
            {
                switch (vkey)
                {
                case 'C':
                case 'V':
                    return false;
                }
            }
        }
    }

    return wxWindow::MSWShouldPreProcessMessage(msg);
}
#endif

void wxDiurnalPeriodCtrl::OnChar(wxKeyEvent &evt) {
    size_t selrs = MIN(m_selStartR, m_selEndR);
    size_t selcs = MIN(m_selStartC, m_selEndC);
    size_t selre = MAX(m_selStartR, m_selEndR);
    size_t selce = MAX(m_selStartC, m_selEndC);

    int key = evt.GetKeyCode();
    if ((ScheduleCharToInt(key) >= m_min) && (ScheduleCharToInt(key) <= m_max)) {
        for (size_t r = selrs; r <= selre && r < m_nrows; r++)
            for (size_t c = selcs; c <= selce && c < m_ncols; c++)
                VALUE(r, c) = ScheduleCharToInt(key);

        Refresh();

        wxCommandEvent change(wxEVT_DIURNALPERIODCTRL_CHANGE, this->GetId());
        change.SetEventObject(this);
        GetEventHandler()->ProcessEvent(change);
    }
}

void wxDiurnalPeriodCtrl::OnMouseDown(wxMouseEvent &evt) {
    m_selStartC = (evt.GetX() - m_rowHeaderSize) / m_cellWidth;
    m_selStartR = (evt.GetY() - m_colHeaderSize) / m_cellHeight;

    if (m_selStartC < 0 || m_selStartC >= static_cast<int>(m_ncols) ||
        m_selStartR < 0 || m_selStartR >= static_cast<int>(m_nrows) ||
        evt.GetX() < m_rowHeaderSize || evt.GetY() < m_colHeaderSize) {
        m_selStartC = m_selStartR = -1;
    } else
        this->SetFocus();

    m_selEndR = m_selStartR;
    m_selEndC = m_selStartC;
    m_mouseDown = true;
    Refresh();
}

void wxDiurnalPeriodCtrl::OnMouseUp(wxMouseEvent &) {
    m_mouseDown = false;
}

void wxDiurnalPeriodCtrl::OnMouseMove(wxMouseEvent &evt) {
    if (!m_mouseDown)
        return;

    int c = (evt.GetX() - m_rowHeaderSize) / m_cellWidth;
    int r = (evt.GetY() - m_colHeaderSize) / m_cellHeight;

    if (r >= 0 && r < static_cast<int>(m_nrows) &&
        c >= 0 && c < static_cast<int>(m_ncols)) {
        m_selEndR = r;
        m_selEndC = c;
        Refresh();
    }
}

void wxDiurnalPeriodCtrl::OnLostFocus(wxFocusEvent &) {
    m_selEndR = m_selStartR = -1;
    m_selEndC = m_selStartC = -1;
    Refresh();
}

void wxDiurnalPeriodCtrl::SetData(double *data, size_t nr, size_t nc) {
    if (nr == m_nrows && nc == m_ncols) {
        m_data.assign(data, data + nr * nc);
        Refresh();
    }
}

double *wxDiurnalPeriodCtrl::GetData(size_t *nr, size_t *nc) {
    *nr = m_nrows;
    *nc = m_ncols;
    return &m_data[0];
}

void wxDiurnalPeriodCtrl::SetupTOUGrid() {
    SetMinMax(0, 9, true);

    m_colours.clear();
    AddColour(wxColour(226, 169, 141));     // 0
    AddColour(wxColour(143, 226, 170));     // 1
    AddColour(wxColour(128, 179, 179));     // 2
    AddColour(wxColour(196, 148, 49));      // 3
    AddColour(wxColour(44, 175, 133));      // 4
    AddColour(wxColour(219, 219, 112));     // 5
    AddColour(wxColour(206, 57, 57));       // 6
    AddColour(wxColour(94, 136, 81));       // 7
    AddColour(wxColour(225, 136, 225));     // 8
    AddColour(wxColour(255, 60, 157));      // 9
    AddColour(wxColour(86, 172, 214));      // 10
    AddColour(wxColour(254, 235, 97));      // 11
    AddColour(wxColour(91, 83, 252));       // 12

    m_rowLabels.clear();
    AddRowLabel("Jan");
    AddRowLabel("Feb");
    AddRowLabel("Mar");
    AddRowLabel("Apr");
    AddRowLabel("May");
    AddRowLabel("Jun");
    AddRowLabel("Jul");
    AddRowLabel("Aug");
    AddRowLabel("Sep");
    AddRowLabel("Oct");
    AddRowLabel("Nov");
    AddRowLabel("Dec");

    m_colLabels.clear();
    AddColLabel("12am");
    AddColLabel("1am");
    AddColLabel("2am");
    AddColLabel("3am");
    AddColLabel("4am");
    AddColLabel("5am");
    AddColLabel("6am");
    AddColLabel("7am");
    AddColLabel("8am");
    AddColLabel("9am");
    AddColLabel("10am");
    AddColLabel("11am");
    AddColLabel("12pm");
    AddColLabel("1pm");
    AddColLabel("2pm");
    AddColLabel("3pm");
    AddColLabel("4pm");
    AddColLabel("5pm");
    AddColLabel("6pm");
    AddColLabel("7pm");
    AddColLabel("8pm");
    AddColLabel("9pm");
    AddColLabel("10pm");
    AddColLabel("11pm");

    UpdateLayout();
    InvalidateBestSize();
}
