#ifndef __wxribbonbook_h
#define __wxribbonbook_h

#include <vector>
#include <wx/bitmap.h>
#include <wx/panel.h>

class wxSimplebook;
class wxRibbonRenderer;

// sends EVT_NOTEBOOK_PAGE_CHANGED event

class wxRibbonNotebook : public wxPanel
{
	friend class wxRibbonRenderer;
public:
	wxRibbonNotebook(wxWindow *parent, int id=-1, const wxPoint &pos=wxDefaultPosition, const wxSize &sz=wxDefaultSize);

	void AddControlPanel(wxPanel *panel, int width=-1);
	void AddPage(wxWindow *win, const wxString &text, const wxBitmap &icon, bool active=false);
	void AddScrolledPage(wxWindow *win, const wxString &text, const wxBitmap &icon, bool active=false);
	int GetPageCount();
	wxWindow *RemovePage( int index );
	void DeletePage( int index );
	int GetPageIndex(wxWindow *win);
	wxWindow *GetPage( int index );

	int GetSelection();
	void SetSelection(int id);
	void SetIcon(int id, const wxBitmap &icon);
	void SetOverlay(int id, const wxBitmap &overlay, unsigned int position = wxBOTTOM|wxRIGHT );
	void SetText(int id, const wxString &text);

	void SetHeaderText( const wxString &s ) { m_headerText = s; Refresh(); }

	void SetRibbonColour( const wxColour &c ) { m_ribbonColour = c; Refresh(); }
	void SetTextOnRight( bool b ) { m_textOnRight = b; Reposition(); Refresh(); }
	void SetSelectColour( const wxColour &c ) { m_selectColour = c; Refresh(); }
	void SetHighlightColour( const wxColour &c ) { m_highlightColour = c; Refresh(); }
	void SetTextColour( const wxColour &c ) { m_textColour = c; Refresh(); }
	void SetRibbonHeight( int pixels ) { m_ribbonHeight = pixels; Reposition(); }
	void SetItemSpacing( int pixels ) { m_xSpacing = pixels; Reposition(); }
	void SetLeftOffset( int pixels ) { m_xOffset = pixels; Reposition(); }
	void SetBorder( int pixels ) { m_border = pixels; Reposition(); Refresh(); }

	void RedrawAll();
	
private:
	wxPanel *m_ctrlPanel;
	int m_ctrlPanelWidth;
	wxRibbonRenderer *m_renderer;	
	wxSimplebook *m_flipper;

	wxColour m_ribbonColour;
	wxColour m_selectColour, m_highlightColour, m_textColour;
	wxString m_headerText;
	bool m_textOnRight;
	int m_ribbonHeight;
	int m_xSpacing, m_xOffset, m_border;

	struct page_info
	{
		wxString text;
		wxBitmap icon;
		wxBitmap overlay;
		unsigned int overlay_pos;
		wxWindow *scroll_win;
	};

	std::vector<page_info> m_pageList;

	void Reposition();
	void OnSize(wxSizeEvent &evt);

	DECLARE_EVENT_TABLE()
};

#endif
