/*
BSD 3-Clause License

Copyright (c) Alliance for Sustainable Energy, LLC. See also https://github.com/NREL/wex/blob/develop/LICENSE
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __DVPlotCtrl_h
#define __DVPlotCtrl_h

/*
 * wxDVPlotCtrl.h
 *
 * This class is the control that contains all of the other DView-like controls in tabs.
 * This will have a tab for hourly, daily, monthly, profile, dmap, etc.
 */

#include "wex/metro.h"

#include "wex/dview/dvdcctrl.h"
#include "wex/dview/dvdmapctrl.h"
#include "wex/dview/dvplotctrlsettings.h"
#include "wex/dview/dvpncdfctrl.h"
#include "wex/dview/dvprofilectrl.h"
#include "wex/dview/dvscatterplotctrl.h"
#include "wex/dview/dvstatisticstablectrl.h"
#include "wex/dview/dvtimeseriesctrl.h"
#include "wex/dview/dvtimeseriesdataset.h"

class wxDVPlotCtrl : public wxMetroNotebook {
public:
    wxDVPlotCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 long style = wxMT_LIGHTTHEME);

    virtual ~wxDVPlotCtrl();

    //When a data set is added, wxDVTimeSeriesCtrl takes ownership and will delete it upon destruction.
    void AddDataSet(wxDVTimeSeriesDataSet *d, bool update_ui = true);

    //RemoveDataSet releases ownership.
    void RemoveDataSet(wxDVTimeSeriesDataSet *d);

    //RemoveAll deletes data sets.
    void RemoveAllDataSets();

    void ReadState(std::string filename);

    void WriteState(std::string filename);

    void SetOkToAccessState(bool okToAccessState) { m_okToAccessState = okToAccessState; }

    wxDVStatisticsTableCtrl *GetStatisticsTable();

    //These methods get and set the view perspective to resume later with the same view.
    wxDVPlotCtrlSettings GetPerspective();

    void SetPerspective(wxDVPlotCtrlSettings &settings);

    enum {
        TAB_TS = 0, TAB_HTS, TAB_DTS, TAB_MTS, TAB_DMAP, TAB_PROFILE, TAB_PDF, TAB_DC, TAB_SCATTER
    };

    void SelectTabIndex(size_t index);

    void SelectDataIndex(size_t index, bool allTabs = false);

    void SelectDataIndexOnTab(size_t index, int tab);

    void SetTimeSeriesMode(int mode);

    void SetupTopYLeft(double min, double max);

    void SetupTopYRight(double min, double max);

    void SetTimeSeriesRange(double start, double end);

    void SetSelectedNames(const wxArrayString &names);

    void SelectDataOnBlankTabs();

    void DisplayTabs();

    double GetMinTimeStep();

private:
    std::vector<wxDVTimeSeriesDataSet *> m_dataSets;

    wxDVTimeSeriesCtrl *m_timeSeries;
    wxDVTimeSeriesCtrl *m_hourlyTimeSeries;
    wxDVTimeSeriesCtrl *m_dailyTimeSeries;
    wxDVTimeSeriesCtrl *m_monthlyTimeSeries;
    wxDVDMapCtrl *m_dMap;
    wxDVProfileCtrl *m_profilePlots;
    wxDVStatisticsTableCtrl *m_statisticsTable;
    wxDVPnCdfCtrl *m_pnCdf;
    wxDVDCCtrl *m_durationCurve;
    wxDVScatterPlotCtrl *m_scatterPlot;

    std::string m_filename;

    bool m_okToAccessState;

DECLARE_EVENT_TABLE()
};

#endif
