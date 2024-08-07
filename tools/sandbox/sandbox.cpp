/*
BSD 3-Clause License

Copyright (c) Alliance for Sustainable Energy, LLC. See also https://github.com/NREL/wex/blob/develop/LICENSE
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <wx/wx.h>
#include <wx/frame.h>
#include <wx/stc/stc.h>
#include <wx/webview.h>
#include <wx/statbmp.h>
#include <wx/numformatter.h>
#include <wx/grid.h>
#include <wx/zstream.h>
#include <wx/dynlib.h>

#include "wex/icons/time.cpng"
#include "wex/icons/dmap.cpng"
#include "wex/icons/calendar.cpng"
#include "wex/icons/barchart.cpng"
#include "wex/icons/curve.cpng"
#include "wex/icons/scatter.cpng"
#include "wex/pdf/pdfdoc.h"
#include "wex/radiochoice.h"
#include "wex/csv.h"
//#include "wex/easycurl.h"


class PngTestApp : public wxApp {
public:
    bool OnInit() {
        wxInitAllImageHandlers();
        wxFrame *frm = new wxFrame(NULL, wxID_ANY, "SchedCtrl", wxDefaultPosition, wxSize(300, 200));
        frm->SetBackgroundColour(*wxWHITE);
        frm->Show();
        return true;
    }
};

//IMPLEMENT_APP( PngTestApp );

#include "wex/plot/plplotctrl.h"
#include "wex/plot/pllineplot.h"
#include "wex/plot/plbarplot.h"
#include "wex/plot/plscatterplot.h"
#include "wex/plot/plwindrose.h"
#include "wex/plot/plcontourplot.h"
#include "wex/plot/plsectorplot.h"

#include "wex/dview/dvplotctrl.h"
#include "wex/dview/dvselectionlist.h"

#include "wex/codeedit.h"
#include "wex/lkscript.h"
#include "wex/diurnal.h"
#include "wex/utils.h"

#include "wex/metro.h"

#include "wex/icons/cirplus.cpng"
#include "wex/icons/qmark.cpng"

#include "demo_bitmap.cpng"

#include "wex/uiform.h"
#include "wex/snaplay.h"

void TestDVSelectionCtrl() {
    wxFrame *frame = new wxFrame(0, wxID_ANY, wxT("wxDVSelectionCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(250, 510));
    wxDVSelectionListCtrl *sel = new wxDVSelectionListCtrl(frame, wxID_ANY,
                                                           2, wxDefaultPosition, wxDefaultSize,
                                                           wxDVSEL_RADIO_FIRST_COL);
    sel->SetBackgroundColour(*wxWHITE);
    //sel->SetFont( *wxSWISS_FONT );

    for (int gg = 1; gg <= 3; gg++)
        for (size_t i = 0; i < 5; i++)
            sel->Append(wxString::Format("Item %d", (int) i + 1), wxString::Format("Group %d", gg));

    sel->Append("Ungrouped A");
    sel->Append("Ungrouped B");
    sel->Append("Ungrouped C");

    frame->Show();
}

void TestSnapLayout(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, wxT("wxSnapLayout in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(850, 500));
#ifdef __WXMSW__
    frame->SetIcon(wxICON(appicon));
#endif

    wxSnapLayout *lay = new wxSnapLayout(frame, wxID_ANY);
    lay->SetShowSizing(true);

    lay->SetBackgroundText("Click on the button to create a new graph");

    wxPLPlotCtrl *plot = new wxPLPlotCtrl(lay, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Demo Plot: using \\theta(x)=sin(x)^2, x_0=1\n\\zeta(x)=3\\dot sin^2(x)"));
    plot->SetClientSize(400, 300);
    wxFont font(*wxNORMAL_FONT);
    font.SetPointSize(16);
    plot->SetFont(font);

    lay->Add(plot, 400, 300);
    lay->Add(new wxButton(lay, wxID_ANY, "EKJRLKEJWKR WJLKER WKEJRLKWJELR WE"));
    lay->Add(new wxButton(lay, wxID_ANY, "WE"));
    lay->Add(new wxCheckListBox(lay, wxID_ANY));
    lay->Add(new wxButton(lay, wxID_ANY, "%IN@)#*^^^^^$@#$_________________!!"));
    lay->Add(new wxCheckListBox(lay, wxID_ANY));

    frame->Show();
}

void TestPLPlot(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, wxT("wxPLPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(850, 500));
#ifdef __WXMSW__
    frame->SetIcon(wxICON(appicon));
#endif

    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Demo Plot: using \\theta(x)=sin(x)^2, x_0=1\n\\zeta(x)=3\\dot sin^2(x)"));

    wxFont font(*wxNORMAL_FONT);
    font.SetPointSize(16);
    plot->SetFont(font);

    wxPLLabelAxis *mx = new wxPLLabelAxis(-1, 12, "Months of the year (\\Chi\\Psi)");
    mx->ShowLabel(false);

    mx->Add(0, "Jan");
    mx->Add(1, "Feb");
    mx->Add(2, "March\nMarzo");
    mx->Add(3, "Apr");
    mx->Add(4, "May");
    mx->Add(5, "June\nJunio");
    mx->Add(6, "July");
    mx->Add(7, "August");
    mx->Add(8, "September");
    mx->Add(9, "October");
    mx->Add(10, "November");
    mx->Add(11, "December\nDeciembre");

    plot->SetXAxis2(mx);
    /*
    plot->SetYAxis1( new wxPLLinearAxis(-140, 150, wxT("y1 label\ntakes up 2 lines")) );
    plot->SetYAxis1( new wxPLLinearAxis(-11, -3, wxT("\\theta(x)")), wxPLPlotCtrl::PLOT_BOTTOM );

    */
    //plot->SetScaleTextSize( true );

    plot->ShowGrid(true, true);

    //  plot->SetXAxis1( new wxPLLogAxis( 0.01, 100, "\\nu  (m^3/kg)" ) );
    //	plot->SetXAxis1( new wxPLTimeAxis( 4, 200 ) );

    std::vector<wxRealPoint> sine_data;
    std::vector<wxRealPoint> cosine_data;
    std::vector<wxRealPoint> tangent_data;
    for (double x = -6; x < 3; x += 0.1) {
        sine_data.push_back(wxRealPoint(x, 3 * sin(x) * sin(x)));
        cosine_data.push_back(wxRealPoint(x / 2, 2 * cos(x / 2) * x));
        tangent_data.push_back(wxRealPoint(x, x * tan(x)));
    }

    plot->AddPlot(new wxPLLinePlot(sine_data, "3\\dot sin^2(x)", "forest green", wxPLLinePlot::DOTTED),
                  wxPLPlotCtrl::X_BOTTOM,
                  wxPLPlotCtrl::Y_LEFT,
                  wxPLPlotCtrl::PLOT_TOP);

    std::vector<wxRealPoint> bar_data;
    bar_data.push_back(wxRealPoint(1, 4));
    bar_data.push_back(wxRealPoint(2, 3));
    bar_data.push_back(wxRealPoint(3, 4));
    bar_data.push_back(wxRealPoint(4, 6));
    bar_data.push_back(wxRealPoint(5, 5));

    wxPLBarPlot *bar1 = new wxPLBarPlot(bar_data, 0.0, "bar1", *wxRED);
    wxPLBarPlot *bar2 = new wxPLBarPlot(bar_data, 0.0, "bar2", *wxGREEN);
    wxPLBarPlot *bar3 = new wxPLBarPlot(bar_data, 0.0, "bar3", *wxBLUE);

    std::vector<wxPLBarPlot *> group;
    group.push_back(bar1);
    group.push_back(bar2);
    group.push_back(bar3);

    bar1->SetGroup(group);
    bar2->SetGroup(group);
    bar3->SetGroup(group);

    plot->AddPlot(bar1);
    plot->AddPlot(bar2);
    plot->AddPlot(bar3);

    bar_data.clear();
    bar_data.push_back(wxRealPoint(-2, -15));
    bar_data.push_back(wxRealPoint(-3, -10));
    bar_data.push_back(wxRealPoint(2, -5));
    bar_data.push_back(wxRealPoint(1, 0));
    bar_data.push_back(wxRealPoint(-0.5, 5));

    wxPLHBarPlot *hbar1 = new wxPLHBarPlot(bar_data, 0.0, "hbar1", wxColour(255, 130, 0, 128));
    plot->AddPlot(hbar1);

    plot->GetXAxis1()->SetLabel("Bottom X Axis has a great sequence of \\nu  values!");
    plot->GetXAxis1()->SetReversed(true);

    plot->AddPlot(new wxPLLinePlot(cosine_data, "cos(\\Omega_\\alpha  )", *wxRED, wxPLLinePlot::DASHED),
                  wxPLPlotCtrl::X_BOTTOM,
                  wxPLPlotCtrl::Y_LEFT,
                  wxPLPlotCtrl::PLOT_TOP);

    wxPLLinePlot *lltan = new wxPLLinePlot(tangent_data, "\\beta\\dot tan(\\beta)", *wxBLUE, wxPLLinePlot::SOLID, 1,
                                           wxPLLinePlot::NO_MARKER);
    lltan->SetAntiAliasing(true);
    plot->AddPlot(lltan,
                  wxPLPlotCtrl::X_BOTTOM,
                  wxPLPlotCtrl::Y_LEFT,
                  wxPLPlotCtrl::PLOT_BOTTOM);

    std::vector<wxRealPoint> pow_data;
    for (double i = 0.01; i < 20; i += 0.1)
        pow_data.push_back(wxRealPoint(i, pow(i, 3) - 0.02 * pow(i, 6)));

    plot->AddPlot(new wxPLLinePlot(pow_data, "i^3 -0.02\\dot i^6", *wxBLACK, wxPLLinePlot::SOLID),
                  wxPLPlotCtrl::X_BOTTOM,
                  wxPLPlotCtrl::Y_LEFT,
                  wxPLPlotCtrl::PLOT_TOP);

    plot->SetHighlightMode(wxPLPlotCtrl::HIGHLIGHT_ZOOM);
    plot->GetYAxis1()->SetLabel("Pressure (kPa)");
    plot->GetYAxis1()->SetColour(*wxRED);
    plot->GetYAxis1()->SetWorld(-20, 20);
    plot->GetYAxis1()->SetReversed(true);

    //plot->SetFont( wxFont( 10, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false ) );

    /*
    wxFrame *frame2 = new wxFrame( 0, wxID_ANY, wxT("GCDC vs DC"), wxDefaultPosition, wxSize( 850, 950 ) );
    TextLayoutDemo *tldemo = new TextLayoutDemo( frame2 );
    frame2->Show();
    */

    wxLogWindow *log = new wxLogWindow(frame, "Log");
    wxLog::SetActiveTarget(log);
    log->Show();

    plot->SetHighlightMode(wxPLPlotCtrl::HIGHLIGHT_ZOOM);

    frame->Show();
}

#include <wx/wfstream.h>

class TextLayoutFrame : public wxFrame {
    wxPLTextLayoutDemo *m_layout;
public:
    TextLayoutFrame() : wxFrame(0, -1, "Text layout", wxDefaultPosition, wxSize(750, 550)) {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(new wxButton(this, wxID_SAVE, "Export to pdf"), 0, wxALL | wxEXPAND, 3);
        m_layout = new wxPLTextLayoutDemo(this);
        sizer->Add(m_layout, 1, wxALL | wxEXPAND, 3);
        SetSizer(sizer);

        Show();
    }

    void OnSave(wxCommandEvent &) {
        int width, height;
        GetClientSize(&width, &height);
        wxPdfDocument doc(wxPORTRAIT, "pt", wxPAPER_A5);
        doc.AddPage(wxPORTRAIT, width, height);

        wxString xml("C:\\Users\\adobos\\Projects\\wex\\pdffonts\\ComputerModernSerifRegular.xml");
        if (!doc.AddFont("CMSSR", wxEmptyString, xml)
            || !doc.SetFont("CMSSR", wxPDF_FONTSTYLE_REGULAR, 12))
            doc.SetFont("Helvetica", wxPDF_FONTSTYLE_REGULAR, 12);

        doc.SetTextColour(*wxBLACK);

        wxPLPdfOutputDevice pdfdc(doc, 12.0);
        m_layout->Draw(pdfdc, wxPLRealRect(0, 0, width, height));

        wxString file(wxFileName::GetHomeDir() + "/graph.pdf");

        const wxMemoryOutputStream &data = doc.CloseAndGetBuffer();
        wxFileOutputStream fp(file);
        if (!fp.IsOk()) return;

        wxMemoryInputStream tmpis(data);
        fp.Write(tmpis);
        fp.Close();

        wxLaunchDefaultBrowser(file);
    }

DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(TextLayoutFrame, wxFrame)
                EVT_BUTTON(wxID_SAVE, TextLayoutFrame::OnSave)
END_EVENT_TABLE()

void TestTextLayout() {
    new TextLayoutFrame;
}

#include <wx/dir.h>
#include <wx/zipstrm.h>
#include <wx/buffer.h>

//#include "c:/Users/adobos/Projects/wex/pdffonts/fontdata.h"

void TestFreeTypeText() {
    wxFrame *frame = new wxFrame(0, wxID_ANY, "FreeType text output", wxDefaultPosition, wxSize(800, 800));
    new wxFreeTypeDemo(frame);
    frame->Show();
}

#include <wex/mtrand.h>
#include <wex/plot/plcolourmap.h>
#include <wex/matrix.h>

void TestContourPlot() {
    wxFrame *frame = new wxFrame(0, wxID_ANY, wxT("wxPLContourPlot in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition,
                                 wxScaleSize(600, 500));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    plot->SetBackgroundColour(*wxWHITE);
    plot->SetHighlightMode(wxPLPlotCtrl::HIGHLIGHT_ZOOM);
//	int np = 49;
 //   plot->SetScaleTextSize(true);
//    wxFont font(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Consolas");
//    wxFont font(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Carlito");
//    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial Black");
//    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Arial Narrow");
//    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Calibri");
//    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Arial");
//    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false);
    //    wxFont font(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Calibri");
//    plot->SetFont(font);
    double zmin = 1e99, zmax = -1e99;
    wxMatrix<double> XX, YY, ZZ;

    // Example 1
    wxPLContourPlot::MeshGrid(-2, 2, 10, -2, 2, 10, XX, YY);
    ZZ.Resize(XX.Rows(), XX.Cols());

    size_t nx = XX.Cols();
    size_t ny = XX.Rows();

    for (size_t i = 0; i < ny; i++) {
        for (size_t j = 0; j < nx; j++) {
            double x = XX(i, j);
            double y = YY(i, j);
            ZZ(i, j) = x * exp(-x * x - y * y);

            if (ZZ(i, j) < zmin) zmin = ZZ(i, j);
            if (ZZ(i, j) > zmax) zmax = ZZ(i, j);
        }
    }
/*
	// Example 2
	wxPLContourPlot::Peaks(100, XX, YY, ZZ, &zmin, &zmax);

	// Example 3: Interpolating data via Delaunay with NaN mask
	std::vector<double> xdata, ydata, zdata;
	if (FILE *fp = fopen("c:/users/adobos/desktop/spray.csv", "r"))
	{
		char buf[256];
		fgets(buf, 255, fp);
		while (!feof(fp))
		{
			fgets(buf, 255, fp);
			double xv, yv, zv;
			sscanf(buf, "%lg,%lg,%lg", &xv, &yv, &zv);
			if (zv < 0) zv = 0;
			if (zv > 10) zv = 10;
			xdata.push_back(xv);
			ydata.push_back(yv);
			zdata.push_back(zv);
		}
		fclose(fp);
	}

	if (xdata.size() > 0)
	{
		double xmin, xmax, ymin, ymax;
		wxPLContourPlot::MinMax(xdata, &xmin, &xmax);
		wxPLContourPlot::MinMax(ydata, &ymin, &ymax);
		wxPLContourPlot::MinMax(zdata, &zmin, &zmax);

		int ngrid = 50;

		wxPLContourPlot::MeshGrid(xmin, xmax, ngrid, ymin, ymax, ngrid, XX, YY);
		bool ok = wxPLContourPlot::GridData(xdata, ydata, zdata, XX, YY, ZZ);
		if (!ok)
		{
			wxMessageBox("Error in delaunay interpoation of data to grid");
		}
	}
	else
		wxMessageBox("Could not load spray.csv");
*/
    //wxPLAxis::ExtendBoundsToNiceNumber( &zmax, &zmin );
    wxPLColourMap *jet = new wxPLJetColourMap(zmin, zmax);
    plot->SetSideWidget(jet);
    plot->ShowGrid(false, false);

    plot->AddPlot(new wxPLContourPlot(XX, YY, ZZ, false, "Test Contour", 24, jet));

    //plot->SetXAxis1( new wxPLLinearAxis( 0, np ) );
    //plot->SetYAxis1( new wxPLLinearAxis( 0, np ) );

    frame->Show();

}

void TestWaveAnnualEnergyPlot() {
    wxFrame *frame = new wxFrame(0, wxID_ANY, wxT("Wave Annual energy"), wxDefaultPosition,
                                 wxScaleSize(600, 500));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    plot->SetBackgroundColour(*wxBLACK);
    plot->SetHighlightMode(wxPLPlotCtrl::HIGHLIGHT_ZOOM);
   // wxFont font(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Consolas");
    //    wxFont font(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Carlito");
    //    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial Black");
    //    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Arial Narrow");
    //    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Calibri");
    //    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Arial");
    //    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false);
        //    wxFont font(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Calibri");
    //plot->SetFont(font);

    double zmin = 1e99, zmax = -1e99;
    wxMatrix<double> XX, YY, ZZ;


    wxCSVData csv;
    csv.ReadFile("c:/Projects/SAM/Documentation/MHK/2019.8.9_Final/WaveAnnualOutput.csv");
    // Example 1
    size_t nx = csv.NumRows(), ny = csv.NumCols();

/* fails to color zero values in grid
	XX.Resize(nx - 1, ny - 1);
	YY.Resize(nx - 1, ny - 1);
	ZZ.Resize(nx - 1, ny - 1);
	for (size_t i = 1; i < nx; i++)
	{
		for (size_t j = 1; j < ny; j++)
		{
			XX.At(i - 1, j - 1) = std::stod(csv.Get(0,j).ToStdString());
			YY.At(i - 1, j - 1) = std::stod(csv.Get(i, 0).ToStdString());
			ZZ.At(i - 1, j - 1) = std::stod(csv.Get(i, j).ToStdString());
			if (ZZ.At(i - 1, j - 1) < zmin) zmin = ZZ.At(i - 1, j - 1);
			if (ZZ.At(i - 1, j - 1) > zmax) zmax = ZZ.At(i - 1, j - 1);
		}
	}
*/

    wxPLContourPlot::MeshGrid(0.5, 20.5, 21, 0.25, 9.75, 20, XX, YY);
    ZZ.Resize(YY.Rows(), XX.Cols());
    for (size_t i = 1; i < nx; i++) {
        for (size_t j = 1; j < ny; j++) {
            ZZ.At(i - 1, j - 1) = std::stod(csv.Get(i, j).ToStdString());
            if (ZZ.At(i - 1, j - 1) < zmin) zmin = ZZ.At(i - 1, j - 1);
            if (ZZ.At(i - 1, j - 1) > zmax) zmax = ZZ.At(i - 1, j - 1);
        }
    }


    wxPLContourPlot *pl = 0;
    wxPLColourMap *jet = new wxPLJetColourMap(zmin, zmax);

    pl = new wxPLContourPlot(XX, YY, ZZ, true, wxEmptyString, 24, jet);
    if (pl != 0) {
        plot->AddPlot(pl, wxPLPlotCtrl::X_TOP, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
        plot->SetSideWidget(jet);
        plot->SetTitle("Annual energy (kWh)");
        /*
        wxArrayString as = jet->GetLabels();
        for (size_t i = 0; i < as.size(); i++)
            as[i] = as[i] + " kWh";
        jet->SetLabels(as);
        */
    }
    //wxPLAxis::ExtendBoundsToNiceNumber( &zmax, &zmin );
//	wxPLColourMap *jet = new wxPLJetColourMap(zmin, zmax);
//	plot->SetSideWidget(jet);
    plot->ShowGrid(true, true);

//	plot->SetYAxis1(new wxPLLinearAxis(0, 1.0, "Wind speed frequecy"));

//	plot->AddPlot(new wxPLContourPlot(XX, YY, ZZ, true, wxEmptyString, 24, jet));



    plot->GetYAxis1()->SetReversed(true);
    plot->GetYAxis1()->SetLabel("Hs = significant wave height (m)");
    plot->GetXAxis2()->SetLabel("Te = wave energy period (s)");
    /*
    wxPLAxis *y = plot->GetYAxis1();
    double ymin=0.25, ymax=9.75;
    std::vector<wxPLAxis::TickData> ta;
    y->GetAxisTicks(ymin, ymax, ta);


    wxPLLabelAxis *hs = new wxPLLabelAxis(ymin, ymax, "Hs = wave height (m)");
//	hs->ShowLabel(false);
//	hs->SetTickSizes(0.25, 0.75);

    for (size_t i = 0; i < ta.size(); i++)
    {
        hs->Add(ta[i].world, wxString::Format("0.2f", ymax - ta[i].world));
    }

    hs->Add(0, "9.75");
    hs->Add(1, "9.25");
    hs->Add(2, "8.75");
    hs->Add(3, "8.25");
    hs->Add(4, "7.75");
    hs->Add(5, "7.25");
    hs->Add(6, "6.75");
    hs->Add(7, "6.25");
    hs->Add(8, "5.75");
    hs->Add(9, "5.25");
    hs->Add(10, "4.75");
    hs->Add(11, "4.25");
    hs->Add(12, "3.75");
    hs->Add(13, "3.25");
    hs->Add(14, "2.75");
    hs->Add(15, "2.25");
    hs->Add(16, "1.75");
    hs->Add(17, "1.25");
    hs->Add(18, "0.75");
    hs->Add(19, "0.25");

    plot->SetYAxis1(hs);
    */

    frame->Show();

}


void TestWindPrufFigure2(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(800, 400));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    plot->ShowGrid(false, false);
    std::vector<wxRealPoint> data1, data2;

    double mu1 = 5000; // kWh mean gross annual energy output
    double sigma1 = mu1 / 50; // standard deviation
    double losses = 0.1 * mu1;
    double mu2 = mu1 - losses; // kWh net annual energy
    double sigma2 = mu2 / 40;

    int interval = 1000;
    double area1 = 0, area2 = 0, ymax = 0;
    for (int i = 0; i < interval; i++) {
        double x1 = mu1 - 3 * sigma1 + i * 6 * sigma1 / interval;
        double y1 = exp(0.0 - ((x1 - mu1) * (x1 - mu1) / (2 * sigma1 * sigma1))) / sqrt(2 * M_PI * sigma1 * sigma1);
        data1.push_back(wxRealPoint(x1, y1));
        if (i > 0)
            area1 += y1 * (x1 - data1[i - 1].x);
        double x2 = mu2 - 3 * sigma2 + i * 6 * sigma2 / interval;
        double y2 = exp(0.0 - ((x2 - mu2) * (x2 - mu2) / (2 * sigma2 * sigma2))) / sqrt(2 * M_PI * sigma2 * sigma2);
        data2.push_back(wxRealPoint(x2, y2));
        if (i > 0)
            area2 += y2 * (x2 - data2[i - 1].x);
        if (y1 > ymax) ymax = y1;
        if (y2 > ymax) ymax = y2;
    }


    plot->AddPlot(new wxPLLinePlot(data1, "Gross energy", wxColour("Black")));
    plot->AddPlot(new wxPLLinePlot(data2, "Net energy", wxColour("Blue")));

    std::vector<wxRealPoint> p50LineGross;
    p50LineGross.push_back(wxRealPoint(mu1, 1.1 * ymax));
    p50LineGross.push_back(wxRealPoint(mu1, 0));
    plot->AddAnnotation(new wxPLLineAnnotation(p50LineGross, 2, *wxBLACK, wxPLOutputDevice::DASH),
                        wxPLAnnotation::AXIS);

    std::vector<wxRealPoint> p50LineNet;
    p50LineNet.push_back(wxRealPoint(mu2, 1.1 * ymax));
    p50LineNet.push_back(wxRealPoint(mu2, 0));
    plot->AddAnnotation(new wxPLLineAnnotation(p50LineNet, 2, *wxBLUE, wxPLOutputDevice::DASH), wxPLAnnotation::AXIS);

    double p90 = 0.94 * mu2;
    std::vector<wxRealPoint> p90LineNet;
    p90LineNet.push_back(wxRealPoint(p90, 0.1 * ymax));
    p90LineNet.push_back(wxRealPoint(p90, 0));
    plot->AddAnnotation(new wxPLLineAnnotation(p90LineNet, 2, *wxBLUE, wxPLOutputDevice::DASH), wxPLAnnotation::AXIS);


    std::vector<wxRealPoint> LossArrow;
    LossArrow.push_back(wxRealPoint(mu1, ymax));
    LossArrow.push_back(wxRealPoint(mu2, ymax));
    plot->AddAnnotation(
            new wxPLLineAnnotation(LossArrow, 2, *wxBLUE, wxPLOutputDevice::DASH, wxPLLineAnnotation::FILLED_ARROW),
            wxPLAnnotation::AXIS);


    plot->AddAnnotation(
            new wxPLTextAnnotation("Predicted Losses", wxRealPoint(mu2 + 0.4 * (mu1 - mu2), 0.95 * ymax), 2.0, 0,
                                   *wxBLACK), wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("Gross Energy P50", wxRealPoint(mu1, 0.5 * ymax), 2.0, 90, *wxBLACK),
                        wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("Net Energy P50", wxRealPoint(mu2, 0.5 * ymax), 2.0, 90, *wxBLUE),
                        wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("P90", wxRealPoint(p90, 0.1 * ymax), 2.0, 0, *wxBLUE),
                        wxPLAnnotation::AXIS);
    plot->GetYAxis1()->Show(false);
    plot->GetXAxis1()->SetLabel("Annual Energy Delivered (kWh)");
    plot->ShowLegend(false);
    plot->SetBorderWidth(0);

    frame->Show();
}


void TestWindPrufFigure5(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(800, 400));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    plot->ShowGrid(false, false);
    std::vector<wxRealPoint> data1, data2;

    // wind speed bins
    // turbine curve

    double rated_power = 5000; // kWh mean gross annual energy output
    double max_speed = 40;
    double cut_out = 30;
    double cut_in = 7.5;

    wxMTRand rng;

    int interval = 2.5; // m/s bin
    for (int i = 0; i < (int) (max_speed / interval); i++) {
        double ws = i * interval;
        double freq = rng.rand();
        double power = 0;
        if (ws > cut_in && ws < cut_out) {
            power = rated_power;
        }
        data1.push_back(wxRealPoint(ws, freq));
        data2.push_back(wxRealPoint(ws, power));
    }


    plot->AddPlot(new wxPLBarPlot(data1, 0.0, "Wind speed frequency", wxColour("Blue")));
    plot->SetYAxis1(new wxPLLinearAxis(0, 1.0, "Wind speed frequecy"));

    plot->AddPlot(new wxPLLinePlot(data2, "Turbine power", wxColour("Gray")), wxPLPlot::X_BOTTOM, wxPLPlot::Y_RIGHT);
    plot->SetYAxis2(new wxPLLinearAxis(0, 1.05 * rated_power, "Turbine power"));


    std::vector<wxRealPoint> PowerLine;
    PowerLine.push_back(wxRealPoint(max_speed, rated_power));
    PowerLine.push_back(wxRealPoint(0.5 * max_speed, rated_power));
    plot->AddAnnotation(new wxPLLineAnnotation(PowerLine, 2, *wxBLUE, wxPLOutputDevice::DASH), wxPLAnnotation::AXIS,
                        wxPLPlot::X_BOTTOM, wxPLPlot::Y_RIGHT);

    plot->ShowLegend(false);


    /*
    std::vector<wxRealPoint> p50LineGross;
    p50LineGross.push_back(wxRealPoint(mu1, 1.1*ymax));
    p50LineGross.push_back(wxRealPoint(mu1, 0));
    plot->AddAnnotation(new wxPLLineAnnotation(p50LineGross, 2, *wxBLACK, wxPLOutputDevice::DASH), wxPLAnnotation::AXIS);

    std::vector<wxRealPoint> p50LineNet;
    p50LineNet.push_back(wxRealPoint(mu2, 1.1*ymax));
    p50LineNet.push_back(wxRealPoint(mu2, 0));
    plot->AddAnnotation(new wxPLLineAnnotation(p50LineNet, 2, *wxBLUE, wxPLOutputDevice::DASH), wxPLAnnotation::AXIS);

    double p90 = 0.94* mu2;
    std::vector<wxRealPoint> p90LineNet;
    p90LineNet.push_back(wxRealPoint(p90, 0.1*ymax));
    p90LineNet.push_back(wxRealPoint(p90, 0));
    plot->AddAnnotation(new wxPLLineAnnotation(p90LineNet, 2, *wxBLUE, wxPLOutputDevice::DASH), wxPLAnnotation::AXIS);


    std::vector<wxRealPoint> LossArrow;
    LossArrow.push_back(wxRealPoint(mu1, ymax));
    LossArrow.push_back(wxRealPoint(mu2, ymax));
    plot->AddAnnotation(new wxPLLineAnnotation(LossArrow, 2, *wxBLUE, wxPLOutputDevice::DASH, wxPLLineAnnotation::FILLED_ARROW), wxPLAnnotation::AXIS);


    plot->AddAnnotation(new wxPLTextAnnotation("Predicted Losses", wxRealPoint(mu2 + 0.4*(mu1 - mu2), 0.95*ymax), 2.0, 0, *wxBLACK), wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("Gross Energy P50", wxRealPoint(mu1, 0.5*ymax), 2.0, 90, *wxBLACK), wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("Net Energy P50", wxRealPoint(mu2, 0.5*ymax), 2.0, 90, *wxBLUE), wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("P90", wxRealPoint(p90, 0.1*ymax), 2.0, 0, *wxBLUE), wxPLAnnotation::AXIS);
    plot->GetYAxis1()->Show(false);
    plot->GetXAxis1()->SetLabel("Annual Energy Delivered (kWh)");
    plot->ShowLegend(false);
    plot->SetBorderWidth(0);
*/
    frame->Show();
}


void TestPlotAnnotations(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY,
                                 wxT("Plots with annotations in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(500, 400));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Example annotations plot for wind technologies in SAM"));
    plot->ShowGrid(false, false);
    std::vector<wxRealPoint> data;
    for (size_t i = 0; i < 20; i++)
        data.push_back(wxRealPoint(i, 20 * sin(i * 14.1)));

    plot->AddPlot(new wxPLBarPlot(data, 0.0, "Test data", wxColour("Forest green")));

    plot->AddAnnotation(new wxPLTextAnnotation("Annotation \\phi\\Phi \nhere^2", wxRealPoint(10, 0), 2, 0, *wxRED,
                                               wxPLTextLayout::RIGHT), wxPLAnnotation::AXIS);
    plot->AddAnnotation(new wxPLTextAnnotation("Origin_{0,0}", wxRealPoint(0, 0), -1, 0, *wxBLUE),
                        wxPLAnnotation::FRACTIONAL);
    plot->AddAnnotation(new wxPLTextAnnotation("((175pt 175pt))", wxRealPoint(175, 175), 0, 90, *wxBLACK),
                        wxPLAnnotation::POINTS);

    std::vector<wxRealPoint> line1;
    line1.push_back(wxRealPoint(5, 10));
    line1.push_back(wxRealPoint(10, 0));
    line1.push_back(wxRealPoint(15, -11));
    plot->AddAnnotation(new wxPLLineAnnotation(line1, 3, *wxBLACK), wxPLAnnotation::AXIS);
    line1.clear();
    line1.push_back(wxRealPoint(10, 10));
    line1.push_back(wxRealPoint(13, 11));
    line1.push_back(wxRealPoint(17, 3));
    line1.push_back(wxRealPoint(1, -3));
    plot->AddAnnotation(new wxPLLineAnnotation(line1, 5, *wxRED, wxPLOutputDevice::DOT), wxPLAnnotation::AXIS);

    line1.clear();
    line1.push_back(wxRealPoint(17, 3));
    line1.push_back(wxRealPoint(8, 9));
    plot->AddAnnotation(new wxPLLineAnnotation(line1, 0.5, *wxBLUE, wxPLOutputDevice::DOT), wxPLAnnotation::AXIS);

    plot->AddAnnotation(new wxPLBraceAnnotation(wxRealPoint(1, 9), wxRealPoint(3, 1), 1.0));
    plot->AddAnnotation(new wxPLBraceAnnotation(wxRealPoint(1, 9), wxRealPoint(7, 1), 1.0));

    frame->Show();
}


void TestMELCOESectorPlot(wxWindow* parent) {
    wxFrame* frame = new wxFrame(parent, wxID_ANY, wxT("ME LCOE"),
        wxDefaultPosition, wxSize(500, 400));
    wxPLPlotCtrl* plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Example of LCOE Contribution Pie Chart for ME SAM Results"));
    plot->ShowGrid(false, false);

    wxPLSectorPlot* sec = new wxPLSectorPlot();
//    sec->AddSector(64.89, "Total device cost");
//    sec->AddSector(17.27, "Total balance of system cost");
//    sec->AddSector(6.79, "Total financial cost");
//    sec->AddSector(11.05, "Total operating cost (annual)");
    sec->AddSector(32, "Total device cost");
    sec->AddSector(9, "Total balance of system cost");
    sec->AddSector(4, "Total financial cost");
    sec->AddSector(55, "Total operating cost (annual)");
    sec->SetCenterHoleSize(0.0);
//    sec->SetCalloutSize(65.9);
//    sec->SetBorder(100.4);
//    sec->ShowSegmentValues(true);
 //   sec->SetTextSpace(20.0);
    sec->SetAntiAliasing(true);
    std::vector<wxColour> clr;
    clr.push_back(wxColour(51,88,153));
    clr.push_back(wxColour(363,160,183));
    clr.push_back(wxColour(121,145,206));
    clr.push_back(wxColour(84,130,53));
    sec->SetColours(clr);

    sec->SetFormat(wxNUMERIC_REAL, wxNUMERIC_GENERIC, false, wxEmptyString, " %");
    plot->AddPlot(sec);
    plot->ShowAxes(false);
    plot->SetBorderWidth(0);
    frame->Show();
}



void TestSectorPlot(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, wxT("Sector plots in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(500, 400));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Example sector plot for wind technologies in SAM"));
    plot->ShowGrid(false, false);

    wxPLSectorPlot *sec = new wxPLSectorPlot();
    sec->AddSector(17, "Rotor");
    sec->AddSector(41, "Nacelle");
    sec->AddSector(13, "Tower");
    sec->AddSector(2, "Development");
    sec->AddSector(1, "Engineering management");
    sec->AddSector(3, "Foundation");
    sec->AddSector(3, "Site access and staging");
    sec->AddSector(2, "Assembly and installation");
    sec->AddSector(9, "Electric infrastracture");
    sec->AddSector(6, "Contingency");
    sec->AddSector(3, "Construction finance");

    sec->AddInnerSector(71, "Turbine\n71 %");
    sec->AddInnerSector(20, "Balance\nof System\n20 %");
    sec->AddInnerSector(9, "Financial\n9 %");

    sec->SetFormat(wxNUMERIC_REAL, wxNUMERIC_GENERIC, false, wxEmptyString, " %");
    plot->AddPlot(sec);
    plot->ShowAxes(false);
    plot->SetBorderWidth(0);
    frame->Show();
}

void TestStackedBarPlot(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY,
                                 wxT("wxPLPolarPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(500, 400));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Demo Plot: using stuff"));

    wxMTRand rng;
    std::vector<wxRealPoint> bars1, bars2, bars3;
    for (size_t i = 0; i < 1; i++) {
        bars1.push_back(wxRealPoint(i, rng()));
        bars2.push_back(wxRealPoint(i, rng()));
        bars3.push_back(wxRealPoint(i, rng()));
    }

    wxPLBarPlot *bar1 = new wxPLBarPlot(bars1, 0.0, "Bar1", *wxRED);

    wxPLBarPlot *bar2 = new wxPLBarPlot(bars2, 0.0, "Bar2", *wxGREEN);
    bar2->SetStackedOn(bar1);
    wxPLBarPlot *bar3 = new wxPLBarPlot(bars3, 0.0, "Bar3", *wxBLUE);
    bar3->SetStackedOn(bar2);

//	std::vector<wxPLBarPlot*> group;
//	group.push_back(bar1);
//	group.push_back(bar2);
//	group.push_back(bar3);

//	bar1->SetGroup(group);
//	bar2->SetGroup(group);
//	bar3->SetGroup(group);


    plot->AddPlot(bar1, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
    plot->AddPlot(bar2, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
    plot->AddPlot(bar3, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);

    plot->X1().SetWorld(-1, 1);
/*
	std::vector<wxRealPoint> hb;
	for (size_t i = 0; i < 48; i++)
		hb.push_back(wxRealPoint(i, rng()));

	plot->AddPlot(new wxPLBarPlot(hb, 0.0, "Test", *wxLIGHT_GREY), wxPLPlot::X_BOTTOM, wxPLPlot::Y_RIGHT, wxPLPlot::PLOT_BOTTOM);
*/
    frame->Show();
}

void TestSAMStackedBarPlot(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY,
                                 wxT("wxPLPolarPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(500, 400));
    wxPLPlotCtrl *plotctrl = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    wxArrayString yvars;
    yvars.push_back("NPV1");
    yvars.push_back("NPV2");
    yvars.push_back("NPV3");
    yvars.push_back("NPV4");
    yvars.push_back("NPV5");
    plotctrl->SetTitle(wxT("SAM Demo Plot: using stuff"));

    std::vector<std::vector<wxRealPoint>> plotdata(yvars.size());

    int cidx = 0; // colour index
//	wxPLBarPlot *last_bar = 0;
//	std::vector<wxPLBarPlot*> bar_group;
    std::vector<wxColour> s_colours;
    s_colours.push_back(wxColour(111, 164, 196));
    s_colours.push_back(wxColour("GREY"));
    s_colours.push_back(wxColour(181, 211, 227));
    s_colours.push_back(*wxLIGHT_GREY);
    s_colours.push_back(wxColour("PALE GREEN"));
    s_colours.push_back(wxColour("GOLDENROD"));
    s_colours.push_back(wxColour("MEDIUM VIOLET RED"));
    s_colours.push_back(wxColour("MEDIUM SEA GREEN"));
    s_colours.push_back(wxColour("DARK SLATE GREY"));
    s_colours.push_back(wxColour("WHEAT"));
    s_colours.push_back(wxColour("FIREBRICK"));
    s_colours.push_back(wxColour("dark orchid"));
    s_colours.push_back(wxColour("dim grey"));
    s_colours.push_back(wxColour("brown"));


    std::vector<wxPLBarPlot *> barplots(yvars.size());
    wxMTRand rng;

    for (size_t i = 0; i < yvars.size(); i++) {
        plotdata[i].push_back(wxRealPoint(0, rng()));
    }
    std::vector<wxRealPoint> rp;
    for (size_t i = 0; i < yvars.size(); i++) {
//		rp.clear();
        // fails
//		rp = plotdata[i];
//		barplots[i] = new wxPLBarPlot(rp, 0.0, yvars[i], s_colours[cidx]);
        /* works
        for (size_t j = 0; j < yvars.size(); j++)
        {
            rp.push_back(wxRealPoint(j, rng()));
        }
        barplots[i] = new wxPLBarPlot(rp, 0.0, yvars[i], s_colours[cidx]);
        for (size_t j = 0; j < plotdata[i].size(); j++)
        {
            wxRealPoint x = plotdata.at(i)[j];
            rp.push_back(x);
        }
        */

//		rp=plotdata[i];
        barplots[i] = new wxPLBarPlot(plotdata[i], 0.0, yvars[i], s_colours[cidx]);
        if (i > 0) barplots[i]->SetStackedOn(barplots[i - 1]);
        if (++cidx >= (int) s_colours.size()) cidx = 0; // incr and wrap around colour index
        plotctrl->AddPlot(barplots[i], wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
    }

    /*

    std::vector<wxRealPoint> rp1, rp2, rp3, rp4, rp5;
    rp1.push_back(wxRealPoint(0, rng()));
    rp2.push_back(wxRealPoint(0, rng()));
    rp3.push_back(wxRealPoint(0, rng()));
    rp4.push_back(wxRealPoint(0, rng()));
    rp5.push_back(wxRealPoint(0, rng()));
    for (size_t i = 0; i < barplots.size(); i++)
    {
        if (barplots[i] != 0)
            plotctrl->AddPlot(barplots[i], wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
    }
    */


/* this works
	wxPLBarPlot* bp1 = new wxPLBarPlot(rp1, 0.0, yvars[0], s_colours[0]);
	wxPLBarPlot* bp2 = new wxPLBarPlot(rp2, 0.0, yvars[1], s_colours[1]);
	wxPLBarPlot* bp3 = new wxPLBarPlot(rp3, 0.0, yvars[2], s_colours[2]);
	wxPLBarPlot* bp4 = new wxPLBarPlot(rp4, 0.0, yvars[3], s_colours[3]);
	wxPLBarPlot* bp5 = new wxPLBarPlot(rp5, 0.0, yvars[4], s_colours[4]);

	bp2->SetStackedOn(bp1);
	bp3->SetStackedOn(bp2);
	bp4->SetStackedOn(bp3);
	bp5->SetStackedOn(bp4);

	plotctrl->AddPlot(bp1);
	plotctrl->AddPlot(bp2);
	plotctrl->AddPlot(bp3);
	plotctrl->AddPlot(bp4);
	plotctrl->AddPlot(bp5);
*/
/* this works
	barplots[0] = new wxPLBarPlot(rp1, 0.0, yvars[0], s_colours[0]);
	barplots[1] = new wxPLBarPlot(rp2, 0.0, yvars[1], s_colours[1]);
	barplots[2] = new wxPLBarPlot(rp3, 0.0, yvars[2], s_colours[2]);
	barplots[3] = new wxPLBarPlot(rp4, 0.0, yvars[3], s_colours[3]);
	barplots[4] = new wxPLBarPlot(rp5, 0.0, yvars[4], s_colours[4]);

	for (size_t i = 0; i < barplots.size(); i++)
	{
		if (i>0) barplots[i]->SetStackedOn(barplots[i - 1]);
		plotctrl->AddPlot(barplots[i], wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
	}
	*/
/* this works
	std::vector<wxRealPoint> rp(1);
	for (size_t i = 0; i < barplots.size(); i++)
	{
		rp.clear();
		rp.push_back(wxRealPoint(0, rng()));
		barplots[i] = new wxPLBarPlot(rp, 0.0, yvars[i], s_colours[i]);
		if (i>0) barplots[i]->SetStackedOn(barplots[i - 1]);
		plotctrl->AddPlot(barplots[i], wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, wxPLPlotCtrl::PLOT_TOP, true);
	}
*/
    plotctrl->X1().SetWorld(-1, 1);


    frame->Show();
}


void TestPLBarPlot(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY,
                                 wxT("wxPLPolarPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(500, 400));
    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Demo Plot: using stuff"));

    wxMTRand rng;
    std::vector<wxRealPoint> bars1, bars2, bars3;
    for (size_t i = 0; i < 16; i++) {
        bars1.push_back(wxRealPoint(i, rng()));
        bars2.push_back(wxRealPoint(i, rng()));
        bars3.push_back(wxRealPoint(i, rng()));
    }

    wxPLBarPlot *bar1 = new wxPLBarPlot(bars1, 0.0, "Bar1", *wxRED);
    wxPLBarPlot *bar2 = new wxPLBarPlot(bars2, 0.0, "Bar2", *wxGREEN);
    wxPLBarPlot *bar3 = new wxPLBarPlot(bars3, 0.0, "Bar3", *wxBLUE);

    std::vector<wxPLBarPlot *> group;
    group.push_back(bar1);
    group.push_back(bar2);
    group.push_back(bar3);

    bar1->SetGroup(group);
    bar2->SetGroup(group);
    bar3->SetGroup(group);

    plot->AddPlot(bar1);
    plot->AddPlot(bar2);
    plot->AddPlot(bar3);

    plot->X1().SetWorld(-1, 16);

    std::vector<wxRealPoint> hb;
    for (size_t i = 0; i < 48; i++)
        hb.push_back(wxRealPoint(i, rng()));

    plot->AddPlot(new wxPLBarPlot(hb, 0.0, "Test", *wxLIGHT_GREY), wxPLPlot::X_BOTTOM, wxPLPlot::Y_RIGHT,
                  wxPLPlot::PLOT_BOTTOM);

    frame->Show();
}

void TestPLPolarPlot(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY,
                                 wxT("wxPLPolarPlotCtrl in \x01dc\x03AE\x03AA\x00C7\x00D6\x018C\x01dd"),
                                 wxDefaultPosition, wxSize(850, 850));
#ifdef __WXMSW__
    frame->SetIcon(wxICON(appicon));
#endif

    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //plot->SetBackgroundColour( *wxWHITE );
    plot->SetTitle(wxT("Demo Plot: using stuff"));

    wxFont font(*wxNORMAL_FONT);
    font.SetPointSize(12);
    plot->SetFont(font);

    // setting X axis 1 to a wxPLPolarAngularAxis will tell the control to plot a polar graph
    //plot->SetXAxis1(new wxPLPolarAngularAxis("Angular Axis"));
    //plot->SetXAxis1(new wxPLPolarAngularAxis("Angular Axis", wxPLPolarAngularAxis::GRADIANS, wxPLPolarAngularAxis::UP));
    plot->SetXAxis1(new wxPLPolarAngularAxis("Angular Axis", wxPLPolarAngularAxis::GRADIANS, wxPLPolarAngularAxis::DOWN));
    //plot->SetScaleTextSize( true );

    plot->ShowGrid(true, true);

//	double pi = 4.0 * atan(1.0);
    std::vector<wxRealPoint> sine_data;
    for (double x = 0; x < 361; x += 1) {
        //sine_data.push_back(wxRealPoint(x, 8 + 0.5*cos(pi * x/10)));
        //sine_data.push_back(wxRealPoint(x, 3 * sin(x/12)*sin(x/12)));
        sine_data.push_back(wxRealPoint(x, 8 + cos(x / 6) *x / 30));
    }

    //sine_data.push_back(wxRealPoint(0, 0));
    //sine_data.push_back(wxRealPoint(15, 2));
    //sine_data.push_back(wxRealPoint(30, 3));
    //sine_data.push_back(wxRealPoint(45, 4));
    //sine_data.push_back(wxRealPoint(60, 4));
    //sine_data.push_back(wxRealPoint(100, 8));
    //sine_data.push_back(wxRealPoint(200, 9));

    //sine_data.push_back(wxRealPoint(0, 0));
    //sine_data.push_back(wxRealPoint(pi/3.0, 5));
    //sine_data.push_back(wxRealPoint(pi/2.0, 3));
    //sine_data.push_back(wxRealPoint(pi, 4));
    //sine_data.push_back(wxRealPoint(pi*1.1, 4));
    //sine_data.push_back(wxRealPoint(pi*1.5, 8));
    //sine_data.push_back(wxRealPoint(pi*2.0, 9));

    //plot->AddPlot(new wxPLScatterPlot(sine_data, "Test data", *wxBLUE, 2));
    plot->AddPlot(new wxPLLinePlot(sine_data, "Test data", *wxBLUE));

    plot->SetYAxis1(new wxPLLinearAxis(0, 20, "Radial Axis Label"));
    frame->Show();
}

void TestWindRose(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, wxT("Testing wind rose"), wxDefaultPosition, wxSize(850, 850));
#ifdef __WXMSW__
    frame->SetIcon(wxICON(appicon));
#endif

    wxPLPlotCtrl *plot = new wxPLPlotCtrl(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    plot->SetTitle(wxT("Test wind rose"));

    plot->ShowGrid(true, true);

    plot->SetYAxis1(new wxPLLinearAxis(0, 22, "Radial Axis"));

    std::vector<wxRealPoint> data;
    //for (int i = 0; i < 18; ++i) {
    //	data.push_back(wxRealPoint(i * 10.0, 5+sin(i/6.0)*2.0) );
    //}

//    for (double x = 0; x < 360; x += 10) {
        for (double x = 0; x < 360; x += 15) {
        //sine_data.push_back(wxRealPoint(x, 8 + 0.5*cos(pi * x/10)));
        //sine_data.push_back(wxRealPoint(x, 3 * sin(x/12)*sin(x/12)));
    //    data.push_back(wxRealPoint(x, 8.0 + cos(x / 6.0) * x / 30.0));
        data.push_back(wxRealPoint(x, 8.0 ));
    }

    wxPLWindRose *wr = new wxPLWindRose(data, "Test data", *wxBLUE);
    wr->SetIgnoreAngle(false);
    wr->ShowInLegend(true);
    plot->AddPlot(wr); // adding a wxPLWindRose plot will automatically set the x-axis to a wxPLPolarAngularAxis

    frame->Show();
}

#include "wex/dview/dvtimeseriesdataset.h"

void TestDView(wxWindow *parent) {
    wxFrame *frame = new wxFrame(parent, wxID_ANY, "Test wxDView", wxDefaultPosition, wxSize(900, 700));
    wxDVPlotCtrl *dview = new wxDVPlotCtrl(frame, wxID_ANY);

    std::vector<double> data(8760);

    for (size_t i = 0; i < 8760; i++)
        data[i] = 123 + i * 100 - i * i * 0.01;
    dview->AddDataSet(new wxDVArrayDataSet("curve 1", data), "Group 1");

    for (size_t i = 0; i < 8760; i++)
        data[i] = ::sin(i * 0.001) * 10000;
    dview->AddDataSet(new wxDVArrayDataSet("curve 2", data), "Group 2");

    for (size_t i = 0; i < 8760; i++)
        data[i] = ::cos(i * 0.01) * 15000;
    dview->AddDataSet(new wxDVArrayDataSet("curve 3", data), "Group 1");

    for (size_t i = 0; i < 8760; i++)
        data[i] = ::sin(i * 0.1) * 5000;
    dview->AddDataSet(new wxDVArrayDataSet("curve 4", data), "Group 2");

    for (size_t i = 0; i < 8760; i++)
        data[i] = ::cos(i * 0.0001) * 2500;
    dview->AddDataSet(new wxDVArrayDataSet("curve 5", data), "Group 2");

    frame->Show();
}

#include <wex/numeric.h>
#include <wex/exttext.h>

enum {
    ID_NUMERIC = wxID_HIGHEST + 121, ID_EXTTEXT, ID_CLEAR, ID_MENU_TEST,
    ID_MENU_FIRST, ID_MENU_LAST = ID_MENU_FIRST + 40
};

class NumericTest : public wxFrame {
    int m_counter;
    wxNumericCtrl *m_num;
    wxExtTextCtrl *m_txt;
    wxTextCtrl *m_log;
    wxButton *m_button;
    wxMetroNotebook *m_nb;
public:
    NumericTest() : wxFrame(NULL, wxID_ANY, "Numeric Test", wxDefaultPosition, wxSize(700, 700)) {
        wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

        m_counter = 0;
        m_num = new wxNumericCtrl(panel, ID_NUMERIC);
        m_txt = new wxExtTextCtrl(panel, ID_EXTTEXT);

        m_log = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY);

        m_button = new wxButton(panel, ID_MENU_TEST, "Test popup");

        wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(m_num, 0, wxALL | wxEXPAND, 10);
        sizer->Add(m_txt, 0, wxALL | wxEXPAND, 10);
        sizer->AddStretchSpacer();
        sizer->Add(new wxButton(panel, ID_CLEAR, "Clear"), 0, wxALL | wxEXPAND, 10);
        sizer->Add(m_button);

        wxBoxSizer *main = new wxBoxSizer(wxVERTICAL);
        main->Add(sizer, 0, wxALL | wxEXPAND, 10);
        main->Add(m_log, 1, wxALL | wxEXPAND, 10);

        m_nb = new wxMetroNotebook(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxMT_LIGHTTHEME);
        m_nb->AddPage(new wxPanel(m_nb), "Page 1", false, true);
        m_nb->AddPage(new wxPanel(m_nb), "Page 2");
        m_nb->AddPage(new wxPanel(m_nb), "Page 3", false, true);
        m_nb->AddPage(new wxPanel(m_nb), "Page 4");

        main->Add(m_nb, 1, wxALL | wxEXPAND, 0);

        panel->SetSizer(main);
    }

    void OnNumeric(wxCommandEvent &) {
        m_log->AppendText(wxString::Format("%d --> numeric changed %.6lf\n", ++m_counter, m_num->Value()));
    }

    void OnExtText(wxCommandEvent &) {
        m_log->AppendText(wxString::Format("%d --> exttext changed\n", ++m_counter));
    }

    void OnClear(wxCommandEvent &) {
        m_log->Clear();
    }

    void OnMenu(wxCommandEvent &evt) {
        m_log->AppendText(wxString::Format("popup menu item clicked: %d\n", evt.GetId()));
    }

    void OnMenuTest(wxCommandEvent &) {
        m_log->AppendText("menu popup initiated\n");
        wxMetroPopupMenu menu;
        //	menu.SetFont( wxMetroTheme::Font( wxMT_LIGHT, 20 ) );

        int id = ID_MENU_FIRST;
        menu.Append(id++, "First item\tF2");
        menu.Append(id++, "Second item\tCtrl-O");
        menu.Append(id++, "Third item");
        menu.Append(id++, "Fourth item");
        menu.AppendSeparator();
        menu.AppendCheckItem(id++, "Check 1");
        menu.AppendCheckItem(id++, "Check 2");
        menu.AppendCheckItem(id++, "Check 3", true);
        menu.AppendSeparator();
        menu.Append(id++, "Exit");
        menu.Popup(this);
    }

DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(NumericTest, wxFrame)
                EVT_NUMERIC(ID_NUMERIC, NumericTest::OnNumeric)
                EVT_TEXT_ENTER(ID_EXTTEXT, NumericTest::OnExtText)
                EVT_BUTTON(ID_CLEAR, NumericTest::OnClear)
                EVT_BUTTON(ID_MENU_TEST, NumericTest::OnMenuTest)
                EVT_MENU_RANGE(ID_MENU_FIRST, ID_MENU_LAST, NumericTest::OnMenu)
END_EVENT_TABLE()

#include "wex/dview/dvtimeseriesdataset.h"
#include "wex/dview/dvtimeseriesctrl.h"
#include "wex/dview/dvdmapctrl.h"
#include "wex/dview/dvprofilectrl.h"
#include "wex/dview/dvpncdfctrl.h"
#include "wex/dview/dvdcctrl.h"
#include "wex/dview/dvscatterplotctrl.h"
#include "wex/dview/dvplotctrlsettings.h"
#include "wex/dview/dvstatisticstablectrl.h"

class MyNoteTest : public wxMetroNotebook {
public:
    MyNoteTest(wxWindow *parent)
            : wxMetroNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxMT_LIGHTTHEME) {
        AddPage(new wxDVTimeSeriesCtrl(this, wxID_ANY, wxDV_RAW, wxDV_AVERAGE), "Time series", true);
        AddPage(new wxDVTimeSeriesCtrl(this, wxID_ANY, wxDV_HOURLY, wxDV_AVERAGE), "Hourly");
        AddPage(new wxDVTimeSeriesCtrl(this, wxID_ANY, wxDV_DAILY, wxDV_AVERAGE), "Daily");
        AddPage(new wxDVTimeSeriesCtrl(this, wxID_ANY, wxDV_MONTHLY, wxDV_AVERAGE), "Monthly");
        AddPage(new wxDVDMapCtrl(this, wxID_ANY), "Heat map");
        AddPage(new wxDVProfileCtrl(this, wxID_ANY), "Profile");
        AddPage(new wxDVStatisticsTableCtrl(this, wxID_ANY), "Statistics");
        AddPage(new wxDVPnCdfCtrl(this, wxID_ANY), "PDF / CDF");
        AddPage(new wxDVDCCtrl(this, wxID_ANY), "Duration curve");
        AddPage(new wxDVScatterPlotCtrl(this, wxID_ANY), "Scatter");
    }

    void OnPageChanging(wxNotebookEvent &e) {
        wxLogStatus("page changing: %d --> %d", e.GetOldSelection(), e.GetSelection());
    }

DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MyNoteTest, wxMetroNotebook)
                EVT_NOTEBOOK_PAGE_CHANGING(wxID_ANY, MyNoteTest::OnPageChanging)
END_EVENT_TABLE()

void TestFormDesigner() {
    wxUIObjectTypeProvider::RegisterBuiltinTypes();
    wxUIFormData *form = new wxUIFormData;
    wxFrame *frm = new wxFrame(0, wxID_ANY, "Form Editor", wxDefaultPosition, wxSize(900, 600));
    wxUIFormDesigner *fd = new wxUIFormDesigner(frm, wxID_ANY);

    wxUIPropertyEditor *pe = new wxUIPropertyEditor(frm, wxID_ANY);
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(pe, 1, wxALL | wxEXPAND, 0);
    sizer->Add(fd, 5, wxALL | wxEXPAND, 0);
    frm->SetSizer(sizer);

    fd->SetFormData(form);
    fd->SetPropertyEditor(pe);
    frm->Show();
}

#include <wex/gleasy.h>

class MyApp : public wxApp {
    wxLocale m_locale;
public:
    bool OnInit() {
        if (!wxApp::OnInit())
            return false;


//		return true;

#ifdef __WXMSW__
        typedef BOOL(WINAPI *SetProcessDPIAware_t)(void);
        wxDynamicLibrary dllUser32(wxT("user32.dll"));
        SetProcessDPIAware_t pfnSetProcessDPIAware =
            (SetProcessDPIAware_t)dllUser32.RawGetSymbol(wxT("SetProcessDPIAware"));
        if (pfnSetProcessDPIAware)
            pfnSetProcessDPIAware();
#endif

        wxInitAllImageHandlers();

        wxString wexdir;
        if (wxGetEnv("WEXDIR", &wexdir)) {
            if (!wxPLPlot::AddPdfFontDir(wexdir + "/pdffonts"))
                wxMessageBox("Could not add font dir: " + wexdir + "/pdffonts");
//            if (!wxPLPlot::SetPdfDefaultFont("Carlito"))
//            if (!wxPLPlot::SetPdfDefaultFont("ComputerModernSansSerifBold"))
//                wxMessageBox("Could not set default pdf font to Computer Modern Sans Serif");
        }

        //	wxStopWatch sw;
        //	int nf = wxFreeTypeLoadAllFonts();
        //	wxMessageBox( wxString::Format("Loaded %d fonts in %d ms.", nf, (int)sw.Time()) );

        TestContourPlot();
//        TestWaveAnnualEnergyPlot();

        TestWindRose(0);
//		TestPLPlot(0);
		TestPLPolarPlot(0);
//		TestPLBarPlot(0);

//        TestContourPlot();
//		TestStackedBarPlot(0);
//		TestSAMStackedBarPlot(0);
//		TestSectorPlot(0);
//        TestMELCOESectorPlot(0);
//		TestTextLayout();
//        TestFreeTypeText();
//		TestPlotAnnotations(0);
		TestWindPrufFigure2(0);
		TestWindPrufFigure5(0);

        //wxFrame *frmgl = new wxFrame( NULL, wxID_ANY, "GL Easy Test", wxDefaultPosition, wxSize(700,700) );
        //new wxGLEasyCanvasTest( frmgl );
        //frmgl->Show();

        //return true;

        ////TestFormDesigner();
        ////return true;

        ////wxMessageBox(wxString::Format("Can handle gzip? %d", wxZlibInputStream::CanHandleGZip() ? 1 : 0 ) );

        ////bool ok = wxGunzipFile( "c:/users/adobos/desktop/geostellar/weather/weather_074654025.tm2.gz",
        ////	"c:/users/adobos/desktop/geostellar/weather/unzipped.tm2" );

        ////if (ok) wxMessageBox("unzipped ok" );
        ////else wxMessageBox("error");

        //m_locale.Init();

        //wxLogWindow *log = new wxLogWindow(0, "Log");
        //wxLog::SetActiveTarget(log);
        //log->Show();

        //wxFrame *frame2 = new wxFrame(0, wxID_ANY, "radiochoice");
        //wxRadioChoice *rad = new wxRadioChoice(frame2, wxID_ANY);
        //rad->SetHorizontal(true);
        //rad->Add("Choice 1 ");
        //rad->Add("Choice 2 ----- x ");
        //wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
        //sizer2->Add(rad, 1, wxALL | wxEXPAND, 10);
        //sizer2->Add(new wxNumericCtrl(frame2, wxID_ANY, 1000000, wxNUMERIC_UNSIGNED), 0, wxALL | wxEXPAND, 10);
        //frame2->SetSizerAndFit(sizer2);
        //frame2->Show();
        //wxSize size2(rad->GetBestSize());
        //wxMessageBox(wxString::Format("%d %d", size2.x, size2.y));

        ////wxFrame *frame = new NumericTest();
        ////frame->Show();

        ////

        ////TestSnapLayout( 0 );
        //return true;

        ////TestDVSelectionCtrl();

        ////TestPLPolarPlot(0);

        ////TestWindRose(0);

        ///*
        //wxFrame *frame = new wxFrame(NULL, wxID_ANY, "Test LKEdit", wxDefaultPosition, wxSize(600,400) );
        //new wxLKScriptCtrl(frame, wxID_ANY );
        //frame->Show();
        //*/

        wxFrame *frm = new wxFrame(NULL, wxID_ANY, "SchedCtrl", wxDefaultPosition, wxSize(1100, 700));
        frm->SetBackgroundColour(*wxWHITE);

        wxBoxSizer *tools = new wxBoxSizer(wxHORIZONTAL);
        tools->Add(new wxMetroButton(frm, wxID_ANY, wxEmptyString, wxBITMAP_PNG_FROM_DATA(demo_bitmap), wxDefaultPosition, wxDefaultSize), 0, wxALL | wxEXPAND, 0);
        tools->Add(new wxMetroButton(frm, wxID_ANY, "New", wxBITMAP_PNG_FROM_DATA(cirplus), wxDefaultPosition, wxDefaultSize), 0, wxALL | wxEXPAND, 0);
        wxMetroTabList *tabs = new wxMetroTabList(frm, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxMT_MENUBUTTONS);
        tabs->Append("photovoltaic #1");
        tabs->Append("solar water");
        tabs->Append("power tower steam");
        tools->Add(tabs, 1, wxALL | wxEXPAND, 0);
        tools->Add(new wxMetroButton(frm, wxID_ANY, wxEmptyString, wxBITMAP_PNG_FROM_DATA(qmark), wxDefaultPosition, wxDefaultSize), 0, wxALL | wxEXPAND, 0);
        //tools->Add( new wxMetroButton( frm, wxID_ANY, wxEmptyString, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxMB_DOWNARROW), 0, wxALL|wxEXPAND, 0 );

        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(tools, 0, wxALL | wxEXPAND, 0);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, "Start", wxBITMAP_PNG_FROM_DATA(demo_bitmap), wxDefaultPosition, wxDefaultSize, wxMB_RIGHTARROW), 0, wxALL, 3);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, "Stretched Start", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxMB_RIGHTARROW), 0, wxALL | wxEXPAND, 3);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, "Test button"), 0, wxALL, 3);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, "Popup menu", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxMB_DOWNARROW), 0, wxALL, 3);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, "Left align label", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxMB_ALIGNLEFT), 0, wxEXPAND | wxALL, 3);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, "Left align arrow", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxMB_ALIGNLEFT | wxMB_RIGHTARROW), 0, wxEXPAND | wxALL, 3);
        sizer->Add(new wxMetroButton(frm, wxID_ANY, wxEmptyString, wxBITMAP_PNG_FROM_DATA(demo_bitmap), wxDefaultPosition, wxDefaultSize), 0, wxALL, 3);

        MyNoteTest *nb = new MyNoteTest(frm);

        /*wxPanel *p = new wxPanel( nb );
        p->SetBackgroundColour(*wxRED);
        nb->AddPage( p, "Case 2: PV+debt" );

        p = new wxPanel( nb );
        p->SetBackgroundColour(*wxBLUE);
        nb->AddPage( p, "Case 1: PV" );

        nb->AddPage( new wxPanel( nb ), "Wind system" );
        nb->AddPage( new wxPanel( nb ), "solar water heat" );*/
        sizer->Add(nb, 1, wxALL | wxEXPAND, 0);

        wxMetroNotebook *nb2 = new wxMetroNotebook(frm, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxMT_LIGHTTHEME);
        nb2->AddPage(new wxPanel(nb), "Base Case");
        nb2->AddPage(new wxPanel(nb), "Parametrics");
        nb2->AddPage(new wxPanel(nb), "Sensitivities");
        nb2->AddPage(new wxPanel(nb), "Statistics");
        nb2->AddPage(new wxPanel(nb), "Scripting");
        sizer->Add(nb2, 1, wxALL | wxEXPAND, 0);

        //wxDiurnalPeriodCtrl *sch = new wxDiurnalPeriodCtrl( frm, wxID_ANY );
        //sch->SetupTOUGrid();
        //sizer->Add( sch, 1, wxALL|wxEXPAND, 5 );

        frm->SetSizer(sizer);
        frm->Show();

        ///*

        //	//	TestPLPlot( 0 );

        //	/*
        //	wxChar sep = ',';
        //	bool use_thousep = wxNumberFormatter::GetThousandsSeparatorIfUsed(&sep);
        //	wxMessageBox( m_locale.GetLocale() + "\n" + wxString::Format( "thousep? %d sep=%c\n\n", use_thousep ? 1:0, (char)sep)
        //	+ wxNumberFormatter::ToString( 12490589.02, 2, wxNumberFormatter::Style_WithThousandsSep ) );

        //	wxFrame *top = new wxFrame(NULL, wxID_ANY, "CSV Read Test", wxDefaultPosition, wxSize(500,500));

        //	wxDiurnalPeriodCtrl *sched = new wxDiurnalPeriodCtrl( top, wxID_ANY );
        //	sched->SetupTOUGrid();

        //	wxGrid *grid = new wxGrid( top, wxID_ANY );

        //	wxCSVData csv;
        //	wxStopWatch sw;
        //	bool ok = csv.ReadFile( "c:/Users/adobos/Desktop/csv_test.csv" );
        //	int msec = sw.Time();
        //	csv.WriteFile("c:/Users/adobos/Desktop/csv_test2.csv");
        //	int nr = (int)csv.NumRows()+1;
        //	int nc = (int)csv.NumCols()+1;
        //	top->SetTitle( wxString::Format("CSVRead: (%d ms) %d %s [%d x %d]", msec, csv.GetErrorLine(), ok?"ok":"fail", nr, nc) );

        //	if (nr > 0 && nc > 0)
        //	{
        //	grid->CreateGrid( nr, nc );
        //	for ( size_t r = 0; r < nr; r++ )
        //	for ( size_t c = 0; c < nc; c++ )
        //	grid->SetCellValue( r, c, csv(r,c) );
        //	}
        //	*/

        //return true;
        //
    }
};

IMPLEMENT_APP(MyApp);
