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
	static wxColour DimHoverColour();
	static wxColour LightHoverColour();
	static wxColour AccentColour();
	static wxColour TextColour();
	static wxColour HighlightColour();
	static wxColour SelectColour();

	static wxBitmap LeftArrow();
	static wxBitmap DownArrow();
	static wxBitmap RightArrow();
	static wxBitmap UpArrow();
};

#define wxMB_RIGHTARROW 0x01
#define wxMB_LEFTARROW 0x02
#define wxMB_DOWNARROW 0x04
#define wxMB_UPARROW 0x08
#define wxMB_ALIGNLEFT 0x10

class wxMetroButton : public wxWindow
{
public:
	wxMetroButton(wxWindow *parent, int id, const wxString &label, const wxBitmap &bitmap = wxNullBitmap,
			const wxPoint &pos=wxDefaultPosition, const wxSize &sz=wxDefaultSize,
			long style = 0);
	
	wxSize DoGetBestSize() const;
private:
	wxString m_label;
	wxBitmap m_bitmap;
	int m_state;
	bool m_pressed;
	long m_style;

	void OnPaint(wxPaintEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnLeftDown(wxMouseEvent &evt);
	void OnLeftUp(wxMouseEvent &evt);
	void OnEnter(wxMouseEvent &evt);
	void OnLeave(wxMouseEvent &evt);
	void OnMotion(wxMouseEvent &evt);

	DECLARE_EVENT_TABLE()
};


class wxSimplebook;
class wxMetroNotebookRenderer;

// sends EVT_NOTEBOOK_PAGE_CHANGED event

class wxMetroNotebook : public wxPanel
{
	friend class wxMetroNotebookRenderer;
public:
	wxMetroNotebook(wxWindow *parent, int id=-1, 
		const wxPoint &pos=wxDefaultPosition, const wxSize &sz=wxDefaultSize );

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

