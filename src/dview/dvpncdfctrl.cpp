#include <algorithm>

#include <wx/wx.h>
#include <wx/busyinfo.h>

#include "wex/plot/plplotctrl.h"
#include "wex/plot/plhistplot.h"
#include "wex/plot/pllineplot.h"

#include "wex/dview/dvpncdfctrl.h"

enum { wxID_DATA_SELECTOR_CHOICE = wxID_HIGHEST + 1, 
	wxID_BIN_COMBO, 
	wxID_NORMALIZE_CHOICE, 
	wxID_Y_MAX_TB  };


BEGIN_EVENT_TABLE(wxDVPnCdfCtrl, wxPanel)
	EVT_CHOICE(wxID_DATA_SELECTOR_CHOICE, wxDVPnCdfCtrl::OnDataSelection)
	EVT_TEXT_ENTER(wxID_Y_MAX_TB, wxDVPnCdfCtrl::OnEnterYMax)
	EVT_CHOICE(wxID_NORMALIZE_CHOICE, wxDVPnCdfCtrl::OnNormalizeChoice)
	EVT_COMBOBOX(wxID_BIN_COMBO, wxDVPnCdfCtrl::OnBinComboSelection)
	EVT_TEXT_ENTER(wxID_BIN_COMBO, wxDVPnCdfCtrl::OnBinTextEnter)
END_EVENT_TABLE()

wxDVPnCdfCtrl::wxDVPnCdfCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, 
	const wxSize& size, long style, const wxString& name)
	: wxPanel(parent, id, pos, size, style, name)
{
	m_plotSurface = new wxPLPlotCtrl( this, wxID_ANY );
	m_pdfPlot = new wxPLHistogramPlot();
	m_cdfPlot = new wxPLLinePlot();
	m_cdfPlot->SetColour( *wxBLACK );
	m_cdfPlot->SetThickness( 2, false );
	m_plotSurface->AddPlot( m_pdfPlot );
	m_plotSurface->AddPlot( m_cdfPlot, wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_RIGHT );
	m_plotSurface->ShowTitle( false );

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(topSizer);

	
	m_maxTextBox = new wxTextCtrl(this, wxID_Y_MAX_TB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

	m_dataSelector = new wxChoice(this, wxID_DATA_SELECTOR_CHOICE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);


	wxBoxSizer *optionsSizer = new wxBoxSizer(wxHORIZONTAL);
	optionsSizer->Add(m_dataSelector, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER, 2);
	optionsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Y Max:")), 0, wxALIGN_CENTER|wxALL|wxALIGN_CENTER_VERTICAL, 2);
	optionsSizer->Add(m_maxTextBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
	optionsSizer->AddStretchSpacer();
	
	m_normalizeChoice = new wxChoice(this, wxID_NORMALIZE_CHOICE);
	m_normalizeChoice->Append(wxT("Histogram"));
	m_normalizeChoice->Append(wxT("Scaled Histogram"));
	m_normalizeChoice->Append(wxT("Scale by Bin Width (Match PDF)"));
	m_normalizeChoice->SetSelection(1);
	optionsSizer->Add(m_normalizeChoice, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL|wxALL, 2);

	wxStaticText *binsLabel = new wxStaticText(this, wxID_ANY, wxT("# Bins:"));
	optionsSizer->Add(binsLabel, 0, wxALIGN_CENTER, 0);
	m_binsCombo = new wxComboBox(this, wxID_BIN_COMBO);
	m_binsCombo->SetWindowStyle(wxTE_PROCESS_ENTER);
	m_binsCombo->Append(wxT("Sturge's Formula"));
	m_binsCombo->Append(wxT("SQRT (Excel)"));
	m_binsCombo->Append(wxT("20"));
	m_binsCombo->Append(wxT("50"));
	m_binsCombo->Append(wxT("100"));
	m_binsCombo->SetSelection(0);
	optionsSizer->Add(m_binsCombo, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);

	topSizer->Add(optionsSizer, 0, wxEXPAND, 0);
	
	topSizer->Add(m_plotSurface, 1, wxEXPAND|wxALL, 10);

	m_plotSurface->SetYAxis1( new wxPLLinearAxis(0, 100, "% of Data Points"));
	m_plotSurface->SetYAxis2( new wxPLLinearAxis(0, 100, "CDF %"));
	m_plotSurface->SetXAxis1( new wxPLLinearAxis(0, 1));
	m_plotSurface->ShowGrid( true, false );
	m_plotSurface->ShowLegend( false );

	m_selectedDataSetIndex = -1;
}

wxDVPnCdfCtrl::~wxDVPnCdfCtrl()
{
	for (int i=0; i < m_cdfPlotData.size(); i++)
		delete m_cdfPlotData[i];
}


// *** DATA SET FUNCTIONS ***
void wxDVPnCdfCtrl::AddDataSet(wxDVTimeSeriesDataSet* d, const wxString& group, bool update_ui)
{
	m_dataSets.push_back(d);

	//Add new plot data array, but leave it empty until we use it.  We'll fill it with sorted values then.
	m_cdfPlotData.push_back( new std::vector<wxRealPoint>() );

	wxString displayedName;
	if (group != wxEmptyString)
		displayedName = "-" + group + "- " + d->GetSeriesTitle() + " (" + d->GetUnits() + ")";
	else
		displayedName = d->GetSeriesTitle() + " (" + d->GetUnits() + ")";

	m_indexedDataNames.push_back(displayedName);
	m_dataSelector->Append(displayedName);

	if (update_ui)
		Layout(); //Resize dataSelector if needed.
}

void wxDVPnCdfCtrl::RemoveDataSet(wxDVTimeSeriesDataSet* d)
{
	if (m_dataSets[m_selectedDataSetIndex] == d)
	{
		ChangePlotDataTo(NULL);
		m_selectedDataSetIndex = -1;
	}

	int index = -1;
	for (size_t i=0;i<m_dataSets.size();i++)
		if ( d == m_dataSets[i] )
			index = i;
	
	if ( index < 0 ) return;

	m_dataSets.erase( m_dataSets.begin() + index);
	delete m_cdfPlotData[index];
	m_cdfPlotData.erase( m_cdfPlotData.begin() + index );

	m_dataSelector->Delete(m_dataSelector->FindString(m_indexedDataNames[index]));
	m_indexedDataNames.erase( m_indexedDataNames.begin() + index );

	InvalidatePlot();
}

void wxDVPnCdfCtrl::RemoveAllDataSets()
{
	ChangePlotDataTo(NULL);

	m_dataSets.clear();

	for (int i=0; i<m_cdfPlotData.size(); i++)
		delete m_cdfPlotData[i];
	m_cdfPlotData.clear();

	m_indexedDataNames.clear();
	m_dataSelector->Clear();

	InvalidatePlot();
}

wxString wxDVPnCdfCtrl::GetCurrentDataName()
{
	return m_dataSelector->GetStringSelection();
}

bool wxDVPnCdfCtrl::SetCurrentDataName(const wxString& name, bool restrictToSmallDataSet)
{
	for (int i=0; i < m_dataSets.size(); i++)
	{
		if (m_dataSets[i]->GetTitleWithUnits() == name)
		{
			if (restrictToSmallDataSet && m_dataSets[i]->Length() > 8760*2) return false;
			m_selectedDataSetIndex = i;
			ChangePlotDataTo(m_dataSets[i]);
			m_dataSelector->SetStringSelection(name);
			InvalidatePlot();
			return true;
		}
	}

	return false;
}

void wxDVPnCdfCtrl::SelectDataSetAtIndex(int index)
{
	if (index < 0 || index >= m_dataSets.size()) return;

	m_selectedDataSetIndex = index;
	ChangePlotDataTo(m_dataSets[index]);
	m_dataSelector->SetSelection(index);
}

void wxDVPnCdfCtrl::SetNumberOfBins(int n)
{
	if (m_binsCombo->GetSelection() == wxNOT_FOUND)
		m_binsCombo->SetValue(wxString::Format("%d", n));

	m_pdfPlot->SetNumberOfBins(n);
	m_plotSurface->GetYAxis1()->SetWorldMax( m_pdfPlot->GetNiceYMax() );
	m_maxTextBox->SetValue(wxString::Format("%lg", m_pdfPlot->GetNiceYMax()));
}

int wxDVPnCdfCtrl::GetNumberOfBins()
{
	return m_pdfPlot->GetNumberOfBins();
}

int wxDVPnCdfCtrl::GetBinSelectionIndex()
{
	return m_binsCombo->GetSelection();
}

void wxDVPnCdfCtrl::SetBinSelectionIndex(int i)
{
	m_binsCombo->SetSelection(i);
}

wxPLHistogramPlot::NormalizeType wxDVPnCdfCtrl::GetNormalizeType()
{
	return m_pdfPlot->GetNormalize();
}

void wxDVPnCdfCtrl::SetNormalizeType(wxPLHistogramPlot::NormalizeType t)
{
	m_normalizeChoice->SetSelection(int(t));

	m_pdfPlot->SetNormalize(t);

	UpdateYAxisLabel();
}

double wxDVPnCdfCtrl::GetYMax()
{
	return m_plotSurface->GetYAxis1()->GetWorldMax();
}

void wxDVPnCdfCtrl::SetYMax(double max)
{
	m_plotSurface->GetYAxis1()->SetWorldMax( max );
	InvalidatePlot();
	m_maxTextBox->SetValue(wxString::Format("%lg", max));
}

void wxDVPnCdfCtrl::RebuildPlotSurface(double maxYPercent)
{
	if ( m_selectedDataSetIndex < 0
		|| m_selectedDataSetIndex >= m_dataSets.size() )
		return;

	wxDVTimeSeriesDataSet *ds = m_dataSets[m_selectedDataSetIndex];

	m_plotSurface->GetYAxis1()->SetWorld(0, maxYPercent);
	m_maxTextBox->SetValue(wxString::Format("%lg", maxYPercent));

	double xMin, xMax;
	ds->GetDataMinAndMax(&xMin, &xMax);
	m_plotSurface->GetXAxis1()->SetWorld(xMin, xMax);

	wxString label = ds->GetSeriesTitle();
	if ( !ds->GetUnits().IsEmpty() )
		label += " (" + ds->GetUnits() + ")";
	m_plotSurface->GetXAxis1()->SetLabel( label );
}

void wxDVPnCdfCtrl::ChangePlotDataTo(wxDVTimeSeriesDataSet* d)
{
	if (d)
	{
		if (m_binsCombo->GetSelection() == 0) //Sturge's
			m_pdfPlot->SetNumberOfBins(m_pdfPlot->GetSturgesBinsFor(d->Length()));
		else if (m_binsCombo->GetSelection() == 1) //SQRT
			m_pdfPlot->SetNumberOfBins(m_pdfPlot->GetSqrtBinsFor(d->Length()));
		//Else it is just a number and we dont need to change it.
	}

	if (d)
	{
		m_pdfPlot->SetData( d->GetDataVector() ); //inefficient?
		m_pdfPlot->SetLabel( d->GetSeriesTitle() );
		m_pdfPlot->SetXDataLabel( m_plotSurface->GetXAxis1()->GetLabel() );
		m_pdfPlot->SetYDataLabel( d->GetSeriesTitle() );
	}
	else
	{
		m_pdfPlot->SetLabel( wxEmptyString );
		m_pdfPlot->SetData( std::vector<wxRealPoint>() );
		m_pdfPlot->SetXDataLabel( wxEmptyString );
		m_pdfPlot->SetYDataLabel( wxEmptyString );
	}

	int index = -1;
	for (size_t i=0;i<m_dataSets.size();i++)
		if ( m_dataSets[i] == d )
			index = i;

	if ( index < 0 )
	{
		m_cdfPlot->SetData( std::vector<wxRealPoint>() ); //clear the data
		m_cdfPlot->SetLabel( wxEmptyString );
		m_cdfPlot->SetXDataLabel( wxEmptyString );
		m_cdfPlot->SetYDataLabel( wxEmptyString );
	}
	else
	{
		// Read Cdf Data (requires sort) if not already sorted.
		if (m_cdfPlotData[index]->size() == 0)
			ReadCdfFrom(*d, m_cdfPlotData[index]);

		m_cdfPlot->SetData( *m_cdfPlotData[index] );
		m_cdfPlot->SetLabel(d->GetSeriesTitle() + " " + _("Percentile"));
		m_cdfPlot->SetXDataLabel( m_plotSurface->GetXAxis1()->GetLabel() );
		m_cdfPlot->SetYDataLabel( m_cdfPlot->GetLabel() );
	}

	if ( d )
		RebuildPlotSurface( m_pdfPlot->GetNiceYMax() );
}

void wxDVPnCdfCtrl::ReadCdfFrom(wxDVTimeSeriesDataSet& d, std::vector<wxRealPoint>* cdfArray)
{
	// This does not use bins.  It is an empirical CDF.  See wikipedia for empirical CDF explanation.
	// This can take a long time because of sorting.

	wxBeginBusyCursor();
	wxBusyInfo wait("Please wait, calculating CDF...");

	std::vector<double> sortedValues;
	sortedValues.reserve(d.Length());

	for(size_t i=0; i<d.Length(); i++)
		sortedValues.push_back(d.At(i).y);

	std::sort( sortedValues.begin(), sortedValues.end() );

	double dataMin = sortedValues[0];
	double dataMax = sortedValues[sortedValues.size()-1];

	cdfArray->reserve( sortedValues.size() );
	for (int i=0; i < sortedValues.size(); i++)
	{
		double x = sortedValues[i];
		double percent = 100 * double(i) / double(sortedValues.size() - 1);
		cdfArray->push_back(wxRealPoint(x, percent));
	}

	wxEndBusyCursor();
}

void wxDVPnCdfCtrl::UpdateYAxisLabel()
{
	switch(m_pdfPlot->GetNormalize())
	{
	case wxPLHistogramPlot::NO_NORMALIZE:
		m_plotSurface->GetYAxis1()->SetLabel( wxT("# of Data Points") );
		break;
	case wxPLHistogramPlot::NORMALIZE:
		m_plotSurface->GetYAxis1()->SetLabel( wxT("% of Data Points") );
		break;
	case wxPLHistogramPlot::NORMALIZE_PDF:
		if (m_selectedDataSetIndex != -1)
			m_plotSurface->GetYAxis1()->SetLabel( wxT("% per ") + m_dataSets[m_selectedDataSetIndex]->GetUnits() );
		else
			m_plotSurface->GetYAxis1()->SetLabel( wxT("PDF %") );
		break;
	}
}

// *** EVENT HANDLERS ***
void wxDVPnCdfCtrl::OnDataSelection(wxCommandEvent& e)
{
	//Assume same indexing.
	m_selectedDataSetIndex = -1;
	for ( size_t i=0;i<m_indexedDataNames.size();i++)
		if ( m_indexedDataNames[i] == m_dataSelector->GetStringSelection() )
			m_selectedDataSetIndex = i;

	if ( m_selectedDataSetIndex < 0 ) return;

	ChangePlotDataTo(m_dataSets[m_selectedDataSetIndex]);
	UpdateYAxisLabel();
	InvalidatePlot();
}

void wxDVPnCdfCtrl::OnEnterYMax(wxCommandEvent& e)
{
	double val;
	if (m_maxTextBox->GetValue().ToDouble(&val))
	{
		m_plotSurface->GetYAxis1()->SetWorldMax( val );
		InvalidatePlot();
	}
}

void wxDVPnCdfCtrl::OnBinComboSelection(wxCommandEvent& e)
{
	if ( m_selectedDataSetIndex < 0 || m_selectedDataSetIndex >= m_dataSets.size() )
		return;

	switch(m_binsCombo->GetSelection())
	{
	case 0:
		SetNumberOfBins(ceil(log10(double(m_dataSets[m_selectedDataSetIndex]->Length()))/log10(2.0) + 1)); //Sturges formula.
		break;
	case 1:
		SetNumberOfBins(sqrt(double(m_dataSets[m_selectedDataSetIndex]->Length()))); //Sqrt-choice.  Used by excel.
		break;
	case 2:
		SetNumberOfBins(20);
		break;
	case 3:
		SetNumberOfBins(50);
		break;
	case 4:
		SetNumberOfBins(100);
		break;
	}
	InvalidatePlot();
}

void wxDVPnCdfCtrl::OnBinTextEnter(wxCommandEvent& e)
{
	long bins;
	if (m_binsCombo->GetValue().ToLong(&bins))
	{
		SetNumberOfBins(bins);
		InvalidatePlot();
	}
	else
	{
		wxMessageBox("Not a valid number of bins!");
	}
}

void wxDVPnCdfCtrl::OnNormalizeChoice(wxCommandEvent& e)
{
	SetNormalizeType(wxPLHistogramPlot::NormalizeType(m_normalizeChoice->GetSelection()));
	m_plotSurface->GetYAxis1()->SetWorldMax( m_pdfPlot->GetNiceYMax() );
	m_maxTextBox->SetValue(wxString::Format("%lg", m_pdfPlot->GetNiceYMax()));
	InvalidatePlot();
}

void wxDVPnCdfCtrl::InvalidatePlot()
{
	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

