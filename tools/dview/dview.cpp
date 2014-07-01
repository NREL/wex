#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/scrolbar.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/accel.h>
#include <wx/image.h>
#include <wx/fs_zip.h>
#include <wx/html/htmlwin.h>
#include <wx/snglinst.h>
#include <wx/progdlg.h>
#include <wx/busyinfo.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/generic/helpext.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/cmdline.h>
#include <wx/tokenzr.h>
#include <wx/msgdlg.h>

#include "wex/dview/dvplotctrl.h"
#include "wex/dview/dvfilereader.h"

#include "wex/plot/plplotctrl.h"
#include "wex/plot/pllineplot.h"
#include "wex/plot/plscatterplot.h"

#define MAX_RECENT 25
enum{ 
		ID_RECENT_FILES = wxID_HIGHEST+1233,			
		// up to 100 recent items can be accommodated
		ID_RECENT,
		ID_RECENT_LAST = ID_RECENT+MAX_RECENT,
};

class DViewFrame : public wxFrame
{
private:
	wxDVPlotCtrl *mPlotCtrl;
	int mRecentCount;
	wxString mLastDir;
	wxMenu *mFileMenu, *mRecentMenu;
	wxString mRecentFiles[MAX_RECENT];
public:

	DViewFrame()
	 : wxFrame( 0, wxID_ANY, "Data Viewer", wxDefaultPosition, wxSize(800,600) )
	{	
		mRecentCount = 0;

#ifdef __WXMSW__
		SetIcon( wxIcon("appicon") );
#endif

		wxMenuBar *menubar = new wxMenuBar;
		mRecentMenu = new wxMenu;
		mFileMenu = new wxMenu;
		mFileMenu->Append(wxID_OPEN, "Open...\tCtrl-O");
		mFileMenu->Append(wxID_CLEAR, "Clear\tCtrl-W");
		mFileMenu->AppendSeparator();
		mFileMenu->Append(ID_RECENT_FILES, "Recent", mRecentMenu);
	
	#ifndef __WXMAC__
		mFileMenu->AppendSeparator();
		mFileMenu->Append(wxID_EXIT);
	#endif
		menubar->Append(mFileMenu, "&File");
	
		wxMenu *help_menu = new wxMenu;
		help_menu->Append(wxID_ABOUT);
		menubar->Append(help_menu, "&Help");

		SetMenuBar( menubar );
		
		mPlotCtrl = new wxDVPlotCtrl(this, wxID_ANY);
	
		wxConfig cfg( "DView", "NREL" );
		long ct = 0;
		if (cfg.Read("RecentCount", &ct))
			mRecentCount = (int)ct;

		if (mRecentCount > MAX_RECENT)
			mRecentCount = MAX_RECENT;

		for (int i=0;i<mRecentCount;i++)
		{
			wxString key;
			key.Printf("RecentFile_%d", i);
			wxString fn;
			if (cfg.Read(key, &fn))
			{
				fn.Replace("\\","/");
				mRecentFiles[i] = fn;
			}
		}

		cfg.Read("LastDirectory", &mLastDir);

		
		int x, y, width, height;
		bool maximized;

		if (cfg.Read("FrameX", &x)
			&& cfg.Read("FrameY", &y)
			&& cfg.Read("FrameWidth", &width)
			&& cfg.Read("FrameHeight", &height)
			&& cfg.Read("FrameMaximized", &maximized))
		{
			SetPosition( wxPoint(x, y) );
			SetClientSize( width, height );
			if (maximized)
				Maximize();
		}

		UpdateRecentMenu();
	}
	
	void UpdateRecentMenu()
	{
		int i;
		for (i=0;i<MAX_RECENT;i++)
		{
			if (mRecentMenu->FindItem(ID_RECENT+i) != NULL)
				mRecentMenu->Destroy(ID_RECENT+i);
		}

		for (i=0;i<mRecentCount;i++)
		{
			wxString name;
			name.Printf("%d ", i+1);
			name += mRecentFiles[i];
			mRecentMenu->Append(ID_RECENT+i, name);
		}


		mFileMenu->Enable(ID_RECENT_FILES, mRecentCount > 0);
	}

	void OnRecent(wxCommandEvent &evt)
	{
		int id = evt.GetId() - ID_RECENT;
		if (id < 0 || id >= MAX_RECENT)
			return;

		if ( !mRecentFiles[id].IsEmpty() )
		{
			wxArrayString files;
			files.Add( mRecentFiles[id] );
			Load( files );
		}
	}

	wxArrayString GetRecentFiles()
	{
		wxArrayString list;
		for (int i=0;i<mRecentCount;i++)
			list.Add( mRecentFiles[i] );
		return list;
	}

	void AddRecent(const wxString &fn)
	{
		wxString norm_fn = fn;
		norm_fn.Replace("\\","/");

		int i;
		int index = -1;
		// find the file in the recent list
		for (i=0;i<mRecentCount;i++)
		{
			if (norm_fn == mRecentFiles[i])
			{
				index = i;
				break;
			}
		}

		if (index >= 0)
		{
			// bring this file to the front of the
			// recent file list

			for (i=index;i>0;i--)
				mRecentFiles[i] = mRecentFiles[i-1];
		}
		else // not found in recent list
		{
			// add this to the front of the recent list
			// and increment the recent count if its 
			// less than MAX_RECENT

			for (i=MAX_RECENT-1;i>0;i--)
				mRecentFiles[i] = mRecentFiles[i-1];

			if (mRecentCount < MAX_RECENT)
				mRecentCount++;
		}
	
		mRecentFiles[0] = norm_fn;
		UpdateRecentMenu();
	}

	void RemoveRecent(const wxString &fn)
	{
		wxString norm_fn = fn;
		norm_fn.Replace("\\","/");

		int i;
		int index = -1;
		// find the file in the recent list
		for (i=0;i<mRecentCount;i++)
		{
			if (norm_fn == mRecentFiles[i])
			{
				index = i;
				break;
			}
		}

		if (index >= 0)
		{
			for (i=index;i<MAX_RECENT-1;i++)
				mRecentFiles[i] = mRecentFiles[i+1];

			mRecentCount--;
			UpdateRecentMenu();
		}
	}

	void OnCloseFrame( wxCloseEvent &evt )
	{	

		/* save window position */
		bool b_maximize = this->IsMaximized();
		int f_x,f_y,f_width,f_height;

		this->GetPosition(&f_x,&f_y);
		this->GetClientSize(&f_width, &f_height);
	
		long ct = (long)mRecentCount;

		wxConfig cfg( "DView", "NREL" );
		cfg.Write("RecentCount", ct);
		for (int i=0;i<mRecentCount;i++)
		{
			wxString key;
			key.Printf("RecentFile_%d", i);
			cfg.Write(key, mRecentFiles[i]);
		}

		cfg.Write("LastDirectory", mLastDir);
		cfg.Write("FrameX", f_x);
		cfg.Write("FrameY", f_y);
		cfg.Write("FrameWidth", f_width);
		cfg.Write("FrameHeight", f_height);
		cfg.Write("FrameMaximized", b_maximize);

	
		Destroy();
	}
	
	bool Load(const wxArrayString& filenames)
	{
		wxBeginBusyCursor();
		for(size_t i=0; i<filenames.GetCount(); i++)
		{	
			if(!wxDVFileReader::FastRead(mPlotCtrl, filenames[i]))
			{
				wxMessageBox( wxT("The selected file is not of the correct format, is corrupt, no longer exists, or you do not have permission to open it."), wxT("Error opening file."), wxICON_ERROR);
				RemoveRecent(filenames[i]);
			}
			else
			{
				AddRecent(filenames[i]);
			}
		}
		
		UpdateRecentMenu();
		wxEndBusyCursor();
		return true;
	}

	void Open()
	{
		wxFileDialog fdlg(this, "Open Data File", mLastDir, "", "All Files|*.*|CSV Files(*.csv)|*.csv|TXT Files(*.txt)|*.txt|TMY3 Files(*.tmy3)|*.tmy3|EPW Files(*.epw)|*.epw", wxFD_OPEN | wxFD_MULTIPLE);
		wxArrayString myFilePaths;
		if (fdlg.ShowModal() == wxID_OK)
		{
			fdlg.GetPaths(myFilePaths);
			Load(myFilePaths);
		}
	}
	
	void OnCommand(wxCommandEvent &evt)
	{
		switch( evt.GetId() )
		{
		case wxID_OPEN:
			Open();
			break;
		case wxID_CLEAR:
			mPlotCtrl->RemoveAllDataSets();
			break;
		case wxID_ABOUT:
		case wxID_HELP:
			wxMessageBox( wxT("DView (" + wxGetLibraryVersionInfo().GetVersionString() + ") Version " __DATE__) );
			break;
		case wxID_EXIT:
			Close( false );
			break;
		}
	}

	wxDVPlotCtrl *GetPlot()
	{
		return mPlotCtrl;
	}

	DECLARE_EVENT_TABLE();
};


BEGIN_EVENT_TABLE(DViewFrame, wxFrame)

	EVT_MENU( wxID_OPEN,     DViewFrame::OnCommand )
	EVT_MENU( wxID_CLEAR,    DViewFrame::OnCommand )
	EVT_MENU( wxID_EXIT,     DViewFrame::OnCommand )
	EVT_MENU( wxID_ABOUT,    DViewFrame::OnCommand )
	EVT_CLOSE( DViewFrame::OnCloseFrame )
	EVT_MENU_RANGE( ID_RECENT, ID_RECENT+MAX_RECENT, DViewFrame::OnRecent)

END_EVENT_TABLE()


class TestSideBarWidget : public wxPLSideWidgetBase
{
public:
	virtual void Render( wxDC &dc, const wxRect &geom )
	{
		dc.GradientFillLinear( geom, *wxRED, *wxBLUE, wxSOUTH );
	}
	virtual wxSize CalculateBestSize()
	{
		return wxSize(20, 300);
	}
};


void TestPLPlot( wxWindow *parent )
{
	wxFrame *frame = new wxFrame( parent, wxID_ANY, wxT("wxPLPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"), wxDefaultPosition, wxSize(850,500) );
	frame->SetIcon( wxIcon( "appicon" ) );
		
	wxPLPlotCtrl *plot = new wxPLPlotCtrl( frame, wxID_ANY, wxDefaultPosition, wxDefaultSize );
	//plot->SetBackgroundColour( *wxWHITE );
	plot->SetTitle( wxT("Demo Plot: using \\theta(x)=sin(x)^2, x_0=1\n\\zeta(x)=3\\dot sin^2(x)") );

		
	wxPLLabelAxis *mx = new wxPLLabelAxis( -1, 12, "Months of the year (\\Chi\\Psi)" );
	mx->ShowLabel( false );

	mx->Add( 0,  "Jan" );
	mx->Add( 1,  "Feb" );
	mx->Add( 2,  "March\nMarzo" );
	mx->Add( 3,  "Apr" );
	mx->Add( 4,  "May" );
	mx->Add( 5,  "June\nJunio" );
	mx->Add( 6,  "July" );
	mx->Add( 7,  "August" );
	mx->Add( 8,  "September" );
	mx->Add( 9,  "October" );
	mx->Add( 10, "November" );
	mx->Add( 11, "December\nDeciembre" );

	plot->SetXAxis2( mx );
	/*
	plot->SetYAxis1( new wxPLLinearAxis(-140, 150, wxT("y1 label\ntakes up 2 lines")) );
	plot->SetYAxis1( new wxPLLinearAxis(-11, -3, wxT("\\theta(x)")), wxPLPlotCtrl::PLOT_BOTTOM );
		
	*/
	//plot->SetScaleTextSize( true );

	plot->ShowGrid( true, true );

	plot->SetSideWidget( new TestSideBarWidget, wxPLPlotCtrl::Y_LEFT );
		
	plot->SetXAxis1( new wxPLLogAxis( 0.01, 100, "\\nu  (m^3/kg)" ) );		
//	plot->SetXAxis1( new wxPLTimeAxis( 4, 200 ) );

	std::vector< wxRealPoint > sine_data;
	std::vector< wxRealPoint > cosine_data;
	std::vector< wxRealPoint > tangent_data;
	for (double x = -6; x < 12; x+= 0.01)
	{
		sine_data.push_back( wxRealPoint( x, 3*sin( x )*sin( x ) ) );
		cosine_data.push_back( wxRealPoint( x/2, 2*cos( x/2 )*x ) );
		tangent_data.push_back( wxRealPoint( x, x*tan( x ) ) );
	}


	plot->AddPlot( new wxPLLinePlot( sine_data, "3\\dot sin^2(x)", "forest green", wxPLLinePlot::DOTTED ), 
		wxPLPlotCtrl::X_BOTTOM, 
		wxPLPlotCtrl::Y_LEFT, 
		wxPLPlotCtrl::PLOT_TOP);


	plot->GetXAxis1()->SetLabel( "Bottom X Axis has a great sequence of \\nu  values!" );
		

	plot->AddPlot( new wxPLLinePlot( cosine_data, "cos(\\Omega_\\alpha  )", *wxRED, wxPLLinePlot::DASHED ), 
		wxPLPlotCtrl::X_BOTTOM, 
		wxPLPlotCtrl::Y_LEFT,
		wxPLPlotCtrl::PLOT_TOP);
		
	wxPLLinePlot *lltan = new wxPLLinePlot( tangent_data, "\\beta\\dot tan(\\beta)", *wxBLUE, wxPLLinePlot::SOLID, 1, false );
	lltan->SetAntiAliasing( true );
	plot->AddPlot( lltan, 
		wxPLPlotCtrl::X_BOTTOM, 
		wxPLPlotCtrl::Y_LEFT,
		wxPLPlotCtrl::PLOT_BOTTOM);


	std::vector< wxRealPoint > pow_data;
	for (double i=0.01;i<20;i+=0.1)
		pow_data.push_back( wxRealPoint(i, pow(i,3)-0.02*pow(i,6) ) );
		

	plot->AddPlot( new wxPLLinePlot( pow_data, "i^3 -0.02\\dot i^6", *wxBLACK, wxPLLinePlot::SOLID ),
		wxPLPlotCtrl::X_BOTTOM,
		wxPLPlotCtrl::Y_LEFT,
		wxPLPlotCtrl::PLOT_TOP );



	plot->GetYAxis1()->SetLabel( "Pressure (kPa)" );
	plot->GetYAxis1()->SetColour( *wxRED );
	plot->GetYAxis1()->SetWorld( -20, 20 );
	//plot->SetFont( wxFont( 10, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false ) );

	/*
	wxFrame *frame2 = new wxFrame( 0, wxID_ANY, wxT("GCDC vs DC"), wxDefaultPosition, wxSize( 850, 950 ) );
	TextLayoutDemo *tldemo = new TextLayoutDemo( frame2 );
	frame2->Show();
	*/

	wxLogWindow *log = new wxLogWindow( frame , "Log");
	wxLog::SetActiveTarget(log);
	log->Show();
		
	plot->SetAllowHighlighting( true );
				
	frame->Show();
}

#include "wex/numeric.h"

class DViewApp : public wxApp
{
private:
	
	bool m_arg_showLog;
	int m_arg_tab, m_arg_data;
	double m_startHour, m_endHour;
	wxArrayString m_variables;
	wxArrayString m_arg_filenames;
	long m_lineMode;

public:
	bool OnInit()
	{
		//wxApp::OnInit handles all of our command line argument stuff.
		if (!wxApp::OnInit())
			return false;

		::wxInitAllImageHandlers();
		wxFileSystem::AddHandler(new wxZipFSHandler);
			
		DViewFrame *frame = new DViewFrame;
		
		if (m_arg_filenames.Count() > 0)
			frame->Load(m_arg_filenames);

		if (m_arg_tab != -1)
			frame->GetPlot()->SelectTabIndex(m_arg_tab);

		if (m_arg_data != -1)
			frame->GetPlot()->SelectDataIndex(m_arg_data);

		if ( m_startHour >= 0 && m_endHour >= 0 && m_startHour < m_endHour )
			frame->GetPlot()->SetTimeSeriesRange( m_startHour, m_endHour );

		if ( m_variables.size() > 0 )
			frame->GetPlot()->SetSelectedNames( m_variables );

		if ( m_lineMode >= 0 && m_lineMode <= 2 )
			frame->GetPlot()->SetTimeSeriesMode( m_lineMode );

		frame->Show();

		//TestPLPlot( frame );

		return true;
	}
	
	void OnInitCmdLine(wxCmdLineParser& parser)
	{
		wxApp::OnInitCmdLine(parser);

		parser.AddSwitch(wxT("l"), wxT("log"), wxT("show log window"));
		parser.AddOption(wxT("t"), wxT("tab"), wxT("initial tab number (zero-indexed)"), wxCMD_LINE_VAL_NUMBER);
		parser.AddOption(wxT("i"), wxT("index"), wxT("variable to display initially (zero-indexed)"), wxCMD_LINE_VAL_NUMBER);
		parser.AddOption(wxT("v"), wxT("variables"), wxT("comma separated list of column names to display initially"), wxCMD_LINE_VAL_STRING);
		parser.AddOption(wxT("s"), wxT("start"), wxT("starting hour to display"), wxCMD_LINE_VAL_DOUBLE);
		parser.AddOption(wxT("e"), wxT("end"), wxT("ending hour to display"), wxCMD_LINE_VAL_DOUBLE );
		parser.AddOption(wxT("m"), wxT("mode"), wxT("time series mode: 0=normal,1=stepped,2=stacked"), wxCMD_LINE_VAL_NUMBER);
		parser.AddParam(wxT("files to load"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	}

	bool OnCmdLineParsed(wxCmdLineParser& parser)
	{
		wxApp::OnCmdLineParsed(parser);

		m_arg_filenames.Alloc(parser.GetParamCount());
		for (size_t i=0; i<parser.GetParamCount(); i++)
			m_arg_filenames.Add(parser.GetParam(i));

		double dd;
		long tabNumber, varNumber;
		if (parser.Found(wxT("l")))
			m_arg_showLog = true;
		else
			m_arg_showLog = false;
		
		m_arg_tab = -1;
		if (parser.Found(wxT("t"), &tabNumber) && tabNumber >= 0)
			m_arg_tab = tabNumber;

		m_arg_data = -1;
		if (parser.Found(wxT("i"), &varNumber) && varNumber >= 0)
			m_arg_data = varNumber;

		m_startHour = -1;
		if ( parser.Found("s", &dd) )
			m_startHour = dd;
			
		m_endHour = -1;
		if ( parser.Found("e", &dd ) )
			m_endHour = dd;

		wxString ss;
		if ( parser.Found("v", &ss) )
			m_variables = wxStringTokenize( ss, ",;|" );

		m_lineMode = -1;
		if ( parser.Found("m", &varNumber )  && varNumber >= 0 && varNumber <= 2 )
			m_lineMode = varNumber;

		return true;
	}


};

IMPLEMENT_APP( DViewApp );
