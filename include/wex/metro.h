#ifndef __wexmetro_h
#define __wexmetro_h

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


#endif

