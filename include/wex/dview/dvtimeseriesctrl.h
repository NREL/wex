/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

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
#include <wx/dialog.h>
#include "wex/numeric.h"
#include "wex/plot/plplotctrl.h"
#include "wex/dview/dvplothelper.h"

class wxCheckBox;
class wxDVSelectionListCtrl;
class wxDVTimeSeriesDataSet;
class wxDVTimeSeriesPlot;
class wxGridSizer;
class wxPLLinePlot;
class wxPLTimeAxis;
class wxRadioChoice;
class wxScrollBar;
class wxSearchCtrl;
class wxTimer;

enum wxDVStatType { wxDV_AVERAGE, wxDV_SUM };
enum wxDVTimeSeriesStyle { wxDV_NORMAL, wxDV_STEPPED };
enum wxDVTimeSeriesType { wxDV_RAW, wxDV_HOURLY, wxDV_DAILY, wxDV_MONTHLY };

class wxDVTimeSeriesSettingsDialog : public wxDialog
{
public:
	wxDVTimeSeriesSettingsDialog(wxWindow *parent, const wxString &title,
		bool isTopRightYVisible = true, bool isBottomGraphVisible = true,
		bool isBottomRightGraphVisible = true);

	void SetTopYBounds(double y1min, double y1max);
	void SetTopY2Bounds(double ymin, double ymax);
	void SetBottomYBounds(double y2min, double y2max);
	void SetBottomY2Bounds(double y2min, double y2max);
	void GetTopYBounds(double *y1min, double *y1max);
	void GetTopY2Bounds(double *ymin, double *ymax);
	void GetBottomYBounds(double *y2min, double *y2max);
	void GetBottomY2Bounds(double *y2min, double *y2max);

	void SetStyle(wxDVTimeSeriesStyle id);
	wxDVTimeSeriesStyle GetStyle();
	void SetStatType(wxDVStatType statType);
	wxDVStatType GetStatType();
	void SetAutoscale(bool b);
	void SetAutoscale2(bool b);
	bool GetAutoscale();
	bool GetAutoscale2();
	void SetBottomAutoscale(bool b);
	bool GetBottomAutoscale();
	void SetBottomAutoscale2(bool b);
	bool GetBottomAutoscale2();
	void SetStacked(bool b);
	bool GetStacked();

protected:
	void OnClickTopHandler(wxCommandEvent& event);
	void OnClickBottomHandler(wxCommandEvent& event);
	void OnClickStatHandler(wxCommandEvent& event);

private:
	wxCheckBox *mStatTypeCheck;
	wxCheckBox *mSteppedLines;
	wxCheckBox *mTopAutoscaleCheck;
	wxCheckBox *mTop2AutoscaleCheck;
	wxCheckBox *mBottomAutoscaleCheck;
	wxCheckBox *mBottom2AutoscaleCheck;
	wxNumericCtrl *mTopYMaxCtrl;
	wxNumericCtrl *mTopYMinCtrl;
	wxNumericCtrl *mTopY2MaxCtrl;
	wxNumericCtrl *mTopY2MinCtrl;
	wxNumericCtrl *mBottomYMaxCtrl;
	wxNumericCtrl *mBottomYMinCtrl;
	wxNumericCtrl *mBottomY2MaxCtrl;
	wxNumericCtrl *mBottomY2MinCtrl;
	wxCheckBox *mStackedArea;

	DECLARE_EVENT_TABLE();
};

class wxDVTimeSeriesCtrl : public wxPanel
{
public:
	wxDVTimeSeriesCtrl(wxWindow *parent, wxWindowID id, wxDVTimeSeriesType seriesType, wxDVStatType statType);
	virtual ~wxDVTimeSeriesCtrl();

	//When a data set is added, wxDVTimeSeriesCtrl creates a plottable with a pointer to that data.  Does not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet *d, bool refresh_ui);
	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //Releases ownership, does not delete. //true if found & removed.
	void RemoveAllDataSets(); //Clears all data sets from graphs and memory.

	//Data Selection:
	wxDVSelectionListCtrl* GetDataSelectionList();
	void SetTopSelectedNames(const wxString& names);
	void SetBottomSelectedNames(const wxString& names);
	void SetSelectedNamesForColIndex(const wxString& names, int index);
	void SelectDataSetAtIndex(int index, unsigned column = 0);
	int GetNumberOfSelections();

	//View Setters/Getters
	void SetViewMin(double min);
	double GetViewMin();
	void SetViewMax(double max);
	double GetViewMax();
	void SetViewRange(double min, double max);

	void SetStyle(wxDVTimeSeriesStyle sty);

	void GetVisibleDataMinAndMax(double* min, double* max, const std::vector<int>& selectedChannelIndices);
	void GetAllDataMinAndMax(double* min, double* max, const std::vector<int>& selectedChannelIndices);
	double GetMinPossibleTimeForVisibleChannels();
	double GetMaxPossibleTimeForVisibleChannels();

	void KeepNewBoundsWithinLimits(double* newMin, double* newMax);
	void KeepNewBoundsWithinLowerLimit(double* newMin, double* newMax);
	void MakeXBoundsNice(double* xMin, double* xMax);

	wxDVTimeSeriesType GetTimeSeriesType();
	wxDVStatType GetStatType();
	void SetStatType(wxDVStatType statType);

	/*Graph Specific Methods*/
	bool CanZoomIn(void);
	bool CanZoomOut(void);
	void ZoomFactorAndUpdate(double factor, double shiftPercent = 0.0); //A factor of 2 would zoom in twice as far as current level.
	void ZoomToFit();
	void PanByPercent(double p); //Negative goes left.
	void UpdateScrollbarPosition(void);
	void AutoscaleYAxis(bool forceUpdate = false, bool ScaleOverAllData = false);
	void AutoscaleYAxisByPlot(bool IsLeftAxis, bool IsTopPlot, int SelectedChannelIndex);

	void SetupTopYLeft(double min = 0, double max = 0);
	void SetupTopYRight(double min = 0, double max = 0);

	void SetStackingOnYLeft(bool b);
	void ClearStacking();
	void UpdateStacking();

	void Invalidate();

	void ReadState(std::string filename);
	void WriteState(std::string filename);

protected:

	void StackUp(wxPLPlotCtrl::AxisPos yap, wxPLPlotCtrl::PlotPos ppos);

	void OnZoomIn(wxCommandEvent& e);
	void OnZoomOut(wxCommandEvent& e);
	void OnZoomFit(wxCommandEvent& e);
	void OnSettings(wxCommandEvent &e);

	void OnDataChannelSelection(wxCommandEvent& e);
	void OnMouseWheel(wxMouseEvent& e);
	void OnHighlight(wxCommandEvent& e);
	void OnGraphScroll(wxScrollEvent& e);
	void OnGraphScrollLineUp(wxScrollEvent& e);
	void OnGraphScrollLineDown(wxScrollEvent& e);
	void OnGraphScrollPageUp(wxScrollEvent& e);
	void OnGraphScrollPageDown(wxScrollEvent& e);
	void OnSearch(wxCommandEvent& e);

	/*
	void OnPlotDrag(wxCommandEvent& e);
	void OnPlotDragStart(wxCommandEvent& e);
	void OnPlotDragEnd(wxCommandEvent& e);
	*/

	void AutoscaleYAxis(wxPLAxis* axisToScale,
		const std::vector<int>& selectedChannelIndices, bool forceUpdate = false, bool ScaleOverAllData = false);

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
	wxSearchCtrl *m_srchCtrl;

	bool m_topAutoScale, m_top2AutoScale, m_bottomAutoScale, m_bottom2AutoScale;
	wxDVTimeSeriesStyle m_style; // line, stepped
	bool m_stackingOnYLeft;
	wxDVTimeSeriesType m_seriesType;
	wxDVStatType m_statType;

	void AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void RemoveGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void SetYAxisLabelText();
	void ClearAllChannelSelections(wxPLPlotCtrl::PlotPos pPos);
	void RefreshDisabledCheckBoxes();
	void RefreshDisabledCheckBoxes(wxPLPlotCtrl::PlotPos pPos);
	std::string GetTabName();

	void OnTimer(wxTimerEvent& event);

	//double TopYMax; // Evan TODO
	//double TopYMin;
	//double TopY2Max;
	//double TopY2Min;
	//double BottomYMax;
	//double BottomYMin;
	//double BottomY2Max;
	//double BottomY2Min;

	wxTimer * m_timer;

	std::vector<std::pair<int, int> > m_selections;
	unsigned m_counter;

	double m_xAxixWorldMin;
	double m_xAxixWorldMax;

	DECLARE_EVENT_TABLE();
};

#endif