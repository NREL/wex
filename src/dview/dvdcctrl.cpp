
#include <algorithm>
#include <numeric>
#include <limits>

#include <wx/tokenzr.h>
#include <wx/busyinfo.h>

#include "plot/pllineplot.h"

#include "dview/dvtimeseriesdataset.h"
#include "dview/dvselectionlist.h"
#include "dview/dvdcctrl.h"

static const wxString NO_UNITS("ThereAreNoUnitsForThisAxis.");

enum {wxID_DC_DATA_SELECTOR = wxID_HIGHEST + 1};

wxDVDCCtrl::wxDVDCCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, 
		const wxSize& size, long style, const wxString& name)
		: wxPanel(parent, id, pos, size, style, name)
{
	wxBoxSizer *topSizer = new wxBoxSizer (wxHORIZONTAL);
	SetSizer(topSizer);

	m_plotSurface = new wxPLPlotCtrl(this, wxID_ANY);
	m_plotSurface->ShowTitle( false );
	m_plotSurface->ShowLegend( false );
	topSizer->Add(m_plotSurface, 1, wxEXPAND|wxALL, 10);

	m_dataSelector = new wxDVSelectionListCtrl(this, wxID_DC_DATA_SELECTOR, 1, false);
	topSizer->Add(m_dataSelector, 0, wxEXPAND, 0);
}

wxDVDCCtrl::~wxDVDCCtrl()
{
	for ( size_t i=0;i<m_plots.size(); i++ )
	{
		// remove it first in case it's shown to release ownership
		m_plotSurface->RemovePlot( m_plots[i]->plot );

		// destructor of PlotSet will delete the actual plot
		delete m_plots[i];
	}
}

BEGIN_EVENT_TABLE(wxDVDCCtrl, wxPanel)
	EVT_DVSELECTIONLIST(wxID_DC_DATA_SELECTOR, wxDVDCCtrl::OnDataChannelSelection)
END_EVENT_TABLE()


// *** DATA SET FUNCTIONS ***
void wxDVDCCtrl::AddDataSet(wxDVTimeSeriesDataSet* d, const wxString& group, bool update_ui)
{
	m_dataSelector->Append(d->GetTitleWithUnits(), group);
	m_plots.push_back( new PlotSet( d ) );

	if (update_ui)
		Layout();
}

void wxDVDCCtrl::RemoveDataSet(wxDVTimeSeriesDataSet* d)
{
	int index = -1;
	for ( int i=0;i<m_plots.size(); i++ )
		if ( m_plots[i]->dataset == d )
			index = i;

	if ( index < 0 )
		return;

	if ( std::find(m_currentlyShownIndices.begin(), m_currentlyShownIndices.end(), index ) 
			!= m_currentlyShownIndices.end() )
		HidePlotAtIndex(index);  // TODO

	m_dataSelector->RemoveAt(index);
	m_plotSurface->RemovePlot( m_plots[index]->plot );
	delete m_plots[index]; // deletes the associated plot
	m_plots.erase( m_plots.begin() + index );

	Layout();
	Refresh();
}

void wxDVDCCtrl::RemoveAllDataSets()
{
	m_dataSelector->RemoveAll();
	
	for ( int i=0;i<m_plots.size(); i++ )
	{
		// remove it first in case it's shown to release ownership
		m_plotSurface->RemovePlot( m_plots[i]->plot );
		delete m_plots[i];
	}
	m_plots.clear();

	Layout();
	Refresh();
}

//Member functions
void wxDVDCCtrl::CalculateDCPlotData( PlotSet *p)
{
	//This method assumes uniform time step for simplicity.
	wxDVTimeSeriesDataSet *d = p->dataset;
	if (p->plot != 0)
		return;

	wxBeginBusyCursor();
	wxBusyInfo("Please wait, calculating duration curve for " + d->GetSeriesTitle() + "...");

	std::vector<double> sortedData;
	
	int len = d->Length();
	sortedData.resize( len, 0.0 );
	for (int i=0; i<len; i++)
		sortedData[i] = d->At(i).y;

	std::sort( sortedData.begin(), sortedData.end() );

	std::vector<wxRealPoint> pd;
	pd.reserve(len);
	for (int i=0; i<len; i++)
		pd.push_back( wxRealPoint(i * d->GetTimeStep(), sortedData[len-i-1]) );
	
	p->plot = new wxPLLinePlot( pd, d->GetSeriesTitle() + " (" + d->GetUnits() + ")" );
	p->plot->SetXDataLabel( _("Hours equaled or exceeded" ));
	p->plot->SetYDataLabel( p->plot->GetLabel() );

	wxEndBusyCursor();
}

void wxDVDCCtrl::ShowPlotAtIndex(int index)
{
	if (index >= 0 && index < m_plots.size())
	{
		CalculateDCPlotData( m_plots[index] );
		m_plots[index]->plot->SetColour( m_dataSelector->GetColourForIndex(index) );
		
		wxPLPlotCtrl::AxisPos yap = wxPLPlotCtrl::Y_LEFT;
		wxString y1Units = NO_UNITS, y2Units = NO_UNITS;

		if ( m_plotSurface->GetYAxis1() )
			y1Units = m_plotSurface->GetYAxis1()->GetLabel();

		if ( m_plotSurface->GetYAxis2() )
			y2Units = m_plotSurface->GetYAxis2()->GetLabel();

		wxString units = m_plots[index]->dataset->GetUnits();

		if ( m_plotSurface->GetYAxis1() && y1Units == units )
			yap = wxPLPlotCtrl::Y_LEFT;
		else if ( m_plotSurface->GetYAxis2() && y2Units == units )
			yap = wxPLPlotCtrl::Y_RIGHT;
		else if ( m_plotSurface->GetYAxis1() == 0 )
			yap = wxPLPlotCtrl::Y_LEFT;
		else
			yap = wxPLPlotCtrl::Y_RIGHT;

		m_plotSurface->AddPlot( m_plots[index]->plot, wxPLPlotCtrl::X_BOTTOM, yap );
		m_plotSurface->GetAxis( yap )->SetLabel( units );

		m_plotSurface->GetXAxis1()->SetLabel( "Hours Equaled or Exceeded" );
		RefreshDisabledCheckBoxes();
	}
}

void wxDVDCCtrl::HidePlotAtIndex(int index, bool update)
{
	if (index < 0 || index >= m_plots.size())
		return;

	m_plotSurface->RemovePlot(m_plots[index]->plot);

	std::vector<int> currently_shown = m_dataSelector->GetSelectionsInCol();
	bool keepAxis = false;
	for (int j=0; j<currently_shown.size(); j++)
	{
		if (m_plots[currently_shown[j]]->dataset->GetUnits() == m_plots[index]->dataset->GetUnits())
		{
			keepAxis = true;
			break;
		}
	}

	if (keepAxis)
	{
		//Scale axis down if we removed data set with highest max.
		m_plotSurface->RescaleAxes();
	}
	else
	{
		
		if (currently_shown.size() == 0)
		{
			m_plotSurface->SetYAxis1(NULL);
			m_plotSurface->SetYAxis2(NULL);
		}
		else
		{
			//If we only have one Y axis, we must use the left y axis.
			//Code in wxPLPlotCtrl uses this assumption.
			if (m_plots[index]->axisPosition == wxPLPlotCtrl::Y_LEFT)
			{
				m_plotSurface->SetYAxis1(NULL); //Force rescaling.
				m_plotSurface->SetYAxis2(NULL);
				//Set the y axis to the left side (instead of the right)
				for (int j=0; j<currently_shown.size(); j++)
				{
					m_plots[currently_shown[j]]->axisPosition = wxPLPlotCtrl::Y_LEFT;
					m_plotSurface->RemovePlot(m_plots[currently_shown[j]]->plot);
					m_plotSurface->AddPlot(m_plots[currently_shown[j]]->plot, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT);
				}
				m_plotSurface->GetYAxis1()->SetLabel( m_plots[currently_shown[0]]->dataset->GetUnits() );
			}

			m_plotSurface->SetYAxis2(NULL);
		}
	}

	RefreshDisabledCheckBoxes();

	if (update)
	{
		m_plotSurface->Invalidate();
		m_plotSurface->Refresh();
	}
}

void wxDVDCCtrl::RefreshDisabledCheckBoxes()
{
	std::vector<int> currently_shown = m_dataSelector->GetSelectionsInCol();
	if (currently_shown.size() < 2)
	{
		for (int i=0; i<m_plots.size(); i++)
			m_dataSelector->Enable(i, 0, true);
		return;
	}

	wxString units1 = m_plots[currently_shown[0]]->dataset->GetUnits();
	wxString units2;
	bool units2Set=false;

	for (int i=1; i<currently_shown.size(); i++)
	{
		if (m_plots[currently_shown[i]]->dataset->GetUnits() != units1)
		{
			units2 = m_plots[currently_shown[i]]->dataset->GetUnits();
			units2Set = true;
			break;
		}
	}

	if (!units2Set)
	{
		for (int i=0; i<m_plots.size(); i++)
			m_dataSelector->Enable(i, 0, true);
		return;
	}
	else
	{
		for (int i=0; i<m_plots.size(); i++)
		{
			m_dataSelector->Enable(i, 0, units1 == m_plots[i]->dataset->GetUnits()
				|| units2 == m_plots[i]->dataset->GetUnits());
		}
	}
}

wxDVSelectionListCtrl* wxDVDCCtrl::GetDataSelectionList()
{
	return m_dataSelector;
}

void wxDVDCCtrl::SetSelectedNames(const wxString& names, bool restrictToSmallDataSets)
{
	//ClearAllChannelSelections();  is this necessary?

	wxStringTokenizer tkz(names, ";");

	while(tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		int index = m_dataSelector->SelectRowWithNameInCol(token, 0);
		if (index != -1)
		{
			if (!(restrictToSmallDataSets && m_plots[index]->dataset->Length() > 8760*2))
				ShowPlotAtIndex(index);
		}
	}
}

void wxDVDCCtrl::SelectDataSetAtIndex(int index)
{
	if (index < 0 || index >= m_plots.size()) return;

	m_dataSelector->SelectRowInCol(index);
	ShowPlotAtIndex(index);
}

// *** EVENT HANDLERS ***
void wxDVDCCtrl::OnDataChannelSelection(wxCommandEvent& e)
{
	int row;
	bool isChecked;
	m_dataSelector->GetLastEventInfo(&row, NULL, &isChecked);

	if (isChecked)
		ShowPlotAtIndex(row);
	else
		HidePlotAtIndex(row);

	m_plotSurface->Refresh();
}

wxDVDCCtrl::PlotSet::PlotSet( wxDVTimeSeriesDataSet *ds )
{
	dataset = ds;
	plot = 0;
}

wxDVDCCtrl::PlotSet::~PlotSet()
{
	if (plot != 0)
		delete plot;
}