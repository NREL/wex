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

#ifndef __DVDCCtrl_h
#define __DVDCCtrl_h

#include <vector>
#include <wx/panel.h>
#include "wex/plot/plplotctrl.h"

class wxDVTimeSeriesDataSet;
class wxPLLinePlot;
class wxDVSelectionListCtrl;
class wxSearchCtrl;

class wxDVDCCtrl : public wxPanel
{
public:
	wxDVDCCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel");
	virtual ~wxDVDCCtrl();

	//Data Set Functions - do not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet* d, bool update_ui);
	void RemoveDataSet(wxDVTimeSeriesDataSet* d);
	void RemoveAllDataSets();

	void ShowPlotAtIndex(int index);
	void HidePlotAtIndex(int index, bool update = true);
	void RefreshDisabledCheckBoxes();

	wxDVSelectionListCtrl* GetDataSelectionList();
	void SetSelectedNames(const wxString& names, bool restrictToSmallDataSets = false);
	void SelectDataSetAtIndex(int index);
	int GetNumberOfSelections();
	void ReadState(std::string filename);
	void WriteState(std::string filename);

	//Event Handlers
	void OnDataChannelSelection(wxCommandEvent& e);
	void OnSearch(wxCommandEvent& e);

private:
	wxPLPlotCtrl *m_plotSurface;
	wxDVSelectionListCtrl *m_dataSelector;
	wxSearchCtrl * m_srchCtrl;

	struct PlotSet
	{
		PlotSet(wxDVTimeSeriesDataSet *ds);
		~PlotSet();

		wxDVTimeSeriesDataSet *dataset;
		wxPLLinePlot *plot;
		wxPLPlotCtrl::AxisPos axisPosition;
	};

	std::vector<PlotSet*> m_plots;
	std::vector<int> m_currentlyShownIndices;

	void CalculateDCPlotData(PlotSet *p);

	DECLARE_EVENT_TABLE()
};

#endif
