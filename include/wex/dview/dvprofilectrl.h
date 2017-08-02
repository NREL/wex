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

#ifndef __DV_ProfileCtrl_h
#define __DV_ProfileCtrl_h

#include "wex/plot/plplotctrl.h"

#include <wx/panel.h>

#include <vector>

class wxCheckBox;
class wxDVSelectionListCtrl;
class wxDVTimeSeriesDataSet;
class wxGridSizer;
class wxPLLinePlot;
class wxSearchCtrl;
class wxTimer;
class wxTimerEvent;

class wxDVProfileCtrl : public wxPanel
{
public:
	wxDVProfileCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel");
	virtual ~wxDVProfileCtrl();

	//Does not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet *d, bool update_ui);
	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //true if found & removed.
	//RemoveAllDataSets does not delete original datasets since we never took ownership.
	void RemoveAllDataSets();

	bool IsMonthIndexSelected(int i);
	void SetMonthIndexSelected(int i, bool value = true);
	void ShowMonthPlotAtIndex(int index, bool show = true);
	void ShowPlotAtIndex(int i);
	void HidePlotAtIndex(int i, bool update = true);
	void HideAllPlots(bool update = true);
	void AutoScaleYAxes();
	void RefreshDisabledCheckBoxes();
	wxDVSelectionListCtrl* GetDataSelectionList();
	void SetSelectedNames(const wxString& names);
	void SelectDataSetAtIndex(int index);
	int GetNumberOfSelections();

	void ReadState(std::string filename);
	void WriteState(std::string filename);

	class VerticalLabelCtrl;

private:
	//Event Handlers
	void OnDataChannelSelection(wxCommandEvent& e);
	void OnMonthSelection(wxCommandEvent& e);
	void OnSelAllMonths(wxCommandEvent& e);
	void OnSearch(wxCommandEvent& e);

	struct PlotSet
	{
		PlotSet(wxDVTimeSeriesDataSet *ds);
		~PlotSet();

		void CalculateProfileData();

		wxDVTimeSeriesDataSet *dataset;
		wxPLLinePlot *plots[13];
		wxPLPlotCtrl::AxisPos axisPosition;
	};

	std::vector<PlotSet*> m_plots; //12 months of a day of data for each added data set.
	wxDVSelectionListCtrl *m_dataSelector;
	wxSearchCtrl *m_srchCtrl;
	wxCheckBox *m_monthCheckBoxes[13];
	wxPLPlotCtrl *m_plotSurfaces[13];
	int m_numberOfPlotSurfacesShown;
	wxGridSizer *m_graphsSizer;

	VerticalLabelCtrl *m_leftAxisLabel;
	VerticalLabelCtrl *m_rightAxisLabel;

	std::string m_filename;

	void CalculateProfilePlotData(PlotSet *ps);
	void MonthSelection(unsigned index);

	void OnTimer(wxTimerEvent& event);

	wxTimer * m_timer;

	std::vector<int> m_selections;
	unsigned m_counter;

	DECLARE_EVENT_TABLE();
};

#endif
