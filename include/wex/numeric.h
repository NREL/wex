#ifndef __wx_numeric_h
#define __wx_numeric_h

#include <wx/textctrl.h>

#define EVT_NUMERIC(id, func) EVT_TEXT_ENTER(id, func)

class wxNumericCtrl : public wxTextCtrl
{
public:
	// restrict to integers if desired
	enum Mode { INTEGER, UNSIGNED, REAL };

	// formatting decimals
	static const int GENERIC = -1;
	static const int EXPONENTIAL = -2;
	static const int HEXADECIMAL = -3; // only integer mode

	wxNumericCtrl( wxWindow *parent, int id = wxID_ANY, 
		double value = 0.0, Mode m = REAL,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize );

	void SetMode( Mode m );
	Mode GetMode() const { return m_mode; }
	
	void SetValue( int val );
	void SetValue( size_t val );
	void SetValue( double val );
	
	double Value() const;
	double AsDouble() const;
	int AsInteger() const;
	size_t AsUnsigned() const;


	void SetRange( double min, double max );
	void ClearRange() { m_min = m_max = 0; }
	double GetMin() { return m_min; }
	double GetMax() { return m_max; }

	void SetFormat( int decimals = -1 /* generic format, like %lg */,
		bool thousands_sep = false,
		const wxString &pre = wxEmptyString,
		const wxString &post = wxEmptyString );

	int GetDecimals() const { return m_decimals; }
	bool GetUsingThousandsSeparator() const { return m_thouSep; }
	wxString GetPrefixText() const { return m_preText; }
	wxString GetSuffixText() const { return m_postText; }

	static wxString Format( double val, Mode m, int deci, bool thousep, const wxString &pre, const wxString &post );

private:
	void OnTextEnter( wxCommandEvent & );
	void OnSetFocus( wxFocusEvent & );
	void OnLoseFocus( wxFocusEvent & );

	void DoFormat();
	void Translate();
	void SetupValidator();
	
	Mode m_mode;
	int m_decimals;
	bool m_thouSep;
	wxString m_preText;
	wxString m_postText;
	double m_min, m_max;

	union ValueType {
		double Real;
		int Int;
		size_t Unsigned;
	};
	ValueType m_value;
	wxString m_focusStrVal;

	DECLARE_EVENT_TABLE()
};

#endif
