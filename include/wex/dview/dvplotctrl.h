#ifndef __DVPlotCtrl_h
#define __DVPlotCtrl_h

/*
 * wxDVPlotCtrl.h
 *
 * This class is the control that contains all of the other DView-like controls in tabs.
 * This will have a tab for hourly, daily, monthly, profile, dmap, etc.
 */

#include <wx/aui/auibook.h>

#include "wex/dview/dvtimeseriesdataset.h"
#include "wex/dview/dvtimeseriesctrl.h"
#include "wex/dview/dvdmapctrl.h"
#include "wex/dview/dvprofilectrl.h"
#include "wex/dview/dvpncdfctrl.h"
#include "wex/dview/dvdcctrl.h"
#include "wex/dview/dvscatterplotctrl.h"
#include "wex/dview/dvplotctrlsettings.h"

class wxRibbonNotebook;

class wxDVPlotCtrl : public wxPanel
{
public:
	wxDVPlotCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize );
	virtual ~wxDVPlotCtrl();

	//When a data set is added, wxDVTimeSeriesCtrl takes ownership and will delete it upon destruction.
	void AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group = wxEmptyString, bool update_ui = true);
	//RemoveDataSet releases ownership.
	void RemoveDataSet(wxDVTimeSeriesDataSet *d);
	//RemoveAll deletes data sets.
	void RemoveAllDataSets();

	//These methods get and set the view perspective to resume later with the same view.
	wxDVPlotCtrlSettings GetPerspective();
	void SetPerspective( wxDVPlotCtrlSettings& settings);

	enum { TAB_TS = 0, TAB_DMAP, TAB_PROFILE, TAB_PDF, TAB_DC, TAB_SCATTER };

	void SelectTabIndex(int index);
	void SelectDataIndex(int index, bool allTabs = false);
	void SelectDataIndexOnTab(int index, int tab);

	void SelectDataOnBlankTabs();

	//Event Handlers
	void OnPageChanging( wxNotebookEvent& e );
	
private:
	std::vector<wxDVTimeSeriesDataSet*> m_dataSets;

	wxRibbonNotebook *m_plotNotebook;
	wxDVTimeSeriesCtrl *m_timeSeries;
	wxDVDMapCtrl *m_dMap;
	wxDVProfileCtrl *m_profilePlots;
	wxDVPnCdfCtrl *m_pnCdf;
	wxDVDCCtrl *m_durationCurve;
	wxDVScatterPlotCtrl *m_scatterPlot;


DECLARE_EVENT_TABLE()
};

#endif

