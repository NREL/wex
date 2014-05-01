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

#include "wex/numeric.h"
#include "wex/plot/plplotctrl.h"
#include "wex/dview/dvplothelper.h"
#include "wex/dview/dvtimeseriesdataset.h"

enum { ID_STATISTICS_CTRL = 50, };

class wxDVVariableStatistics //: public wxPLPlottable
{
public:
	wxDVVariableStatistics(wxDVStatisticsDataSet *ds, wxString GroupName, bool OwnsDataset = false);

	~wxDVVariableStatistics();

	//virtual wxString GetXDataLabel() const;
	//virtual wxString GetYDataLabel() const;
	//virtual wxRealPoint At(size_t i) const;
	StatisticsPoint At(size_t i, double m_offset, double m_timestep) const;
	//virtual size_t Len() const;

	//virtual void Draw(wxDC &dc, const wxPLDeviceMapping &map);
	//virtual void DrawInLegend(wxDC &dc, const wxRect &rct);
	double GetPeriodLowerBoundary(double hourNumber);
	double GetPeriodUpperBoundary(double hourNumber);
	wxDVStatisticsDataSet *GetDataSet() const { return m_data; }
	wxString GetGroupName();

	std::vector<wxString> GetExportableDatasetHeaders(wxUniChar sep, StatisticsType type) const;
	std::vector<wxRealPoint> GetExportableDataset(StatisticsType type) const;

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

public:     //TODO:  change these to private and implement getters/setters
	double m_avg;
	double m_min;
	double m_max;
	double m_sum;
	double m_stdev;
	double m_avgdailymin;
	double m_avgdailymax;
	wxString m_nodeName;
	bool m_container;

private:
	dvStatisticsTreeModelNode *m_parent;
	std::vector<dvStatisticsTreeModelNode*> m_children;
};

class dvStatisticsTreeModel : public wxDataViewModel
{
public:
	dvStatisticsTreeModel();
	~dvStatisticsTreeModel() { delete m_root; }

	void Refresh(std::vector<wxDVVariableStatistics*> stats);

	// override sorting to always sort branches ascendingly
	int Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int column, bool ascending) const;

	// implementation of base class virtuals to define model
	virtual unsigned int GetColumnCount() const { return 8; }
	virtual wxString GetColumnType(unsigned int col) const;
	virtual void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const;
	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
	virtual bool IsEnabled(const wxDataViewItem &item, unsigned int col) const { return false; }
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

	void AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group);
	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //Releases ownership, does not delete. //true if found & removed.
	void RemoveAllDataSets(); //Clears all data sets from graphs and memory.

	void Invalidate();

private:
	std::vector<wxDVVariableStatistics*> m_variableStatistics;
	//wxPLPlotCtrl *m_plotSurface;
	wxDataViewCtrl *m_ctrl;
	wxObjectDataPtr<dvStatisticsTreeModel> m_StatisticsModel;
	wxDataViewColumn* m_col;
	wxDataViewColumn* m_attributes;

	// event handlers
	void OnCollapse(wxCommandEvent& event);
	void OnExpand(wxCommandEvent& event);
};

#endif