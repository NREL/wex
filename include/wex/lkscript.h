#ifndef __lkscript_h
#define __lkscript_h

#include <wx/wx.h>
#include <wx/thread.h>

#define LK_USE_WXWIDGETS 1

#include <lk_absyn.h>
#include <lk_env.h>

#include "wex/codeedit.h"

#define wxLK_STDLIB_BASIC 0x01
#define wxLK_STDLIB_STRING 0x02
#define wxLK_STDLIB_MATH 0x04
#define wxLK_STDLIB_WXUI 0x08
#define wxLK_STDLIB_WEXPLOT 0x10

#define wxLK_STDLIB_ALL (wxLK_STDLIB_BASIC|wxLK_STDLIB_STRING| \
	wxLK_STDLIB_MATH|wxLK_STDLIB_WXUI|wxLK_STDLIB_WEXPLOT)

class wxPLPlotCtrl;

void wxLKSetToplevelParent( wxWindow *parent );
void wxLKSetPlotTarget( wxPLPlotCtrl *plot );
wxPLPlotCtrl *wxLKGetPlotTarget();

lk::fcall_t* wxLKPlotFunctions(); // newplot, plot, plotopt, plotpng
lk::fcall_t* wxLKHttpFunctions(); // httpget, httpdownload
lk::fcall_t* wxLKMiscFunctions(); // rand, decompress

class wxLKScriptCtrl : public wxCodeEditCtrl
{
public:
	wxLKScriptCtrl( wxWindow *parent, int id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
		unsigned long libs = wxLK_STDLIB_ALL );

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
	lk::node_t *m_tree;
	bool m_scriptRunning;
	bool m_stopScriptFlag;
	wxWindow *m_topLevelWindow;

	DECLARE_EVENT_TABLE();
};

#endif

