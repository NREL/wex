#ifndef __lkscript_h
#define __lkscript_h

#ifdef WEX_USE_LK

#include <wx/wx.h>
#include <wx/thread.h>

#define LK_USE_WXWIDGETS 1

#include <lk_absyn.h>
#include <lk_env.h>

#include "wex/codeedit.h"

class wxLKScriptCtrl : public wxCodeEditCtrl
{
public:
	wxLKScriptCtrl( wxWindow *parent, int id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );

	virtual ~wxLKScriptCtrl();

	virtual bool OnEval( int line );
	virtual void OnOutput( const wxString & );
	
	void RegisterLibrary( lk::fcall_t *funcs, const wxString &group = "Miscellaneous", void *user_data = 0);

	wxString GetHtmlDocs();
	void ShowHelpDialog( wxWindow *custom_parent = 0 );

	bool IsScriptRunning();
	bool IsStopFlagSet();
	void Stop();
	bool Execute( const wxString &run_dir = wxEmptyString, wxWindow *toplevel = 0 );
	
	void UpdateInfo();

	lk::env_t *GetEnvironment() { return m_env; }
	wxWindow *GetTopLevelWindowForScript() { return m_topLevelWindow; }
private:

	void OnScriptTextChanged( wxStyledTextEvent & );
	void OnTimer( wxTimerEvent & );

	wxTimer m_timer;
	
	struct libdata
	{
		lk::fcall_t *library;
		wxString name;
	};

	std::vector<libdata> m_libs;
	lk::env_t *m_env;
	bool m_scriptRunning;
	bool m_stopScriptFlag;
	wxWindow *m_topLevelWindow;

	DECLARE_EVENT_TABLE();
};






#endif // WEX_USE_LK

#endif

