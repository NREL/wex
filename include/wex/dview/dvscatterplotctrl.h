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

#ifndef __DVScatterplotCtrl_h
#define __DVScatterplotCtrl_h

#include <wx/panel.h>

#include <vector>

class wxCheckBox;
class wxDVSelectionListCtrl;
class wxDVTimeSeriesDataSet;
class wxSearchCtrl;
class wxPLPlotCtrl;

class wxDVScatterPlotCtrl : public wxPanel
{
public:
	wxDVScatterPlotCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel");

	virtual ~wxDVScatterPlotCtrl();

	void AddDataSet(wxDVTimeSeriesDataSet* d, bool update_ui);
	void RemoveDataSet(wxDVTimeSeriesDataSet* d);
	void RemoveAllDataSets();

	wxDVSelectionListCtrl* GetScatterSelectionList();
	void SetXSelectedName(const wxString& name);
	void SetYSelectedNames(const wxString& names);
	void SelectXDataAtIndex(int index);
	void SelectYDataAtIndex(int index);
	bool IsAnythingSelected();
	void ReadState(std::string filename);
	void WriteState(std::string filename);

	//EVENT HANDLERS
	void OnChannelSelection(wxCommandEvent &);
	void OnShowLine(wxCommandEvent &);
	void RefreshPlot();
	void OnSearch(wxCommandEvent& e);

private:
	std::vector<wxDVTimeSeriesDataSet*> m_dataSets;

	wxDVSelectionListCtrl *m_dataSelectionList;
	wxSearchCtrl *m_srchCtrl;
	int m_xDataIndex;
	std::vector<int> m_yDataIndices;

	wxPLPlotCtrl *m_plotSurface;
	wxCheckBox *m_showPerfAgreeLine;
	bool m_showLine;

	void SetXAxisChannel(int index);
	void AddYAxisChannel(int index);
	void RemoveYAxisChannel(int index);
	void UpdatePlotWithChannelSelections();
	void RefreshDisabledCheckBoxes();
	void ShowLine();

	DECLARE_EVENT_TABLE()
};

#endif
