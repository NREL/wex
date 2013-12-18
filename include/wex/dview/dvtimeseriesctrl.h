#ifndef __DVTimeSeriesCtrl_h
#define __DVTimeSeriesCtrl_h

/*
 * wxDVTimeSeriesCtrl.h
 *
 * This is a wxPanel that contains DView time series functionality.
 * It contains the plot window (WPPlotSurface2D) as well as the scrollbars,
 * axes, and ability to turn multiple plots on or off.
 *
 * This control will display a line plot on a time series axis.
 */

#include <wx/panel.h>

#include "wex/plot/plplotctrl.h"

class wxDVTimeSeriesDataSet;
class wxPLTimeAxis;
class wxPLLinePlot;
class wxDVSelectionListCtrl;
class wxGridSizer;
class wxCheckBox;
class wxDVTimeSeriesPlot;
class wxScrollBar;

enum StatType { AVERAGE = 0, SUM };
enum TimeSeriesType { HOURLY_TIME_SERIES = 0, DAILY_TIME_SERIES, MONTHLY_TIME_SERIES};

class wxDVTimeSeriesCtrl : public wxPanel
{
public:
	wxDVTimeSeriesCtrl(wxWindow *parent, wxWindowID id, TimeSeriesType seriesType, StatType statType);
	virtual ~wxDVTimeSeriesCtrl();

	//When a data set is added, wxDVTimeSeriesCtrl creates a plottable with a pointer to that data.  Does not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group, bool refresh_ui);
	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //Releases ownership, does not delete. //true if found & removed.
	void RemoveAllDataSets(); //Clears all data sets from graphs and memory.

	//Data Selection:
	wxDVSelectionListCtrl* GetDataSelectionList();
	void SetTopSelectedNames(const wxString& names);
	void SetBottomSelectedNames(const wxString& names);
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

	bool GetSyncWithHeatMap();
	void SetSyncWithHeatMap(bool b);
	TimeSeriesType GetTimeSeriesType();
	StatType GetStatType();
	void SetStatType(StatType statType);

	/*Graph Specific Methods*/
	bool CanZoomIn(void);
	bool CanZoomOut(void);
	void ZoomFactorAndUpdate(double factor, double shiftPercent = 0.0); //A factor of 2 would zoom in twice as far as current level.
	void ZoomToFit();
	void PanByPercent(double p); //Negative goes left.
	void UpdateScrollbarPosition(void);
	void AutoscaleYAxis(bool forceUpdate = false);
	
	void Invalidate();

protected:
	void OnZoomIn(wxCommandEvent& e);
	void OnZoomOut(wxCommandEvent& e);
	void OnZoomFit(wxCommandEvent& e);
	void OnSettings( wxCommandEvent &e );

	void OnDataChannelSelection(wxCommandEvent& e);
	void OnMouseWheel(wxMouseEvent& e);
	void OnHighlight(wxCommandEvent& e);
	void OnGraphScroll(wxScrollEvent& e);
	void OnGraphScrollLineUp(wxScrollEvent& e);
	void OnGraphScrollLineDown(wxScrollEvent& e);
	void OnGraphScrollPageUp(wxScrollEvent& e);
	void OnGraphScrollPageDown(wxScrollEvent& e);
	/*
	void OnPlotDrag(wxCommandEvent& e);
	void OnPlotDragStart(wxCommandEvent& e);
	void OnPlotDragEnd(wxCommandEvent& e);
	*/
	
	
	void AutoscaleYAxis(wxPLAxis* axisToScale, 
		const std::vector<int>& selectedChannelIndices, bool forceUpdate = false);
	
private:
	std::vector<wxDVTimeSeriesPlot*> m_plots;

	enum GraphAxisPosition
	{
		TOP_LEFT_AXIS = 0, 
		TOP_RIGHT_AXIS,
		BOTTOM_LEFT_AXIS, 
		BOTTOM_RIGHT_AXIS,
		GRAPH_AXIS_POSITION_COUNT
	};

	//This array contains the visible graphs associated with each axis position on each graph.
	std::vector<std::vector<int>*> m_selectedChannelIndices; 

	wxPLPlotCtrl *m_plotSurface;
	wxPLTimeAxis *m_xAxis;
	wxScrollBar *m_graphScrollBar;
	wxDVSelectionListCtrl *m_dataSelector;

	bool m_topAutoScale, m_bottomAutoScale, m_syncToHeatMap;
	int m_lineStyle; // line, stepped, points
	TimeSeriesType m_seriesType;
	StatType m_statType;

	void AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void RemoveGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void ClearAllChannelSelections(wxPLPlotCtrl::PlotPos pPos);
	void RefreshDisabledCheckBoxes();
	void RefreshDisabledCheckBoxes(wxPLPlotCtrl::PlotPos pPos);

	DECLARE_EVENT_TABLE()
};

#endif