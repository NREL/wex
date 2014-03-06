#include <wx/settings.h>
#include <wx/fontenum.h>
#include <wx/dcbuffer.h>
#include <wx/simplebook.h>
#include <wx/scrolwin.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/event.h>

#include <algorithm>
#include "wex/metro.h"

#include "wex/icons/up_arrow_13.cpng"
#include "wex/icons/down_arrow_13.cpng"
#include "wex/icons/left_arrow_13.cpng"
#include "wex/icons/right_arrow_13.cpng"

static wxMetroThemeProvider g_basicTheme;
static wxMetroThemeProvider *g_otherTheme = 0;

void wxMetroTheme::SetTheme( wxMetroThemeProvider *theme )
{
	if ( g_otherTheme == theme ) return;
	if ( g_otherTheme != 0 ) delete g_otherTheme;
	g_otherTheme = theme;
}

wxMetroThemeProvider &wxMetroTheme::GetTheme()
{
	if ( g_otherTheme != 0 ) return *g_otherTheme;
	else return g_basicTheme;
}

wxFont wxMetroTheme::Font( int style, int size )
{
	return GetTheme().Font( style, size );
}

wxColour wxMetroTheme::Colour( int id )
{
	return GetTheme().Colour( id );
}

wxBitmap wxMetroTheme::Bitmap( int id )
{
	return GetTheme().Bitmap( id );
}



wxMetroThemeProvider::~wxMetroThemeProvider()
{
	 // nothing here
}

wxFont wxMetroThemeProvider::Font( int style, int size )
{
	wxFont font( wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

	if ( size > 1 )
		font.SetPointSize( size );

	wxString face = "Segoe UI";
	if ( style == wxMT_LIGHT ) face = "Segoe UI Light";
	else if ( style == wxMT_SEMIBOLD ) face = "Segoe UI Semibold";

	if ( wxFontEnumerator::IsValidFacename( face ) )
		font.SetFaceName( face );

	return font;
}

wxColour wxMetroThemeProvider::Colour( int id )
{
	switch( id )
	{
	case wxMT_FOREGROUND: return wxColour( 0, 114, 198 );
	case wxMT_BACKGROUND:  return *wxWHITE;
	case wxMT_HOVER: return wxColour( 0, 88, 153 );
	case wxMT_DIMHOVER: return wxColour( 0, 107, 186 );
	case wxMT_LIGHTHOVER: return wxColour( 231, 232, 238 );
	case wxMT_ACCENT: return wxColour( 255, 143, 50 );
	case wxMT_TEXT: return wxColour( 135, 135, 135 ); 
	case wxMT_ACTIVE: return wxColour( 0, 114, 198 );
	case wxMT_SELECT:  return wxColour(193,210,238);
	case wxMT_HIGHLIGHT: return wxColour(224,232,246);
	default: return wxNullColour;
	}
}

wxBitmap wxMetroThemeProvider::Bitmap( int id )
{
static wxBitmap left_arrow;
	if ( left_arrow.IsNull() ) left_arrow = wxBITMAP_PNG_FROM_DATA( left_arrow_13 );
	
static wxBitmap down_arrow;
	if ( down_arrow.IsNull() ) down_arrow = wxBITMAP_PNG_FROM_DATA( down_arrow_13 );
	
static wxBitmap right_arrow;
	if ( right_arrow.IsNull() ) right_arrow = wxBITMAP_PNG_FROM_DATA( right_arrow_13 );
	
static wxBitmap up_arrow;
	if ( up_arrow.IsNull() ) up_arrow = wxBITMAP_PNG_FROM_DATA( up_arrow_13 );

	switch ( id )
	{
	case wxMT_LEFTARROW: return left_arrow;
	case wxMT_RIGHTARROW: return right_arrow;
	case wxMT_UPARROW: return up_arrow;
	case wxMT_DOWNARROW: return down_arrow;
	default: return wxNullBitmap;
	}
}




BEGIN_EVENT_TABLE(wxMetroButton, wxWindow)
	EVT_PAINT(wxMetroButton::OnPaint)
	EVT_SIZE(wxMetroButton::OnResize)
	EVT_LEFT_DOWN(wxMetroButton::OnLeftDown)
	EVT_LEFT_DCLICK( wxMetroButton::OnLeftDown )
	EVT_LEFT_UP(wxMetroButton::OnLeftUp)
	EVT_ENTER_WINDOW(wxMetroButton::OnEnter)
	EVT_LEAVE_WINDOW(wxMetroButton::OnLeave)
	EVT_MOTION(wxMetroButton::OnMotion)
END_EVENT_TABLE()


wxMetroButton::wxMetroButton(wxWindow *parent, int id, const wxString &label, const wxBitmap &bitmap,
	const wxPoint &pos, const wxSize &sz, long style)
	: wxWindow(parent, id, pos, sz), m_label( label ), m_bitmap( bitmap ), m_style( style )
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	m_state = 0;  // state: 0=normal, 1=hover, 2=click
	m_pressed = false;

	SetFont( wxMetroTheme::Font( wxMT_NORMAL, 11) );
}

#define MB_SPACE 6

wxSize wxMetroButton::DoGetBestSize() const
{
	wxClientDC dc( const_cast<wxMetroButton*>( this ) );
	dc.SetFont( GetFont() );
	int tw, th;
	dc.GetTextExtent( m_label, &tw, &th );
	
	wxBitmap icon(wxNullBitmap);

	if ( m_style & wxMB_RIGHTARROW )
		icon = wxMetroTheme::Bitmap( wxMT_RIGHTARROW );
	else if ( m_style & wxMB_DOWNARROW )
		icon = wxMetroTheme::Bitmap( wxMT_DOWNARROW );
	else if ( m_style & wxMB_UPARROW )
		icon = wxMetroTheme::Bitmap( wxMT_UPARROW );
	else if ( m_style & wxMB_LEFTARROW )
		icon = wxMetroTheme::Bitmap( wxMT_LEFTARROW );

	if ( !icon.IsNull() )
	{
		tw += icon.GetWidth() + ( tw > 0 ? MB_SPACE : 0 );
		if ( icon.GetHeight() > th )
			th = icon.GetHeight();
	}

	if ( !m_bitmap.IsNull() )
	{
		tw += m_bitmap.GetWidth() + ( tw > 0 ? MB_SPACE : 0 );
		if ( m_bitmap.GetHeight() > th )
			th = m_bitmap.GetHeight();
	}

	int xspace = 0;
	if ( !m_label.IsEmpty() ) xspace = 20;
	else if ( !m_bitmap.IsNull() ) xspace = 10;
	else if ( !icon.IsNull() ) xspace = 5;

	return wxSize( tw + xspace, th + 10 );
}

void wxMetroButton::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC dc(this);
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);

	dc.SetBackground( m_state > 0 
		? wxMetroTheme::Colour( wxMT_HOVER ) 
		: wxMetroTheme::Colour( wxMT_FOREGROUND ) );
	dc.Clear();

	int tw, th;
	dc.GetTextExtent( m_label, &tw, &th );

	wxBitmap icon(wxNullBitmap);
	int xoffset_icon = 0;	
	int yoffset_icon = 0;
	int icon_width = 0;
	int icon_height = 0;

	if ( m_style & wxMB_RIGHTARROW )
		icon = wxMetroTheme::Bitmap( wxMT_RIGHTARROW );	
	else if ( m_style & wxMB_DOWNARROW )
	{
		icon = wxMetroTheme::Bitmap( wxMT_DOWNARROW );
		yoffset_icon++;
	}
	else if ( m_style & wxMB_UPARROW )
		icon = wxMetroTheme::Bitmap( wxMT_UPARROW );
	else if ( m_style & wxMB_LEFTARROW )
		icon = wxMetroTheme::Bitmap( wxMT_LEFTARROW );
	
	int xoffset = 0;
	if ( !icon.IsNull() )
	{
		int space = ( tw > 0 ? MB_SPACE : 0 );
		tw += icon.GetWidth() + space;
		xoffset -= icon.GetWidth() + space;
		icon_width = icon.GetWidth();
		icon_height = icon.GetHeight();
	}

	int bit_width = 0;
	int bit_height = 0;
	int bit_space = 0;
	int yoffset_bit = 0;
	if ( !m_bitmap.IsNull() )
	{
		bit_width = m_bitmap.GetWidth();
		bit_height = m_bitmap.GetHeight();
		tw += bit_width;
		xoffset -= bit_width;
		if ( !icon.IsNull() || !m_label.IsEmpty() )
		{
			tw += MB_SPACE;
			xoffset -= MB_SPACE;
		}
		bit_space = MB_SPACE/2;
	}

	dc.SetFont( GetFont() );
	dc.SetTextForeground( m_state == 1 
		? wxMetroTheme::Colour( wxMT_HIGHLIGHT ) 
		: wxMetroTheme::Colour( wxMT_BACKGROUND ) );
	
	int yoffset = 0;
	if ( !m_label.IsEmpty() && m_state == 2 )
		yoffset++;
	else if ( m_state == 2 && m_style & wxMB_LEFTARROW )
		xoffset_icon--;
	else if ( m_state ==2 && m_style & wxMB_RIGHTARROW )
		xoffset_icon++;
	else if ( m_state == 2 && m_style & wxMB_DOWNARROW )
		yoffset_icon++;
	else if ( m_state == 2 && !m_bitmap.IsNull() && m_label.IsEmpty() && icon.IsNull() )
		yoffset_bit++;

	int bit_x = cwidth/2 - tw/2;
	int text_x = bit_x + bit_width + bit_space;
	int icon_x = cwidth/2 + tw/2 - icon.GetWidth();

	if ( m_style & wxMB_ALIGNLEFT )
	{
		bit_x = 10;
		text_x = bit_x + bit_width + bit_space;
		icon_x = 10 + tw - icon.GetWidth();
	}

	if ( !m_bitmap.IsNull() )
		dc.DrawBitmap( m_bitmap, bit_x, cheight/2 - bit_height/2 + yoffset_bit );

	dc.DrawText( m_label, text_x, cheight/2 - th/2+yoffset );
	
	if ( !icon.IsNull() )
		dc.DrawBitmap( icon, icon_x+xoffset_icon, cheight/2 - icon_height/2 + yoffset_icon );
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

void wxMetroButton::OnMotion( wxMouseEvent &)
{
	if ( m_state != 1 )
	{
		m_state = 1;
		Refresh();
	}
}




enum { ID_TAB0 = wxID_HIGHEST+142 };


#define TB_SPACE 2
#define TB_XPADDING 10
#define TB_YPADDING 12

BEGIN_EVENT_TABLE(wxMetroTabList, wxWindow)
	EVT_LEFT_DCLICK( wxMetroTabList::OnLeftDown)
	EVT_LEFT_DOWN( wxMetroTabList::OnLeftDown )
	EVT_LEFT_UP( wxMetroTabList::OnLeftUp)
	EVT_MOTION(wxMetroTabList::OnMouseMove)
	EVT_PAINT( wxMetroTabList::OnPaint )
	EVT_SIZE( wxMetroTabList::OnSize )
	EVT_LEAVE_WINDOW( wxMetroTabList::OnLeave )
	EVT_MENU_RANGE( ID_TAB0, ID_TAB0+100, wxMetroTabList::OnMenu )
END_EVENT_TABLE()

wxMetroTabList::wxMetroTabList( wxWindow *parent, int id,
	const wxPoint &pos, const wxSize &size, long style )
	: wxWindow( parent, id, pos, size )
{
	SetBackgroundStyle( wxBG_STYLE_PAINT );
	m_dotdotWidth = 0;
	m_dotdotHover = false;
	m_pressIdx = -1;
	m_hoverIdx = -1;
	m_buttonHover = false;
	m_style = style;
	m_selection = 0;

	SetFont( wxMetroTheme::Font( wxMT_LIGHT, 16 ) );
}

void wxMetroTabList::Append( const wxString &label )
{
	m_items.push_back( item(label) );
}

void wxMetroTabList::Insert( const wxString &label, size_t pos )
{
	m_items.insert( m_items.begin()+pos, item(label) );
}

void wxMetroTabList::Remove( const wxString &label )
{
	int idx = Find( label );
	if ( idx >= 0 )
		m_items.erase( m_items.begin()+idx );
}

int wxMetroTabList::Find( const wxString &label )
{
	for ( size_t i=0;i<m_items.size();i++ )
		if ( m_items[i].label == label ) 
			return i;

	return -1;
}


void wxMetroTabList::ReorderLeft( size_t idx )
{
	if ( idx > 0 && idx < m_items.size() )
	{
		item x( m_items[idx-1] );
		m_items[idx-1] = m_items[idx];
		m_items[idx] = x;

		if ( m_selection == idx ) m_selection--;

		m_hoverIdx = -1;
		Refresh();
	}
}

void wxMetroTabList::ReorderRight( size_t idx )
{
	if ( idx >= 0 && idx < m_items.size()-1)
	{
		item x( m_items[idx+1] );
		m_items[idx+1] = m_items[idx];
		m_items[idx] = x;

		if ( m_selection == idx ) m_selection++;

		m_hoverIdx = -1;
		Refresh();
	}
}

wxString wxMetroTabList::GetLabel( size_t idx )
{
	if ( idx < m_items.size() ) return m_items[idx].label;
	else return wxEmptyString;
}

wxArrayString wxMetroTabList::GetLabels()
{
	wxArrayString list;
	for( size_t i=0;i<m_items.size();i++ )
		list.Add( m_items[i].label );
	return list;
}

void wxMetroTabList::SetLabel( size_t idx, const wxString &text )
{
	if ( idx < m_items.size() )
	{
		m_items[idx].label = text;
		Refresh();
	}
}

size_t wxMetroTabList::Count()
{
	return m_items.size();
}

void wxMetroTabList::Clear()
{
	m_items.clear();
}

void wxMetroTabList::SetSelection( size_t i )
{
	m_selection = i;
	if ( m_selection >= m_items.size() )
		m_selection = m_items.size()-1;
}

size_t wxMetroTabList::GetSelection()
{
	return m_selection;
}

wxString wxMetroTabList::GetStringSelection()
{
	if( m_selection >= 0 && m_selection < m_items.size() )
		return m_items[m_selection].label;
	else
		return wxEmptyString;
}

wxSize wxMetroTabList::DoGetBestSize() const
{
	wxClientDC dc( const_cast<wxMetroTabList*>( this ) );
	dc.SetFont( GetFont() );

	int width = 0;

	int button_width = 0;
	if ( m_style & wxMT_MENUBUTTONS )
		button_width = wxMetroTheme::Bitmap( wxMT_DOWNARROW ).GetWidth() + TB_SPACE + TB_SPACE;

	for ( size_t i=0;i<m_items.size(); i++ )
	{
		int tw, th;
		dc.GetTextExtent( m_items[i].label, &tw, &th );
		width += tw + TB_SPACE + TB_XPADDING;
	}

	int height = dc.GetCharHeight() + TB_YPADDING;

	return wxSize( width, height );
}

void wxMetroTabList::OnSize(wxSizeEvent &)
{
	Refresh();
}

void wxMetroTabList::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC dc(this);

	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);
	
	bool light = ( m_style & wxMT_LIGHTTHEME );

	dc.SetBackground( light ? *wxWHITE : wxMetroTheme::Colour( wxMT_FOREGROUND ) );
	dc.Clear();

	int x = TB_SPACE;

	m_dotdotWidth = 0;
	
	wxBitmap button_icon( wxNullBitmap );
	int button_width = 0;
	int button_height = 0;
	if ( m_style & wxMT_MENUBUTTONS )
	{
		button_icon = wxMetroTheme::Bitmap( wxMT_DOWNARROW );
		button_width = button_icon.GetWidth() + TB_SPACE + TB_SPACE;
		button_height = button_icon.GetHeight();
	}
	
	dc.SetFont( GetFont() );
	int CharHeight = dc.GetCharHeight();
	for (size_t i=0;i<m_items.size();i++)
	{
		int txtw, txth;
		dc.GetTextExtent( m_items[i].label, &txtw, &txth );
		m_items[i].x_start = x;
		m_items[i].width = txtw+TB_XPADDING+TB_XPADDING + button_width;
		x += m_items[i].width + TB_SPACE;

		if ( i > 0 && x > cwidth - 25 ) // 25 approximates width of '...'	
			m_dotdotWidth = 1;

		m_items[i].shown = ( m_dotdotWidth == 0 );
	}

	// compute size of '...'
	int dotdot_height = 0;
	if ( m_dotdotWidth > 0 )
	{
		wxFont font( wxMetroTheme::Font( wxMT_NORMAL, 18) );
		font.SetWeight( wxFONTWEIGHT_BOLD );
		dc.SetFont( font );
		dc.GetTextExtent( "...", &m_dotdotWidth, &dotdot_height );
		m_dotdotWidth += TB_SPACE + TB_SPACE;
		dc.SetFont( GetFont() ); // restore font
	}

	// if the selected item is not shown, shift it into place and hide any others that it might cover up
	if ( m_dotdotWidth > 0 
		&& m_selection >= 0 
		&& m_selection < (int) m_items.size()
		&& !m_items[m_selection].shown )
	{
		int shifted_x = cwidth - m_dotdotWidth - m_items[m_selection].width;

		for ( int i = (int)m_items.size()-1; i >= 0; i-- )
		{
			if ( m_items[i].x_start + m_items[i].width >= shifted_x )
				m_items[i].shown = false;
			else
				break;
		}

		m_items[m_selection].shown = true;
		m_items[m_selection].x_start = shifted_x;
	}
	

	// now draw all the items that have been determined to be visible
	for ( size_t i=0; i<m_items.size(); i++ )
	{
		if ( !m_items[i].shown ) continue;

		if ( !light )
		{
			wxColour col( i==m_hoverIdx||i==m_selection 
				? wxMetroTheme::Colour( wxMT_HOVER ) 
				: wxMetroTheme::Colour( wxMT_FOREGROUND ) );
			dc.SetPen( wxPen(col, 1) );
			dc.SetBrush( wxBrush(col) );
			dc.DrawRectangle( m_items[i].x_start, 0, m_items[i].width, cheight );

			if ( button_width > 0
				&& m_hoverIdx == i
				&&  m_buttonHover  )
			{
				dc.SetPen( wxPen( wxMetroTheme::Colour( wxMT_DIMHOVER ), 1 ) );
				dc.SetBrush( wxBrush( wxMetroTheme::Colour( wxMT_DIMHOVER ) ) );
				dc.DrawRectangle( m_items[i].x_start + m_items[i].width - button_width - TB_XPADDING + TB_SPACE,
					0, button_width + TB_XPADDING - TB_SPACE, cheight );
			}
		}

		wxColour text( *wxWHITE );
		if ( light )
		{
			text = ( i == (int)m_selection 
				? wxMetroTheme::Colour( wxMT_FOREGROUND ) 
				: ( i==m_hoverIdx 
					? wxMetroTheme::Colour( wxMT_SELECT ) 
					: wxMetroTheme::Colour( wxMT_TEXT ) ) );
		}

		dc.SetTextForeground( text );			
		dc.DrawText( m_items[i].label, m_items[i].x_start + TB_XPADDING, cheight/2-CharHeight/2-1 );

		if ( button_width > 0 )
		{	
			dc.DrawBitmap( button_icon, 
				m_items[i].x_start + m_items[i].width - TB_SPACE - button_width, 
				cheight/2-button_height/2+1 );
		}

	}

	if ( m_dotdotWidth > 0 )
	{
		if ( !light && m_dotdotHover )
		{
			dc.SetPen( wxPen(wxMetroTheme::Colour( wxMT_HOVER ), 1) );
			dc.SetBrush( wxBrush(wxMetroTheme::Colour( wxMT_HOVER ) ) );
			dc.DrawRectangle( cwidth - m_dotdotWidth, 0, m_dotdotWidth, cheight );
		}

		wxFont font( wxMetroTheme::Font( wxMT_NORMAL, 18) );
		font.SetWeight( wxFONTWEIGHT_BOLD );
		dc.SetFont( font );
		dc.SetTextForeground( light 
			? (m_dotdotHover ?  wxMetroTheme::Colour( wxMT_SELECT ) : wxMetroTheme::Colour( wxMT_TEXT )) 
			: *wxWHITE );

		dc.DrawText( "...", cwidth - m_dotdotWidth + TB_SPACE, cheight/2 - dotdot_height/2-1 );
	}

	if ( light )
	{
		dc.SetPen( wxPen( wxColour(209,209,209) ));
		dc.DrawLine( 0, cheight-1, cwidth, cheight-1);
	}
}
	
void wxMetroTabList::OnLeftDown(wxMouseEvent &evt)
{
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);

	int mouse_x = evt.GetX();
	for (size_t i=0;i<m_items.size();i++)
	{
		if ( m_items[i].shown
			&& mouse_x >= m_items[i].x_start 
			&& mouse_x < m_items[i].x_start + m_items[i].width)
		{
			if ( m_style & wxMT_MENUBUTTONS 
				&& IsOverButton( mouse_x, i ) )
			{
				SwitchPage( i ); // first switch pages if we're going to fire the context menu up

				wxCommandEvent evt( wxEVT_BUTTON, GetId() );
				evt.SetEventObject( this );
				evt.SetInt( i );
				evt.SetString( m_items[i].label );
				ProcessEvent( evt );
				return;
			}

			m_pressIdx = i;
			if (this->HasCapture())
				this->ReleaseMouse();

			this->CaptureMouse();
			return;
		}
	}

	if ( m_selection >= 0 && m_selection < m_items.size()
		&& m_dotdotWidth > 0 && mouse_x > cwidth - m_dotdotWidth )
	{
		wxMenu menu;			
		for ( size_t i=0;i< m_items.size();i++)
			menu.AppendCheckItem( ID_TAB0+i, m_items[i].label );
		
		menu.Check( ID_TAB0+m_selection, true );
		PopupMenu( &menu );
	}
}

void wxMetroTabList::OnMouseMove(wxMouseEvent &evt)
{
	int cwidth, cheight;
	GetClientSize(&cwidth, &cheight);
	m_hoverIdx = -1;
	m_buttonHover = false;
	int mouse_x = evt.GetX();

	bool was_hovering = m_dotdotHover;
	m_dotdotHover = ( m_dotdotWidth > 0 && mouse_x > cwidth - m_dotdotWidth );

	for (size_t i=0;i<m_items.size();i++)
	{
		if ( m_items[i].shown 
			&& mouse_x >= m_items[i].x_start 
			&& mouse_x < m_items[i].x_start + m_items[i].width)
		{
			if ( m_style & wxMT_MENUBUTTONS && IsOverButton( mouse_x, i ) )
				m_buttonHover = true;

			if (m_hoverIdx != (int)i)
				Refresh();
			m_hoverIdx = i;
			return;
		}
	}

	if ( m_dotdotHover != was_hovering )
		Refresh();
}

bool wxMetroTabList::IsOverButton( int mouse_x, size_t i )
{
	if ( m_style & wxMT_MENUBUTTONS )
	{
		int button_width = wxMetroTheme::Bitmap( wxMT_DOWNARROW ).GetWidth() + TB_SPACE + TB_SPACE;
		if ( mouse_x > m_items[i].x_start + m_items[i].width - button_width - TB_XPADDING + TB_SPACE )
			return true;
	}

	return false;
}

void wxMetroTabList::OnLeftUp(wxMouseEvent &evt)
{
	if (m_pressIdx >= 0 && this->HasCapture())
		this->ReleaseMouse();

	int mouse_x = evt.GetX();
	for (size_t i=0;i<m_items.size();i++)
	{
		if ( m_items[i].shown 
			&& mouse_x >= m_items[i].x_start 
			&& mouse_x < m_items[i].x_start + m_items[i].width)
		{
			if ( IsOverButton( mouse_x, i ) )
				return;

			SwitchPage( i );
			return;
		}
	}
}

void wxMetroTabList::OnMenu( wxCommandEvent &evt )
{
	SwitchPage( evt.GetId() - ID_TAB0 );
}

void wxMetroTabList::OnLeave(wxMouseEvent &)
{
	m_pressIdx = -1;
	m_hoverIdx = -1;
	m_dotdotHover = false;
	Refresh();
}

void wxMetroTabList::SwitchPage( size_t i )
{
	m_selection = i;
	wxCommandEvent evt( wxEVT_LISTBOX, GetId() );
	evt.SetEventObject( this );
	evt.SetInt( i );
	evt.SetString( m_items[i].label );
	ProcessEvent( evt );
	Refresh();

}

/************* wxMetroNotebook ************** */

enum { ID_TABLIST = wxID_HIGHEST+124 };

BEGIN_EVENT_TABLE(wxMetroNotebook, wxPanel)
	EVT_SIZE( wxMetroNotebook::OnSize )
	EVT_LISTBOX( ID_TABLIST, wxMetroNotebook::OnTabList )
END_EVENT_TABLE()

wxMetroNotebook::wxMetroNotebook(wxWindow *parent, int id, const wxPoint &pos, const wxSize &sz, long style)
	: wxPanel(parent, id, pos, sz)
{
	m_list = new wxMetroTabList(this, ID_TABLIST, wxDefaultPosition, wxDefaultSize, style );
	m_flipper = new wxSimplebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );

	wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
	sizer->Add( m_list, 0, wxALL|wxEXPAND, 0 );
	sizer->Add( m_flipper, 1, wxALL|wxEXPAND, 0 );
	SetSizer( sizer );

	SetBackgroundColour( wxMetroTheme::Colour( wxMT_BACKGROUND ) );
	SetForegroundColour( wxMetroTheme::Colour( wxMT_FOREGROUND ) );
	
}

void wxMetroNotebook::AddPage(wxWindow *win, const wxString &text, bool active)
{
	win->Reparent( m_flipper );
	m_flipper->AddPage(win, text);

	page_info x;
	x.text = text;
	x.scroll_win = 0;

	m_pageList.push_back( x );

	if (active)
	{
		m_list->SetSelection( m_flipper->GetPageCount()-1 );
		m_flipper->ChangeSelection( m_flipper->GetPageCount()-1 );
	}
	UpdateTabList();
	ComputeScrolledWindows();
}

int wxMetroNotebook::GetPageIndex(wxWindow *win)
{
	if ( win == NULL ) return -1;
	int ndx = -1;
	for (size_t i=0; i<m_flipper->GetPageCount(); i++)
	{
		if ( m_flipper->GetPage(i) == win )
			ndx = i;
	}
	return ndx;
}


wxWindow *wxMetroNotebook::RemovePage( int ndx )
{
	if ( ndx < 0 || ndx >= (int)m_pageList.size() )
		return 0;
	
	wxWindow *win = m_flipper->GetPage( ndx );

	int cursel = m_flipper->GetSelection();
	if ( cursel >= ndx )
	{
		cursel--;
		if ( cursel < 0 )
			cursel = 0;
	}

	if ( m_pageList[ndx].scroll_win != 0
		&& win->GetParent() == m_pageList[ndx].scroll_win )
	{
		win->SetParent( this );
		m_pageList[ndx].scroll_win->Destroy();
		m_pageList[ndx].scroll_win = 0;
	}

	m_flipper->RemovePage(ndx);
	m_pageList.erase( m_pageList.begin() + ndx );
		
	UpdateTabList();

	if ( m_flipper->GetSelection() != cursel )
	{
		m_flipper->ChangeSelection(cursel);
		m_list->SetSelection(cursel);
	}

	ComputeScrolledWindows();
	return win;
}

void wxMetroNotebook::DeletePage( int ndx )
{
	wxWindow *win = RemovePage( ndx );
	if ( win != 0 ) win->Destroy();

}

wxWindow *wxMetroNotebook::GetPage( int index )
{
	return m_flipper->GetPage( index );
}

void wxMetroNotebook::AddScrolledPage(wxWindow *win, const wxString &text, bool active)
{
	wxScrolledWindow *scrollwin = new wxScrolledWindow( m_flipper );
	win->Reparent( scrollwin );

	int cw, ch;
	win->GetClientSize(&cw,&ch);

	scrollwin->SetScrollbars( 1, 1, cw, ch, 0, 0 );
	scrollwin->SetScrollRate(25,25);
	
	win->Move(0,0);

	m_flipper->AddPage( scrollwin, text );

	page_info x;
	x.text = text;
	x.scroll_win = win;

	m_pageList.push_back( x );
	UpdateTabList();

	if (active)
	{
		m_list->SetSelection( m_flipper->GetPageCount()-1 );
		m_flipper->ChangeSelection( m_flipper->GetPageCount()-1 );
	}

	ComputeScrolledWindows();
}

int wxMetroNotebook::GetSelection()
{
	return m_flipper->GetSelection();
}

void wxMetroNotebook::SetText(int id, const wxString &text)
{
	if ((id < (int)m_pageList.size()) && (id >= 0))
	{
		m_pageList[id].text = text;
		UpdateTabList();
	}
}

void wxMetroNotebook::SetSelection(int id)
{
	m_flipper->SetSelection(id);
	m_list->SetSelection(id);
}

int wxMetroNotebook::GetPageCount()
{
	return m_flipper->GetPageCount();
}

void wxMetroNotebook::OnSize(wxSizeEvent &evt)
{
	ComputeScrolledWindows();
	evt.Skip();
}

void wxMetroNotebook::ComputeScrolledWindows()
{
	for (size_t i=0;i<m_pageList.size();i++)
	{
		if ( m_pageList[i].scroll_win != 0 )
		{
			if (wxScrolledWindow *parent = dynamic_cast<wxScrolledWindow*>( m_flipper->GetPage(i) ))
			{
				int cw, ch;
				m_pageList[i].scroll_win->GetClientSize(&cw,&ch);
				int vw, vh;
				parent->GetVirtualSize(&vw,&vh);

				if (vw != cw || vh != ch)
				{
					parent->SetScrollbars(1,1, cw, ch);
					parent->SetScrollRate(25,25);
				}

				int x,y;
				m_pageList[i].scroll_win->GetPosition(&x,&y);
				if (vw > cw && vh > ch 
					&& (x != 0 || y != 0))
					m_pageList[i].scroll_win->Move(0,0);
		
			}
		}
	}
}

void wxMetroNotebook::UpdateTabList()
{
	size_t sel = m_list->GetSelection();
	m_list->Clear();
	for (size_t i=0;i<m_pageList.size();i++)
		m_list->Append( m_pageList[i].text );

	m_list->SetSelection( sel );
}

void wxMetroNotebook::OnTabList( wxCommandEvent &evt )
{
	SwitchPage( evt.GetInt() );
}

void wxMetroNotebook::SwitchPage( size_t i )
{	
	if ( i == GetSelection() ) return;
		
	wxNotebookEvent evt(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING, GetId() );
	evt.SetEventObject( this );
	evt.SetOldSelection( GetSelection() );
	evt.SetSelection(i);
	ProcessEvent(evt);

	if ( !evt.IsAllowed() )
	{
		m_list->SetSelection( GetSelection() );
		return; // abort the selection if the changing event was vetoed.
	}

	m_flipper->ChangeSelection(i);
	m_list->SetSelection(i);

	// fire EVT_NOTEBOOK_PAGE_CHANGED
	wxNotebookEvent evt2(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, GetId() );
	evt2.SetEventObject( this );
	evt2.SetSelection(i);
	ProcessEvent(evt2);
}

#define SCRL_RATE 25

BEGIN_EVENT_TABLE(wxMetroListBox, wxScrolledWindow)
	EVT_SIZE( wxMetroListBox::OnResize )	
	EVT_LEFT_DOWN( wxMetroListBox::OnLeftDown )
	EVT_LEFT_DCLICK( wxMetroListBox::OnDClick )
	EVT_PAINT( wxMetroListBox::OnPaint )
	EVT_MOTION( wxMetroListBox::OnMouseMove )
	EVT_LEAVE_WINDOW( wxMetroListBox::OnLeave )
	EVT_ERASE_BACKGROUND( wxMetroListBox::OnErase )
END_EVENT_TABLE()

wxMetroListBox::wxMetroListBox(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size)
	: wxScrolledWindow(parent,id, pos, size, wxBORDER_NONE)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	SetBackgroundColour( *wxWHITE );
	SetFont( wxMetroTheme::Font( wxMT_LIGHT, 15 ) );	
	m_selectedIdx = -1;
	m_hoverIdx = -1;
	
}

wxMetroListBox::~wxMetroListBox()
{
	/* nothing to do */
}


void wxMetroListBox::Add(const wxString &item)
{
	_item x;
	x.name = item;
	m_items.push_back( x );
}

void wxMetroListBox::Add( const wxArrayString &list )
{
	for( size_t i=0;i<list.size();i++ )
		Add( list[i] );
}

void wxMetroListBox::Delete( size_t idx )
{
	if ( idx < m_items.size() )
		m_items.erase( m_items.begin() + idx );
}

int wxMetroListBox::Find(const wxString &item)
{
	for (size_t i=0;i<m_items.size();i++)
		if (m_items[i].name == item)
			return i;
	return -1;
}

wxString wxMetroListBox::Get(size_t idx)
{
	if (idx >= 0 && idx < m_items.size())
		return m_items[idx].name;
	else
		return wxEmptyString;
}

wxString wxMetroListBox::GetValue()
{
	return m_selectedIdx >= 0 ? Get(m_selectedIdx) : wxEmptyString;
}

void wxMetroListBox::Clear()
{
	m_items.clear();
	Invalidate();
}

int wxMetroListBox::Count()
{
	return m_items.size();
}

void wxMetroListBox::SetSelection(int idx)
{
	m_selectedIdx = idx;
	Refresh();
}

bool wxMetroListBox::SetSelectionString( const wxString &s )
{
	m_selectedIdx = Find( s );
	Refresh();
	return m_selectedIdx >= 0;
}

wxString wxMetroListBox::GetSelectionString()
{
	return GetValue();
}

int wxMetroListBox::GetSelection()
{
	return m_selectedIdx;
}

void wxMetroListBox::Invalidate()
{
	int hpos, vpos;
	GetViewStart( &hpos, &vpos );
	hpos *= SCRL_RATE;
	vpos *= SCRL_RATE;

	wxSize sz = GetClientSize();
	int width,height;
	width=height=0;

	wxClientDC dc( this );
	dc.SetFont( GetFont() );


	int y = 0;
	for (int i=0;i<m_items.size();i++)
	{
		
		int height = dc.GetCharHeight()+10;
		m_items[i].geom.x = 0;
		m_items[i].geom.y = y;
		m_items[i].geom.width = sz.GetWidth()+1;
		m_items[i].geom.height = height;
		y += height;
	}

	SetScrollbars(1,1,sz.GetWidth(),y, hpos, vpos);
	SetScrollRate( SCRL_RATE, SCRL_RATE );
	Refresh();
}

void wxMetroListBox::OnResize(wxSizeEvent &evt)
{
	Invalidate();
}



void wxMetroListBox::OnPaint(wxPaintEvent &evt)
{
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC( dc );
	
	wxColour bg = GetBackgroundColour();
	dc.SetBrush(wxBrush(bg));
	dc.SetPen(wxPen(bg,1));
	wxRect windowRect( wxPoint(0,0), GetClientSize() );
	CalcUnscrolledPosition(windowRect.x, windowRect.y,
		&windowRect.x, &windowRect.y);
	dc.DrawRectangle(windowRect);
	dc.SetFont( GetFont() );
	dc.SetTextForeground( *wxBLACK );
	int height = dc.GetCharHeight();
	for (int i=0;i<m_items.size();i++)	
	{
		wxColour bcol = (m_selectedIdx == i) ? wxColour(50,50,50) :
			( (m_hoverIdx == i) ? wxColour(231,231,231) : GetBackgroundColour() );
		dc.SetPen( wxPen( bcol ) );
		dc.SetBrush( wxBrush( bcol ) );
		dc.DrawRectangle( m_items[i].geom );
		dc.SetTextForeground( (m_selectedIdx==i) ? *wxWHITE : *wxBLACK );
		dc.DrawText( m_items[i].name, 6, m_items[i].geom.y + m_items[i].geom.height/2 - height/2 );
	}			
}

void wxMetroListBox::OnErase( wxEraseEvent & )
{
	/* nothing to do */
}

void wxMetroListBox::OnLeftDown(wxMouseEvent &evt)
{
	int vsx, vsy;
	GetViewStart( &vsx, &vsy );
	vsx *= SCRL_RATE;
	vsy *= SCRL_RATE;

	SetFocus();

	for (int i=0;i<m_items.size();i++)
	{
		if (evt.GetY()+vsy > m_items[i].geom.y 
			&& evt.GetY()+vsy < m_items[i].geom.y+m_items[i].geom.height )
		{
			
			m_selectedIdx = i;
			Refresh();
				
			wxCommandEvent selevt(wxEVT_COMMAND_LISTBOX_SELECTED, this->GetId() );
			selevt.SetEventObject(this);
			selevt.SetInt(i);
			selevt.SetString(GetValue());
			GetEventHandler()->ProcessEvent(selevt);
			return;
		}
	}

	m_selectedIdx = -1;
	Refresh();
}

void wxMetroListBox::OnDClick( wxMouseEvent &evt )
{
	wxCommandEvent selevt(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, this->GetId() );
	selevt.SetEventObject(this);
	selevt.SetInt( m_selectedIdx );
	selevt.SetString(GetValue());
	GetEventHandler()->ProcessEvent(selevt);
}

void wxMetroListBox::OnMouseMove(wxMouseEvent &evt)
{	
	int vsx, vsy;
	GetViewStart( &vsx, &vsy );
	vsx *= SCRL_RATE;
	vsy *= SCRL_RATE;

	for (int i=0;i<m_items.size();i++)
	{
		if (evt.GetY()+vsy > m_items[i].geom.y 
			&& evt.GetY()+vsy < m_items[i].geom.y+m_items[i].geom.height )
		{
			
			if ( m_hoverIdx != i )
			{
				m_hoverIdx = i;
				Refresh();
			}

			return;
		}
	}

	if ( m_hoverIdx != -1 )
	{
		m_hoverIdx = -1;
		Refresh();
	}
}

void wxMetroListBox::OnLeave(wxMouseEvent &evt)
{
	m_hoverIdx = -1;
	Refresh();
}

