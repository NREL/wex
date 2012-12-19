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

private:

	void OnScriptTextChanged( wxStyledTextEvent & );
	void OnTimer( wxTimerEvent & );

	wxTimer m_timer;

	DECLARE_EVENT_TABLE();
};






#endif // WEX_USE_LK

#endif

