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

#include "wex/dview/dvscatterplotctrl.h"
#include "wex/dview/dvselectionlist.h"
#include "wex/dview/dvtimeseriesdataset.h"

#include "wex/plot/plplotctrl.h"
#include "wex/plot/plscatterplot.h"

#include <wx/config.h>
#include "wx/srchctrl.h"
#include <wx/tokenzr.h>
#include <wx/wx.h>

#include <algorithm>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>

static const wxString NO_UNITS("ThereAreNoUnitsForThisAxis.");

class wxDVScatterPlot : public wxPLScatterPlot
{
private:
	wxDVTimeSeriesDataSet *m_x, *m_y;
public:
	wxDVScatterPlot(wxDVTimeSeriesDataSet *x, wxDVTimeSeriesDataSet *y)
		: m_x(x), m_y(y)
	{
	}

	virtual wxString GetXDataLabel(wxPLPlot *) const
	{
		return m_x->GetTitleWithUnits();
	}

	virtual wxString GetYDataLabel(wxPLPlot *) const
	{
		return m_y->GetTitleWithUnits();
	}

	virtual wxRealPoint At(size_t i) const
	{
		double xx = i < m_x->Length() ? m_x->At(i).y : std::numeric_limits<double>::quiet_NaN();
		double yy = i < m_y->Length() ? m_y->At(i).y : std::numeric_limits<double>::quiet_NaN();
		return wxRealPoint(xx, yy);
	}

	virtual size_t Len() const
	{
		size_t xlen = m_x->Length();
		size_t ylen = m_y->Length();
		return xlen < ylen ? xlen : ylen;
	}
};

enum { wxID_SCATTER_DATA_SELECTOR = wxID_HIGHEST + 1, wxID_PERFECT_AGREE_LINE };

BEGIN_EVENT_TABLE(wxDVScatterPlotCtrl, wxPanel)
EVT_DVSELECTIONLIST(wxID_SCATTER_DATA_SELECTOR, wxDVScatterPlotCtrl::OnChannelSelection)
EVT_CHECKBOX(wxID_PERFECT_AGREE_LINE, wxDVScatterPlotCtrl::OnShowLine)
EVT_TEXT(wxID_ANY, wxDVScatterPlotCtrl::OnSearch)
END_EVENT_TABLE()

wxDVScatterPlotCtrl::wxDVScatterPlotCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
const wxSize& size, long style, const wxString& name)
: wxPanel(parent, id, pos, size, style, name)
{
	m_srchCtrl = NULL;

	m_plotSurface = new wxPLPlotCtrl(this, wxID_ANY);
	m_plotSurface->ShowTitle(false);
	m_plotSurface->SetBackgroundColour(*wxWHITE);
	m_plotSurface->ShowLegend(false);

	m_srchCtrl = new wxSearchCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0);
	m_dataSelectionList = new wxDVSelectionListCtrl(this, wxID_SCATTER_DATA_SELECTOR, 2, wxDefaultPosition, wxDefaultSize, wxDVSEL_RADIO_FIRST_COL);
	wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_srchCtrl, 0, wxALL | wxEXPAND, 0);
	sizer->Add(m_dataSelectionList, 0, wxALL | wxALIGN_CENTER, 0);

	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSizer);

	wxBoxSizer *optionsSizer = new wxBoxSizer(wxHORIZONTAL);
	m_showPerfAgreeLine = new wxCheckBox(this, wxID_PERFECT_AGREE_LINE, "Show Line of Perfect Agreement", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	optionsSizer->Add(m_showPerfAgreeLine, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(m_plotSurface, 1, wxEXPAND | wxALL, 10);
	topSizer->Add(sizer, 0, wxEXPAND, 0);

	mainSizer->Add(optionsSizer, 0, wxEXPAND, 0);
	mainSizer->Add(topSizer, 1, wxEXPAND | wxALL, 0);

	m_xDataIndex = -1;

	m_showLine = false;
}

wxDVScatterPlotCtrl::~wxDVScatterPlotCtrl()
{
	WriteState(m_filename);
}

void wxDVScatterPlotCtrl::ReadState(std::string filename)
{
	wxConfig cfg("DView", "NREL");

	wxString s;
	bool success;
	bool debugging = false;
	std::string key = filename;
	std::string tabName("Scatter");

	key = tabName + "ShowLine";
	success = cfg.Read(key, &s);
	if (debugging) assert(success);
	m_showPerfAgreeLine->SetValue((s == "false") ? false : true);
	// Must manually call the function as wxWidgets does not emit a signal when a widget state is set programmatically
	ShowLine();

	key = tabName + "Selections";
	success = cfg.Read(key, &s);
	if (debugging) assert(success);

	{
		wxStringTokenizer tokenizer(s, ",");
		while (tokenizer.HasMoreTokens())
		{
			wxString str = tokenizer.GetNextToken();
			SelectXDataAtIndex(wxAtoi(str));
		}
	}

	key = tabName + "Selections1";
	success = cfg.Read(key, &s);
	if (debugging) assert(success);

	{
		wxStringTokenizer tokenizer(s, ",");
		while (tokenizer.HasMoreTokens())
		{
			wxString str = tokenizer.GetNextToken();
			SelectYDataAtIndex(GetScatterSelectionList()->GetUnsortedRowIndex(wxAtoi(str)));
		}
	}

	if (GetScatterSelectionList()->GetSelectedNamesInCol(0).size() == 0) {
		SelectXDataAtIndex(0);
	}

	if (GetScatterSelectionList()->GetSelectedNamesInCol(1).size() == 0) {
		SelectYDataAtIndex(GetScatterSelectionList()->GetUnsortedRowIndex(1));
	}
}

void wxDVScatterPlotCtrl::WriteState(std::string filename)
{
	wxConfig cfg("DView", "NREL");

	m_filename = filename;

	bool success;
	bool debugging = false;
	std::string s;
	std::string key = filename;
	std::string tabName("Scatter");
	std::stringstream  ss;

	key = tabName + "ShowLine";
	s = (m_showPerfAgreeLine->GetValue()) ? "true" : "false";
	success = cfg.Write(key, s.c_str());
	if (debugging) assert(success);

	auto selections = m_dataSelectionList->GetSelectionsInCol();
	for (auto selection : selections){
		ss << selection;
		ss << ',';
	}
	key = tabName + "Selections";
	success = cfg.Write(key, ss.str().c_str());
	if (debugging) assert(success);

	ss.clear();
	ss.str(std::string());
	auto selections1 = m_dataSelectionList->GetSelectionsInCol(1);
	for (auto selection : selections1){
		ss << selection;
		ss << ',';
	}
	key = tabName + "Selections1";
	success = cfg.Write(key, ss.str().c_str());
	if (debugging) assert(success);
}

//*** DATA SET HANDLING ***
void wxDVScatterPlotCtrl::AddDataSet(wxDVTimeSeriesDataSet* d, bool update_ui)
{
	m_dataSets.push_back(d);
	m_dataSelectionList->Append(d->GetTitleWithUnits(), d->GetGroupName());

	if (update_ui)
		Layout();
}

void wxDVScatterPlotCtrl::RemoveDataSet(wxDVTimeSeriesDataSet* d)
{
	int index = -1;
	for (size_t i = 0; i < m_dataSets.size(); i++)
		if (m_dataSets[i] == d)
			index = i;

	if (index == m_xDataIndex)
		m_xDataIndex = -1;

	m_yDataIndices.erase(std::find(m_yDataIndices.begin(), m_yDataIndices.end(), index));

	m_dataSets.erase(m_dataSets.begin() + index);
	m_dataSelectionList->RemoveAt(index);
	m_dataSelectionList->Refresh();
	m_dataSelectionList->Update();

	UpdatePlotWithChannelSelections();

	Layout();
	Refresh();
}

void wxDVScatterPlotCtrl::RemoveAllDataSets()
{
	m_dataSets.clear();
	m_dataSelectionList->RemoveAll();

	m_yDataIndices.clear();

	m_xDataIndex = -1;

	UpdatePlotWithChannelSelections();

	Layout();
	Refresh();
}

//*** MEMBER FUNCTIONS ***//
void wxDVScatterPlotCtrl::SetXAxisChannel(int index)
{
	m_xDataIndex = index;
	UpdatePlotWithChannelSelections();
}

void wxDVScatterPlotCtrl::AddYAxisChannel(int index)
{
	m_yDataIndices.push_back(index);
	UpdatePlotWithChannelSelections();
	RefreshDisabledCheckBoxes();
}

bool wxDVScatterPlotCtrl::IsAnythingSelected()
{
	return m_dataSelectionList->GetNumberOfSelections() > 0;
}

void wxDVScatterPlotCtrl::RemoveYAxisChannel(int index)
{
	m_yDataIndices.erase(std::find(m_yDataIndices.begin(), m_yDataIndices.end(), index));
	UpdatePlotWithChannelSelections();
	RefreshDisabledCheckBoxes();
}

void wxDVScatterPlotCtrl::UpdatePlotWithChannelSelections()
{
	m_plotSurface->DeleteAllPlots();
	m_plotSurface->DeleteAxes();

	if (m_xDataIndex < 0 || (size_t)m_xDataIndex >= m_dataSets.size())
		return;

	wxString YLabelText;
	size_t NumY1AxisSelections = 0;
	size_t NumY2AxisSelections = 0;
	for (size_t i = 0; i < m_yDataIndices.size(); i++)
	{
		if ((size_t)m_yDataIndices[i] < m_dataSets.size())
		{
			wxDVScatterPlot *p = new wxDVScatterPlot(m_dataSets[m_xDataIndex], m_dataSets[m_yDataIndices[i]]);
			p->SetLineOfPerfectAgreementFlag(m_showLine);
			p->SetLabel(m_dataSets[m_yDataIndices[i]]->GetSeriesTitle());
			p->SetSize(2);
			p->SetColour(m_dataSelectionList->GetColourForIndex(m_yDataIndices[i]));

			wxString units = m_dataSets[m_yDataIndices[i]]->GetUnits();

			wxPLPlotCtrl::AxisPos yap = wxPLPlotCtrl::Y_LEFT;
			wxString y1Units = NO_UNITS, y2Units = NO_UNITS;

			if (m_plotSurface->GetYAxis1())
			{
				y1Units = m_plotSurface->GetYAxis1()->GetUnits();
			}

			if (m_plotSurface->GetYAxis2())
			{
				y2Units = m_plotSurface->GetYAxis2()->GetUnits();
			}

			if (m_plotSurface->GetYAxis1() && y1Units == units)
			{
				yap = wxPLPlotCtrl::Y_LEFT;
			}
			else if (m_plotSurface->GetYAxis2() && y2Units == units)
			{
				yap = wxPLPlotCtrl::Y_RIGHT;
			}
			else if (m_plotSurface->GetYAxis1() == 0)
			{
				yap = wxPLPlotCtrl::Y_LEFT;
			}
			else
			{
				yap = wxPLPlotCtrl::Y_RIGHT;
			}

			m_plotSurface->AddPlot(p, wxPLPlotCtrl::X_BOTTOM, yap);

			m_plotSurface->GetAxis(yap)->SetUnits(units);
			YLabelText = units;
			if (m_dataSelectionList->IsSelected(m_yDataIndices[i], 1) && m_dataSets[m_yDataIndices[i]]->GetUnits() == units)
			{
				if (yap == wxPLPlotCtrl::Y_LEFT)
				{
					NumY1AxisSelections++;
				}
				else
				{
					NumY2AxisSelections++;
				}
			}
			if ((NumY1AxisSelections == 1 && yap == wxPLPlotCtrl::Y_LEFT) || (NumY2AxisSelections == 1 && yap == wxPLPlotCtrl::Y_RIGHT)) { YLabelText = m_dataSets[m_yDataIndices[i]]->GetLabel(); }
			m_plotSurface->GetAxis(yap)->SetLabel(YLabelText);
		}
	}

	if (m_plotSurface->GetXAxis1())
		m_plotSurface->GetXAxis1()->SetLabel(m_dataSets[m_xDataIndex]->GetSeriesTitle() +
		" (" + m_dataSets[m_xDataIndex]->GetUnits() + ")");
}

void wxDVScatterPlotCtrl::RefreshDisabledCheckBoxes()
{
	wxString axis1Label = NO_UNITS;
	wxString axis2Label = NO_UNITS;

	if (m_plotSurface->GetYAxis1())
		axis1Label = m_plotSurface->GetYAxis1()->GetUnits();
	if (m_plotSurface->GetYAxis2())
		axis2Label = m_plotSurface->GetYAxis2()->GetUnits();

	if (axis1Label != NO_UNITS
		&& axis2Label != NO_UNITS
		&& axis1Label != axis2Label)
	{
		for (int i = 0; i < m_dataSelectionList->Length(); i++)
			m_dataSelectionList->Enable(i, 1, axis1Label == m_dataSets[i]->GetUnits()
			|| axis2Label == m_dataSets[i]->GetUnits());
	}
	else
	{
		for (int i = 0; i < m_dataSelectionList->Length(); i++)
			m_dataSelectionList->Enable(i, 1, true);
	}
}

wxDVSelectionListCtrl* wxDVScatterPlotCtrl::GetScatterSelectionList()
{
	return m_dataSelectionList;
}

void wxDVScatterPlotCtrl::SelectXDataAtIndex(int index)
{
	if (m_dataSelectionList->IsSelected(index, 0)) return;

	m_dataSelectionList->SelectRowInCol(index, 0);
	SetXAxisChannel(index);
}

void wxDVScatterPlotCtrl::SelectYDataAtIndex(int index)
{
	if (m_dataSelectionList->IsSelected(index, 1)) return;

	m_dataSelectionList->SelectRowInCol(index, 1);
	AddYAxisChannel(index);
}

void wxDVScatterPlotCtrl::SetXSelectedName(const wxString& name)
{
	int index = m_dataSelectionList->SelectRowWithNameInCol(name, 0);
	if (index != -1)
		SetXAxisChannel(index);

	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

void wxDVScatterPlotCtrl::SetYSelectedNames(const wxString& names)
{
	//ClearAllChannelSelections();

	wxStringTokenizer tkz(names, ";");

	while (tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		int index = m_dataSelectionList->SelectRowWithNameInCol(token, 1);
		if (index != -1)
			AddYAxisChannel(index);
	}
}

void wxDVScatterPlotCtrl::OnChannelSelection(wxCommandEvent &)
{
	RefreshPlot();
}

void wxDVScatterPlotCtrl::OnShowLine(wxCommandEvent&)
{
	ShowLine();
}

void wxDVScatterPlotCtrl::ShowLine()
{
	m_showLine = m_showPerfAgreeLine->GetValue();
	UpdatePlotWithChannelSelections();
	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

void wxDVScatterPlotCtrl::RefreshPlot()
{
	int row, col;
	bool selected;

	m_dataSelectionList->GetLastEventInfo(&row, &col, &selected);

	if (col == 0)
	{
		SetXAxisChannel(row);
	}
	else if (col == 1)
	{
		if (selected)
			AddYAxisChannel(row);
		else
			RemoveYAxisChannel(row);
	}

	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

void wxDVScatterPlotCtrl::OnSearch(wxCommandEvent&)
{
	m_dataSelectionList->Filter(m_srchCtrl->GetValue().Lower());
}