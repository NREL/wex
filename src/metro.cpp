#include <wx/settings.h>
#include <wx/fontenum.h>
#include <wx/dcbuffer.h>

#include "wex/metro.h"

wxFont wxMetroTheme::NormalFont( int size )
{
	wxFont font( wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

	if ( size > 1 )
		font.SetPointSize( size );

	if ( wxFontEnumerator::IsValidFacename( "Segoe UI" ) )
		font.SetFaceName( "Segoe UI" );

	return font;
}

wxFont wxMetroTheme::LightFont( int size )
{
	wxFont font( wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

	if ( size > 1 )
		font.SetPointSize( size );

	if ( wxFontEnumerator::IsValidFacename( "Segoe UI Light" ) )
		font.SetFaceName( "Segoe UI Light" );

	return font;
}

wxColour wxMetroTheme::Background()
{
	return *wxWHITE;
}

wxColour wxMetroTheme::Foreground()
{
	return wxColour( 3, 100, 181 ); // "nrel blue" colour
}

wxColour wxMetroTheme::AccentColour()
{
	return wxColour( 255, 148, 0 ); // "nrel orange" colour
}

wxColour wxMetroTheme::TextColour()
{
	return wxColour( 140, 140, 140 ); // "medium gray"
}

wxColour wxMetroTheme::HighlightColour()
{
	return wxColour(224,232,246);
}

wxColour wxMetroTheme::SelectColour()
{
	return wxColour(193,210,238);
}



BEGIN_EVENT_TABLE(wxMetroButton, wxWindow)
	EVT_PAINT(wxMetroButton::OnPaint)
	EVT_SIZE(wxMetroButton::OnResize)
	EVT_LEFT_DOWN(wxMetroButton::OnLeftDown)
	EVT_LEFT_DCLICK( wxMetroButton::OnLeftDown )
	EVT_LEFT_UP(wxMetroButton::OnLeftUp)
	EVT_ENTER_WINDOW(wxMetroButton::OnEnter)
	EVT_LEAVE_WINDOW(wxMetroButton::OnLeave)
END_EVENT_TABLE()


wxMetroButton::wxMetroButton(wxWindow *parent, int id, const wxString &label, const wxPoint &pos, const wxSize &sz)
	: wxWindow(parent, id, pos, sz)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	m_label = label;
	m_state = 0;  // state: 0=normal, 1=hover, 2=click
	m_pressed = false;

	SetBackgroundColour( wxMetroTheme::Background() );
	SetForegroundColour( wxMetroTheme::Foreground() );
	SetFont( wxMetroTheme::NormalFont() );
}

wxSize wxMetroButton::DoGetBestSize() const
{
	wxClientDC dc( const_cast<wxMetroButton*>( this ) );
	dc.SetFont( GetFont() );
	int tw, th;
	dc.GetTextExtent( m_label, &tw, &th );
	return wxSize( tw + 20, th + 10 );
}

void wxMetroButton::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC dc(this);
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);

	dc.SetBrush( wxBrush( m_state == 1 ? wxMetroTheme::HighlightColour() : GetBackgroundColour() ) ) ;
	dc.SetPen( wxPen( GetForegroundColour(), 3) );
	dc.DrawRectangle(0,0,cwidth,cheight);

	int tw, th;
	dc.GetTextExtent( m_label, &tw, &th );
	dc.SetFont( GetFont() );
	dc.SetTextForeground( GetForegroundColour() );

	int offset = 0;
	if ( m_state == 2 ) offset = 1;

	dc.DrawText( m_label, cwidth/2 - tw/2+offset, cheight/2 - th/2+offset );
}

void wxMetroButton::OnResize(wxSizeEvent &)
{
	Refresh();
}

void wxMetroButton::OnLeftDown(wxMouseEvent &)
{
	SetFocus();
	CaptureMouse();
	m_pressed = true;
	m_state = 2;
	Refresh();
}

void wxMetroButton::OnLeftUp(wxMouseEvent &evt)
{
	if (HasCapture())
	{
		m_pressed = false;
		ReleaseMouse();
		m_state = 0;
		Refresh();

		wxSize sz = GetClientSize();
		int x = evt.GetX();
		int y = evt.GetY();
		if (x>=0&&y>=0 && x<=sz.GetWidth() && y<=sz.GetHeight())
		{
			wxCommandEvent press(wxEVT_COMMAND_BUTTON_CLICKED, this->GetId() );
			press.SetEventObject(this);
			GetEventHandler()->ProcessEvent(press);
		}
	}

}

void wxMetroButton::OnEnter(wxMouseEvent &)
{
	m_state = m_pressed?2:1;
	Refresh();
}

void wxMetroButton::OnLeave(wxMouseEvent &)
{
	m_state = 0;
	Refresh();
}
