#include <wx/wx.h>
#include <wx/frame.h>
#include <wx/stc/stc.h>
#include <wx/webview.h>

class MyApp : public wxApp
{
public:
	bool OnInit()
	{
		wxMessageBox( wxT("Hello, \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd in wxWidgets 3.0!") );

		wxFrame *frame1 = new wxFrame( 0, wxID_ANY, wxT("Editor"), wxDefaultPosition, wxSize(700,700) );
		new wxStyledTextCtrl( frame1, wxID_ANY );

		wxFrame *frame2 = new wxFrame( 0, wxID_ANY, wxT("WebView"), wxDefaultPosition, wxSize(700,700) );
		wxWebView::New( frame2, wxID_ANY, wxT("http://www.google.com") );
		
		frame1->Show();
		frame2->Show();

		return true;
	}
};

IMPLEMENT_APP( MyApp );
