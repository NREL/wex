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

#ifndef __DVStatisticsTableCtrl_h

#define __DVStatisticsTableCtrl_h

/*

* wxStatisticsTableCtrl.h

*

* This is a wxPanel that contains a table of statistics based on underlying data

*/

// For compilers that support precompilation, includes "wx/wx.h".

#include <wx/wxprec.h>

#ifndef WX_PRECOMP

#include <wx/wx.h>

#endif

#include <wx/dataview.h>

#include <wx/panel.h>

#include <wx/menu.h>

#include <wx/stream.h>

#include "wex/numeric.h"

#include "wex/dview/dvtimeseriesdataset.h"

enum { ID_STATISTICS_CTRL = 50, };

class wxDVVariableStatistics

{
public:

	wxDVVariableStatistics(wxDVStatisticsDataSet *ds, wxString GroupName, bool OwnsDataset = false);

	~wxDVVariableStatistics();

	StatisticsPoint At(size_t i, double m_offset, double m_timestep) const;

	wxDVStatisticsDataSet *GetDataSet() const { return m_data; }

	wxString GetGroupName();

private:

	wxDVStatisticsDataSet *m_data;

	bool m_ownsDataset;

	wxString m_groupName;
};

class dvStatisticsTreeModelNode

{
public:

	dvStatisticsTreeModelNode(dvStatisticsTreeModelNode* parent, wxString variableName);

	dvStatisticsTreeModelNode(dvStatisticsTreeModelNode* parent, wxString nodeName,

		double avg, double min, double max, double sum, double stdev, double avgdailymin, double avgdailymax);

	~dvStatisticsTreeModelNode();

	bool IsContainer() const;

	dvStatisticsTreeModelNode* GetParent();

	std::vector<dvStatisticsTreeModelNode*> GetChildren();

	dvStatisticsTreeModelNode* GetNthChild(unsigned int n);

	void Append(dvStatisticsTreeModelNode* child);

	unsigned int GetChildCount() const;

	void RemoveAllChildren();

	wxString GetName();

	double GetMean();

	double GetMin();

	double GetMax();

	double GetSum();

	double GetStDev();

	double GetAvgDailyMin();

	double GetAvgDailyMax();

private:

	double m_avg;

	double m_min;

	double m_max;

	double m_sum;

	double m_stdev;

	double m_avgdailymin;

	double m_avgdailymax;

	wxString m_nodeName;

	bool m_container;

	dvStatisticsTreeModelNode *m_parent;

	std::vector<dvStatisticsTreeModelNode*> m_children;
};

class dvStatisticsTreeModel : public wxDataViewModel

{
public:

	dvStatisticsTreeModel();

	~dvStatisticsTreeModel() { delete m_root; }

	void Refresh(std::vector<wxDVVariableStatistics*> stats, bool showMonths);

	wxDataViewItem GetRoot();

	// override sorting to always sort branches ascendingly

	int Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int column, bool ascending) const;

	// implementation of base class virtuals to define model

	virtual unsigned int GetColumnCount() const { return 8; }

	virtual wxString GetColumnType(unsigned int col) const;

	virtual void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const;

	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);

	virtual bool IsEnabled(const wxDataViewItem &, unsigned int) const { return false; }

	virtual wxDataViewItem GetParent(const wxDataViewItem &item) const;

	virtual bool IsContainer(const wxDataViewItem &item) const;

	virtual unsigned int GetChildren(const wxDataViewItem &parent, wxDataViewItemArray &array) const;

private:

	dvStatisticsTreeModelNode*   m_root;
};

class wxDVStatisticsTableCtrl : public wxPanel

{
public:

	wxDVStatisticsTableCtrl(wxWindow *parent, wxWindowID id);

	virtual ~wxDVStatisticsTableCtrl();

	void RebuildDataViewCtrl();

	void AddDataSet(wxDVTimeSeriesDataSet *d);

	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //Releases ownership, does not delete. //true if found & removed.

	void RemoveAllDataSets(); //Clears all data sets from graphs and memory.

	void WriteDataAsText(wxUniChar sep, wxOutputStream &os, bool visible_only = true, bool include_x = true);

	void Invalidate();

	wxMenu &GetContextMenu() { return m_contextMenu; }

	void OnShowMonthsClick(wxCommandEvent &e);

private:

	std::vector<wxDVVariableStatistics*> m_variableStatistics;

	wxDataViewCtrl *m_ctrl;

	wxObjectDataPtr<dvStatisticsTreeModel> m_StatisticsModel;

	wxDataViewColumn* m_col;

	wxMenu m_contextMenu;

	wxCheckBox *m_chkShowMonths;

	bool m_showMonths;

	// event handlers

	void OnCollapse(wxCommandEvent& event);

	void OnExpand(wxCommandEvent& event);

	void OnPopupMenu(wxCommandEvent& event);

	void OnContextMenu(wxDataViewEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif