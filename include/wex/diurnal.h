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

#ifndef __diurnal_h
#define __diurnal_h

#include <vector>
#include <wx/window.h>

// Diurnal Period control from sched control in wex

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_DIURNALPERIODCTRL_CHANGE, 0)
END_DECLARE_EVENT_TYPES()

#define EVT_DIURNALPERIODCTRL(id, func) EVT_COMMAND(id, wxEVT_DIURNALPERIODCTRL_CHANGE, func)

class wxDiurnalPeriodCtrl : public wxWindow
{
public:
	wxDiurnalPeriodCtrl(wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &sz = wxDefaultSize);
	virtual ~wxDiurnalPeriodCtrl();

	bool Enable(bool enable = true);

	void SetupTOUGrid();
	void SetupDefaultColours();

	void AddColour(const wxColour &c);
	bool GetColour(int i, wxColour &c);
	void SetMinMax(int min, int max, bool clamp = false);
	void Set(size_t r, size_t c, int val);
	void Set(int val);
	int Get(size_t r, size_t c) const;
	void AddRowLabel(const wxString &s);
	void AddColLabel(const wxString &s);
	void ClearLabels();
	void ClearRowLabels();
	void ClearColLabels();
	void SetData(float *data, size_t nr, size_t nc);
	float *GetData(size_t *nr, size_t *nc);

	bool Schedule(const wxString &sched);
	wxString Schedule() const;

	void SetMin(int min);
	int GetMin();

	void SetMax(int max);
	int GetMax();

	static int ScheduleCharToInt(char c);
	static char ScheduleIntToChar(int d);

	virtual wxSize DoGetBestSize() const;

private:

	void OnErase(wxEraseEvent &);
	void OnPaint(wxPaintEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnKeyDown(wxKeyEvent &evt);
	void OnChar(wxKeyEvent &evt);
	void OnMouseDown(wxMouseEvent &evt);
	void OnMouseUp(wxMouseEvent &evt);
	void OnMouseMove(wxMouseEvent &evt);
	void OnLostFocus(wxFocusEvent &evt);

#ifdef __WXMSW__
	virtual bool MSWShouldPreProcessMessage(WXMSG* msg);
#endif

	void Copy();
	void Paste();

	static const size_t m_nrows = 12;
	static const size_t m_ncols = 24;

	float m_data[m_nrows*m_ncols];
	int m_cols;

	bool m_hasFocus, m_mouseDown;
	int m_rowHeaderSize, m_colHeaderSize, m_cellSize;

	std::vector<wxColour> m_colours;
	int m_selStartR, m_selStartC, m_selEndR, m_selEndC;
	wxArrayString m_rowLabels;
	wxArrayString m_colLabels;
	bool m_colLabelsVertical;
	int m_min, m_max;

	void UpdateLayout();

	DECLARE_EVENT_TABLE()
};

#endif
