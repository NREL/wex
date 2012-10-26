#include <wx/wx.h>
#include <wx/frame.h>
#include <wx/stc/stc.h>
#include <wx/webview.h>

#include "plot/plplotctrl.h"
#include "plot/pllineplot.h"

class MyApp : public wxApp
{
public:
	bool OnInit()
	{
		wxFrame *frame = new wxFrame( 0, wxID_ANY, wxT("wxPLPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"), wxDefaultPosition, wxSize(850,500) );
		frame->SetIcon( wxICON( appicon ) );
		
		wxPLPlotCtrl *plot = new wxPLPlotCtrl( frame, wxID_ANY, wxDefaultPosition, wxDefaultSize );
		//plot->SetBackgroundColour( *wxWHITE );
		plot->SetTitle( wxT("Demo Plot: using \\theta(x)=sin(x)^2, x_0=1 && \\zeta(x)=3\\dot sin^2(x)") );

		
		wxPLLabelAxis *mx = new wxPLLabelAxis( -1, 12, "Months of the year (\\Chi\\Psi)" );
		mx->ShowLabel( false );

		mx->Add( 0,  "Jan" );
		mx->Add( 1,  "Feb" );
		mx->Add( 2,  "Mar" );
		mx->Add( 3,  "Apr" );
		mx->Add( 4,  "May" );
		mx->Add( 5,  "Jun" );
		mx->Add( 6,  "Jul" );
		mx->Add( 7,  "Aug" );
		mx->Add( 8,  "Sep" );
		mx->Add( 9,  "Oct" );
		mx->Add( 10, "Nov" );
		mx->Add( 11, "Dec" );

		//plot->SetXAxis2( mx );
		/*
		plot->SetYAxis1( new wxPLLinearAxis(-140, 150, wxT("y1 label\ntakes up 2 lines")) );
		plot->SetYAxis1( new wxPLLinearAxis(-11, -3, wxT("\\theta(x)")), wxPLPlotCtrl::PLOT_BOTTOM );
		
		*/
		//plot->SetScaleTextSize( true );

		plot->ShowGrid( true, true );
		
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
			wxPLPlotCtrl::PLOT_TOP);


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

		return true;
	}
};

IMPLEMENT_APP( MyApp );
