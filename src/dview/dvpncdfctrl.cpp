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

#include <algorithm>
#include <string>
#include <sstream>

#include <wx/wx.h>
#include <wx/busyinfo.h>
#include "wx/srchctrl.h"
#include <wx/tokenzr.h>
#include <wx/config.h>

#include "wex/plot/plplotctrl.h"
#include "wex/plot/plhistplot.h"
#include "wex/plot/pllineplot.h"

#include "wex/dview/dvselectionlist.h"
#include "wex/dview/dvpncdfctrl.h"

enum {
    ID_DATA_SELECTOR = wxID_HIGHEST + 1,
    wxID_BIN_COMBO,
    wxID_NORMALIZE_CHOICE,
    wxID_Y_MAX_TB,
    wxID_PLOT_TYPE
};

BEGIN_EVENT_TABLE(wxDVPnCdfCtrl, wxPanel)
                EVT_DVSELECTIONLIST(ID_DATA_SELECTOR, wxDVPnCdfCtrl::OnDataChannelSelection)
                EVT_TEXT_ENTER(wxID_Y_MAX_TB, wxDVPnCdfCtrl::OnEnterYMax)
                EVT_CHOICE(wxID_NORMALIZE_CHOICE, wxDVPnCdfCtrl::OnNormalizeChoice)
                EVT_COMBOBOX(wxID_BIN_COMBO, wxDVPnCdfCtrl::OnBinComboSelection)
                EVT_TEXT_ENTER(wxID_BIN_COMBO, wxDVPnCdfCtrl::OnBinTextEnter)
                EVT_CHECKBOX(wxID_ANY, wxDVPnCdfCtrl::OnShowZerosClick)
                EVT_CHOICE(wxID_PLOT_TYPE, wxDVPnCdfCtrl::OnPlotTypeSelection)
                EVT_TEXT(wxID_ANY, wxDVPnCdfCtrl::OnSearch)
END_EVENT_TABLE()

wxDVPnCdfCtrl::wxDVPnCdfCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                             const wxSize &size, long style, const wxString &name)
        : wxPanel(parent, id, pos, size, style, name) {
    m_srchCtrl = NULL;
    m_plotSurface = new wxPLPlotCtrl(this, wxID_ANY);
    m_plotSurface->SetBackgroundColour(*wxWHITE);
    m_pdfPlot = new wxPLHistogramPlot();
    m_cdfPlot = new wxPLLinePlot();
    m_cdfPlot->SetColour(*wxBLACK);
    m_cdfPlot->SetThickness(2);
    m_plotSurface->AddPlot(m_pdfPlot);
    m_plotSurface->AddPlot(m_cdfPlot, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_RIGHT);
    m_plotSurface->ShowTitle(false);

    m_maxTextBox = new wxTextCtrl(this, wxID_Y_MAX_TB, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                  wxTE_PROCESS_ENTER);

    m_srchCtrl = new wxSearchCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0);
    m_selector = new wxDVSelectionListCtrl(this, ID_DATA_SELECTOR, 1, wxDefaultPosition, wxDefaultSize,
                                           wxDVSEL_RADIO_FIRST_COL | wxDVSEL_NO_COLOURS);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_srchCtrl, 0, wxALL | wxEXPAND, 0);
    sizer->Add(m_selector, 0, wxALL | wxALIGN_CENTER, 0);

    m_hideZeros = new wxCheckBox(this, wxID_ANY, "Exclude Zero Values", wxDefaultPosition, wxDefaultSize,
                                 wxALIGN_RIGHT);

    m_PlotTypeDisplayed = new wxChoice(this, wxID_PLOT_TYPE);
    m_PlotTypeDisplayed->Append(wxT("PDF and CDF"));
    m_PlotTypeDisplayed->Append(wxT("PDF Only"));
    m_PlotTypeDisplayed->Append(wxT("CDF Only"));
    m_PlotTypeDisplayed->SetSelection(0);

    wxBoxSizer *optionsSizer = new wxBoxSizer(wxHORIZONTAL);

    optionsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Y max:")), 0,
                      wxALIGN_CENTER | wxALL | wxALIGN_CENTER_VERTICAL, 2);
    optionsSizer->Add(m_maxTextBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    optionsSizer->Add(m_hideZeros, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    optionsSizer->AddStretchSpacer();

    optionsSizer->Add(m_PlotTypeDisplayed, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    m_normalizeChoice = new wxChoice(this, wxID_NORMALIZE_CHOICE);
    m_normalizeChoice->Append(wxT("Histogram"));
    m_normalizeChoice->Append(wxT("Scaled Histogram"));
    m_normalizeChoice->Append(wxT("Scale by Bin Width (Match PDF)"));
    m_normalizeChoice->SetSelection(1);
    optionsSizer->Add(m_normalizeChoice, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    optionsSizer->AddSpacer(5);

    wxStaticText *binsLabel = new wxStaticText(this, wxID_ANY, wxT("# Bins:"));
    optionsSizer->Add(binsLabel, 0, wxALIGN_CENTER, 0);
    m_binsCombo = new wxComboBox(this, wxID_BIN_COMBO);
    m_binsCombo->SetWindowStyle(wxTE_PROCESS_ENTER);
    m_binsCombo->Append(wxT("Freedman-Diaconis Formula"));
    m_binsCombo->Append(wxT("Sturge's Formula"));
    m_binsCombo->Append(wxT("SQRT (Excel)"));
    m_binsCombo->Append(wxT("20"));
    m_binsCombo->Append(wxT("50"));
    m_binsCombo->Append(wxT("100"));
    m_binsCombo->SetSelection(0);
    optionsSizer->Add(m_binsCombo, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(optionsSizer, 0, wxEXPAND, 0);
    leftSizer->Add(m_plotSurface, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(leftSizer, 1, wxALL | wxEXPAND, 0);
    mainSizer->Add(sizer, 0, wxALL | wxEXPAND, 0);
    SetSizer(mainSizer);

    m_plotSurface->SetYAxis1(new wxPLLinearAxis(0, 100, "% of Data Points"));
    m_plotSurface->SetYAxis2(new wxPLLinearAxis(0, 100, "CDF %"));
    m_plotSurface->SetXAxis1(new wxPLLinearAxis(0, 1));
    m_plotSurface->ShowGrid(true, false);
    m_plotSurface->ShowLegend(false);

    m_selectedDataSetIndex = -1;
}

wxDVPnCdfCtrl::~wxDVPnCdfCtrl() {
    for (size_t i = 0; i < m_cdfPlotData.size(); i++)
        delete m_cdfPlotData[i];
}

void wxDVPnCdfCtrl::ReadState(std::string filename) {
    wxConfig cfg("DView", "NREL");

    wxString s;
    bool success;
    bool debugging = false;

    std::string prefix = "/AppState/" + filename + "/PDFCDF/";

    std::string key("");

    key = prefix + "ExcludeZeros";
    success = cfg.Read(key, &s);
    if (debugging) assert(success);
    m_hideZeros->SetValue((s == "false") ? false : true);
    ShowZerosClick();

    key = prefix + "PlotType";
    success = cfg.Read(key, &s);
    if (debugging) assert(success);
    m_PlotTypeDisplayed->SetSelection(wxAtoi(s));
    PlotTypeSelection();

    key = prefix + "Normalize";
    success = cfg.Read(key, &s);
    if (debugging) assert(success);
    m_normalizeChoice->SetSelection(wxAtoi(s));
    NormalizeChoice();

    key = prefix + "Bins";
    success = cfg.Read(key, &s);
    if (debugging) assert(success);
    m_binsCombo->SetSelection(wxAtoi(s));
    BinComboSelection();

    key = prefix + "Selections";
    success = cfg.Read(key, &s);
    if (debugging) assert(success);

    wxStringTokenizer tokenizer(s, ",");
    while (tokenizer.HasMoreTokens()) {
        wxString str = tokenizer.GetNextToken();
        SelectDataSetAtIndex(wxAtoi(str));
    }

    if (m_selector->GetSelectedNamesInCol().size() == 0) {
        SelectDataSetAtIndex(0);
    }

    // Set this value after settings selections, so they don't get stepped on
    key = prefix + "YMax";
    success = cfg.Read(key, &s);
    if (debugging) assert(success);
    m_maxTextBox->SetValue(s);
    EnterYMax();
}

void wxDVPnCdfCtrl::WriteState(std::string filename) {
    wxConfig cfg("DView", "NREL");

    bool success;
    bool debugging = false;
    std::string s;
    std::stringstream ss;

    std::string prefix = "/AppState/" + filename + "/PDFCDF/";

    std::string key("");

    key = prefix + "YMax";
    s = m_maxTextBox->GetValue();
    success = cfg.Write(key, s.c_str());
    if (debugging) assert(success);

    key = prefix + "ExcludeZeros";
    s = (m_hideZeros->GetValue()) ? "true" : "false";
    success = cfg.Write(key, s.c_str());
    if (debugging) assert(success);

    key = prefix + "PlotType";
    s = wxString::Format(wxT("%d"), (int) m_PlotTypeDisplayed->GetSelection());
    success = cfg.Write(key, s.c_str());
    if (debugging) assert(success);

    key = prefix + "Normalize";
    s = wxString::Format(wxT("%d"), (int) m_normalizeChoice->GetSelection());
    success = cfg.Write(key, s.c_str());
    if (debugging) assert(success);

    key = prefix + "Bins";
    s = wxString::Format(wxT("%d"), (int) m_binsCombo->GetSelection());
    success = cfg.Write(key, s.c_str());
    if (debugging) assert(success);

    auto selections = this->m_selector->GetSelectionsInCol();
    for (auto selection : selections) {
        ss << selection;
        ss << ',';
    }
    s = ss.str();
    key = prefix + "Selections";
    success = cfg.Write(key, s.c_str());
    if (debugging) assert(success);
}

// *** DATA SET FUNCTIONS ***
void wxDVPnCdfCtrl::AddDataSet(wxDVTimeSeriesDataSet *d, bool update_ui) {
    m_dataSets.push_back(d);
    m_selector->Append(d->GetTitleWithUnits(), d->GetGroupName());

    //Add new plot data array, but leave it empty until we use it.  We'll fill it with sorted values then.
    m_cdfPlotData.push_back(new std::vector<wxRealPoint>());

    if (update_ui)
        Layout();
}

int wxDVPnCdfCtrl::GetNumberOfSelections() {
    return m_selector->GetNumberOfSelections();
}

void wxDVPnCdfCtrl::RemoveDataSet(wxDVTimeSeriesDataSet *d) {
    if (m_selectedDataSetIndex >= 0 && static_cast<unsigned>(m_selectedDataSetIndex) < m_dataSets.size()
        && m_dataSets[m_selectedDataSetIndex] == d) {
        ChangePlotDataTo(NULL);
        m_selectedDataSetIndex = -1;
    }

    int index = -1;
    for (size_t i = 0; i < m_dataSets.size(); i++)
        if (d == m_dataSets[i])
            index = i;

    if (index < 0) return;

    m_dataSets.erase(m_dataSets.begin() + index);
    delete m_cdfPlotData[index];
    m_cdfPlotData.erase(m_cdfPlotData.begin() + index);

    m_selector->RemoveAt(index);

    InvalidatePlot();
}

void wxDVPnCdfCtrl::RemoveAllDataSets() {
    ChangePlotDataTo(NULL);

    m_dataSets.clear();

    for (size_t i = 0; i < m_cdfPlotData.size(); i++)
        delete m_cdfPlotData[i];
    m_cdfPlotData.clear();
    m_selector->RemoveAll();

    InvalidatePlot();
}

wxString wxDVPnCdfCtrl::GetCurrentDataName() {
    if (m_selectedDataSetIndex >= 0 && m_selectedDataSetIndex < static_cast<int>(m_dataSets.size()))
        return m_selector->GetRowLabelWithGroup(m_selectedDataSetIndex);
    else
        return wxEmptyString;
}

bool wxDVPnCdfCtrl::SetCurrentDataName(const wxString &name, bool restrictToSmallDataSet) {
    for (size_t i = 0; i < m_dataSets.size(); i++) {
        if (m_selector->GetRowLabelWithGroup(i) == name) {
            if (restrictToSmallDataSet && m_dataSets[i]->Length() > 8760 * 2) return false;
            m_selectedDataSetIndex = i;
            ChangePlotDataTo(m_dataSets[i]);
            m_selector->SelectRowInCol(i);
            InvalidatePlot();
            return true;
        }
    }

    return false;
}

void wxDVPnCdfCtrl::SelectDataSetAtIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_dataSets.size())) return;

    m_selectedDataSetIndex = index;
    ChangePlotDataTo(m_dataSets[index]);
    m_selector->SelectRowInCol(index);
}

void wxDVPnCdfCtrl::SetNumberOfBins(int n) {
    if (m_binsCombo->GetSelection() == wxNOT_FOUND)
        m_binsCombo->SetValue(wxString::Format("%d", n));

    m_pdfPlot->SetNumberOfBins(n);
    m_plotSurface->GetYAxis1()->SetWorldMax(m_pdfPlot->GetNiceYMax());
    m_maxTextBox->SetValue(wxString::Format("%lg", m_pdfPlot->GetNiceYMax()));
}

int wxDVPnCdfCtrl::GetNumberOfBins() {
    return m_pdfPlot->GetNumberOfBins();
}

int wxDVPnCdfCtrl::GetBinSelectionIndex() {
    return m_binsCombo->GetSelection();
}

void wxDVPnCdfCtrl::SetBinSelectionIndex(int i) {
    m_binsCombo->SetSelection(i);
}

wxPLHistogramPlot::NormalizeType wxDVPnCdfCtrl::GetNormalizeType() {
    return m_pdfPlot->GetNormalize();
}

void wxDVPnCdfCtrl::SetNormalizeType(wxPLHistogramPlot::NormalizeType t) {
    m_normalizeChoice->SetSelection(int(t));

    m_pdfPlot->SetNormalize(t);

    UpdateYAxisLabel();
}

double wxDVPnCdfCtrl::GetYMax() {
    return m_plotSurface->GetYAxis1()->GetWorldMax();
}

void wxDVPnCdfCtrl::SetYMax(double max) {
    m_plotSurface->GetYAxis1()->SetWorldMax(max);
    InvalidatePlot();
    m_maxTextBox->SetValue(wxString::Format("%lg", max));
}

void wxDVPnCdfCtrl::RebuildPlotSurface(double maxYPercent) {
    if (m_selectedDataSetIndex < 0
        || m_selectedDataSetIndex >= static_cast<int>(m_dataSets.size()))
        return;

    wxDVTimeSeriesDataSet *ds = m_dataSets[m_selectedDataSetIndex];

    m_plotSurface->GetYAxis1()->SetWorld(0, maxYPercent);
    m_maxTextBox->SetValue(wxString::Format("%lg", maxYPercent));

    double xMin, xMax;
    ds->GetDataMinAndMax(&xMin, &xMax);
    m_plotSurface->GetXAxis1()->SetWorld(xMin, xMax);

    wxString label = ds->GetSeriesTitle();
    if (!ds->GetUnits().IsEmpty())
        label += " (" + ds->GetUnits() + ")";
    m_plotSurface->GetXAxis1()->SetLabel(label);
}

void wxDVPnCdfCtrl::ChangePlotDataTo(wxDVTimeSeriesDataSet *d, bool forceDataRefresh) {
    if (d) {
        if (m_binsCombo->GetSelection() == 0) //Freedman-Diaconis with simplified interquantile range
            m_pdfPlot->SetNumberOfBins(m_pdfPlot->GetFreedmanDiaconisBinsFor(d->Length()));
        else if (m_binsCombo->GetSelection() == 1) //Sturge's
            m_pdfPlot->SetNumberOfBins(m_pdfPlot->GetSturgesBinsFor(d->Length()));
        else if (m_binsCombo->GetSelection() == 2) //SQRT
            m_pdfPlot->SetNumberOfBins(m_pdfPlot->GetSqrtBinsFor(d->Length()));

        m_pdfPlot->SetData(d->GetDataVector()); //inefficient?
        m_pdfPlot->SetLabel(d->GetSeriesTitle());
        m_pdfPlot->SetXDataLabel(m_plotSurface->GetXAxis1()->GetLabel());
        m_pdfPlot->SetYDataLabel(d->GetSeriesTitle());
    } else {
        m_pdfPlot->SetLabel(wxEmptyString);
        m_pdfPlot->SetData(std::vector<wxRealPoint>());
        m_pdfPlot->SetXDataLabel(wxEmptyString);
        m_pdfPlot->SetYDataLabel(wxEmptyString);
    }

    int index = -1;
    for (size_t i = 0; i < m_dataSets.size(); i++)
        if (m_dataSets[i] == d)
            index = i;

    if (index < 0) {
        m_cdfPlot->SetData(std::vector<wxRealPoint>()); //clear the data
        m_cdfPlot->SetLabel(wxEmptyString);
        m_cdfPlot->SetXDataLabel(wxEmptyString);
        m_cdfPlot->SetYDataLabel(wxEmptyString);
    } else {
        // Read Cdf Data (requires sort) if not already sorted.
        if (m_cdfPlotData[index]->size() == 0 || forceDataRefresh) {
            if (forceDataRefresh) { m_cdfPlotData[index]->clear(); }
            ReadCdfFrom(*d, m_cdfPlotData[index]);
        }

        m_cdfPlot->SetData(*m_cdfPlotData[index]);
        m_cdfPlot->SetLabel(d->GetSeriesTitle() + " " + _("Percentile"));
        m_cdfPlot->SetXDataLabel(m_plotSurface->GetXAxis1()->GetLabel());
        m_cdfPlot->SetYDataLabel(m_cdfPlot->GetLabel());
    }

    if (d)
        RebuildPlotSurface(m_pdfPlot->GetNiceYMax());
}

void wxDVPnCdfCtrl::ReadCdfFrom(wxDVTimeSeriesDataSet &d, std::vector<wxRealPoint> *cdfArray) {
    // This does not use bins.  It is an empirical CDF.  See wikipedia for empirical CDF explanation.
    // This can take a long time because of sorting.

    wxBeginBusyCursor();
    wxBusyInfo wait("Please wait, calculating CDF...");

    std::vector<double> sortedValues;

    for (size_t i = 0; i < d.Length(); i++) {
        if (!m_cdfPlot->GetIgnoreZeros() || d.At(i).y != 0.0) { sortedValues.push_back(d.At(i).y); }
    }

    if (sortedValues.size() > 0) {
        std::sort(sortedValues.begin(), sortedValues.end());

        //double dataMin = sortedValues[0];
        //double dataMax = sortedValues[sortedValues.size() - 1];
    }

    cdfArray->reserve(sortedValues.size());
    for (size_t i = 0; i < sortedValues.size(); i++) {
        double x = sortedValues[i];
        double percent = 100 * double(i) / double(sortedValues.size() - 1);
        cdfArray->push_back(wxRealPoint(x, percent));
    }

    wxEndBusyCursor();
}

void wxDVPnCdfCtrl::UpdateYAxisLabel() {
    switch (m_pdfPlot->GetNormalize()) {
        case wxPLHistogramPlot::NO_NORMALIZE:
            m_plotSurface->GetYAxis1()->SetLabel(wxT("# of Data Points"));
            break;
        case wxPLHistogramPlot::NORMALIZE:
            m_plotSurface->GetYAxis1()->SetLabel(wxT("% of Data Points"));
            break;
        case wxPLHistogramPlot::NORMALIZE_PDF:
            if (m_selectedDataSetIndex != -1)
                m_plotSurface->GetYAxis1()->SetLabel(wxT("% per ") + m_dataSets[m_selectedDataSetIndex]->GetUnits());
            else
                m_plotSurface->GetYAxis1()->SetLabel(wxT("PDF %"));
            break;
    }
}

// *** EVENT HANDLERS ***
void wxDVPnCdfCtrl::OnDataChannelSelection(wxCommandEvent &) {
    m_selectedDataSetIndex = -1;
    bool isChecked;
    m_selector->GetLastEventInfo(&m_selectedDataSetIndex, 0, &isChecked);

    if (m_selectedDataSetIndex < 0) return;

    ChangePlotDataTo(m_dataSets[m_selectedDataSetIndex], true);
    UpdateYAxisLabel();
    InvalidatePlot();
}

void wxDVPnCdfCtrl::OnSearch(wxCommandEvent &) {
    m_selector->Filter(m_srchCtrl->GetValue().Lower());
}

void wxDVPnCdfCtrl::OnEnterYMax(wxCommandEvent &) {
    EnterYMax();
}

void wxDVPnCdfCtrl::EnterYMax() {
    double val;
    if (m_maxTextBox->GetValue().ToDouble(&val)) {
        m_plotSurface->GetYAxis1()->SetWorldMax(val);
        InvalidatePlot();
    }
}

void wxDVPnCdfCtrl::OnBinComboSelection(wxCommandEvent &) {
    BinComboSelection();
}

void wxDVPnCdfCtrl::BinComboSelection() {
    if (m_selectedDataSetIndex < 0 || m_selectedDataSetIndex >= static_cast<int>(m_dataSets.size()))
        return;

    switch (m_binsCombo->GetSelection()) {
    case 0:
        // quantile width - assume interquantile range = 1/2 of data range to simplify to n^1/3
        // bin width
        // number of bins
        SetNumberOfBins(ceil(pow(double(m_dataSets[m_selectedDataSetIndex]->Length()), 1.0/3.0))); //Freedman-Diaconis with simplified assumption on interquantile range.
        break;
    case 1:
        SetNumberOfBins(ceil(log10(double(m_dataSets[m_selectedDataSetIndex]->Length())) / log10(2.0) + 1)); //Sturges formula.
        break;
    case 2:
        SetNumberOfBins(sqrt(double(m_dataSets[m_selectedDataSetIndex]->Length()))); //Sqrt-choice.  Used by excel.
        break;
    case 3:
        SetNumberOfBins(20);
        break;
    case 4:
        SetNumberOfBins(50);
        break;
    case 5:
        SetNumberOfBins(100);
        break;
    }
    InvalidatePlot();
}

void wxDVPnCdfCtrl::OnBinTextEnter(wxCommandEvent &) {
    long bins;
    if (m_binsCombo->GetValue().ToLong(&bins)) {
        SetNumberOfBins(bins);
        InvalidatePlot();
    } else {
        wxMessageBox("Not a valid number of bins!");
    }
}

void wxDVPnCdfCtrl::OnNormalizeChoice(wxCommandEvent &) {
    NormalizeChoice();
}

void wxDVPnCdfCtrl::NormalizeChoice() {
    SetNormalizeType(wxPLHistogramPlot::NormalizeType(m_normalizeChoice->GetSelection()));
    m_plotSurface->GetYAxis1()->SetWorldMax(m_pdfPlot->GetNiceYMax());
    m_maxTextBox->SetValue(wxString::Format("%lg", m_pdfPlot->GetNiceYMax()));
    InvalidatePlot();
}

void wxDVPnCdfCtrl::OnShowZerosClick(wxCommandEvent &) {
    ShowZerosClick();
}

void wxDVPnCdfCtrl::ShowZerosClick() {
    bool ignoreZeros = m_hideZeros->GetValue();
    //int index = -1;

    m_pdfPlot->SetIgnoreZeros(ignoreZeros);
    m_cdfPlot->SetIgnoreZeros(ignoreZeros);

    if (m_selectedDataSetIndex > -1 && m_selectedDataSetIndex < static_cast<int>(m_cdfPlotData.size())) {
        m_cdfPlotData[m_selectedDataSetIndex]->clear();
        ReadCdfFrom(*m_dataSets[m_selectedDataSetIndex], m_cdfPlotData[m_selectedDataSetIndex]);
        m_cdfPlot->SetData(*m_cdfPlotData[m_selectedDataSetIndex]);

        m_plotSurface->GetYAxis1()->SetWorldMax(m_pdfPlot->GetNiceYMax());
        m_maxTextBox->SetValue(wxString::Format("%lg", m_pdfPlot->GetNiceYMax()));
        InvalidatePlot();
    }
}

void wxDVPnCdfCtrl::OnPlotTypeSelection(wxCommandEvent &) {
    PlotTypeSelection();
}

void wxDVPnCdfCtrl::PlotTypeSelection() {
    int type = m_PlotTypeDisplayed->GetSelection();

    if (type == 1) {
        m_plotSurface->RemovePlot(m_cdfPlot);
        if (!m_plotSurface->ContainsPlot(m_pdfPlot)) { m_plotSurface->AddPlot(m_pdfPlot); }
    } else if (type == 2) {
        m_plotSurface->RemovePlot(m_pdfPlot);
        if (!m_plotSurface->ContainsPlot(m_cdfPlot)) { m_plotSurface->AddPlot(m_cdfPlot); }
    } else {
        if (!m_plotSurface->ContainsPlot(m_pdfPlot)) { m_plotSurface->AddPlot(m_pdfPlot); }
        if (!m_plotSurface->ContainsPlot(m_cdfPlot)) {
            m_plotSurface->AddPlot(m_cdfPlot, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_RIGHT);
        }
    }

    InvalidatePlot();
}

void wxDVPnCdfCtrl::InvalidatePlot() {
    m_plotSurface->Invalidate();
    m_plotSurface->Refresh();
}
