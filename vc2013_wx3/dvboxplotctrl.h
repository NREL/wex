#ifndef __DVBoxPlotCtrl_h
#define __DVBoxPlotCtrl_h

/*
* wxDVBoxPlotCtrl.h
*
* This is a wxPanel that contains DView box plot functionality, as well as
* a list of channels (different data sets) that can be viewed.
*
* This control will display a box plot on a axis of months.
*/

#include <wx/panel.h>
#include <wx/dialog.h>
#include "wex/numeric.h"
#include "wex/plot/plplotctrl.h"
#include "wex/dview/dvplothelper.h"

class wxDVTimeSeriesDataSet;
class wxPLTimeAxis;
class wxPLLinePlot;
class wxDVSelectionListCtrl;
class wxGridSizer;
class wxCheckBox;
class wxDVBoxPlot;

class wxDVBoxPlotCtrl : public wxPanel
{
public:
	wxDVBoxPlotCtrl(wxWindow *parent, wxWindowID id);
	virtual ~wxDVBoxPlotCtrl();

	//When a data set is added, wxDVTimeSeriesCtrl creates a plottable with a pointer to that data.  Does not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group, bool refresh_ui);
	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //Releases ownership, does not delete. //true if found & removed.
	void RemoveAllDataSets(); //Clears all data sets from graphs and memory.

	//Data Selection:
	wxDVSelectionListCtrl* GetDataSelectionList();
	void SetSelectedNames(const wxString& names);
	void SetSelectedNamesForColIndex(const wxString& names, int index);
	void SelectDataSetAtIndex(int index);

	//View Setters/Getters
	void SetViewMin(double min);
	double GetViewMin();
	void SetViewMax(double max);
	double GetViewMax();
	void SetViewRange(double min, double max);

	void GetVisibleDataMinAndMax(double* min, double* max, const std::vector<int>& selectedChannelIndices);
	double GetMinPossibleTimeForVisibleChannels();
	double GetMaxPossibleTimeForVisibleChannels();

	void KeepNewBoundsWithinLimits(double* newMin, double* newMax);
	void KeepNewBoundsWithinLowerLimit(double* newMin, double* newMax);
	void MakeXBoundsNice(double* xMin, double* xMax);

	/*Graph Specific Methods*/
	void AutoscaleYAxis(bool forceUpdate = false);

	void Invalidate();

protected:
	void OnDataChannelSelection(wxCommandEvent& e);

	void AutoscaleYAxis(wxPLAxis* axisToScale,
		const std::vector<int>& selectedChannelIndices, bool forceUpdate = false);

private:
	std::vector<wxDVBoxPlot*> m_plots;

	//This array contains the visible graphs associated with each axis position on each graph.
	std::vector<std::vector<int>*> m_selectedChannelIndices;

	bool m_autoScale;
	wxPLPlotCtrl *m_plotSurface;
	wxPLTimeAxis *m_xAxis;
	wxDVSelectionListCtrl *m_dataSelector;

	void AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void RemoveGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void ClearAllChannelSelections(wxPLPlotCtrl::PlotPos pPos);
	void RefreshDisabledCheckBoxes();
	void RefreshDisabledCheckBoxes(wxPLPlotCtrl::PlotPos pPos);

	DECLARE_EVENT_TABLE()
};

#endif