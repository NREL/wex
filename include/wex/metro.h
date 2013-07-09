#ifndef __wexmetro_h
#define __wexmetro_h

#include <vector>

#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/window.h>


class wxMetroTheme
{
public:
	static wxFont NormalFont( int size = -1 );
	static wxFont LightFont( int size = -1 );

	static wxColour Background();
	static wxColour Foreground();
	static wxColour HoverColour();
	static wxColour AccentColour();
	static wxColour TextColour();
	static wxColour HighlightColour();
	static wxColour SelectColour();
};

class wxMetroButton : public wxWindow
{
public:
	wxMetroButton(wxWindow *parent, int id, const wxString &label,
			const wxPoint &pos=wxDefaultPosition, const wxSize &sz=wxDefaultSize);

	wxSize DoGetBestSize() const;
private:
	wxString m_label;
	int m_state;
	bool m_pressed;

	void OnPaint(wxPaintEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnLeftDown(wxMouseEvent &evt);
	void OnLeftUp(wxMouseEvent &evt);
	void OnEnter(wxMouseEvent &evt);
	void OnLeave(wxMouseEvent &evt);

	DECLARE_EVENT_TABLE()
};


class wxSimplebook;
class wxMetroNotebookRenderer;

// sends EVT_NOTEBOOK_PAGE_CHANGED event

#define wxMNB_REVERSED 0x01
#define wxMNB_NEWTAB 0x02

class wxMetroNotebook : public wxPanel
{
	friend class wxMetroNotebookRenderer;
public:
	wxMetroNotebook(wxWindow *parent, int id=-1, 
		const wxPoint &pos=wxDefaultPosition, const wxSize &sz=wxDefaultSize,
		long style = 0 );

	void AddPage(wxWindow *win, const wxString &text, bool active=false);
	void AddScrolledPage(wxWindow *win, const wxString &text, bool active=false);
	int GetPageCount();
	wxWindow *RemovePage( int index );
	void DeletePage( int index );
	int GetPageIndex(wxWindow *win);
	wxWindow *GetPage( int index );

	int GetSelection();
	void SetSelection(int id);
	void SetText(int id, const wxString &text);

private:
	wxMetroNotebookRenderer *m_renderer;	
	wxSimplebook *m_flipper;
	long m_style;
	int m_tabHeight;

	int m_xSpacing, m_xOffset, m_xPadding, m_yPadding;

	struct page_info
	{
		wxString text;
		wxWindow *scroll_win;
	};

	std::vector<page_info> m_pageList;

	void Reposition();
	void OnSize(wxSizeEvent &evt);

	DECLARE_EVENT_TABLE()
};

#endif

