#ifndef __snaplayout_h
#define __snaplayout_h

#include <vector>

#include <wx/scrolwin.h>

class wxSnapLayout : public wxScrolledWindow
{
public:
	wxSnapLayout( wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize );
	virtual ~wxSnapLayout();
	
	void Add( wxWindow *win, int width = -1, int height = -1 );
	void Delete( wxWindow * );
	void DeleteAll();	
	void ScrollTo( wxWindow * );	
	void AutoLayout();
	void ClearHighlights();
	void Highlight( wxWindow * );

private:

	struct layout_box {
		wxWindow *win;
		wxSize req;
		wxRect rect;
		wxRect active;
		int row;
		int col;
		bool highlight;
	};

	int Find( wxWindow *w );

	int m_space;
	std::vector<layout_box*> m_list;


	void OnLeftDown( wxMouseEvent & );
	void OnLeftUp( wxMouseEvent & );
	void OnMotion( wxMouseEvent & );
	void OnCaptureLost( wxMouseCaptureLostEvent & );
	void OnSize( wxSizeEvent & );
	void OnPaint( wxPaintEvent & );
	void OnErase( wxEraseEvent & );

	DECLARE_EVENT_TABLE();
};

#endif