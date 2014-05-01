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
#include "wex/numeric.h"
#include "wex/plot/plplotctrl.h"
#include "wex/dview/dvplothelper.h"
#include "wex/dview/dvtimeseriesdataset.h"
#include "wex/dview/dvselectionlist.h"

class wxDVBoxPlot : public wxPLPlottable
{
public:
	wxDVBoxPlot(wxDVStatisticsDataSet *ds, bool OwnsDataset = false);

	~wxDVBoxPlot();

	void SetColour(const wxColour &col);
	virtual wxString GetXDataLabel() const;
	virtual wxString GetYDataLabel() const;
	virtual wxRealPoint At(size_t i) const;
	StatisticsPoint At(size_t i, double m_offset, double m_timestep) const;
	virtual size_t Len() const;

	virtual void Draw(wxDC &dc, const wxPLDeviceMapping &map);
	virtual void DrawInLegend(wxDC &dc, const wxRect &rct);
	double GetPeriodLowerBoundary(double hourNumber);
	double GetPeriodUpperBoundary(double hourNumber);
	wxDVStatisticsDataSet *GetDataSet() const { return m_data; }

	std::vector<wxString> GetExportableDatasetHeaders(wxUniChar sep, StatisticsType type) const;
	std::vector<wxRealPoint> GetExportableDataset(StatisticsType type) const;

private:
	wxDVStatisticsDataSet *m_data;
	wxColour m_colour;
	bool m_ownsDataset;
};

class wxDVBoxPlotCtrl : public wxPanel
{
public:
	wxDVBoxPlotCtrl(wxWindow *parent, wxWindowID id);
	virtual ~wxDVBoxPlotCtrl();

	//When a data set is added, wxDVBoxPlotCtrl creates a plottable with a pointer to that data.  Does not take ownership.
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

	void AddGraphAfterChannelSelection(int index);
	void RemoveGraphAfterChannelSelection(int index);
	void ClearAllChannelSelections();
	void RefreshDisabledCheckBoxes();

	DECLARE_EVENT_TABLE()
};

#endif