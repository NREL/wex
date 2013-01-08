#ifndef __schedctrl_h
#define __schedctrl_h

#include <vector>
#include <wx/window.h>

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE( wxEVT_SCHEDCTRL_CHANGE, 0)
END_DECLARE_EVENT_TYPES()

#define EVT_SCHEDCTRL(id, func) EVT_COMMAND(id, wxEVT_SCHEDCTRL_CHANGE, func)

class wxSchedCtrl : public wxWindow
{
public:
	wxSchedCtrl(wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &sz = wxDefaultSize);
	virtual ~wxSchedCtrl();

	void SetupTOUGrid();
	void SetupDefaultColours();

	void AddColour(const wxColour &c);
	bool GetColour(int i, wxColour &c);
	void SetMinMax(int min, int max, bool clamp=false);
	void Set(int r, int c, int val);
	void Set(int val);
	int Get(int r, int c) const;
	void SetGrid(int nr, int nc);
	int NRows() const;
	int NCols() const;
	void AddRowLabel(const wxString &s);
	void AddColLabel(const wxString &s);
	void ClearLabels();
	void ClearRowLabels();
	void ClearColLabels();
	bool Schedule(const wxString &sched);
	wxString Schedule() const;
	
	virtual wxSize DoGetBestSize() const;
	void Draw(wxDC &dc, const wxRect &geom);

	static bool TranslateSchedule( int tod[8760],
		const char *weekday, const char *weekend, 
		int min_val=0, int max_val=9 );

private:
	void AutosizeHeaders();

	void OnPaint(wxPaintEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnChar(wxKeyEvent &evt);
	void OnMouseDown(wxMouseEvent &evt);
	void OnMouseUp(wxMouseEvent &evt);
	void OnMouseMove(wxMouseEvent &evt);
	void OnLostFocus(wxFocusEvent &evt);

	std::vector<int> m_data;
	int m_cols;

	bool m_hasFocus, m_mouseDown;
	int m_rowHeaderSize, m_colHeaderSize, m_cellSize;

	std::vector<wxColour> m_colours;
	int m_selStartR, m_selStartC, m_selEndR, m_selEndC;
	wxArrayString m_rowLabels;
	wxArrayString m_colLabels;
	bool m_colLabelsVertical;
	bool m_autosizeHeaders;
	int m_min, m_max;

	DECLARE_EVENT_TABLE()
};

#endif
