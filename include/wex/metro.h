#ifndef __wexmetro_h
#define __wexmetro_h

#include <vector>

#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/window.h>


class wxMetroThemeProvider
{
public:
	virtual ~wxMetroThemeProvider();
	virtual wxFont Font( int style, int size );
	virtual wxColour Colour( int id );
	virtual wxBitmap Bitmap( int id );
};


enum {
	// fonts
	wxMT_NORMAL,
	wxMT_LIGHT, 
	wxMT_SEMIBOLD,

	// colors
	wxMT_FOREGROUND,
	wxMT_BACKGROUND,
	wxMT_HOVER,
	wxMT_DIMHOVER,
	wxMT_LIGHTHOVER,
	wxMT_ACCENT,
	wxMT_TEXT,
	wxMT_ACTIVE,
	wxMT_SELECT,
	wxMT_HIGHLIGHT,

	// bitmaps
	wxMT_LEFTARROW,
	wxMT_RIGHTARROW,
	wxMT_DOWNARROW,
	wxMT_UPARROW
};

class wxMetroTheme
{
public:
	static void SetTheme( wxMetroThemeProvider *theme );
	static wxMetroThemeProvider &GetTheme();

	static wxFont Font( int style = wxMT_NORMAL, int size = -1 );
	static wxColour Colour( int id );
	static wxBitmap Bitmap( int id );
};


// widget styles for

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


// styles for wxMetroTabList, wxMetroNotebook
#define wxMT_LIGHTTHEME 0x01
#define wxMT_MENUBUTTONS 0x02

// issues wxEVT_LISTBOX when a new item is selected
// issues wxEVT_BUTTON when a menu button is clicked
class wxMetroTabList : public wxWindow
{
public:
	wxMetroTabList( wxWindow *parent, int id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = 0 );

	void Append( const wxString &label );
	void Insert( const wxString &label, size_t pos );
	void Remove( const wxString &label );
	int Find( const wxString &label );
	void Clear();
	size_t Count();
	wxString GetLabel( size_t idx );
	void SetLabel( size_t idx, const wxString &text );
	void SetSelection( size_t idx );
	size_t GetSelection();
	wxString GetStringSelection();
	
	wxSize DoGetBestSize() const;

protected:
	struct item
	{
		item( const wxString &l ) : label(l), x_start(0), width(0), shown(true) { }
		wxString label;
		int x_start;
		int width;
		bool shown;
	};

	std::vector<item> m_items;
	int m_dotdotWidth;
	bool m_dotdotHover;
	int m_selection;
	int m_pressIdx;
	int m_hoverIdx;
	long m_style;
	bool m_buttonHover;

	bool IsOverButton( int mouse_x, size_t i );

	void OnMenu( wxCommandEvent & );
	void OnSize( wxSizeEvent & );
	void OnPaint( wxPaintEvent & );
	void OnLeftDown( wxMouseEvent & );
	void OnLeftUp( wxMouseEvent & );
	void OnLeave( wxMouseEvent & );
	void OnMouseMove( wxMouseEvent & );

	void SwitchPage( size_t i );

	DECLARE_EVENT_TABLE();
};

// sends EVT_NOTEBOOK_PAGE_CHANGED event

class wxMetroNotebook : public wxPanel
{
public:
	wxMetroNotebook(wxWindow *parent, int id=-1, 
		const wxPoint &pos=wxDefaultPosition, const wxSize &sz=wxDefaultSize, long style = 0 );

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
	wxMetroTabList *m_list;	
	wxSimplebook *m_flipper;

	struct page_info
	{
		wxString text;
		wxWindow *scroll_win;
	};

	std::vector<page_info> m_pageList;

	void OnSize(wxSizeEvent &evt);
	void UpdateTabList();
	void OnTabList( wxCommandEvent & );
	void SwitchPage( size_t i );
	void ComputeScrolledWindows();

	DECLARE_EVENT_TABLE()
};

class wxMetroListBox : public wxScrolledWindow
{
public:
	wxMetroListBox( wxWindow *parent, int id, 
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );
	virtual ~wxMetroListBox();

	int Add( const wxString &label, const wxString &caption = wxEmptyString );
	void Delete( int idx );
	void Clear();
	int Find( const wxString &label );
	int Count();
	void SetLabel( int idx, const wxString &label );
	void SetCaption( int idx, const wxString &capt );
	wxString GetItem( int idx );
	wxString GetCaption( int idx );
	int GetSelection();
	void SetSelection( int idx );

private:
	struct item_data {
		wxString Label;
		wxString Caption;
	};

	std::vector<item_data> m_items;
	int m_hover;
	int m_selection;

	void OnPaint( wxPaintEvent &evt );
	void OnErase( wxEraseEvent &evt );
	void OnLeftDown( wxMouseEvent &evt );
	void OnMouseMove( wxMouseEvent &evt );
	void OnLeave( wxMouseEvent &evt );
	void OnDoubleClick( wxMouseEvent &evt );

	DECLARE_EVENT_TABLE();
};

#endif

