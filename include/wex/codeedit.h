#ifndef __codedit_h
#define __codedit_h

#if defined(__APPLE__)||defined(__GNUC__)
#include <tr1/unordered_map>
using namespace std::tr1;
#else
#include <unordered_map>
using namespace std;
#endif

#include <wx/vector.h>
#include <wx/fdrepdlg.h>
#include <wx/stc/stc.h>

class FRDialog;

class wxCodeEditCtrl : public wxStyledTextCtrl
{
public:
	wxCodeEditCtrl( wxWindow *parent, int id = wxID_ANY, 
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );

	enum Language { NONE, CPP, C, LK, VBA, HTML, TEXT, TRNSYS, PYTHON };

	void SetLanguage( const wxString &fileName );
	void SetLanguage( Language lang );
	Language GetLanguage();
	static Language GetLanguage( const wxString &fileName );
	void SetKnownIdentifiers( const wxString &names ); // space separated list of identifiers to highlight

	void EnableCallTips( bool en );
	void ClearCallTips();
	void ConfigureCallTips( wxUniChar start, wxUniChar end, bool case_sensitive );
	void AddCallTip( const wxString &key, const wxString &value );
	
	void ShowBreakpoints( bool show );
	void AddBreakpoint( int line );
	void RemoveBreakpoint( int line );
	void ToggleBreakpoint( int line );
	bool HasBreakpoint( int line );
	void ClearBreakpoints();
	std::vector<int> GetBreakpoints();

	void ShowLineArrow( int line );
	void HideLineArrow();

	void ShowFindReplaceDialog();
	void FindNext();
	void ReplaceNext();
	void ReplaceAll();
	
	int	FindNext( const wxString &text, int fr_text_len = -1, /* used internally by replace - typically -1 is OK */ 
			bool match_case = true, bool whole_word = false );
	int ReplaceNext( const wxString &text, const wxString &replace, bool stop_at_find = false, 
			bool match_case = true, bool whole_word = false  );	
	int ReplaceAll( const wxString &text, const wxString &replace, 
			bool match_case = true, bool whole_word = false, bool show_message = true );
	
	void SelectLine( int line );
	void YankLine();
	void PutLine();

private:	
	void OnMarginClick( wxStyledTextEvent &evt );
    void OnCharAdded( wxStyledTextEvent &evt );
	void OnUpdateUI( wxStyledTextEvent &evt );

	void DoBraceMatch();
	bool FindMatchingBracePosition( int &braceAtCaret, int &braceOpposite, bool sloppy );
	
	int m_lastFindPos;
	int m_lastReplacePos;
	
	static const int m_markCircle = 0;
	static const int m_markArrow = 1;
	static const int m_lineNumMarginId = 0;
	static const int m_breakpointMarginId = 1;
	static const int m_foldingMarginId = 2;

	FRDialog *m_frDialog;
	Language m_lang;
	bool m_callTipsEnabled;
	wxUniChar m_ctStart, m_ctEnd;
	bool m_ctCaseSensitive;
	wxArrayString m_ctStack;
	
	std::vector<int> m_breakPoints;
	unordered_map< wxString, wxString, wxStringHash, wxStringEqual > m_callTips;

	wxString m_yankText;

    DECLARE_EVENT_TABLE()
};

#endif
