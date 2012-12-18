#ifndef __codedit_h
#define __codedit_h

#include <unordered_map>
#include <wx/fdrepdlg.h>
#include <wx/stc/stc.h>

class wxCodeEditCtrl : public wxStyledTextCtrl
{
public:
	wxCodeEditCtrl( wxWindow *parent, int id = wxID_ANY, 
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );

	
	virtual ~wxCodeEditCtrl();	

	enum Language { NONE, CPP, C, LK, VBA, HTML, TEXT, TRNSYS, PYTHON };

	void SetLanguage( const wxString &fileName );
	void SetLanguage( Language lang );
	Language GetLanguage();
	static Language GetLanguage( const wxString &fileName );

	void EnableCallTips( bool en );
	void ClearCallTips();
	void ConfigureCallTips( wxUniChar start, wxUniChar end, bool case_sensitive );
	void AddCallTip( const wxString &key, const wxString &value );

	void SetKnownIdentifiers( const wxString &names ); // space separated list of identifiers to highlight
	  
	void ShowFindDialog();
	void ShowReplaceDialog();
	void HideFindReplaceDialog();
	int	FindNext( int frtxt_len = -1 );
	int ReplaceNext( bool stop_at_find = false );
	wxString GetFindString();
	void SetFindString( const wxString &s );
	
	void JumpToLine( int line, bool highlight );
	void YankLine();
	void PutLine();

private:	
	void OnMarginClick( wxStyledTextEvent &evt );
    void OnCharAdded( wxStyledTextEvent &evt );
	void OnUpdateUI( wxStyledTextEvent &evt );
	void OnFindDialog( wxFindDialogEvent &evt );

	void DoBraceMatch();
	bool FindMatchingBracePosition( int &braceAtCaret, int &braceOpposite, bool sloppy );
	
	wxFindReplaceDialog *m_findDialog;
	wxFindReplaceData m_findData;
	int m_lastFindPos;
	int m_lastReplacePos;
	
	Language m_lang;
	bool m_callTipsEnabled;
	wxUniChar m_ctStart, m_ctEnd;
	bool m_ctCaseSensitive;
	wxArrayString m_ctStack;

	std::unordered_map< wxString, wxString, wxStringHash, wxStringEqual > m_callTips;
	wxString m_yankText;

    DECLARE_EVENT_TABLE()
};

#endif
