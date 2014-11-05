#include "wex/numeric.h"

#include <wx/wx.h>
#include <wx/valtext.h>

BEGIN_EVENT_TABLE( wxNumericCtrl, wxTextCtrl )
	EVT_TEXT_ENTER( wxID_ANY, wxNumericCtrl::OnTextEnter )
	EVT_KILL_FOCUS( wxNumericCtrl::OnLoseFocus )
	EVT_SET_FOCUS( wxNumericCtrl::OnSetFocus )
END_EVENT_TABLE()

wxNumericCtrl::wxNumericCtrl( wxWindow *parent, int id, 
		double value, Mode m,
		const wxPoint &pos,
		const wxSize &size )
	: wxTextCtrl(parent, id, wxEmptyString, pos, size, 
			wxTE_PROCESS_ENTER|wxTE_RIGHT)
{
	m_mode = m;
	m_decimals = GENERIC;
	m_thouSep = false;
	
	SetupValidator();
	
	SetValue( value );
}

void wxNumericCtrl::OnTextEnter( wxCommandEvent &evt )
{
	if ( m_focusStrVal != GetValue() )
	{
		m_focusStrVal = GetValue();
		Translate();
		SetSelection(0,this->GetValue().Len());
		evt.Skip();
	}
}

void wxNumericCtrl::OnSetFocus( wxFocusEvent &evt )
{
	m_focusStrVal = GetValue();
	SetSelection(0,m_focusStrVal.Len());
	evt.Skip();
}

void wxNumericCtrl::OnLoseFocus( wxFocusEvent &evt )
{
	if ( m_focusStrVal != GetValue() )
	{
		wxCommandEvent enterpress(wxEVT_COMMAND_TEXT_ENTER, this->GetId() );
		enterpress.SetEventObject( this );
		enterpress.SetString( GetValue() );
		GetEventHandler()->ProcessEvent(enterpress);
	}
	evt.Skip();
}

void wxNumericCtrl::SetupValidator()
{
	wxArrayString excludes;
	excludes.Add( wxString(wxChar(',')) ); // thousands separator

	if ( m_mode == INTEGER )
	{
		excludes.Add("+");
		excludes.Add("e");
		excludes.Add("E");
		excludes.Add(".");
	}

	wxTextValidator val( wxFILTER_NUMERIC|wxFILTER_EXCLUDE_CHAR_LIST );
	val.SetExcludes( excludes );
	SetValidator( val );
}

static bool is_valid_char( bool intonly, wxUniChar c, bool additional, wxUniChar c1 )
{
	if (intonly) return wxIsdigit(c) || c == '-' || c=='+' || (additional && c == c1);
	else return wxIsdigit(c) || c == '-' || c=='+' || c == '.' || c == 'e' || c == 'E' || (additional && c == c1);
}

void wxNumericCtrl::Translate()
{
	wxString buf;
	wxString strval = GetValue();
	int len = strval.Len();
	int i;
	
	wxUniChar decimsep('.');

	// find start of number (all numbers start like integers or a dot)
	i=0;
	while(i<len && !is_valid_char(true, strval[i], true, decimsep))
		i++;


	wxUniChar thousep(','); // default thousands separator

	// get all valid number characters
	while(i<len && is_valid_char( m_mode==INTEGER, strval[i], true, thousep ))
	{
		if ( strval[i]!=thousep ) buf += strval[i];
		i++;
	}

	SetValue( wxAtof( buf ) );
}

static void AddThousandsSeparators(wxString& s)
{
    wxChar thousandsSep(',');

    size_t pos = s.find( wxChar('.') );
    if ( pos == wxString::npos )
    {
        // Start grouping at the end of an integer number.
        pos = s.length();
    }

    // End grouping at the beginning of the digits -- there could be at a sign
    // before their start.
    const size_t start = s.find_first_of("0123456789");

    // We currently group digits by 3 independently of the locale. This is not
    // the right thing to do and we should use lconv::grouping (under POSIX)
    // and GetLocaleInfo(LOCALE_SGROUPING) (under MSW) to get information about
    // the correct grouping to use. This is something that needs to be done at
    // wxLocale level first and then used here in the future (TODO).
    const size_t GROUP_LEN = 3;

    while ( pos > start + GROUP_LEN )
    {
        pos -= GROUP_LEN;
        s.insert(pos, thousandsSep);
    }
}

wxString wxNumericCtrl::Format( double val, Mode mode, int deci, bool thousep, const wxString &pre, const wxString &post )
{
	wxString buf;

	if ( mode == INTEGER )
	{
		if ( deci == HEXADECIMAL ) buf.Printf( "0x%x", (int)val );
		else
		{
			buf.Printf( "%d", (int)val );
			if ( thousep ) AddThousandsSeparators( buf );
		}
	}
	else
	{
		if ( deci == GENERIC ) buf.Printf( "%lg", val );
		else if ( deci == EXPONENTIAL ) buf.Printf( "%le", val );
		else
		{
			wxString fmt;
			fmt.Printf( "%%.%dlf", deci );
			buf.Printf( fmt, val );
			if ( thousep ) AddThousandsSeparators( buf );
		}
	}

	return pre + buf + post;
}

void wxNumericCtrl::DoFormat()
{	
	ChangeValue( Format( m_value, m_mode, m_decimals, m_thouSep, m_preText, m_postText ) );
}

void wxNumericCtrl::SetValue( double val )
{
	if ( m_mode == INTEGER ) m_value = (int)val;
	else m_value = val;

	DoFormat();
}


void wxNumericCtrl::SetMode( Mode m )
{
	m_mode = m;
	if ( m_mode == INTEGER ) m_value = (int)m_value;
	SetupValidator();

	DoFormat();
}


void wxNumericCtrl::SetFormat( int decimals,
		bool thousands_sep,
		const wxString &pre,
		const wxString &post )
{
	m_decimals = decimals;
	m_thouSep = thousands_sep;
	m_preText = pre;
	m_postText = post;

	DoFormat();
}
