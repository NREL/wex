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

#ifndef __DVPnCdfCtrl_h
#define __DVPnCdfCtrl_h

#include <wx/panel.h>
#include <vector>

#include "wex/plot/plhistplot.h"

#include "wex/dview/dvtimeseriesdataset.h"

class wxCheckBox;

class wxChoice;

class wxComboBox;

class wxDVSelectionListCtrl;

class wxPLLinePlot;

class wxPlPlotCtrl;

class wxSearchCtrl;

class wxTextCtrl;

class wxStaticText;

class wxDVPnCdfCtrl : public wxPanel {
public:
    wxDVPnCdfCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
                  const wxSize &size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString &name = "panel", const bool& bshowsearch = true, const bool& bshowselector = true, const bool& bshowpvalue = true, const bool& bshowhidezeros = true);

    virtual ~wxDVPnCdfCtrl();

    //Data Set Functions - do not take ownership.
    void AddDataSet(wxDVTimeSeriesDataSet *d, bool update_ui);

    void RemoveDataSet(wxDVTimeSeriesDataSet *d);

    void RemoveAllDataSets();

    wxString GetCurrentDataName();

    bool SetCurrentDataName(const wxString &name, bool restrictNoSmallDataSet = false);

    void SelectDataSetAtIndex(int index);

    int GetNumberOfSelections();

    void SetNumberOfBins(int n);

    int GetNumberOfBins();

    int GetBinSelectionIndex();

    void SetBinSelectionIndex(int i);

    wxPLHistogramPlot::NormalizeType GetNormalizeType();

    void SetNormalizeType(wxPLHistogramPlot::NormalizeType t);

    double GetY1Max();
    void SetY1Max(double max);
    double GetY2Max();
    void SetY2Max(double max);
    double GetPValue();
    void SetPValue(double pValue);
    double GetPValueX() { return m_pValue_x; };

    void ReadCdfFrom(wxDVTimeSeriesDataSet &d, std::vector<wxRealPoint> *cdfArray);

    void ChangePlotDataTo(wxDVTimeSeriesDataSet *d, bool forceDataRefresh = false);

    void RebuildPlotSurface(double maxYPercent);

    void ReadState(std::string filename);

    void WriteState(std::string filename);

    // Event Handlers
    void OnDataChannelSelection(wxCommandEvent &e);

    void OnSearch(wxCommandEvent &e);

//    void OnEnterY1Max(wxCommandEvent&);
//    void OnEnterY2Max(wxCommandEvent&);
    void OnEnterPValue(wxCommandEvent&);

    void OnBinComboSelection(wxCommandEvent &);

    void OnBinTextEnter(wxCommandEvent &);

    void OnNormalizeChoice(wxCommandEvent &);

    void OnShowZerosClick(wxCommandEvent &);

 //   void OnPlotTypeSelection(wxCommandEvent &);

private:
    std::vector<wxDVTimeSeriesDataSet *> m_dataSets;
    int m_selectedDataSetIndex;
    double m_pValue; // user entered or set programmatically
    double m_pValue_x; // x coordinant of user specified p Value
    std::vector<std::vector<wxRealPoint> *> m_cdfPlotData; //We track cdf plots since they take long to calculate.

    bool m_bshowpvalue;
    bool m_bshowhidezeros;
 //   wxTextCtrl* m_y1MaxTextBox;
 //   wxTextCtrl* m_y2MaxTextBox;
    
    wxTextCtrl* m_pValueTextBox; // input

    // resultant pvalue
    wxStaticText* m_pValueResultLabel;
    wxTextCtrl* m_pValueResultTextBox;
    wxStaticText* m_pValueResultUnits;


    wxDVSelectionListCtrl *m_selector;
    wxSearchCtrl *m_srchCtrl;
    wxComboBox *m_binsCombo;
    wxChoice *m_normalizeChoice;
    wxCheckBox *m_hideZeros;
  //  wxChoice *m_PlotTypeDisplayed;

    wxPLPlotCtrl *m_plotSurface;
    wxPLHistogramPlot *m_pdfPlot;
    wxPLLinePlot *m_cdfPlot;

    void UpdateYAxisLabel();

    void InvalidatePlot();

 //   void EnterY1Max();
 //   void EnterY2Max();
    void EnterPValue();

    void ShowZerosClick();

 //   void PlotTypeSelection();

    void NormalizeChoice();

    void BinComboSelection();

DECLARE_EVENT_TABLE()
};

#endif
