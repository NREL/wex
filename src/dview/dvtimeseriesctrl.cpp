
/*
 * wxDVTimeSeriesCtrl.cpp
 *
 * This class Is a wxPanel that contains a time-series graph with its axes and a scrollbar, as well as
 * a list of channels (different data sets) that can be viewed.
 *
 * Note: Plot units are hours on the x-axis.  To handle these, we use wxTimeDate objects to get the date
 * for axis labels.  If no year is specified, we use 1970 because the year doesn't matter.
 */

#include <wx/scrolbar.h>
#include <wx/gbsizer.h>
#include <wx/tokenzr.h>
#include <wx/statline.h>
#include <wx/gdicmn.h>

#include "wex/plot/pllineplot.h"

#include "wex/icons/zoom_in.cpng"
#include "wex/icons/zoom_out.cpng"
#include "wex/icons/zoom_fit.cpng"
#include "wex/icons/preferences.cpng"

#include "wex/dview/dvselectionlist.h"
#include "wex/dview/dvtimeseriesdataset.h"
#include "wex/dview/dvtimeseriesctrl.h"

static const wxString NO_UNITS("ThereAreNoUnitsForThisAxis.");
enum { ID_TopCheckbox = wxID_HIGHEST + 1, ID_BottomCheckbox, ID_StatCheckbox };

class wxDVTimeSeriesPlot : public wxPLPlottable
{
	public:
		enum Style { NORMAL, STEPPED };

	private:
		wxDVTimeSeriesDataSet *m_data;
		wxColour m_colour;
		Style m_style;
		TimeSeriesType m_seriesType;
		bool m_ownsDataset;

	public:
		wxDVTimeSeriesPlot( wxDVTimeSeriesDataSet *ds, TimeSeriesType seriesType, bool OwnsDataset = false )
			: m_data(ds)
		{
			assert( ds != 0 );

			m_colour = *wxRED;
			m_seriesType = seriesType;
			m_style = (seriesType == RAW_DATA_TIME_SERIES || seriesType == HOURLY_TIME_SERIES) ? NORMAL : STEPPED;
			m_ownsDataset = OwnsDataset;
		}

		~wxDVTimeSeriesPlot()
		{
			if(m_ownsDataset)
			{
				delete m_data;
			}
		}

		void SetStyle( Style ss ) { m_style = ss; }
		void SetColour( const wxColour &col ) { m_colour = col; }

		virtual wxString GetXDataLabel() const 
		{
			return _("Hours since 00:00 Jan 1");
		}

		virtual wxString GetYDataLabel() const
		{
			wxString label = m_data->GetSeriesTitle();
			if ( !m_data->GetUnits().IsEmpty() )
				label += " (" + m_data->GetUnits() + ")";
			return label;
		}

		virtual wxRealPoint At( size_t i ) const
		{
			return ( i < m_data->Length() )
				? m_data->At(i) 
				: wxRealPoint( std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::quiet_NaN() ); 
		}

		virtual size_t Len() const
		{
			return m_data->Length();
		}

		virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map )
		{
			if ( !m_data || m_data->Length() < 2 ) return;

			size_t len;
			std::vector< wxPoint > points;
			wxRealPoint rpt;
			wxRealPoint rpt2;
			double tempY;
			wxRealPoint wmin = map.GetWorldMinimum();
			wxRealPoint wmax = map.GetWorldMaximum();
		
			dc.SetPen( wxPen( m_colour, 2, wxPENSTYLE_SOLID ) );

			if(m_style == NORMAL)
			{
				len = m_data->Length();
				if(m_data->At(0).x < wmin.x) { len++; }
				if(m_data->At(m_data->Length() - 1).x > wmax.x) { len++; }

				points.reserve( len );

				//If this is a line plot then add a point at the left edge of the graph if there isn't one there in the data
				if(m_style == NORMAL && m_data->At(0).x < wmin.x)
				{
					for ( size_t i = 1; i < m_data->Length(); i++ )
					{
						rpt = m_data->At(i);
						rpt2 = m_data->At(i - 1);
						if ( rpt.x > wmin.x )
						{
							tempY = rpt2.y + ((rpt.y - rpt2.y) * (wmin.x - rpt2.x) / (rpt.x - rpt2.x));
							points.push_back( map.ToDevice( wxRealPoint(wmin.x, tempY) ) );
							break;
						}
					}
				}

				for ( size_t i = 0; i < m_data->Length(); i++ )
				{
					rpt = m_data->At(i);
					if ( rpt.x < wmin.x || rpt.x > wmax.x ) continue;
					points.push_back( map.ToDevice( m_data->At(i) ) );
				}

				//If this is a line plot then add a point at the right edge of the graph if there isn't one there in the data
				if(m_style == NORMAL && m_data->At(m_data->Length() - 1).x > wmax.x)
				{
					for ( size_t i = m_data->Length() - 2; i >= 0; i-- )
					{
						rpt = m_data->At(i);
						rpt2 = m_data->At(i + 1);
						if ( rpt.x < wmax.x )
						{
							tempY = rpt.y + ((rpt2.y - rpt.y) * (wmax.x - rpt.x) / (rpt2.x - rpt.x));
							points.push_back( map.ToDevice( wxRealPoint(wmax.x, tempY) ) );
							break;
						}
					}
				}
			}
			else
			{
				//For stepped graphs create an array twice as big as the original and replace each single x value with two x values, one with the prior y value and one with the current y value
				len = m_data->Length() * 2;
				points.reserve( len );
				double timeStep = m_data->GetTimeStep();
				double lowX;
				double highX;
				double priorY;
				double nextY;

				for (size_t i = 0; i < m_data->Length(); i++)
				{
					rpt = m_data->At(i);
					lowX = GetPeriodLowerBoundary(rpt.x, timeStep);
					highX = GetPeriodUpperBoundary(rpt.x, timeStep);

					
					if(lowX >= wmin.x && highX <= wmax.x)	//Draw points for the lower and upper X boundaries of the point's horizontal range for each range that fits in the boundaries of the plot
					{
						//If the prior point's lower X boundary is off the left edge of the plot then draw points for the left edge of the plot and the visible point's lower X boundary at the prior point's Y
						if(i > 0 && GetPeriodLowerBoundary(m_data->At(i - 1).x, timeStep) < wmin.x)
						{
							priorY = m_data->At(i - 1).y;
							points.push_back( map.ToDevice( wxRealPoint(wmin.x, priorY) ) );
							points.push_back( map.ToDevice( wxRealPoint(lowX, priorY) ) );
						}

						points.push_back( map.ToDevice( wxRealPoint(lowX, rpt.y) ) );
						points.push_back( map.ToDevice( wxRealPoint(highX, rpt.y) ) );

						//If the next point's upper X boundary is off the right edge of the plot then draw points for the visible point's upper X boundary and the right edge of the plot at the next point's Y
						if(i < m_data->Length() - 1 && GetPeriodUpperBoundary(m_data->At(i + 1).x, timeStep) > wmax.x)
						{
							nextY = m_data->At(i + 1).y;
							points.push_back( map.ToDevice( wxRealPoint(highX, nextY) ) );
							points.push_back( map.ToDevice( wxRealPoint(wmax.x, nextY) ) );
							break;	//Any future points are outside the bounds of the graph
						}
					}
					else if(lowX < wmin.x && highX > wmin.x && highX <= wmax.x)	//Draw points for the plot left edge and point's upper X boundary at the point's Y if the lower boundary (only) is off the plot's left edge
					{
						points.push_back( map.ToDevice( wxRealPoint(wmin.x, rpt.y) ) );
						points.push_back( map.ToDevice( wxRealPoint(highX, rpt.y) ) );

						//If the next point's upper X boundary is off the right edge of the plot then draw points for the visible point's upper X boundary and the right edge of the plot at the next point's Y
						if(i < m_data->Length() - 1 && GetPeriodUpperBoundary(m_data->At(i + 1).x, timeStep) > wmax.x)
						{
							nextY = m_data->At(i + 1).y;
							points.push_back( map.ToDevice( wxRealPoint(highX, nextY) ) );
							points.push_back( map.ToDevice( wxRealPoint(wmax.x, nextY) ) );
							break;	//Any future points are outside the bounds of the graph
						}
					}
					else if(highX > wmax.x && lowX < wmax.x && lowX >= wmin.x)	//Draw points for the point's upper X boundary and the plot right edge at the point's Y if the upper boundary (only) is off the plot's right edge
					{
						//If the prior point's lower X boundary is off the left edge of the plot then draw points for the left edge of the plot and the visible point's lower X boundary at the prior point's Y
						if(i > 0 && GetPeriodLowerBoundary(m_data->At(i - 1).x, timeStep) < wmin.x)
						{
							priorY = m_data->At(i - 1).y;
							points.push_back( map.ToDevice( wxRealPoint(wmin.x, priorY) ) );
							points.push_back( map.ToDevice( wxRealPoint(lowX, priorY) ) );
						}

						points.push_back( map.ToDevice( wxRealPoint(wmin.x, rpt.y) ) );
						points.push_back( map.ToDevice( wxRealPoint(highX, rpt.y) ) );
					}
					else if(lowX < wmin.x && highX > wmax.x)	//Draw points for the plot's left and right edges and point's Y if the point's lower X boundary is off the plot's left edge and the upper X boundary is off the right edge
					{
						points.push_back( map.ToDevice( wxRealPoint(wmin.x, rpt.y) ) );
						points.push_back( map.ToDevice( wxRealPoint(highX, rpt.y) ) );
					}
				}
			}

			if ( points.size() == 0 ) return;

			dc.DrawLines( points.size(), &points[0] );
		}

		virtual wxString GetLabel() const
		{
			if ( !m_data ) return wxEmptyString;
			else return m_data->GetTitleWithUnits();
		}

		virtual void DrawInLegend( wxDC &dc, const wxRect &rct )
		{
			dc.SetPen( wxPen( m_colour, 3, wxCAP_ROUND ) );
			dc.DrawLine( rct.x, rct.y+rct.height/2, rct.x+rct.width, rct.y+rct.height/2 );
		}

		double GetPeriodLowerBoundary(double hourNumber, double timeStep)
		{
			if(m_seriesType == DAILY_TIME_SERIES)
			{
				hourNumber = hourNumber - fmod(hourNumber, 24);
			}
			else if (m_seriesType == MONTHLY_TIME_SERIES)
			{
				hourNumber = fmod(hourNumber, 8760);

				if(hourNumber >= 0.0 && hourNumber < 744.0) { hourNumber = 0.0; }
				else if(hourNumber >= 744.0 && hourNumber < 1416.0) { hourNumber = 744.0; }
				else if(hourNumber >= 1416.0 && hourNumber < 2160.0) { hourNumber = 1416.0; }
				else if(hourNumber >= 2160.0 && hourNumber < 2880.0) { hourNumber = 2160.0; }
				else if(hourNumber >= 2880.0 && hourNumber < 3624.0) { hourNumber = 2880.0; }
				else if(hourNumber >= 3624.0 && hourNumber < 4344.0) { hourNumber = 3624.0; }
				else if(hourNumber >= 4344.0 && hourNumber < 5088.0) { hourNumber = 4344.0; }
				else if(hourNumber >= 5088.0 && hourNumber < 5832.0) { hourNumber = 5088.0; }
				else if(hourNumber >= 5832.0 && hourNumber < 6552.0) { hourNumber = 5832.0; }
				else if(hourNumber >= 6552.0 && hourNumber < 7296.0) { hourNumber = 6552.0; }
				else if(hourNumber >= 7296.0 && hourNumber < 8016.0) { hourNumber = 7296.0; }
				else if(hourNumber >= 8016.0 && hourNumber < 8760.0) { hourNumber = 8016.0; }
			}
			else
			{
				hourNumber = hourNumber - fmod(hourNumber, timeStep);
			}

			return hourNumber;
		}

		double GetPeriodUpperBoundary(double hourNumber, double timeStep)
		{
			if(m_seriesType == DAILY_TIME_SERIES)
			{
				hourNumber = hourNumber - fmod(hourNumber, 24) + 24;
			}
			else if (m_seriesType == MONTHLY_TIME_SERIES)
			{
				hourNumber = fmod(hourNumber, 8760);

				if(hourNumber >= 0.0 && hourNumber < 744.0) { hourNumber = 744.0; }
				else if(hourNumber >= 744.0 && hourNumber < 1416.0) { hourNumber = 1416.0; }
				else if(hourNumber >= 1416.0 && hourNumber < 2160.0) { hourNumber = 2160.0; }
				else if(hourNumber >= 2160.0 && hourNumber < 2880.0) { hourNumber = 2880.0; }
				else if(hourNumber >= 2880.0 && hourNumber < 3624.0) { hourNumber = 3624.0; }
				else if(hourNumber >= 3624.0 && hourNumber < 4344.0) { hourNumber = 4344.0; }
				else if(hourNumber >= 4344.0 && hourNumber < 5088.0) { hourNumber = 5088.0; }
				else if(hourNumber >= 5088.0 && hourNumber < 5832.0) { hourNumber = 5832.0; }
				else if(hourNumber >= 5832.0 && hourNumber < 6552.0) { hourNumber = 6552.0; }
				else if(hourNumber >= 6552.0 && hourNumber < 7296.0) { hourNumber = 7296.0; }
				else if(hourNumber >= 7296.0 && hourNumber < 8016.0) { hourNumber = 8016.0; }
				else if(hourNumber >= 8016.0 && hourNumber < 8760.0) { hourNumber = 8760.0; }
			}
			else
			{
				hourNumber = hourNumber - fmod(hourNumber, timeStep) + timeStep;
			}

			return hourNumber;
		}

		virtual void UpdateSummaryData(bool divide)
		{
			double factor = 24.0;

			for(size_t i = 0; i < m_data->Length(); i++)
			{
				if (m_seriesType == MONTHLY_TIME_SERIES)
				{
					factor = fmod(m_data->At(i).x, 8760);

					if(factor >= 0.0 && factor < 744.0) { factor = 744.0; }
					else if(factor >= 744.0 && factor < 1416.0) { factor = 672.0; }
					else if(factor >= 1416.0 && factor < 2160.0) { factor = 744.0; }
					else if(factor >= 2160.0 && factor < 2880.0) { factor = 720.0; }
					else if(factor >= 2880.0 && factor < 3624.0) { factor = 744.0; }
					else if(factor >= 3624.0 && factor < 4344.0) { factor = 720.0; }
					else if(factor >= 4344.0 && factor < 5088.0) { factor = 744.0; }
					else if(factor >= 5088.0 && factor < 5832.0) { factor = 744.0; }
					else if(factor >= 5832.0 && factor < 6552.0) { factor = 720.0; }
					else if(factor >= 6552.0 && factor < 7296.0) { factor = 744.0; }
					else if(factor >= 7296.0 && factor < 8016.0) { factor = 720.0; }
					else if(factor >= 8016.0 && factor < 8760.0) { factor = 744.0; }
				}
				else if (m_seriesType == DAILY_TIME_SERIES)
				{
					factor = 24.0;
				}
				else
				{
					factor = m_data->GetTimeStep();
				}


				if ( wxDVArrayDataSet *arrdata = dynamic_cast<wxDVArrayDataSet*>( m_data ) )
				{
					if( divide ) arrdata->SetY( i, m_data->At(i).y / factor );
					else arrdata->SetY( i, m_data->At(i).y * factor );
				}
			}
		}

		wxDVTimeSeriesDataSet *GetDataSet() const { return m_data; }
};

BEGIN_EVENT_TABLE(wxDVTimeSeriesSettingsDialog, wxDialog)
	EVT_CHECKBOX(ID_TopCheckbox, wxDVTimeSeriesSettingsDialog::OnClickTopHandler)
	EVT_CHECKBOX(ID_BottomCheckbox, wxDVTimeSeriesSettingsDialog::OnClickBottomHandler)
	EVT_CHECKBOX(ID_StatCheckbox, wxDVTimeSeriesSettingsDialog::OnClickStatHandler)
END_EVENT_TABLE()

wxDVTimeSeriesSettingsDialog::wxDVTimeSeriesSettingsDialog( wxWindow *parent, const wxString &title, bool isBottomGraphVisible )
	: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE)
{
	mSyncCheck = new wxCheckBox(this, wxID_ANY, "Synchronize view with heat map" );
	mStatTypeCheck = new wxCheckBox(this, ID_StatCheckbox, "Use SUM (not AVG) for plots" );
		
	wxArrayString choices;
	choices.Add( "Line graph");
	choices.Add( "Stepped line graph" );
	mLineStyleCombo = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
				
	mTopAutoscaleCheck = new wxCheckBox(this, ID_TopCheckbox, "Autoscale top y1-axis");
	mBottomTopAutoscaleCheck = new wxCheckBox(this, ID_BottomCheckbox, "Autoscale bottom y1-axis");
		
	wxFlexGridSizer *yTopBoundSizer = new wxFlexGridSizer(2, 2, 0);
	yTopBoundSizer->Add(new wxStaticText(this, wxID_ANY, "Top Y Min:"), 0, wxLEFT|wxRIGHT|wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
	mTopYMinCtrl = new wxNumericCtrl(this);
	yTopBoundSizer->Add(mTopYMinCtrl, 0, wxLEFT|wxRIGHT, 2);
	yTopBoundSizer->Add(new wxStaticText(this, wxID_ANY, "Top Y Max:"), 0, wxLEFT|wxRIGHT|wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
	mTopYMaxCtrl = new wxNumericCtrl(this);
	yTopBoundSizer->Add(mTopYMaxCtrl, 0, wxLEFT|wxRIGHT, 2);
		
	wxFlexGridSizer *yBottomBoundSizer = new wxFlexGridSizer(2, 2, 0);
	if(isBottomGraphVisible) { yBottomBoundSizer->Add(new wxStaticText(this, wxID_ANY, "Bottom Y Min:"), 0, wxLEFT|wxRIGHT|wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2); }
	mBottomYMinCtrl = new wxNumericCtrl(this);
	yBottomBoundSizer->Add(mBottomYMinCtrl, 0, wxLEFT|wxRIGHT|wxBOTTOM, 2);
	if(isBottomGraphVisible) { yBottomBoundSizer->Add(new wxStaticText(this, wxID_ANY, "Bottom Y Max:"), 0, wxLEFT|wxRIGHT|wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2); }
	mBottomYMaxCtrl = new wxNumericCtrl(this);
	yBottomBoundSizer->Add(mBottomYMaxCtrl, 0, wxLEFT|wxRIGHT|wxBOTTOM, 2);
		
	wxBoxSizer *boxmain = new wxBoxSizer(wxVERTICAL);
	boxmain->Add( mSyncCheck, 0, wxALL|wxEXPAND, 10 );
	boxmain->Add( mStatTypeCheck, 0, wxALL|wxEXPAND, 10 );
	boxmain->Add( mLineStyleCombo, 0, wxALL|wxEXPAND, 10 );
	boxmain->Add( new wxStaticLine( this ), 0, wxALL|wxEXPAND, 0 );
	boxmain->Add( mTopAutoscaleCheck, 0, wxALL|wxEXPAND, 10 );
	boxmain->Add( yTopBoundSizer, 1, wxALL|wxEXPAND, 10 );
	if(isBottomGraphVisible) { boxmain->Add( new wxStaticLine( this ), 0, wxALL|wxEXPAND, 0 ); }
	boxmain->Add( mBottomTopAutoscaleCheck, 0, wxALL|wxEXPAND, 10 );
	boxmain->Add( yBottomBoundSizer, 1, wxALL|wxEXPAND, 10 );
	boxmain->Add( new wxStaticLine( this ), 0, wxALL|wxEXPAND, 0 );
	boxmain->Add( CreateButtonSizer( wxOK|wxCANCEL ), 0, wxALL|wxEXPAND, 20 );
	SetSizer(boxmain);
	Fit();

	if(!isBottomGraphVisible)
	{
		mBottomTopAutoscaleCheck->Hide();
		mBottomYMaxCtrl->Hide();
		mBottomYMinCtrl->Hide();
	}
}

wxDVTimeSeriesSettingsDialog::~wxDVTimeSeriesSettingsDialog()
{
}

void wxDVTimeSeriesSettingsDialog::SetTopYBounds( double y1min, double y1max )
{
	mTopYMinCtrl->SetValue( y1min );
	mTopYMaxCtrl->SetValue( y1max );
}

void wxDVTimeSeriesSettingsDialog::SetBottomYBounds( double y2min, double y2max )
{
	mBottomYMinCtrl->SetValue( y2min );
	mBottomYMaxCtrl->SetValue( y2max );
}

void wxDVTimeSeriesSettingsDialog::GetTopYBounds( double *y1min, double *y1max )
{
	*y1min = mTopYMinCtrl->Value();
	*y1max = mTopYMaxCtrl->Value();
}

void wxDVTimeSeriesSettingsDialog::GetBottomYBounds( double *y2min, double *y2max )
{
	*y2min = mBottomYMinCtrl->Value();
	*y2max = mBottomYMaxCtrl->Value();
}

void wxDVTimeSeriesSettingsDialog::SetLineStyle( int id ) { mLineStyleCombo->SetSelection(id); }
int wxDVTimeSeriesSettingsDialog::GetLineStyle() { return mLineStyleCombo->GetSelection(); }

void wxDVTimeSeriesSettingsDialog::SetSync( bool b ) { mSyncCheck->SetValue( b ); }
bool wxDVTimeSeriesSettingsDialog::GetSync() { return mSyncCheck->GetValue(); }

void wxDVTimeSeriesSettingsDialog::SetStatType( StatType statType ) { mStatTypeCheck->SetValue( statType == SUM ? true : false ); }
StatType wxDVTimeSeriesSettingsDialog::GetStatType() { return mStatTypeCheck->GetValue() ? SUM : AVERAGE; }

void wxDVTimeSeriesSettingsDialog::SetAutoscale( bool b ) 
{ 
	mTopAutoscaleCheck->SetValue( b ); 
	mTopYMaxCtrl->Enable(!b);
	mTopYMinCtrl->Enable(!b);
}
bool wxDVTimeSeriesSettingsDialog::GetAutoscale() { return mTopAutoscaleCheck->GetValue(); }

void wxDVTimeSeriesSettingsDialog::SetBottomAutoscale( bool b ) 
{ 
	mBottomTopAutoscaleCheck->SetValue( b ); 
	mBottomYMaxCtrl->Enable(!b);
	mBottomYMinCtrl->Enable(!b);
}
bool wxDVTimeSeriesSettingsDialog::GetBottomAutoscale() { return mBottomTopAutoscaleCheck->GetValue(); }

void wxDVTimeSeriesSettingsDialog::OnClickTopHandler(wxCommandEvent& event)
{
	SetAutoscale( mTopAutoscaleCheck->IsChecked() );
}

void wxDVTimeSeriesSettingsDialog::OnClickBottomHandler(wxCommandEvent& event)
{
	SetBottomAutoscale( mBottomTopAutoscaleCheck->IsChecked() );
}

void wxDVTimeSeriesSettingsDialog::OnClickStatHandler(wxCommandEvent& event)
{
	SetStatType( mStatTypeCheck->IsChecked() ? SUM : AVERAGE );
}


enum{
		ID_DATA_CHANNEL_SELECTOR = wxID_HIGHEST+1, 
		ID_GRAPH_SCROLLBAR,
		ID_PLOT_SURFACE };

BEGIN_EVENT_TABLE(wxDVTimeSeriesCtrl, wxPanel)
	EVT_BUTTON(wxID_ZOOM_IN, wxDVTimeSeriesCtrl::OnZoomIn)
	EVT_BUTTON(wxID_ZOOM_OUT, wxDVTimeSeriesCtrl::OnZoomOut)
	EVT_BUTTON(wxID_ZOOM_FIT, wxDVTimeSeriesCtrl::OnZoomFit)
	EVT_BUTTON(wxID_PREFERENCES, wxDVTimeSeriesCtrl::OnSettings)

	EVT_MOUSEWHEEL(wxDVTimeSeriesCtrl::OnMouseWheel)

	EVT_PLOT_HIGHLIGHT(ID_PLOT_SURFACE, wxDVTimeSeriesCtrl::OnHighlight)
	
/*
	EVT_WPPLOT_DRAG(ID_PLOT_SURFACE, wxDVTimeSeriesCtrl::OnPlotDrag)
	EVT_WPPLOT_DRAG_START(ID_PLOT_SURFACE, wxDVTimeSeriesCtrl::OnPlotDragStart)
	EVT_WPPLOT_DRAG_END(ID_PLOT_SURFACE, wxDVTimeSeriesCtrl::OnPlotDragEnd)
*/

	EVT_DVSELECTIONLIST(ID_DATA_CHANNEL_SELECTOR, wxDVTimeSeriesCtrl::OnDataChannelSelection)

	EVT_COMMAND_SCROLL_THUMBTRACK(ID_GRAPH_SCROLLBAR, wxDVTimeSeriesCtrl::OnGraphScroll)
	EVT_COMMAND_SCROLL_LINEUP(ID_GRAPH_SCROLLBAR, wxDVTimeSeriesCtrl::OnGraphScrollLineUp)
	EVT_COMMAND_SCROLL_LINEDOWN(ID_GRAPH_SCROLLBAR, wxDVTimeSeriesCtrl::OnGraphScrollLineDown)
	//EVT_COMMAND_SCROLL_CHANGED(ID_GRAPH_SCROLLBAR, wxDVTimeSeriesCtrl::OnGraphScroll)
	EVT_COMMAND_SCROLL_PAGEDOWN(ID_GRAPH_SCROLLBAR, wxDVTimeSeriesCtrl::OnGraphScrollPageDown)
	EVT_COMMAND_SCROLL_PAGEUP(ID_GRAPH_SCROLLBAR, wxDVTimeSeriesCtrl::OnGraphScrollPageUp)

END_EVENT_TABLE()


/*Constructors and Destructors*/
wxDVTimeSeriesCtrl::wxDVTimeSeriesCtrl(wxWindow *parent, wxWindowID id, TimeSeriesType seriesType, StatType statType)
: wxPanel(parent, id)
{	
	m_topAutoScale = true;
	m_bottomAutoScale = true;
	m_syncToHeatMap = false;
	m_lineStyle = ((seriesType == RAW_DATA_TIME_SERIES || seriesType == HOURLY_TIME_SERIES) ? wxDVTimeSeriesPlot::NORMAL : wxDVTimeSeriesPlot::STEPPED); // line, stepped, points
	m_seriesType = seriesType;
	m_statType = statType;

	m_plotSurface = new wxPLPlotCtrl(this, ID_PLOT_SURFACE); 
	m_plotSurface->SetAllowHighlighting(true);
	m_plotSurface->ShowTitle( false );
	m_plotSurface->ShowLegend( false );
	//m_plotSurface->SetLegendLocation( wxPLPlotCtrl::RIGHT );
	m_plotSurface->SetIncludeLegendOnExport( true );
	m_plotSurface->ShowGrid( true, true );
	m_xAxis = new wxPLTimeAxis( 0, 8760 );
	m_plotSurface->SetXAxis1( m_xAxis );

	wxBoxSizer *scrollerAndZoomSizer = new wxBoxSizer(wxHORIZONTAL);
	m_graphScrollBar = new wxScrollBar(this, ID_GRAPH_SCROLLBAR, wxDefaultPosition, wxDefaultSize, wxHORIZONTAL);
	scrollerAndZoomSizer->Add(m_graphScrollBar, 1, wxALL|wxALIGN_CENTER_VERTICAL, 3);

	wxBitmapButton *zoom_in =  new wxBitmapButton( this, wxID_ZOOM_IN, wxBITMAP_PNG_FROM_DATA( zoom_in ));
	zoom_in->SetToolTip("Zoom in");
	scrollerAndZoomSizer->Add( zoom_in, 0, wxALL|wxEXPAND, 1);

	wxBitmapButton *zoom_out = new wxBitmapButton( this, wxID_ZOOM_OUT, wxBITMAP_PNG_FROM_DATA( zoom_out ));
	zoom_out->SetToolTip("Zoom out");
	scrollerAndZoomSizer->Add( zoom_out, 0, wxALL|wxEXPAND, 1);

	wxBitmapButton *zoom_fit = new wxBitmapButton( this, wxID_ZOOM_FIT, wxBITMAP_PNG_FROM_DATA( zoom_fit ));
	zoom_fit->SetToolTip("Zoom fit");
	scrollerAndZoomSizer->Add( zoom_fit , 0, wxALL|wxEXPAND, 1);

	wxBitmapButton *pref_btn = new wxBitmapButton( this, wxID_PREFERENCES, wxBITMAP_PNG_FROM_DATA( preferences ));
	pref_btn->SetToolTip("Edit view settings and graph scaling...");
	scrollerAndZoomSizer->Add( pref_btn, 0, wxALL|wxEXPAND, 1);
	
	//Contains boxes to turn lines on or off.
	m_dataSelector = new wxDVSelectionListCtrl(this, ID_DATA_CHANNEL_SELECTOR, 2);
		
	wxBoxSizer *graph_sizer = new wxBoxSizer(wxHORIZONTAL);	
	graph_sizer->Add( m_plotSurface, 1, wxEXPAND|wxALL, 4);
	graph_sizer->Add( m_dataSelector, 0, wxEXPAND|wxALL, 0);

	wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
	top_sizer->Add( graph_sizer, 1, wxALL|wxEXPAND, 0 );
	top_sizer->Add( scrollerAndZoomSizer, 0, wxEXPAND, 0);
	SetSizer(top_sizer);

	for (int i=0; i<GRAPH_AXIS_POSITION_COUNT; i++)
		m_selectedChannelIndices.push_back(new std::vector<int>());

	UpdateScrollbarPosition();
}

wxDVTimeSeriesCtrl::~wxDVTimeSeriesCtrl(void)
{
	RemoveAllDataSets();

	for (int i=0; i<GRAPH_AXIS_POSITION_COUNT; i++)
		delete m_selectedChannelIndices[i];
}

void wxDVTimeSeriesCtrl::Invalidate()
{
	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

TimeSeriesType wxDVTimeSeriesCtrl::GetTimeSeriesType()
{
	return m_seriesType;
}


/*** EVENT HANDLERS ***/

void wxDVTimeSeriesCtrl::OnZoomIn(wxCommandEvent& e)
{
	if ( m_plots.size() == 0)
		return;

	//Make sure we are not already zoomed in too far.
	if ( CanZoomIn() )
		ZoomFactorAndUpdate(2.0);
}

void wxDVTimeSeriesCtrl::OnZoomOut(wxCommandEvent& e)
{
	if ( m_plots.size() == 0)
		return;

	//Make sure we don't zoom out past the data range.
	if ( CanZoomOut() )
		ZoomFactorAndUpdate(0.5);
}
void wxDVTimeSeriesCtrl::OnZoomFit(wxCommandEvent& e)
{
	ZoomToFit();		
}

void wxDVTimeSeriesCtrl::OnSettings( wxCommandEvent &e )
{
	double y1min = 0, y1max = 0, y2min = 0, y2max = 0;
	bool isBottomGraphVisible = false;

	for(int i = 0; i < m_dataSelector->Length(); i++)
	{
		if(m_dataSelector->IsSelected(i, 1))
		{
			isBottomGraphVisible = true;
			break;
		}
	}

	if ( m_plotSurface->GetYAxis1( wxPLPlotCtrl::PLOT_TOP ) != 0 )
		m_plotSurface->GetYAxis1( wxPLPlotCtrl::PLOT_TOP )->GetWorld( &y1min, &y1max );

	if ( m_plotSurface->GetYAxis1( wxPLPlotCtrl::PLOT_BOTTOM ) != 0 )
		m_plotSurface->GetYAxis1( wxPLPlotCtrl::PLOT_BOTTOM )->GetWorld( &y2min, &y2max );

	wxDVTimeSeriesSettingsDialog dlg(  this, "View Settings", isBottomGraphVisible );
	dlg.CentreOnParent();
	dlg.SetSync( m_syncToHeatMap );
	dlg.SetStatType( m_statType );
	dlg.SetLineStyle( m_lineStyle );
	dlg.SetAutoscale( m_topAutoScale );
	dlg.SetBottomAutoscale( m_bottomAutoScale );
	dlg.SetTopYBounds( y1min, y1max );
	dlg.SetBottomYBounds( y2min, y2max );
	if (wxID_OK == dlg.ShowModal())
	{
		m_syncToHeatMap = dlg.GetSync();
		m_lineStyle = dlg.GetLineStyle();

		if(m_statType != dlg.GetStatType())
		{
			m_statType = dlg.GetStatType();
			wxString nonmodifiables;

			for(size_t i = 0; i < m_plots.size(); i++)
			{
				m_plots[i]->SetStyle( (wxDVTimeSeriesPlot::Style) m_lineStyle );
				m_plots[i]->UpdateSummaryData(m_statType == AVERAGE ? true : false);

				if ( 0 == dynamic_cast<wxDVArrayDataSet*>( m_plots[i]->GetDataSet() ) )
					nonmodifiables += m_plots[i]->GetDataSet()->GetSeriesTitle() + "\n";
			}

			if ( nonmodifiables.size() > 0 )
				wxMessageBox("The following plots could not be configured for the modified statistic type:\n\n" + nonmodifiables );
		}
		else
		{
			for (size_t i=0; i<m_plots.size(); i++)
			{
				m_plots[i]->SetStyle( (wxDVTimeSeriesPlot::Style) m_lineStyle );
			}
		}

		m_topAutoScale = dlg.GetAutoscale();
		m_bottomAutoScale = dlg.GetBottomAutoscale();
		if ( m_topAutoScale )
		{
			if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP))
				AutoscaleYAxis(m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP), *m_selectedChannelIndices[TOP_LEFT_AXIS], true);
			if (m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_TOP))
				AutoscaleYAxis(m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_TOP), 
				m_selectedChannelIndices[TOP_RIGHT_AXIS]->size() > 0 ? *m_selectedChannelIndices[TOP_RIGHT_AXIS] : *m_selectedChannelIndices[TOP_LEFT_AXIS], 
				true);
		}
		else
		{
			dlg.GetTopYBounds( &y1min, &y1max );

			if (y1max > y1min)
			{
				if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP))
					m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP)->SetWorld( y1min, y1max );
			}
		}

		if ( m_bottomAutoScale )
		{
			if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM))
				AutoscaleYAxis(m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM), *m_selectedChannelIndices[BOTTOM_LEFT_AXIS], true);
			if (m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_BOTTOM))
				AutoscaleYAxis(m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_BOTTOM), 
				m_selectedChannelIndices[BOTTOM_RIGHT_AXIS]->size() > 0 ? *m_selectedChannelIndices[BOTTOM_RIGHT_AXIS] : *m_selectedChannelIndices[BOTTOM_LEFT_AXIS], 
				true);	
		}
		else
		{
			dlg.GetBottomYBounds( &y2min, &y2max );

			if (y2max > y2min)
			{		
				if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM))
					m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM)->SetWorld( y2min, y2max );
			}
		}

		Invalidate();
	}
}

void wxDVTimeSeriesCtrl::OnMouseWheel(wxMouseEvent& e)
{
	if (!m_xAxis || m_plots.size() == 0)
		return;

	if (e.ShiftDown()) //shift + wheel: scroll
	{
		//Negative rotation will go left.
		PanByPercent(-0.25 * e.GetWheelRotation() / e.GetWheelDelta()); 
	}
	else //wheel only: zoom
	{
		if (e.GetWheelRotation() > 0 && !CanZoomIn()) return;
		//Center zooming on the location of the mouse.
		wxCoord xPos;
		e.GetPosition(&xPos, NULL);

		double min = m_xAxis->GetWorldMin();
		double max = m_xAxis->GetWorldMax();
		wxRect rr = m_plotSurface->GetClientRect(); 
		wxDVPlotHelper::MouseWheelZoom( &min, &max, 
			xPos, 0, rr.width, e.GetWheelRotation() / e.GetWheelDelta()); // TODO: replace 0, rr.width with physical xaxis coordinates

		MakeXBoundsNice(&min, &max);
		m_xAxis->SetWorld( min, max );
	}

	AutoscaleYAxis();
	UpdateScrollbarPosition();
	Invalidate();
}

void wxDVTimeSeriesCtrl::OnHighlight(wxCommandEvent& e)
{
	double left, right;
	m_plotSurface->GetHighlightBounds( &left, &right );

	double leftWorld = 0, rightWorld = 1;
	double wmin, wmax;
	m_xAxis->GetWorld(&wmin, &wmax);

	leftWorld = wmin + left/100.0 * (wmax-wmin);
	rightWorld = wmin + right/100.0 * (wmax-wmin);
	SetViewRange(leftWorld, rightWorld);
}

void wxDVTimeSeriesCtrl::OnGraphScroll(wxScrollEvent &e)
{
	double dataRange = m_xAxis->GetWorldLength();
	double min = e.GetPosition() + GetMinPossibleTimeForVisibleChannels();
	double max = min + dataRange;
	wxDVPlotHelper::SetRangeEndpointsToDays( &min, &max );
	m_xAxis->SetWorld( min, max );
	AutoscaleYAxis();
	Invalidate();
}

//Scrolling the graph a line up or a line down occurs when the user clicks the left or right button on the scrollbar.
void wxDVTimeSeriesCtrl::OnGraphScrollLineUp(wxScrollEvent& e)
{
	PanByPercent(-0.25);
}
void wxDVTimeSeriesCtrl::OnGraphScrollLineDown(wxScrollEvent& e)
{
	PanByPercent(0.25);
}
void wxDVTimeSeriesCtrl::OnGraphScrollPageUp(wxScrollEvent& e)
{
	PanByPercent(-1.0);
}
void wxDVTimeSeriesCtrl::OnGraphScrollPageDown(wxScrollEvent& e)
{
	PanByPercent(1.0);
}
void wxDVTimeSeriesCtrl::OnDataChannelSelection(wxCommandEvent& e)
{
	int row, col;
	bool isChecked;

	m_dataSelector->GetLastEventInfo(&row, &col, &isChecked);

	if (isChecked)
		AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos(col), row);
	else
		RemoveGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos(col), row);
}
/*

void wxDVTimeSeriesCtrl::OnPlotDrag(wxCommandEvent& e)
{
	//dragDim is the physical coords of where the drag started (top left) and where it is now (bottom right).
	wxRect* dragDim = static_cast<wxRect*>(e.GetClientData());

	double xDiff = m_xAxis->PhysicalToWorld(wxPoint(dragDim->x + dragDim->width, 0))  
		- m_xAxis->PhysicalToWorld(wxPoint(dragDim->x, 0));

	double newMin = m_dragStartWorldMin - xDiff;
	double newMax = m_dragStartWorldMax - xDiff;

	if (newMin < GetMinPossibleTimeForVisibleChannels())
		newMin = GetMinPossibleTimeForVisibleChannels();
	if (newMax > GetMaxPossibleTimeForVisibleChannels())
		newMax = GetMaxPossibleTimeForVisibleChannels();

	SetViewMin(newMin);
	SetViewMax(newMax);
	//Don't call SetViewRange 'cause that snaps to days.
}

void wxDVTimeSeriesCtrl::OnPlotDragStart(wxCommandEvent& e)
{
	m_dragStartWorldMin = m_xWorldMin;
	m_dragStartWorldMax = m_xWorldMax;
}

void wxDVTimeSeriesCtrl::OnPlotDragEnd(wxCommandEvent& e)
{
	//Snap to days and update.
	SetViewRange(m_xWorldMin, m_xWorldMax);
}
*/


void wxDVTimeSeriesCtrl::AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group, bool refresh_ui)
{
	wxDVTimeSeriesPlot *p;

	//For daily and monthly time series create a dataset with the average x and y values for the day/month
	//For stepped graphs we have to start the array with the average y value for the first period at x = 0, have each avg y value at the beginning of the period (leftmost x for the period),
	//and end it with the average y value for the final period duplicated at x = max
	double sum = 0.0;
	double avg = 0.0;
	double counter = 0.0;
	double timestep = d->GetTimeStep();
	double MinHrs = d->GetMinHours();
	double MaxHrs = d->GetMaxHours();
	wxDVArrayDataSet *d2 = 0;
	bool IsDataSetEmpty = true;

	if (m_seriesType == RAW_DATA_TIME_SERIES)
	{
		IsDataSetEmpty = false;
	}
	else if (m_seriesType == HOURLY_TIME_SERIES && timestep < 1.0)
	{
		//Create hourly data set (avg value of data by day) from m_data if timestep < 1
		IsDataSetEmpty = false;
		double nextHour = 0.0;
		double currentHour = 0.0;

		d2 = new wxDVArrayDataSet(d->GetSeriesTitle(), d->GetUnits(), 1.0 / timestep);

		while (nextHour < MinHrs)
		{
			nextHour += 1.0;
		}

		currentHour = nextHour - 1.0;

		for (size_t i = 0; i < d->Length(); i++)
		{
			if (d->At(i).x >= nextHour)
			{
				if (i != 0 && counter != 0)
				{
					avg = sum / counter;
					d2->Append(wxRealPoint((double)currentHour + (double)(nextHour - currentHour) / 2.0, (m_statType == AVERAGE ? avg : sum)));
					currentHour = nextHour;
					nextHour += 1;
				}

				counter = 0.0;
				sum = 0.0;
			}

			counter += 1.0;
			sum += d->At(i).y;
		}

		avg = sum / counter;

		//Prevent appending the final point if it represents 12/31 24:00, which the system interprets as 1/1 0:00 and creates a point for January of the next year
		if (MaxHrs > 0.0 && fmod(MaxHrs, 8760.0) != 0)
		{
			d2->Append(wxRealPoint((double)currentHour + (double)(nextHour - currentHour) / 2.0, (m_statType == AVERAGE ? avg : sum)));
		}
	}
	else if (m_seriesType == DAILY_TIME_SERIES && timestep < 24.0)
	{
		//Create daily data set (avg value of data by day) from m_data if timestep < 24
		IsDataSetEmpty = false;
		double nextDay = 0.0;
		double currentDay = 0.0;

		d2 = new wxDVArrayDataSet(d->GetSeriesTitle(), d->GetUnits(), 24.0 / timestep);

		while (nextDay < MinHrs)
		{
			nextDay += 24.0;
		}

		currentDay = nextDay - 24.0;

		for (size_t i = 0; i < d->Length(); i++)
		{
			if(d->At(i).x >= nextDay)
			{
				if(i != 0 && counter != 0)
				{ 
					avg = sum / counter;
					d2->Append(wxRealPoint((double) currentDay + (double)(nextDay - currentDay) / 2.0, (m_statType == AVERAGE ? avg : sum))); 
					currentDay = nextDay;
					nextDay += 24.0;
				}

				counter = 0.0;
				sum = 0.0;
			}

			counter += 1.0;
			sum += d->At(i).y;
		}

		avg = sum / counter;

		//Prevent appending the final point if it represents 12/31 24:00, which the system interprets as 1/1 0:00 and creates a point for January of the next year
		if(MaxHrs > 0.0 && fmod(MaxHrs, 8760.0) != 0)
		{
			d2->Append(wxRealPoint((double) currentDay + (double)(nextDay - currentDay) / 2.0, (m_statType == AVERAGE ? avg : sum))); 
		}
	}
	else if (m_seriesType == MONTHLY_TIME_SERIES && timestep < 672.0)	//672 hours = 28 days = shortest possible month
	{
		//Create monthly data set (avg value of data by month) from m_data
		IsDataSetEmpty = false;
		double nextMonth = 0.0;
		double currentMonth = 0.0;
		double year = 0.0;

		d2 = new wxDVArrayDataSet(d->GetSeriesTitle(), d->GetUnits(), 744.0 / timestep);

		while(MinHrs > 8760.0)
		{
			year += 8760.0;
			MinHrs -= 8760.0;
		}

		if(MinHrs >= 0.0 && MinHrs < 744.0) { currentMonth = year; nextMonth = year + 744.0; }
		else if(MinHrs >= 744.0 && MinHrs < 1416.0) { currentMonth = year + 744.0; nextMonth = year + 1416.0; }
		else if(MinHrs >= 1416.0 && MinHrs < 2160.0) { currentMonth = year + 1416.0; nextMonth = year + 2160.0; }
		else if(MinHrs >= 2160.0 && MinHrs < 2880.0) { currentMonth = year + 2160.0; nextMonth = year + 2880.0; }
		else if(MinHrs >= 2880.0 && MinHrs < 3624.0) { currentMonth = year + 2880.0; nextMonth = year + 3624.0; }
		else if(MinHrs >= 3624.0 && MinHrs < 4344.0) { currentMonth = year + 3624.0; nextMonth = year + 4344.0; }
		else if(MinHrs >= 4344.0 && MinHrs < 5088.0) { currentMonth = year + 4344.0; nextMonth = year + 5088.0; }
		else if(MinHrs >= 5088.0 && MinHrs < 5832.0) { currentMonth = year + 5088.0; nextMonth = year + 5832.0; }
		else if(MinHrs >= 5832.0 && MinHrs < 6552.0) { currentMonth = year + 5832.0; nextMonth = year + 6552.0; }
		else if(MinHrs >= 6552.0 && MinHrs < 7296.0) { currentMonth = year + 6552.0; nextMonth = year + 7296.0; }
		else if(MinHrs >= 7296.0 && MinHrs < 8016.0) { currentMonth = year + 7296.0; nextMonth = year + 8016.0; }
		else if(MinHrs >= 8016.0 && MinHrs < 8760.0) { currentMonth = year + 8016.0; nextMonth = year + 8760.0; }

		for (size_t i = 0; i < d->Length(); i++)
		{
			if(d->At(i).x >= nextMonth)
			{
				if(i != 0 && counter != 0)
				{ 
					avg = sum / counter;

					d2->Append(wxRealPoint((double) currentMonth + (double)(nextMonth - currentMonth) / 2.0, (m_statType == AVERAGE ? avg : sum))); 

					currentMonth = nextMonth;
					if(nextMonth == 744.0 + year) { nextMonth = 1416.0 + year; }
					else if(nextMonth == 1416.0 + year) { nextMonth = 2160.0 + year; }
					else if(nextMonth == 2160.0 + year) { nextMonth = 2880.0 + year; }
					else if(nextMonth == 2880.0 + year) { nextMonth = 3624.0 + year; }
					else if(nextMonth == 3624.0 + year) { nextMonth = 4344.0 + year; }
					else if(nextMonth == 4344.0 + year) { nextMonth = 5088.0 + year; }
					else if(nextMonth == 5088.0 + year) { nextMonth = 5832.0 + year; }
					else if(nextMonth == 5832.0 + year) { nextMonth = 6552.0 + year; }
					else if(nextMonth == 6552.0 + year) { nextMonth = 7296.0 + year; }
					else if(nextMonth == 7296.0 + year) { nextMonth = 8016.0 + year; }
					else if(nextMonth == 8016.0 + year) { nextMonth = 8760.0 + year; }
					else if(nextMonth == 8760.0 + year) 
					{ 
						year += 8760.0;
						nextMonth = 744.0 + year; 
					}
				}

				counter = 0.0;
				sum = 0.0;
			}

			counter += 1.0;
			sum += d->At(i).y;
		}

		avg = sum / counter;

		//Prevent appending the final point if it represents 12/31 24:00, which the system interprets as 1/1 0:00 and creates a point for January of the next year
		if(MaxHrs > 0.0 && fmod(MaxHrs, 8760.0) != 0)
		{
			d2->Append(wxRealPoint((double) currentMonth + (double)(nextMonth - currentMonth) / 2.0, (m_statType == AVERAGE ? avg : sum))); 
		}
	}
	
	if (!IsDataSetEmpty)
	{
		if(m_seriesType == RAW_DATA_TIME_SERIES)
		{
			p = new wxDVTimeSeriesPlot(d, m_seriesType);
		}
		else
		{
			p = new wxDVTimeSeriesPlot(d2, m_seriesType, true);
		}

		p->SetStyle((wxDVTimeSeriesPlot::Style) m_lineStyle);
		m_plots.push_back(p); //Add to data sets list.
		m_dataSelector->Append( d->GetTitleWithUnits(), group );

		if ( refresh_ui )
		{
			Layout();
			RefreshDisabledCheckBoxes();
		}
	}
}

bool wxDVTimeSeriesCtrl::RemoveDataSet(wxDVTimeSeriesDataSet *d)
{
	wxDVTimeSeriesPlot *plotToRemove = NULL;
	int removedIndex = 0;
	//Find the plottable:
	for (size_t i=0; i<m_plots.size(); i++)
	{
		if (m_plots[i]->GetDataSet() == d)
		{
			removedIndex = i;
			plotToRemove = m_plots[i];
			break;
		}
	}

	if (!plotToRemove)
		return false;

	m_dataSelector->RemoveAt(removedIndex);

	for (int i=0; i<wxPLPlotCtrl::NPLOTPOS; i++)
		m_plotSurface->RemovePlot(plotToRemove);

	Invalidate();

	m_plots.erase( m_plots.begin() + removedIndex); //This is more efficient than remove when we already know the index.

	//We base our logic for showing/hiding plots, etc on indices, so when a single data set is removed
	//we have to re-index everything.
	for (size_t i=0; i<m_selectedChannelIndices.size(); i++)
	{
		for(size_t k=0; k<m_selectedChannelIndices[i]->size(); k++)
		{
			if ((*m_selectedChannelIndices[i])[k] > removedIndex)
				(*m_selectedChannelIndices[i])[k]--;
		}
	}

	m_dataSelector->Layout(); //We removed some check boxes.
	RefreshDisabledCheckBoxes();
	return true;
}

void wxDVTimeSeriesCtrl::RemoveAllDataSets( )
{
	ClearAllChannelSelections( wxPLPlotCtrl::PLOT_BOTTOM );
	ClearAllChannelSelections( wxPLPlotCtrl::PLOT_TOP );

	/*
	//Remove plottables from plot surfaces
	for (size_t i=0; i<m_selectedChannelIndices.size(); i++)
	{
		for (size_t k=0; k<m_selectedChannelIndices[i]->size(); k++)
			m_plotSurface->RemovePlot(m_plots[(*m_selectedChannelIndices[i])[k]] );

		m_selectedChannelIndices[i]->clear();
	}
	
	Invalidate();
	*/
	
	m_dataSelector->RemoveAll();

	//Remove all data sets. Deleting a data set also deletes its plottable.
	for(size_t i=0; i<m_plots.size(); i++)
		delete m_plots[i];

	m_plots.clear();
}


/*** VIEW METHODS ***/

double wxDVTimeSeriesCtrl::GetViewMin()
{
	return m_xAxis->GetWorldMin();
}

double wxDVTimeSeriesCtrl::GetViewMax()
{
	return m_xAxis->GetWorldMax();
}

void wxDVTimeSeriesCtrl::SetViewMin(double min)
{
	m_xAxis->SetWorldMin( min );
	AutoscaleYAxis();
	UpdateScrollbarPosition();
	Invalidate();
}

void wxDVTimeSeriesCtrl::SetViewMax(double max)
{
	m_xAxis->SetWorldMax( max );
	AutoscaleYAxis();
	UpdateScrollbarPosition();
	Invalidate();
}

//This setter also sets endpoints to days.
void wxDVTimeSeriesCtrl::SetViewRange(double min, double max)
{
	wxDVPlotHelper::SetRangeEndpointsToDays(&min, &max);
	m_xAxis->SetWorld(min, max);
	AutoscaleYAxis();
	UpdateScrollbarPosition();
	Invalidate();
}

void wxDVTimeSeriesCtrl::GetVisibleDataMinAndMax(double* min, double* max, const std::vector<int>& selectedChannelIndices)
{
	*min = 0;
	*max = 0; 

	double tempMin = 0;
	double tempMax = 0;
	if (m_xAxis)
	{
		double worldMin = m_xAxis->GetWorldMin();
		double worldMax = m_xAxis->GetWorldMax();

		for(size_t i=0; i<selectedChannelIndices.size(); i++)
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
		for(size_t i=0; i<selectedChannelIndices.size(); i++)
		{
			m_plots[selectedChannelIndices[i]]->GetDataSet()->GetDataMinAndMax(&tempMin, &tempMax);
			if (tempMin < *min)
				*min = tempMin;
			if (tempMax > *max)
				*max = tempMax;		
		}
	}	
}

double wxDVTimeSeriesCtrl::GetMinPossibleTimeForVisibleChannels()
{
	double min = 8760;
	for(size_t i=0; i<m_plots.size(); i++)
	{
		if (m_dataSelector->IsRowSelected(i))
		{
			if (m_plots[i]->GetDataSet()->GetMinHours() < min)
				min = m_plots[i]->GetDataSet()->GetMinHours();
		}
	}

	return min;
}

double wxDVTimeSeriesCtrl::GetMaxPossibleTimeForVisibleChannels()
{
	double max = 0;
	for(size_t i=0; i<m_plots.size(); i++)
	{
		if (m_dataSelector->IsRowSelected(i))
		{
			if (m_plots[i]->GetDataSet()->GetMaxHours() > max)
				max = m_plots[i]->GetDataSet()->GetMaxHours();
		}
	}

	return max;
}

void wxDVTimeSeriesCtrl::MakeXBoundsNice(double* xMin, double* xMax)
{
	KeepNewBoundsWithinLimits(xMin, xMax);
	KeepNewBoundsWithinLowerLimit(xMin, xMax);
	wxDVPlotHelper::SetRangeEndpointsToDays(xMin, xMax);
}

void wxDVTimeSeriesCtrl::KeepNewBoundsWithinLimits(double* newMin, double* newMax)
{
	//Make sure we don't zoom out past the data range.
	//This can only happen zooming out.
	int visDataMin = m_plots[0]->GetDataSet()->GetMinHours();
	int visDataMax = m_plots[0]->GetDataSet()->GetMaxHours();
	for (size_t i=1; i<m_plots.size(); i++)
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

void wxDVTimeSeriesCtrl::KeepNewBoundsWithinLowerLimit(double* newMin, double* newMax)
{
	//Don't zoom in too far.  
	double lowerLim = 12;

	if (*newMax - *newMin > lowerLim)
		return;

	int visDataMin = m_plots[0]->GetDataSet()->GetMinHours();
	int visDataMax = m_plots[0]->GetDataSet()->GetMaxHours();
	for (size_t i=1; i<m_plots.size(); i++)
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

bool wxDVTimeSeriesCtrl::CanZoomIn()
{
	if (!m_xAxis) return false;
	//Don't zoom in past 12 hours.
	double min, max;
	m_xAxis->GetWorld(&min,&max);
	return (max-min) > 12;
}

bool wxDVTimeSeriesCtrl::CanZoomOut()
{
	double min, max;
	m_xAxis->GetWorld(&min,&max);
	//Don't zoom out past data range.
	for (size_t i=0; i<m_selectedChannelIndices.size(); i++)
	{
		for (size_t k=0; k<m_selectedChannelIndices[i]->size(); k++)
		{
			if ( max-min < m_plots[(*m_selectedChannelIndices[i])[k]]->GetDataSet()->GetTotalHours())
				return true;
		}
	}

	return false;
}

void wxDVTimeSeriesCtrl::ZoomFactorAndUpdate(double factor, double shiftPercent)
{
	double min = m_xAxis->GetWorldMin();
	double max = m_xAxis->GetWorldMax();
	wxDVPlotHelper::ZoomFactor(&min, &max, factor, shiftPercent);
	MakeXBoundsNice(&min, &max);
	m_xAxis->SetWorld(min, max);
	AutoscaleYAxis();
	UpdateScrollbarPosition();
	Invalidate();
}

void wxDVTimeSeriesCtrl::ZoomToFit()
{
	double min = GetMinPossibleTimeForVisibleChannels();
	double max = GetMaxPossibleTimeForVisibleChannels();
	if ( max <= min || (min==0&&max==0) )
	{
		min = 0;
		max = 8760;
	}

	SetViewRange(min, max);
}

void wxDVTimeSeriesCtrl::PanByPercent(double p)
{
	//If p is 0.5, for example, the center becomes the new left bound.

	if (!m_xAxis) return;

	double min, max;
	m_xAxis->GetWorld(&min,&max);

	double newMin = min + (max-min) * p;
	double newMax = max + (max-min) * p;

	double highestTimeValue = GetMaxPossibleTimeForVisibleChannels();
	if (newMax > highestTimeValue)
	{
		newMin -= newMax - highestTimeValue;
		newMax = highestTimeValue;
	}

	double timeMin = GetMinPossibleTimeForVisibleChannels();
	if (newMin < timeMin)
	{
		newMax += timeMin - newMin;
		newMin = timeMin;
	}

	SetViewRange(newMin, newMax);
}

void wxDVTimeSeriesCtrl::UpdateScrollbarPosition()
{
	if(m_plots.size() == 0)
	{
		//Don't Call m_plotSurface1->GetXAxis1()->WorldLength() because we don't have a graph yet.
		m_graphScrollBar->SetScrollbar(0, 1, 1, 1, true);
		return;
	}

	else
	{
		//All units are hours.
		int thumbSize = m_xAxis->GetWorldLength();
		int pageSize = thumbSize;
		int range = 0;

		double timeMin = GetMinPossibleTimeForVisibleChannels();
		range = GetMaxPossibleTimeForVisibleChannels() - timeMin;
		int position = m_xAxis->GetWorldMin() - timeMin;
		
		m_graphScrollBar->SetScrollbar(position, thumbSize, range, pageSize, true);
	}
}

void wxDVTimeSeriesCtrl::AutoscaleYAxis(bool forceUpdate)
{
	//It is probably best to avoid this function and use the more specific version where possible.
	if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP))
		AutoscaleYAxis(m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP), *m_selectedChannelIndices[TOP_LEFT_AXIS], forceUpdate);
	if (m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_TOP))
		AutoscaleYAxis(m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_TOP), 
		m_selectedChannelIndices[TOP_RIGHT_AXIS]->size() > 0 ? *m_selectedChannelIndices[TOP_RIGHT_AXIS] : *m_selectedChannelIndices[TOP_LEFT_AXIS], 
		forceUpdate);
	if (m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM))
		AutoscaleYAxis(m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM), *m_selectedChannelIndices[BOTTOM_LEFT_AXIS], forceUpdate);
	if (m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_BOTTOM))
		AutoscaleYAxis(m_plotSurface->GetYAxis2(wxPLPlotCtrl::PLOT_BOTTOM), 
		m_selectedChannelIndices[BOTTOM_RIGHT_AXIS]->size() > 0 ? *m_selectedChannelIndices[BOTTOM_RIGHT_AXIS] : *m_selectedChannelIndices[BOTTOM_LEFT_AXIS], 
		forceUpdate);	
}

void wxDVTimeSeriesCtrl::AutoscaleYAxis( wxPLAxis *axisToScale, const std::vector<int>& selectedChannelIndices, bool forceUpdate)
{
	//If autoscaling is off don't scale y1 axis
	// But do scale y2 axis (since we don't allow manual scaling there for UI simplicity).
	if (!m_topAutoScale && axisToScale == m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_TOP)) { return; }
	if (!m_bottomAutoScale && axisToScale == m_plotSurface->GetYAxis1(wxPLPlotCtrl::PLOT_BOTTOM)) { return; }

	//Force Update is used to rescale even if data is still in acceptable range
	bool needsRescale = false;
	double dataMax; 
	double dataMin; 
	double timestep = 1.0;

	GetVisibleDataMinAndMax(&dataMin, &dataMax, selectedChannelIndices);

	//If the maximum of the visible data is outside the acceptable range
	if(forceUpdate || (dataMax > 0 && (dataMax >= axisToScale->GetWorldMax() || dataMax < axisToScale->GetWorldMax()/2.0)))
	{	
		needsRescale = true;
	}
	if(forceUpdate || (dataMin < 0 && (dataMin <= axisToScale->GetWorldMin() || dataMin > axisToScale->GetWorldMin()/2.0)))
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

void wxDVTimeSeriesCtrl::AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index)
{
	if (index < 0 || index >= (int)m_plots.size()) return;

	size_t idx = (size_t)index;

	//Set our line colour correctly.  Assigned by data selection window.
	m_plots[idx]->SetColour(m_dataSelector->GetColourForIndex(index) );
	
	wxPLPlotCtrl::AxisPos yap = wxPLPlotCtrl::Y_LEFT;
	wxString units = m_plots[idx]->GetDataSet()->GetUnits();

		
	wxString y1Units = NO_UNITS, y2Units = NO_UNITS;

	if ( m_plotSurface->GetYAxis1( pPos ) )
		y1Units = m_plotSurface->GetYAxis1( pPos )->GetLabel();

	if ( m_plotSurface->GetYAxis2( pPos ) )
		y2Units = m_plotSurface->GetYAxis2( pPos )->GetLabel();

	if ( m_plotSurface->GetYAxis1( pPos ) && y1Units == units )
		yap = wxPLPlotCtrl::Y_LEFT;
	else if ( m_plotSurface->GetYAxis2( pPos ) && y2Units == units )
		yap = wxPLPlotCtrl::Y_RIGHT;
	else if ( m_plotSurface->GetYAxis1( pPos ) == 0 )
		yap = wxPLPlotCtrl::Y_LEFT;
	else
		yap = wxPLPlotCtrl::Y_RIGHT;
	
	m_plotSurface->AddPlot( m_plots[index], wxPLPlotCtrl::X_BOTTOM, yap, pPos, false );
	m_plotSurface->GetAxis( yap, pPos )->SetLabel( units );
		
	//Calculate index from 0-3.  0,1 are top graph L,R axis.  2,3 are L,R axis on bottom graph.
	int graphIndex = TOP_LEFT_AXIS;
	if (pPos == wxPLPlotCtrl::PLOT_BOTTOM)
		graphIndex += 2;
	if (yap == wxPLPlotCtrl::Y_RIGHT)
		graphIndex += 1;

	m_selectedChannelIndices[graphIndex]->push_back(idx);
	
	switch(yap)
	{
	case wxPLPlotCtrl::Y_LEFT:
		AutoscaleYAxis(m_plotSurface->GetYAxis1(pPos), *m_selectedChannelIndices[graphIndex], true);
		break;
	case wxPLPlotCtrl::Y_RIGHT:
		AutoscaleYAxis(m_plotSurface->GetYAxis2(pPos), *m_selectedChannelIndices[graphIndex], true);
		break;
	}

	UpdateScrollbarPosition();
	RefreshDisabledCheckBoxes();
	Invalidate();
}
void wxDVTimeSeriesCtrl::RemoveGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index)
{
	//Find our GraphAxisPosition, and remove from selected indices list.
	//Have to do it this way because of ambiguous use of int in an array storing ints.
	int graphIndex = 0;
	if (pPos == wxPLPlotCtrl::PLOT_BOTTOM)
		graphIndex += 2;
	for (int i = graphIndex; i < graphIndex+2; i++)
	{
		std::vector<int>::iterator it = std::find( m_selectedChannelIndices[i]->begin(), m_selectedChannelIndices[i]->end(), index );
		if ( it != m_selectedChannelIndices[i]->end())
		{
			m_selectedChannelIndices[i]->erase( it );
			graphIndex = i;
			break;
		}
	}

	bool keepAxis = false;
	if (m_selectedChannelIndices[graphIndex]->size() > 0)
		keepAxis = true;

	m_plotSurface->RemovePlot(m_plots[index], pPos);

	//See if axis is still in use or not, and to some cleanup.
	wxPLLinearAxis *axisThatWasUsed;
	if (graphIndex % 2 == 0)
		axisThatWasUsed = dynamic_cast<wxPLLinearAxis*>( m_plotSurface->GetYAxis1(pPos) );
	else
		axisThatWasUsed = dynamic_cast<wxPLLinearAxis*>( m_plotSurface->GetYAxis2(pPos) );


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
		case BOTTOM_LEFT_AXIS:
			otherAxisIndex = BOTTOM_RIGHT_AXIS;
			break;
		case BOTTOM_RIGHT_AXIS:
			otherAxisIndex = BOTTOM_LEFT_AXIS;
			break;
		}

		if (m_selectedChannelIndices[otherAxisIndex]->size() == 0)
		{
			m_plotSurface->SetYAxis1(NULL, pPos);
			m_plotSurface->SetYAxis2(NULL, pPos);
		}
		else
		{
			//If we only have one Y axis, we must use the left y axis.
			//Code in wpplotsurface2D uses this assumption.
			if (axisThatWasUsed == m_plotSurface->GetYAxis1(pPos))
			{
				std::vector<int> *temp = m_selectedChannelIndices[graphIndex];
				m_selectedChannelIndices[graphIndex] = m_selectedChannelIndices[otherAxisIndex];
				m_selectedChannelIndices[otherAxisIndex] = temp;

				//Set the y axis to the left side (instead of the right)
				for (size_t i=0; i<m_selectedChannelIndices[graphIndex]->size(); i++)
				{
					m_plotSurface->RemovePlot(m_plots[(*m_selectedChannelIndices[graphIndex])[i]]);
					m_plotSurface->AddPlot( m_plots[(*m_selectedChannelIndices[graphIndex])[i]], 
						wxPLPlotCtrl::X_BOTTOM, wxPLPlotCtrl::Y_LEFT, pPos);
				}

				m_plotSurface->GetYAxis1(pPos)->SetLabel( m_plots[(*m_selectedChannelIndices[graphIndex])[0]]->GetDataSet()->GetUnits() );
				AutoscaleYAxis(m_plotSurface->GetYAxis1(pPos), *m_selectedChannelIndices[graphIndex], true);
			}
			m_plotSurface->SetYAxis2(NULL, pPos);
		}
	}
	else
	{
		AutoscaleYAxis(axisThatWasUsed, *m_selectedChannelIndices[graphIndex], true); //Pass true in case we removed a tall graph from a shorter one.
	}

	RefreshDisabledCheckBoxes();
	Invalidate();
}

void wxDVTimeSeriesCtrl::ClearAllChannelSelections(wxPLPlotCtrl::PlotPos pPos)
{
	m_dataSelector->ClearColumn(int(pPos));

	int graphIndex = 0;
	if (pPos == wxPLPlotCtrl::PLOT_BOTTOM)
		graphIndex += 2;
	for (int i=graphIndex; i<graphIndex+2; i++)
	{
		for (size_t j=0; j<m_selectedChannelIndices[i]->size(); j++)
		{
			m_plotSurface->RemovePlot(m_plots[m_selectedChannelIndices[i]->at(j)] ); 
		}
		m_selectedChannelIndices[i]->clear();
	}

	m_plotSurface->SetYAxis1(NULL, pPos);
	m_plotSurface->SetYAxis2(NULL, pPos);

	RefreshDisabledCheckBoxes(pPos);
	Invalidate();
}

void wxDVTimeSeriesCtrl::RefreshDisabledCheckBoxes()
{
	RefreshDisabledCheckBoxes(wxPLPlotCtrl::PLOT_TOP);
	RefreshDisabledCheckBoxes(wxPLPlotCtrl::PLOT_BOTTOM);
}
void wxDVTimeSeriesCtrl::RefreshDisabledCheckBoxes(wxPLPlotCtrl::PlotPos pPos)
{
	wxString axis1Label = NO_UNITS;
	wxString axis2Label = NO_UNITS;
	
	if(m_plotSurface->GetYAxis1(pPos))
		axis1Label = m_plotSurface->GetYAxis1(pPos)->GetLabel();
	if (m_plotSurface->GetYAxis2(pPos))
		axis2Label = m_plotSurface->GetYAxis2(pPos)->GetLabel();

	if (axis1Label != NO_UNITS
		&& axis2Label != NO_UNITS
		&& axis1Label != axis2Label)
	{
		for (int i=0; i<m_dataSelector->Length(); i++)
		{
			m_dataSelector->Enable(i, pPos, axis1Label == m_plots[i]->GetDataSet()->GetUnits() 
				|| axis2Label == m_plots[i]->GetDataSet()->GetUnits());
		}
	}
	else
	{
		for (int i=0; i<m_dataSelector->Length(); i++)
		{
			m_dataSelector->Enable(i, pPos, true);
		}
	}
}

wxDVSelectionListCtrl* wxDVTimeSeriesCtrl::GetDataSelectionList()
{
	return m_dataSelector;
}

void wxDVTimeSeriesCtrl::SetTopSelectedNames(const wxString& names)
{
	SetSelectedNamesForColIndex(names, 0);
}

void wxDVTimeSeriesCtrl::SetBottomSelectedNames(const wxString& names)
{
	SetSelectedNamesForColIndex(names, 1);
}

void wxDVTimeSeriesCtrl::SetSelectedNamesForColIndex(const wxString& names, int col)
{
	ClearAllChannelSelections(wxPLPlotCtrl::PlotPos(col));

	wxStringTokenizer tkz(names, ";");

	while(tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		int row = m_dataSelector->SelectRowWithNameInCol(token, col);
		if (row != -1)
			AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos(col), row);
	}
}

void wxDVTimeSeriesCtrl::SelectDataSetAtIndex(int index)
{
	m_dataSelector->SelectRowInCol(index, 0);
	AddGraphAfterChannelSelection(wxPLPlotCtrl::PLOT_TOP, index);
}

bool wxDVTimeSeriesCtrl::GetSyncWithHeatMap()
{
	return m_syncToHeatMap;
}

void wxDVTimeSeriesCtrl::SetSyncWithHeatMap(bool b)
{
	m_syncToHeatMap = b;
}

StatType wxDVTimeSeriesCtrl::GetStatType()
{
	return m_statType;
}

void wxDVTimeSeriesCtrl::SetStatType(StatType statType)
{
	m_statType = statType;
}

