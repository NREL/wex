#include "wex/numeric.h"

#include <wx/wx.h>

#include <wx/numformatter.h>
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
	
	wxChar thousep = ','; // default thousands separator
	wxNumberFormatter::GetThousandsSeparatorIfUsed(&thousep);
	excludes.Add( wxString(thousep) );

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
	
	wxUniChar decimsep = wxNumberFormatter::GetDecimalSeparator();

	// find start of number (all numbers start like integers or a dot)
	i=0;
	while(i<len && !is_valid_char(true, strval[i], true, decimsep))
		i++;


	wxChar thousep = ','; // default thousands separator
	wxNumberFormatter::GetThousandsSeparatorIfUsed(&thousep);

	// get all valid number characters
	while(i<len && is_valid_char( m_mode==INTEGER, strval[i], true, thousep ))
	{
		if ( strval[i]!=thousep ) buf += strval[i];
		i++;
	}

	SetValue( wxAtof( buf ) );
}

wxString wxNumericCtrl::Format( double val, Mode mode, int deci, bool thousep, const wxString &pre, const wxString &post )
{
	wxString buf;

	if ( mode == INTEGER )
	{
		if ( deci == HEXADECIMAL ) buf.Printf( "0x%x", (int)val );
		else
		{
			if ( thousep ) buf = wxNumberFormatter::ToString( (long)val, wxNumberFormatter::Style_WithThousandsSep );
			else buf.Printf( "%d", (int)val );
		}
	}
	else
	{
		if ( deci == GENERIC ) buf.Printf( "%lg", val );
		else if ( deci == EXPONENTIAL ) buf.Printf( "%le", val );
		else
		{
			/*
			if ( deci <= 0 )
			{
				if ( thousep ) buf = wxNumberFormatter::ToString( (long)val, wxNumberFormatter::Style_WithThousandsSep );
				else buf.Printf( "%d", (int)val );
			}
			else
			{*/
				if ( thousep )
					buf = wxNumberFormatter::ToString( val, deci, wxNumberFormatter::Style_WithThousandsSep );
				else
				{
					wxString fmt;
					fmt.Printf( "%%.%dlf", deci );
					buf.Printf( fmt, val );
				}
			//}
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
