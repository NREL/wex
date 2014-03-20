/*
* wxDVStatisticsCtrl.cpp
*
* This class Is a wxPanel that contains a box plot with its axes, as well as
* a list of channels (different data sets) that can be viewed.
*
* Note: Plot units are months on the x-axis.  To handle these, we use wxTimeDate objects to get the date
* for axis labels.  If no year is specified, we use 1970 because the year doesn't matter.
*/

#include <wx/scrolbar.h>
#include <wx/gbsizer.h>
#include <wx/tokenzr.h>
#include <wx/statline.h>
#include <wx/gdicmn.h>
#include <math.h>

#include "wex/plot/pllineplot.h"

#include "wex/icons/zoom_in.cpng"
#include "wex/icons/zoom_out.cpng"
#include "wex/icons/zoom_fit.cpng"
#include "wex/icons/preferences.cpng"

#include "wex/dview/dvselectionlist.h"
#include "wex/dview/dvstatisticsctrl.h"
#include "wex/plot/plplotctrl.h"

static const wxString NO_UNITS("ThereAreNoUnitsForThisAxis.");

wxDVStatisticsPlot::wxDVStatisticsPlot(wxDVStatisticsDataSet *ds, bool OwnsDataset)
	: m_data(ds)
{
	m_colour = *wxRED;
	m_ownsDataset = OwnsDataset;
}

wxDVStatisticsPlot::~wxDVStatisticsPlot()
{
	if (m_ownsDataset)
	{
		delete m_data;
	}
}

void wxDVStatisticsPlot::SetColour(const wxColour &col) { m_colour = col; }

wxString wxDVStatisticsPlot::GetXDataLabel() const
{
	return _("Hours since 00:00 Jan 1");
}

wxString wxDVStatisticsPlot::GetYDataLabel() const
{
	wxString label = m_data->GetSeriesTitle();
	if (!m_data->GetUnits().IsEmpty())
		label += " (" + m_data->GetUnits() + ")";
	return label;
}

wxRealPoint wxDVStatisticsPlot::At(size_t i) const
{
	return (i < m_data->Length())
		? wxRealPoint(m_data->At(i).x, m_data->At(i).Mean)
		: wxRealPoint(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
}

StatisticsPoint wxDVStatisticsPlot::At(size_t i, double m_offset, double m_timestep) const
{
	StatisticsPoint p = StatisticsPoint();

	if ((i < m_data->Length()) && (i >= 0))
	{
		p.x = m_data->At(i).x;
		p.Sum = m_data->At(i).Sum;
		p.Min = m_data->At(i).Min;
		p.Max = m_data->At(i).Max;
		p.Mean = m_data->At(i).Mean;
		p.StDev = m_data->At(i).StDev;
		p.AvgDailyMin = m_data->At(i).AvgDailyMin;
		p.AvgDailyMax = m_data->At(i).AvgDailyMax;
	}
	else
	{
		p.x = m_offset + (i * m_timestep);
		p.Sum = 0.0;
		p.Min = 0.0;
		p.Max = 0.0;
		p.Mean = 0.0;
		p.StDev = 0.0;
		p.AvgDailyMin = 0.0;
		p.AvgDailyMax = 0.0;
	}

	return p;
}

size_t wxDVStatisticsPlot::Len() const
{
	return m_data->Length();
}

void wxDVStatisticsPlot::Draw(wxDC &dc, const wxPLDeviceMapping &map)
{
	if (!m_data || m_data->Length() < 2) return;

	size_t len;
	std::vector< wxPoint > points;
	wxRealPoint rpt;
	wxRealPoint rpt2;
	double tempY;
	wxRealPoint wmin = map.GetWorldMinimum();
	wxRealPoint wmax = map.GetWorldMaximum();

	dc.SetPen(wxPen(m_colour, 2, wxPENSTYLE_SOLID));

	//TODO:  UPDATE BELOW CODE TO DO BOX PLOT INSTEAD OF LINE GRAPH
	//len = m_data->Length();
	//if (m_data->At(0).x < wmin.x) { len++; }
	//if (m_data->At(m_data->Length() - 1).x > wmax.x) { len++; }

	//points.reserve(len);

	//for (size_t i = 0; i < m_data->Length(); i++)
	//{
	//	rpt = m_data->At(i);
	//	if (rpt.x < wmin.x || rpt.x > wmax.x) continue;
	//	points.push_back(map.ToDevice(m_data->At(i)));
	//}

	//if (points.size() == 0) return;

	//dc.DrawLines(points.size(), &points[0]);
}

void wxDVStatisticsPlot::DrawInLegend(wxDC &dc, const wxRect &rct)
{
	// nothing to do here
}

double wxDVStatisticsPlot::GetPeriodLowerBoundary(double hourNumber)
{
	hourNumber = fmod(hourNumber, 8760);

	if (hourNumber >= 0.0 && hourNumber < 744.0) { hourNumber = 0.0; }
	else if (hourNumber >= 744.0 && hourNumber < 1416.0) { hourNumber = 744.0; }
	else if (hourNumber >= 1416.0 && hourNumber < 2160.0) { hourNumber = 1416.0; }
	else if (hourNumber >= 2160.0 && hourNumber < 2880.0) { hourNumber = 2160.0; }
	else if (hourNumber >= 2880.0 && hourNumber < 3624.0) { hourNumber = 2880.0; }
	else if (hourNumber >= 3624.0 && hourNumber < 4344.0) { hourNumber = 3624.0; }
	else if (hourNumber >= 4344.0 && hourNumber < 5088.0) { hourNumber = 4344.0; }
	else if (hourNumber >= 5088.0 && hourNumber < 5832.0) { hourNumber = 5088.0; }
	else if (hourNumber >= 5832.0 && hourNumber < 6552.0) { hourNumber = 5832.0; }
	else if (hourNumber >= 6552.0 && hourNumber < 7296.0) { hourNumber = 6552.0; }
	else if (hourNumber >= 7296.0 && hourNumber < 8016.0) { hourNumber = 7296.0; }
	else if (hourNumber >= 8016.0 && hourNumber < 8760.0) { hourNumber = 8016.0; }

	return hourNumber;
}

double wxDVStatisticsPlot::GetPeriodUpperBoundary(double hourNumber)
{
	hourNumber = fmod(hourNumber, 8760);

	if (hourNumber >= 0.0 && hourNumber < 744.0) { hourNumber = 744.0; }
	else if (hourNumber >= 744.0 && hourNumber < 1416.0) { hourNumber = 1416.0; }
	else if (hourNumber >= 1416.0 && hourNumber < 2160.0) { hourNumber = 2160.0; }
	else if (hourNumber >= 2160.0 && hourNumber < 2880.0) { hourNumber = 2880.0; }
	else if (hourNumber >= 2880.0 && hourNumber < 3624.0) { hourNumber = 3624.0; }
	else if (hourNumber >= 3624.0 && hourNumber < 4344.0) { hourNumber = 4344.0; }
	else if (hourNumber >= 4344.0 && hourNumber < 5088.0) { hourNumber = 5088.0; }
	else if (hourNumber >= 5088.0 && hourNumber < 5832.0) { hourNumber = 5832.0; }
	else if (hourNumber >= 5832.0 && hourNumber < 6552.0) { hourNumber = 6552.0; }
	else if (hourNumber >= 6552.0 && hourNumber < 7296.0) { hourNumber = 7296.0; }
	else if (hourNumber >= 7296.0 && hourNumber < 8016.0) { hourNumber = 8016.0; }
	else if (hourNumber >= 8016.0 && hourNumber < 8760.0) { hourNumber = 8760.0; }

	return hourNumber;
}

std::vector<wxString> wxDVStatisticsPlot::GetExportableDatasetHeaders(wxUniChar sep, StatisticsType type) const
{
	std::vector<wxString> tt;
	wxString xLabel = GetXDataLabel();
	wxString yLabel = GetYDataLabel();

	if (xLabel.size() == 0) { xLabel = "Month"; }

	//Remove sep chars that we don't want
	while (xLabel.Find(sep) != wxNOT_FOUND)
	{
		xLabel = xLabel.BeforeFirst(sep) + xLabel.AfterFirst(sep);
	}

	while (yLabel.Find(sep) != wxNOT_FOUND)
	{
		yLabel = yLabel.BeforeFirst(sep) + yLabel.AfterFirst(sep);
	}

	tt.push_back(xLabel);
	if (type == MEAN) { tt.push_back("Mean " + yLabel); }
	if (type == MIN) { tt.push_back("Min. " + yLabel); }
	if (type == MAX) { tt.push_back("Max. " + yLabel); }
	if (type == SUMMATION) { tt.push_back("Sum " + yLabel); }
	if (type == STDEV) { tt.push_back("St. Dev. " + yLabel); }
	if (type == AVGDAILYMIN) { tt.push_back("Avg. Daily Min. " + yLabel); }
	if (type == AVGDAILYMAX) { tt.push_back("Avg. Daily Max. " + yLabel); }

	return tt;
}

std::vector<wxRealPoint> wxDVStatisticsPlot::GetExportableDataset(StatisticsType type) const
{
	std::vector<wxRealPoint> data;
	wxRealPoint pt;
	StatisticsPoint sp;

	for (size_t i = 0; i < Len(); i++)
	{
		sp = At(i, m_data->GetOffset(), m_data->GetTimeStep());

		if (type == MEAN) { pt = wxRealPoint(sp.x, sp.Mean); }
		if (type == MIN) { pt = wxRealPoint(sp.x, sp.Min); }
		if (type == MAX) { pt = wxRealPoint(sp.x, sp.Max); }
		if (type == SUMMATION) { pt = wxRealPoint(sp.x, sp.Sum); }
		if (type == STDEV) { pt = wxRealPoint(sp.x, sp.StDev); }
		if (type == AVGDAILYMIN) { pt = wxRealPoint(sp.x, sp.AvgDailyMin); }
		if (type == AVGDAILYMAX) { pt = wxRealPoint(sp.x, sp.AvgDailyMax); }

		data.push_back(pt);
	}

	return data;
}

enum{
	ID_DATA_CHANNEL_SELECTOR = wxID_HIGHEST + 1,
	ID_PLOT_SURFACE
};

BEGIN_EVENT_TABLE(wxDVStatisticsCtrl, wxPanel)

EVT_DVSELECTIONLIST(ID_DATA_CHANNEL_SELECTOR, wxDVStatisticsCtrl::OnDataChannelSelection)

END_EVENT_TABLE()


enum StatGraphAxisPosition
{
	TOP_LEFT_AXIS = 0,
	TOP_RIGHT_AXIS,
	GRAPH_AXIS_POSITION_COUNT
};

/*Constructors and Destructors*/
wxDVStatisticsCtrl::wxDVStatisticsCtrl(wxWindow *parent, wxWindowID id)
: wxPanel(parent, id)
{
	m_autoScale = true;

	m_plotSurface = new wxPLPlotCtrl(this, ID_PLOT_SURFACE);
	m_plotSurface->SetAllowHighlighting(true);
	m_plotSurface->ShowTitle(false);
	m_plotSurface->ShowLegend(false);
	m_plotSurface->ShowGrid(true, true);
	m_xAxis = new wxPLTimeAxis(0, 8760);
	m_plotSurface->SetXAxis1(m_xAxis);

	//Contains boxes to turn lines on or off.
	m_dataSelector = new wxDVSelectionListCtrl(this, ID_DATA_CHANNEL_SELECTOR, 2, false);

	wxBoxSizer *graph_sizer = new wxBoxSizer(wxHORIZONTAL);
	graph_sizer->Add(m_plotSurface, 1, wxEXPAND | wxALL, 4);
	graph_sizer->Add(m_dataSelector, 0, wxEXPAND | wxALL, 0);

	wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
	top_sizer->Add(graph_sizer, 1, wxALL | wxEXPAND, 0);
	SetSizer(top_sizer);

	for (int i = 0; i < GRAPH_AXIS_POSITION_COUNT; i++)
		m_selectedChannelIndices.push_back(new std::vector<int>());
}

wxDVStatisticsCtrl::~wxDVStatisticsCtrl(void)
{
	RemoveAllDataSets();

	for (int i = 0; i < GRAPH_AXIS_POSITION_COUNT; i++)
		delete m_selectedChannelIndices[i];
}

void wxDVStatisticsCtrl::Invalidate()
{
	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

/*** EVENT HANDLERS ***/

void wxDVStatisticsCtrl::OnDataChannelSelection(wxCommandEvent& e)
{
	int row, col;
	bool isChecked;

	m_dataSelector->GetLastEventInfo(&row, &col, &isChecked);

	if (isChecked)
		AddGraphAfterChannelSelection(row);
	else
		RemoveGraphAfterChannelSelection(row);
}

void wxDVStatisticsCtrl::AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group, bool refresh_ui, double xOffset)
{
	double timestep = d->GetTimeStep();
	double MinHrs = d->GetMinHours();
	double MaxHrs = d->GetMaxHours();
	wxDVStatisticsDataSet *s;
	wxDVStatisticsPlot *p;

	//Create monthly statistics from m_data
	double nextDay = 0.0;
	double nextMonth = 0.0;
	double currentMonth = 0.0;
	double year = 0.0;
	double counter = 0.0;
	double sum = 0.0;
	double avg = 0.0;
	double min = 0.0;
	double max = 0.0;
	double StDev = 0.0;
	double AvgDailyMin = 0.0;
	double AvgDailyMax = 0.0;
	int DayNumber = 0;
	int FirstOrdinalOfMonth = 0;
	double DayCounter = 0.0;
	StatisticsPoint sp;
	wxRealPoint DayStats[31];
	double StDevCounter = 0.0;

	s = new wxDVStatisticsDataSet(d->GetSeriesTitle(), d->GetUnits(), 744.0 / timestep);

	while (MinHrs > 8760.0)
	{
		year += 8760.0;
		MinHrs -= 8760.0;
	}

	while (nextDay < MinHrs)
	{
		nextDay += 24.0;
	}

	if (MinHrs >= 0.0 && MinHrs < 744.0) { currentMonth = year; nextMonth = year + 744.0; }
	else if (MinHrs >= 744.0 && MinHrs < 1416.0) { currentMonth = year + 744.0; nextMonth = year + 1416.0; }
	else if (MinHrs >= 1416.0 && MinHrs < 2160.0) { currentMonth = year + 1416.0; nextMonth = year + 2160.0; }
	else if (MinHrs >= 2160.0 && MinHrs < 2880.0) { currentMonth = year + 2160.0; nextMonth = year + 2880.0; }
	else if (MinHrs >= 2880.0 && MinHrs < 3624.0) { currentMonth = year + 2880.0; nextMonth = year + 3624.0; }
	else if (MinHrs >= 3624.0 && MinHrs < 4344.0) { currentMonth = year + 3624.0; nextMonth = year + 4344.0; }
	else if (MinHrs >= 4344.0 && MinHrs < 5088.0) { currentMonth = year + 4344.0; nextMonth = year + 5088.0; }
	else if (MinHrs >= 5088.0 && MinHrs < 5832.0) { currentMonth = year + 5088.0; nextMonth = year + 5832.0; }
	else if (MinHrs >= 5832.0 && MinHrs < 6552.0) { currentMonth = year + 5832.0; nextMonth = year + 6552.0; }
	else if (MinHrs >= 6552.0 && MinHrs < 7296.0) { currentMonth = year + 6552.0; nextMonth = year + 7296.0; }
	else if (MinHrs >= 7296.0 && MinHrs < 8016.0) { currentMonth = year + 7296.0; nextMonth = year + 8016.0; }
	else if (MinHrs >= 8016.0 && MinHrs < 8760.0) { currentMonth = year + 8016.0; nextMonth = year + 8760.0; }

	//TODO:  calculate StDev
	for (size_t i = 0; i < d->Length(); i++)
	{
		if (d->At(i).x >= nextDay)
		{
			if (i != 0)
			{
				DayStats[DayNumber] = wxRealPoint(AvgDailyMin, AvgDailyMax);
				DayNumber++;
				AvgDailyMin = 0.0;
				AvgDailyMax = 0.0;
				nextDay += 24.0;
			}
		}

		if (d->At(i).x >= nextMonth)
		{
			if (i != 0 && counter != 0)
			{
				avg = sum / counter;

				//Calculate standard deviation for the month's data
				StDev = 0.0;
				StDevCounter = 0.0;
				for (size_t j = FirstOrdinalOfMonth; j < i; j++)
				{
					StDev += (d->At(j).y - avg) * (d->At(j).y - avg);
					StDevCounter += 1.0;
				}
				if (StDevCounter > 0.0)
				{
					StDev = StDev / StDevCounter;
					StDev = sqrt(StDev);
				}

				//Summarize the daily Min and Max values into average daily min and max values for the month.
				for (size_t j = 0; j < 31; j++)
				{
					if (DayStats[j].y > 0)	//Not every month will have 31 days and endpoints may be missing some days in the month, as denoted by a 0 maximum value
					{
						AvgDailyMin += DayStats[j].x;
						AvgDailyMax += DayStats[j].y;
						DayCounter += 1.0;
					}
				}
				if (DayCounter > 0.0)
				{
					AvgDailyMax = AvgDailyMax / DayCounter;
					AvgDailyMin = AvgDailyMin / DayCounter;
				}

				sp = StatisticsPoint();
				if (xOffset < 672.0 && xOffset > 0.0)	//xOffset is within number of hours in the shortest month
				{
					sp.x = currentMonth + xOffset;
				}
				else
				{
					sp.x = currentMonth + ((nextMonth - currentMonth) / 2.0);	//Make x the middle of the month
				}
				sp.Max = max;
				sp.Min = min;
				sp.Sum = sum;
				sp.Mean = avg;
				sp.StDev = StDev;
				sp.AvgDailyMax = AvgDailyMax;
				sp.AvgDailyMin = AvgDailyMin;

				s->Append(sp);

				currentMonth = nextMonth;
				if (nextMonth == 744.0 + year) { nextMonth = 1416.0 + year; }
				else if (nextMonth == 1416.0 + year) { nextMonth = 2160.0 + year; }
				else if (nextMonth == 2160.0 + year) { nextMonth = 2880.0 + year; }
				else if (nextMonth == 2880.0 + year) { nextMonth = 3624.0 + year; }
				else if (nextMonth == 3624.0 + year) { nextMonth = 4344.0 + year; }
				else if (nextMonth == 4344.0 + year) { nextMonth = 5088.0 + year; }
				else if (nextMonth == 5088.0 + year) { nextMonth = 5832.0 + year; }
				else if (nextMonth == 5832.0 + year) { nextMonth = 6552.0 + year; }
				else if (nextMonth == 6552.0 + year) { nextMonth = 7296.0 + year; }
				else if (nextMonth == 7296.0 + year) { nextMonth = 8016.0 + year; }
				else if (nextMonth == 8016.0 + year) { nextMonth = 8760.0 + year; }
				else if (nextMonth == 8760.0 + year)
				{
					year += 8760.0;
					nextMonth = 744.0 + year;
				}
			}

			counter = 0.0;
			sum = 0.0;
			avg = 0.0;
			min = 0.0;
			max = 0.0;
			StDev = 0.0;
			AvgDailyMin = 0.0;
			AvgDailyMax = 0.0;
			DayNumber = 0;
			DayCounter = 0.0;
			FirstOrdinalOfMonth = i;
		}

		counter += 1.0;
		sum += d->At(i).y;
		if (min > d->At(i).y) { min = d->At(i).y; }
		if (max < d->At(i).y) { max = d->At(i).y; }
		if (AvgDailyMin > d->At(i).y) { AvgDailyMin = d->At(i).y; }
		if (AvgDailyMax < d->At(i).y) { AvgDailyMax = d->At(i).y; }
	}

	//Prevent appending the final point if it represents 12/31 24:00, which the system interprets as 1/1 0:00 and creates a point for January of the next year
	if (MaxHrs > 0.0 && fmod(MaxHrs, 8760.0) != 0)
	{
		DayStats[DayNumber] = wxRealPoint(AvgDailyMin, AvgDailyMax);
		AvgDailyMin = 0.0;
		AvgDailyMax = 0.0;

		avg = sum / counter;

		//Calculate standard deviation for the month's data
		StDev = 0.0;
		StDevCounter = 0.0;
		for (size_t j = FirstOrdinalOfMonth; j < d->Length(); j++)
		{
			StDev += (d->At(j).y - avg) * (d->At(j).y - avg);
			StDevCounter += 1.0;
		}
		if (StDevCounter > 0.0)
		{
			StDev = StDev / StDevCounter;
			StDev = sqrt(StDev);
		}

		//Summarize the daily Min and Max values into average daily min and max values for the month.
		for (size_t j = 0; j < 31; j++)
		{
			if (DayStats[j].y > 0)	//Not every month will have 31 days and endpoints may be missing some days in the month, as denoted by a 0 maximum value
			{
				AvgDailyMin += DayStats[j].x;
				AvgDailyMax += DayStats[j].y;
				DayCounter += 1.0;
			}
		}
		if (DayCounter > 0.0)
		{
			AvgDailyMax = AvgDailyMax / DayCounter;
			AvgDailyMin = AvgDailyMin / DayCounter;
		}

		sp = StatisticsPoint();
		if (xOffset < 672.0 && xOffset > 0.0)	//xOffset is within number of hours in the shortest month
		{
			sp.x = currentMonth + xOffset;
		}
		else
		{
			sp.x = currentMonth + ((nextMonth - currentMonth) / 2.0);	//Make x the middle of the month
		}
		sp.Max = max;
		sp.Min = min;
		sp.Sum = sum;
		sp.Mean = avg;
		sp.StDev = StDev;
		sp.AvgDailyMax = AvgDailyMax;
		sp.AvgDailyMin = AvgDailyMin;

		s->Append(sp);
	}

	p = new wxDVStatisticsPlot(s, true);

	m_plots.push_back(p); //Add to data sets list.
	m_dataSelector->Append(d->GetTitleWithUnits(), group);

	if (refresh_ui)
	{
		Layout();
		RefreshDisabledCheckBoxes();
	}
}

bool wxDVStatisticsCtrl::RemoveDataSet(wxDVTimeSeriesDataSet *d)
{
	wxDVStatisticsPlot *plotToRemove = NULL;
	wxDVStatisticsDataSet *ds;
	int removedIndex = 0;
	//Find the plottable:
	for (size_t i = 0; i<m_plots.size(); i++)
	{
		ds = m_plots[i]->GetDataSet();

		//TODO:  Is there a better way to identify the statistics dataset that corresponds with the passed time series dataset?
		if (ds->GetSeriesTitle() == d->GetSeriesTitle() && ds->GetTimeStep() == d->GetTimeStep() && ds->GetUnits() == d->GetUnits()
			&& ds->GetMinHours() == d->GetMinHours() && ds->GetMaxHours() == d->GetMaxHours())
		{
			removedIndex = i;
			plotToRemove = m_plots[i];
			break;
		}
	}

	if (!plotToRemove)
		return false;

	m_dataSelector->RemoveAt(removedIndex);

	for (int i = 0; i<wxPLPlotCtrl::NPLOTPOS; i++)
		m_plotSurface->RemovePlot(plotToRemove);

	Invalidate();

	m_plots.erase(m_plots.begin() + removedIndex); //This is more efficient than remove when we already know the index.

	//We base our logic for showing/hiding plots, etc on indices, so when a single data set is removed
	//we have to re-index everything.
	for (size_t i = 0; i<m_selectedChannelIndices.size(); i++)
	{
		for (size_t k = 0; k<m_selectedChannelIndices[i]->size(); k++)
		{
			if ((*m_selectedChannelIndices[i])[k] > removedIndex)
				(*m_selectedChannelIndices[i])[k]--;
		}
	}

	m_dataSelector->Layout(); //We removed some check boxes.
	RefreshDisabledCheckBoxes();
	return true;
}

void wxDVStatisticsCtrl::RemoveAllDataSets()
{
	ClearAllChannelSelections();

	m_dataSelector->RemoveAll();

	//Remove all data sets. Deleting a data set also deletes its plottable.
	for (size_t i = 0; i<m_plots.size(); i++)
		delete m_plots[i];

	m_plots.clear();
}

/*** VIEW METHODS ***/

double wxDVStatisticsCtrl::GetViewMin()
{
	return m_xAxis->GetWorldMin();
}

double wxDVStatisticsCtrl::GetViewMax()
{
	return m_xAxis->GetWorldMax();
}

void wxDVStatisticsCtrl::SetViewMin(double min)
{
	m_xAxis->SetWorldMin(min);
	AutoscaleYAxis();
	Invalidate();
}

void wxDVStatisticsCtrl::SetViewMax(double max)
{
	m_xAxis->SetWorldMax(max);
	AutoscaleYAxis();
	Invalidate();
}

//This setter also sets endpoints to days.
void wxDVStatisticsCtrl::SetViewRange(double min, double max)
{
	wxDVPlotHelper::SetRangeEndpointsToDays(&min, &max);
	m_xAxis->SetWorld(min, max);
	AutoscaleYAxis();
	Invalidate();
}

void wxDVStatisticsCtrl::GetVisibleDataMinAndMax(double* min, double* max, const std::vector<int>& selectedChannelIndices)
{
	*min = 0;
	*max = 0;

	double tempMin = 0;
	double tempMax = 0;
	if (m_xAxis)
	{
		double worldMin = m_xAxis->GetWorldMin();
		double worldMax = m_xAxis->GetWorldMax();

		for (size_t i = 0; i<selectedChannelIndices.size(); i++)
		{
			m_plots[selectedChannelIndices[i]]->GetDataSet()->GetMinAndMaxInRange(&tempMin, &tempMax, worldMin, worldMax);
			if (tempMin < *min)
				*min = tempMin;
			if (tempMax > *max)
				*max = tempMax;
		}
	}
	else
	{
		for (size_t i = 0; i<selectedChannelIndices.size(); i++)
		{
			m_plots[selectedChannelIndices[i]]->GetDataSet()->GetDataMinAndMax(&tempMin, &tempMax);
			if (tempMin < *min)
				*min = tempMin;
			if (tempMax > *max)
				*max = tempMax;
		}
	}
}

double wxDVStatisticsCtrl::GetMinPossibleTimeForVisibleChannels()
{
	double min = 8760;
	for (size_t i = 0; i<m_plots.size(); i++)
	{
		if (m_dataSelector->IsRowSelected(i))
		{
			if (m_plots[i]->GetDataSet()->GetMinHours() < min)
				min = m_plots[i]->GetDataSet()->GetMinHours();
		}
	}

	return min;
}

double wxDVStatisticsCtrl::GetMaxPossibleTimeForVisibleChannels()
{
	double max = 0;
	for (size_t i = 0; i<m_plots.size(); i++)
	{
		if (m_dataSelector->IsRowSelected(i))
		{
			if (m_plots[i]->GetDataSet()->GetMaxHours() > max)
				max = m_plots[i]->GetDataSet()->GetMaxHours();
		}
	}

	return max;
}

void wxDVStatisticsCtrl::MakeXBoundsNice(double* xMin, double* xMax)
{
	KeepNewBoundsWithinLimits(xMin, xMax);
	KeepNewBoundsWithinLowerLimit(xMin, xMax);
	wxDVPlotHelper::SetRangeEndpointsToDays(xMin, xMax);
}

void wxDVStatisticsCtrl::KeepNewBoundsWithinLimits(double* newMin, double* newMax)
{
	//Make sure we don't zoom out past the data range.
	//This can only happen zooming out.
	int visDataMin = m_plots[0]->GetDataSet()->GetMinHours();
	int visDataMax = m_plots[0]->GetDataSet()->GetMaxHours();
	for (size_t i = 1; i<m_plots.size(); i++)
	{
		if (m_dataSelector->IsRowSelected(i))
		{
			if (visDataMin > m_plots[i]->GetDataSet()->GetMinHours())
				visDataMin = m_plots[i]->GetDataSet()->GetMinHours();
			if (visDataMax < m_plots[i]->GetDataSet()->GetMaxHours())
				visDataMax = m_plots[i]->GetDataSet()->GetMaxHours();
		}
	}

	if (*newMin < visDataMin)
	{
		*newMax += visDataMin - *newMin;
		*newMin = visDataMin;
	}

	if (*newMax > visDataMax)
	{
		*newMin -= *newMax - visDataMax;
		if (*newMin < visDataMin)
			*newMin = visDataMin;
		*newMax = visDataMax;
	}

}

void wxDVStatisticsCtrl::KeepNewBoundsWithinLowerLimit(double* newMin, double* newMax)
{
	//Don't zoom in too far.  
	double lowerLim = 12;

	if (*newMax - *newMin > lowerLim)
		return;

	int visDataMin = m_plots[0]->GetDataSet()->GetMinHours();
	int visDataMax = m_plots[0]->GetDataSet()->GetMaxHours();
	for (size_t i = 1; i<m_plots.size(); i++)
	{
		if (m_dataSelector->IsRowSelected(i))
		{
			if (visDataMin > m_plots[i]->GetDataSet()->GetMinHours())
				visDataMin = m_plots[i]->GetDataSet()->GetMinHours();
			if (visDataMax < m_plots[i]->GetDataSet()->GetMaxHours())
				visDataMax = m_plots[i]->GetDataSet()->GetMaxHours();
		}
	}

	//If we make it here, we need to extend our bounds.
	if (visDataMax - *newMax > lowerLim - (*newMax - *newMin))
	{
		//Extend max and we are done.
		*newMax = *newMin + lowerLim;
		return;
	}
	else
	{
		//Extend as far as we can, and we aren't done.
		*newMax = visDataMax;
	}
	if (*newMin - visDataMin > lowerLim - (*newMax - *newMin))
	{
		*newMin = *newMax - lowerLim;
		return;
	}
	else
	{
		//Worst case.  Not extended to lower lim, but extended to data range.
		*newMin = visDataMin;
	}
}


/*** GRAPH RELATED METHODS ***/

void wxDVStatisticsCtrl::AutoscaleYAxis(bool forceUpdate)
{
	//It is probably best to avoid this function and use the more specific version where possible.
	if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP))
		AutoscaleYAxis(m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP), *m_selectedChannelIndices[TOP_LEFT_AXIS], forceUpdate);
	if (m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_TOP))
		AutoscaleYAxis(m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_TOP),
		m_selectedChannelIndices[TOP_RIGHT_AXIS]->size() > 0 ? *m_selectedChannelIndices[TOP_RIGHT_AXIS] : *m_selectedChannelIndices[TOP_LEFT_AXIS],
		forceUpdate);
}

void wxDVStatisticsCtrl::AutoscaleYAxis(wxPLAxis *axisToScale, const std::vector<int>& selectedChannelIndices, bool forceUpdate)
{
	//If autoscaling is off don't scale y1 axis
	// But do scale y2 axis (since we don't allow manual scaling there for UI simplicity).
	if (!m_autoScale && axisToScale == m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP)) { return; }

	//Force Update is used to rescale even if data is still in acceptable range
	bool needsRescale = false;
	double dataMax;
	double dataMin;

	GetVisibleDataMinAndMax(&dataMin, &dataMax, selectedChannelIndices);

	//If the maximum of the visible data is outside the acceptable range
	if (forceUpdate || (dataMax > 0 && (dataMax >= axisToScale->GetWorldMax() || dataMax < axisToScale->GetWorldMax() / 2.0)))
	{
		needsRescale = true;
	}
	if (forceUpdate || (dataMin < 0 && (dataMin <= axisToScale->GetWorldMin() || dataMin > axisToScale->GetWorldMin() / 2.0)))
	{
		needsRescale = true;
	}

	if (needsRescale)
	{
		wxDVPlotHelper::ExtendBoundToNiceNumber(&dataMax);
		wxDVPlotHelper::ExtendBoundToNiceNumber(&dataMin);

		//0 should always be visible.
		if (dataMin > 0)
			dataMin = 0;
		if (dataMax < 0)
			dataMax = 0;

		//applog("Setting y axis bounds to (%.2f, %.2f) \n", dataMin, dataMax);
		axisToScale->SetWorld(dataMin, dataMax);
		/*
		if (axisToScale == m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP))
		{
		m_y1Min = dataMin;
		m_y1Max = dataMax;
		}
		else if (axisToScale == m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM))
		{
		m_y2Min = dataMin;
		m_y2Max = dataMax;
		}*/
	}
}

void wxDVStatisticsCtrl::AddGraphAfterChannelSelection(int index)
{
	if (index < 0 || index >= (int)m_plots.size()) return;

	size_t idx = (size_t)index;

	//Set our line colour correctly.  Assigned by data selection window.
	m_plots[idx]->SetColour(m_dataSelector->GetColourForIndex(index));

	wxPLPlotCtrl::AxisPos yap = wxPLPlotCtrl::Y_LEFT;
	wxString units = m_plots[idx]->GetDataSet()->GetUnits();


	wxString y1Units = NO_UNITS, y2Units = NO_UNITS;

	if (m_plotSurface->GetYAxis1())
		y1Units = m_plotSurface->GetYAxis1()->GetLabel();

	if (m_plotSurface->GetYAxis2())
		y2Units = m_plotSurface->GetYAxis2()->GetLabel();

	if (m_plotSurface->GetYAxis1() && y1Units == units)
		yap = wxPLPlotCtrl::Y_LEFT;
	else if (m_plotSurface->GetYAxis2() && y2Units == units)
		yap = wxPLPlotCtrl::Y_RIGHT;
	else if (m_plotSurface->GetYAxis1() == 0)
		yap = wxPLPlotCtrl::Y_LEFT;
	else
		yap = wxPLPlotCtrl::Y_RIGHT;

	m_plotSurface->AddPlot(m_plots[index], wxPLPlotCtrl::X_BOTTOM, yap, wxPLPlotCtrl::PLOT_TOP, false);
	m_plotSurface->GetAxis(yap)->SetLabel(units);

	//Calculate index from 0-3.  0,1 are top graph L,R axis.  2,3 are L,R axis on bottom graph.
	int graphIndex = TOP_LEFT_AXIS;
	if (yap == wxPLPlotCtrl::Y_RIGHT)
		graphIndex += 1;

	m_selectedChannelIndices[graphIndex]->push_back(idx);

	switch (yap)
	{
	case wxPLPlotCtrl::Y_LEFT:
		AutoscaleYAxis(m_plotSurface->GetYAxis1(), *m_selectedChannelIndices[graphIndex], true);
		break;
	case wxPLPlotCtrl::Y_RIGHT:
		AutoscaleYAxis(m_plotSurface->GetYAxis2(), *m_selectedChannelIndices[graphIndex], true);
		break;
	}

	RefreshDisabledCheckBoxes();
	Invalidate();
}

void wxDVStatisticsCtrl::RemoveGraphAfterChannelSelection(int index)
{
	//Find our GraphAxisPosition, and remove from selected indices list.
	//Have to do it this way because of ambiguous use of int in an array storing ints.
	int graphIndex = 0;
	for (int i = graphIndex; i < graphIndex + 2; i++)
	{
		std::vector<int>::iterator it = std::find(m_selectedChannelIndices[i]->begin(), m_selectedChannelIndices[i]->end(), index);
		if (it != m_selectedChannelIndices[i]->end())
		{
			m_selectedChannelIndices[i]->erase(it);
			graphIndex = i;
			break;
		}
	}

	bool keepAxis = false;
	if (m_selectedChannelIndices[graphIndex]->size() > 0)
		keepAxis = true;

	m_plotSurface->RemovePlot(m_plots[index]);

	//See if axis is still in use or not, and to some cleanup.
	wxPLLinearAxis *axisThatWasUsed;
	if (graphIndex % 2 == 0)
		axisThatWasUsed = dynamic_cast<wxPLLinearAxis*>(m_plotSurface->GetYAxis1());
	else
		axisThatWasUsed = dynamic_cast<wxPLLinearAxis*>(m_plotSurface->GetYAxis2());


	if (!keepAxis)
	{
		int otherAxisIndex = TOP_LEFT_AXIS;
		switch (graphIndex)
		{
		case TOP_LEFT_AXIS:
			otherAxisIndex = TOP_RIGHT_AXIS;
			break;
		case TOP_RIGHT_AXIS:
			otherAxisIndex = TOP_LEFT_AXIS;
			break;
		}

		if (m_selectedChannelIndices[otherAxisIndex]->size() == 0)
		{
			m_plotSurface->SetYAxis1(NULL);
			m_plotSurface->SetYAxis2(NULL);
		}
		else
		{
			//If we only have one Y axis, we must use the left y axis.
			//Code in wpplotsurface2D uses this assumption.
			if (axisThatWasUsed == m_plotSurface->GetYAxis1())
			{
				std::vector<int> *temp = m_selectedChannelIndices[graphIndex];
				m_selectedChannelIndices[graphIndex] = m_selectedChannelIndices[otherAxisIndex];
				m_selectedChannelIndices[otherAxisIndex] = temp;

				//Set the y axis to the left side (instead of the right)
				for (size_t i = 0; i<m_selectedChannelIndices[graphIndex]->size(); i++)
				{
					m_plotSurface->RemovePlot(m_plots[(*m_selectedChannelIndices[graphIndex])[i]]);
					m_plotSurface->AddPlot(m_plots[(*m_selectedChannelIndices[graphIndex])[i]],
						wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT);
				}

				m_plotSurface->GetYAxis1()->SetLabel(m_plots[(*m_selectedChannelIndices[graphIndex])[0]]->GetDataSet()->GetUnits());
				AutoscaleYAxis(m_plotSurface->GetYAxis1(), *m_selectedChannelIndices[graphIndex], true);
			}
			m_plotSurface->SetYAxis2(NULL);
		}
	}
	else
	{
		AutoscaleYAxis(axisThatWasUsed, *m_selectedChannelIndices[graphIndex], true); //Pass true in case we removed a tall graph from a shorter one.
	}

	RefreshDisabledCheckBoxes();
	Invalidate();
}

void wxDVStatisticsCtrl::ClearAllChannelSelections()
{
	m_dataSelector->ClearColumn(1);

	int graphIndex = 0;

	for (int i = graphIndex; i<graphIndex + 2; i++)
	{
		for (size_t j = 0; j<m_selectedChannelIndices[i]->size(); j++)
		{
			m_plotSurface->RemovePlot(m_plots[m_selectedChannelIndices[i]->at(j)]);
		}
		m_selectedChannelIndices[i]->clear();
	}

	m_plotSurface->SetYAxis1(NULL);
	m_plotSurface->SetYAxis2(NULL);

	RefreshDisabledCheckBoxes();
	Invalidate();
}

void wxDVStatisticsCtrl::RefreshDisabledCheckBoxes()
{
	wxString axis1Label = NO_UNITS;
	wxString axis2Label = NO_UNITS;

	if (m_plotSurface->GetYAxis1())
		axis1Label = m_plotSurface->GetYAxis1()->GetLabel();
	if (m_plotSurface->GetYAxis2())
		axis2Label = m_plotSurface->GetYAxis2()->GetLabel();

	if (axis1Label != NO_UNITS
		&& axis2Label != NO_UNITS
		&& axis1Label != axis2Label)
	{
		for (int i = 0; i<m_dataSelector->Length(); i++)
		{
			m_dataSelector->Enable(i, wxPLPlotCtrl::PLOT_TOP, axis1Label == m_plots[i]->GetDataSet()->GetUnits()
				|| axis2Label == m_plots[i]->GetDataSet()->GetUnits());
		}
	}
	else
	{
		for (int i = 0; i<m_dataSelector->Length(); i++)
		{
			m_dataSelector->Enable(i, wxPLPlotCtrl::PLOT_TOP, true);
		}
	}
}

wxDVSelectionListCtrl* wxDVStatisticsCtrl::GetDataSelectionList()
{
	return m_dataSelector;
}

void wxDVStatisticsCtrl::SetSelectedNames(const wxString& names)
{
	SetSelectedNamesForColIndex(names, 0);
}

void wxDVStatisticsCtrl::SetSelectedNamesForColIndex(const wxString& names, int col)
{
	ClearAllChannelSelections();

	wxStringTokenizer tkz(names, ";");

	while (tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		int row = m_dataSelector->SelectRowWithNameInCol(token, col);
		if (row != -1)
			AddGraphAfterChannelSelection(row);
	}
}

void wxDVStatisticsCtrl::SelectDataSetAtIndex(int index)
{
	m_dataSelector->SelectRowInCol(index, 0);
	AddGraphAfterChannelSelection(index);
}

