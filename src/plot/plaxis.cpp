#include <math.h>
#include <cmath>
#include <numeric>
#include <wx/datetime.h>

#include "wex/plot/plaxis.h"

#ifdef __WXOSX__
#define my_isnan(x) std::isnan(x)
#else
#define my_isnan(x) wxIsNaN(x)
#endif

wxPLAxis::wxPLAxis()
{
	Init();
}

wxPLAxis::wxPLAxis( double min, double max, const wxString &label )
{
	Init();
	m_min = min;
	m_max = max;
	m_label = label;
}

wxPLAxis::wxPLAxis( const wxPLAxis &rhs )
{
	m_min = rhs.m_min;
	m_max = rhs.m_max;
	m_label = rhs.m_label;
	m_colour = rhs.m_colour;
	m_showLabel = rhs.m_showLabel;
	m_showTickText = rhs.m_showTickText;
	m_smallTickSize = rhs.m_smallTickSize;
	m_largeTickSize = rhs.m_largeTickSize;
}

void wxPLAxis::Init()
{
	m_min = m_max = std::numeric_limits<double>::quiet_NaN();
	m_showLabel = m_showTickText = true;
	m_smallTickSize = 2;
	m_largeTickSize = 5;
	m_colour = *wxBLACK;
}

wxPLAxis::~wxPLAxis()
{
	/* nothing to do here */
}

wxCoord wxPLAxis::WorldToPhysical( double coord, wxCoord phys_min, wxCoord phys_max )
{
	wxCoord pmin = phys_min;
	wxCoord pmax = phys_max;

	double range = m_max - m_min;
	if (range == 0)
		return 1;

	double prop = (double)((coord - m_min) / range);
	// calculate the physical coordinate.
	return pmin + prop * (pmax - pmin);
}

double wxPLAxis::PhysicalToWorld( wxCoord point, wxCoord phys_min, wxCoord phys_max )
{
	wxCoord pmin = phys_min;
	wxCoord pmax = phys_max;

	double len = pmax - pmin;
	double prop = (point-pmin) / len;
	return prop * (m_max - m_min) + m_min;
}

void wxPLAxis::ExtendBound( wxPLAxis *a )
{
	if ( !a ) return;

	// mins
	if ( !my_isnan(a->m_min) )
	{
		if (my_isnan(m_min))
		{
			m_min = a->m_min;
		}
		else
		{
			if (a->m_min < m_min)
			{
				m_min = a->m_min;
			}
		}
	}

	// maxs.
	if ( !my_isnan(a->m_max) )
	{
		if (my_isnan(m_max))
		{
			m_max = a->m_max;
		}
		else
		{
			if (a->m_max > m_max)
			{
				m_max = a->m_max;
			}
		}
	}	
}

wxPLLinearAxis::wxPLLinearAxis( double min, double max, const wxString &label )
	: wxPLAxis( min, max, label )
{
	m_offset = 0.0;
	m_scale = 1.0;
	m_approxNumberLargeTicks = 3.0;
	m_mantissas.push_back( 1.0 );
	m_mantissas.push_back( 2.0 );
	m_mantissas.push_back( 5.0 );
	m_smallTickCounts.push_back( 4 );
	m_smallTickCounts.push_back( 1 );
	m_smallTickCounts.push_back( 4 );
}

wxPLLinearAxis::wxPLLinearAxis( const wxPLLinearAxis &rhs )
	: wxPLAxis( rhs )
{
	m_offset = rhs.m_offset;
	m_scale = rhs.m_scale;
	m_approxNumberLargeTicks = rhs.m_approxNumberLargeTicks;
	m_mantissas = rhs.m_mantissas;
	m_smallTickCounts = rhs.m_smallTickCounts;
}

wxPLAxis *wxPLLinearAxis::Duplicate()
{
	return new wxPLLinearAxis( *this );
}

void wxPLLinearAxis::GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list )
{
	std::vector<double> largeticks, smallticks;
	CalcTicksFirstPass( phys_min, phys_max, largeticks, smallticks );
	CalcTicksSecondPass( phys_min, phys_max, largeticks, smallticks );

	for ( size_t i=0; i<largeticks.size(); i++ )
	{
		// TODO: Find out why zero is sometimes significantly not zero [seen as high as 10^-16].
		double labelNumber = largeticks[i];
		if ( fabs(labelNumber) < 0.000000000000001 )
			labelNumber = 0.0;
		list.push_back( TickData( labelNumber, wxString::Format("%lg", labelNumber), TickData::LARGE ) );
	}

	for ( size_t i=0; i<smallticks.size(); i++ )
		list.push_back( TickData( smallticks[i], wxEmptyString, TickData::SMALL ) );
}


void wxPLLinearAxis::CalcTicksFirstPass(wxCoord phys_min, wxCoord phys_max, 
		std::vector<double> &largeticks, std::vector<double> &smallticks)
{
	if ( m_min == m_max )
		return;
	
	double adjustedMax = AdjustedWorldValue( m_max );
	double adjustedMin = AdjustedWorldValue( m_min );

	// (2) determine distance between large ticks.
	bool should_cull_middle;
	double tickDist = DetermineLargeTickStep( fabs((double)phys_min-(double)phys_max), should_cull_middle );

	// (3) determine starting position.
	double first = 0.0;
	if( adjustedMin > 0.0 )
	{
		double nToFirst = floor(adjustedMin / tickDist) + 1.0f;
		first = nToFirst * tickDist;
	}
	else
	{
		double nToFirst = floor(-adjustedMin/tickDist) - 1.0f;
		first = -nToFirst * tickDist;
	}

	// could miss one, if first is just below zero.
	if ((first - tickDist) >= adjustedMin)
		first -= tickDist;
	
	// (4) now make list of large tick positions.	
	if (tickDist < 0.0) // some sanity checking. TODO: remove this.
		return;

	double position = first;
	int safetyCount = 0;
	while ( (position <= adjustedMax)
		&& (++safetyCount < 5000) )
	{
		largeticks.push_back( position );
		position += tickDist;
	}

	// (5) if the physical extent is too small, and the middle 
	// ticks should be turned into small ticks, then do this now.
	smallticks.clear();
	if (should_cull_middle)
	{
		if (largeticks.size() > 2)
		{
			for (size_t i=1; i<largeticks.size()-1; ++i)
			{
				smallticks.push_back( largeticks[i] );
			}
		}

		std::vector<double> culledPositions;
		culledPositions.push_back( largeticks[0] );
		culledPositions.push_back( largeticks[largeticks.size()-1] );
		largeticks = culledPositions;
	}
}

void wxPLLinearAxis::CalcTicksSecondPass(wxCoord phys_min, wxCoord phys_max, 
		std::vector<double> &largeticks, std::vector<double> &smallticks)
{
		//return if already generated.
	if (smallticks.size() > 0)
		return;

	double physicalAxisLength = fabs( (double)phys_min - (double)phys_max );

	double adjustedMax = AdjustedWorldValue( m_max );
	double adjustedMin = AdjustedWorldValue( m_min );

	smallticks.clear();

	// TODO: Can optimize this now.
	bool should_cull_middle;
	double bigTickSpacing = DetermineLargeTickStep( physicalAxisLength, should_cull_middle );

	size_t nSmall = DetermineNumberSmallTicks( bigTickSpacing );
	double smallTickSpacing = bigTickSpacing / (double)nSmall;

	// if there is at least one big tick
	if (largeticks.size() > 0)
	{
		double pos1 = (double)largeticks[0] - smallTickSpacing;
		while (pos1 > adjustedMin)
		{
			smallticks.push_back( pos1 );
			pos1 -= smallTickSpacing;
		}
	}

	for (size_t i = 0; i < largeticks.size(); ++i )
	{
		for (size_t j = 1; j < nSmall; ++j )
		{
			double pos = (double)largeticks[i] + ((double)j) * smallTickSpacing;
			if (pos <= adjustedMax)
			{
				smallticks.push_back( pos );
			}
		}
	}
}
	
double wxPLLinearAxis::AdjustedWorldValue( double world )
{
	return world * m_scale + m_offset;
}

double wxPLLinearAxis::DetermineLargeTickStep( double physical_len, bool &should_cull_middle)
{
	should_cull_middle = false;

	if ( m_min == m_max )
		return 1.0;

	// adjust world max and min for offset and scale properties of axis.
	double adjustedMax = AdjustedWorldValue( m_max );
	double adjustedMin = AdjustedWorldValue( m_min );
	double range = adjustedMax - adjustedMin;

	// if axis has zero world length, then return arbitrary number.
	if ( adjustedMax == adjustedMin )
		return 1.0;

	int minPhysLargeTickStep = 40;
	double approxTickStep = (minPhysLargeTickStep / physical_len) * range;
	double exponent = floor( log10( approxTickStep ) );
	double mantissa = pow( 10.0, log10( approxTickStep ) - exponent );

	// determine next whole mantissa below the approx one.
	size_t mantissaIndex = m_mantissas.size()-1;
	for (size_t i=1; i<m_mantissas.size(); ++i)
	{
		if (mantissa < m_mantissas[i])
		{
			mantissaIndex = i-1;
			break;
		}
	}
	
	// then choose next largest spacing. 
	mantissaIndex += 1;
	if (mantissaIndex == m_mantissas.size())
	{
		mantissaIndex = 0;
		exponent += 1.0;
	}

	// now make sure that the returned value is such that at least two 
	// large tick marks will be displayed.
	double tickStep = pow( 10.0, exponent ) * m_mantissas[mantissaIndex];
	float physicalStep = (float)((tickStep / range) * physical_len);

	while (physicalStep > physical_len/2)
	{
		should_cull_middle = true;

		mantissaIndex -= 1;
		if (mantissaIndex == -1)
		{
			mantissaIndex = m_mantissas.size()-1;
			exponent -= 1.0;
		}

		tickStep = pow( 10.0, exponent ) * m_mantissas[mantissaIndex];
		physicalStep = (float)((tickStep / range) * physical_len);
	}

	// and we're done.
	return pow( 10.0, exponent ) * m_mantissas[mantissaIndex];
}

size_t wxPLLinearAxis::DetermineNumberSmallTicks( double big_tick_dist )
{
	if (m_smallTickCounts.size() != m_mantissas.size())
		return 0;

	if (big_tick_dist > 0.0)
	{
		double exponent = floor( log10( big_tick_dist ) );
		double mantissa = pow( 10.0, log10( big_tick_dist ) - exponent );

		for (size_t i=0; i<m_mantissas.size(); ++i)
			if ( fabs(mantissa-m_mantissas[i]) < 0.001 )
				return m_smallTickCounts[i]+1;
	}

	return 0;
}


wxPLLabelAxis::wxPLLabelAxis( double min, double max, const wxString &label )
	: wxPLAxis( min, max, label )
{
	/* nothing to do */
}

wxPLLabelAxis::wxPLLabelAxis( const wxPLLabelAxis &rhs )
	: wxPLAxis( rhs )
{
	m_tickLabels = rhs.m_tickLabels;
}

wxPLAxis *wxPLLabelAxis::Duplicate()
{
	return new wxPLLabelAxis( *this );
}

void wxPLLabelAxis::GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list )
{
	double min_shown = PhysicalToWorld( phys_min, phys_min, phys_max );
	double max_shown = PhysicalToWorld( phys_max, phys_min, phys_max );

	for ( size_t i=0;i<m_tickLabels.size(); i++)
		if ( m_tickLabels[i].world >= min_shown 
			&& m_tickLabels[i].world <= max_shown )
			list.push_back( m_tickLabels[i] );
}

void wxPLLabelAxis::Add( double world, const wxString &text )
{
	m_tickLabels.push_back( TickData( world, text, TickData::LARGE ) );
}

void wxPLLabelAxis::Clear()
{
	m_tickLabels.clear();
}


wxPLLogAxis::wxPLLogAxis( double min, double max, const wxString &label )
	: wxPLAxis( min, max, label )
{
	/* nothing to configure here */
}

wxPLLogAxis::wxPLLogAxis( const wxPLLogAxis &rhs )
	: wxPLAxis( rhs )
{
	/* nothing to configure here */
}

wxPLAxis *wxPLLogAxis::Duplicate()
{
	return new wxPLLogAxis( *this );
}

void wxPLLogAxis::GetAxisTicks( wxCoord, wxCoord, std::vector<TickData> &list )
{
	std::vector<double> largeticks, smallticks;
	CalcTicksFirstPass( largeticks, smallticks );
	CalcTicksSecondPass( largeticks, smallticks );

	for ( size_t i=0; i<largeticks.size(); i++ )
	{
		// TODO: Find out why zero is sometimes significantly not zero [seen as high as 10^-16].
		double labelNumber = largeticks[i];
		if ( fabs(labelNumber) < 0.000000000000001 )
			labelNumber = 0.0;
		list.push_back( TickData( labelNumber, wxString::Format("%lg", labelNumber), TickData::LARGE ) );
	}

	for ( size_t i=0; i<smallticks.size(); i++ )
		list.push_back( TickData( smallticks[i], wxEmptyString, TickData::SMALL ) );
}

wxCoord wxPLLogAxis::WorldToPhysical( double coord, wxCoord phys_min, wxCoord phys_max )
{
	if (coord <= 0.0 || m_min <= 0.0) return 0; // error

	// inside range or don't want to clip.
	double lrange = (double)(log10(m_max) - log10(m_min));
	double prop = (double)((log10(coord) - log10(m_min)) / lrange);
	double offset = prop * (phys_max - phys_min);
	return phys_min + offset;
}

double wxPLLogAxis::PhysicalToWorld( wxCoord p, wxCoord phys_min, wxCoord phys_max )
{
	double t = wxPLAxis::PhysicalToWorld( p, phys_min, phys_max );

	// now reconstruct phys dist prop along this assuming linear scale as base method did.
	double v = (t - m_min) / (m_max - m_min);
	return m_min*pow( m_max / m_min, v );
}

void wxPLLogAxis::CalcTicksFirstPass( std::vector<double> &largeticks, std::vector<double> &smallticks) 
{
	smallticks.clear();
	largeticks.clear();

	double roundTickDist = DetermineTickSpacing( );

	// now determine first tick position.
	double first = 0.0f;

	// if the user hasn't specified a large tick position.
	if( m_min > 0.0 )
	{

		double nToFirst = floor(log10(m_min) / roundTickDist)+1.0f;
		first = nToFirst * roundTickDist;
	}

	// could miss one, if first is just below zero.
	if (first-roundTickDist >= log10(m_min))
	{
		first -= roundTickDist;
	}

	double mark = first;
	while (mark <= log10(m_max))
	{
		// up to here only logs are dealt with, but I want to return
		// a real value in the arraylist
		double val;
		val = pow( 10.0, mark );
		largeticks.push_back( val );
		mark += roundTickDist;
	}
}

void wxPLLogAxis::CalcTicksSecondPass( std::vector<double> &largeticks, std::vector<double> &smallticks )
{
	smallticks.clear();

	// retrieve the spacing of the big ticks. Remember this is decades!
	double bigTickSpacing = DetermineTickSpacing();
	int nSmall = DetermineNumberSmallTicks( bigTickSpacing );

	// now we have to set the ticks
	// let us start with the easy case where the major tick distance
	// is larger than a decade
	if ( bigTickSpacing > 1.0f )
	{
		if (largeticks.size() > 0)
		{
			// deal with the smallticks preceding the
			// first big tick
			double pos1 = (double)largeticks[0];
			while (pos1 > m_min)
			{
				pos1 = pos1 / 10.0f;
				smallticks.push_back( pos1 );
			}
			// now go on for all other Major ticks
			for ( size_t i=0; i<largeticks.size(); ++i )
			{
				double pos = (double)largeticks[i];
				for (int j=1; j<=nSmall; ++j )
				{
					pos=pos*10.0F;
					// check to see if we are still in the range
					if (pos < m_max)
					{
						smallticks.push_back( pos );
					}
				}
			}
		}
	}
	else
	{
		// guess what...
		double m[8] = { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
		// Then we deal with the other ticks
		if (largeticks.size() > 0)
		{
			// first deal with the smallticks preceding the first big tick
			// positioning before the first tick
			double pos1=(double)largeticks[0]/10.0f;
			for (int i=0; i<8; i++)
			{
				double pos=pos1*m[i];
				if (pos>m_min)
				{
					smallticks.push_back(pos);
				}
			}
			// now go on for all other Major ticks
			for ( size_t i=0; i<largeticks.size(); ++i )
			{
				pos1=(double)largeticks[i];
				for (int j=0; j<8; ++j )
				{
					double pos=pos1*m[j];
					// check to see if we are still in the range
					if (pos < m_max)
					{
						smallticks.push_back( pos );
					}
				}
			}
		}
		else
		{
			// probably a minor tick would anyway fall in the range
			// find the decade preceding the minimum
			double dec=floor(log10(m_min));
			double pos1=pow(10.0,dec);
			for (int i=0; i<8; i++)
			{
				double pos=pos1*m[i];
				if (pos>m_min && pos< m_max )
				{
					smallticks.push_back(pos);
				}
			}
		}
	}
}
	
double wxPLLogAxis::DetermineTickSpacing()
{
	double mag_range = (double)( floor(log10(m_max)) - floor(log10(m_min))+1.0 );

	if ( mag_range > 0.0 )
	{
		// for now, a simple logic
		// start with a major tick every order of magnitude, and
		// increment if in order not to have more than 10 ticks in
		// the plot.
		double round_tick_dist=1.0;
		int nticks=(int)(mag_range/round_tick_dist);
		while (nticks > 10)
		{
			round_tick_dist++;
			nticks=(int)(mag_range/round_tick_dist);
		}
		return round_tick_dist;
	}
	else
	{
		return 0.0f;
	}
}

size_t wxPLLogAxis::DetermineNumberSmallTicks( double big_tick_dist )
{	
	if (big_tick_dist == 1.0f)
	{
		return 8;
	}
	else
		return (int)big_tick_dist - 1;
}



wxPLTimeAxis::wxPLTimeAxis( double min, double max, const wxString &label )
	: wxPLAxis( min, max, label )
{
	m_lastMin = m_lastMax = 0.0;
}

wxPLTimeAxis::wxPLTimeAxis( const wxPLTimeAxis &rhs )
	: wxPLAxis( rhs )
{
	m_lastMin = m_lastMax = 0.0;
}

wxPLAxis *wxPLTimeAxis::Duplicate()
{
	return new wxPLTimeAxis( *this );
}

wxString wxPLTimeAxis::GetLabel()
{
	RecalculateTicksAndLabel(); // only does it if necessary
	return m_timeLabel;
}

void wxPLTimeAxis::GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list )
{
	RecalculateTicksAndLabel(); // only does it if necessary
	list = m_tickList;
}
	
void wxPLTimeAxis::RecalculateTicksAndLabel()
{
	if ( m_lastMin == m_min
		&& m_lastMax == m_max ) return; // no need to recalculate, same time range is shown

	m_timeLabel.clear();
	m_tickList.clear();

	//We need to figure out whether we are looking at hours, days, or months, and label the graph appropriately.
	wxDateTime timeKeeper( 1, wxDateTime::Jan, 1970, 0, 0, 0 );

	double world_len = m_max-m_min;
	double time = m_min;
	timeKeeper.Add( wxTimeSpan::Minutes(60 * time) );

	// Handle DST.
	if (timeKeeper.IsDST())
		timeKeeper.Subtract(wxTimeSpan::Hour());


	if( world_len <= 72 )
	{
		if ( floor(time) != time )
		{
			timeKeeper.Add( wxTimeSpan::Minutes(60 * (floor(time) + 2 - time)) );
			time = floor(time) + 1;
		}

		if ( world_len > 6 )
		{
			//Label every 3rd hour.
			while(fmod(time, 3) != 0)
			{
				time += 1;
				timeKeeper.Add( wxTimeSpan::Hour() );
			}
		}

		// Less than 2 days.  Need to label time.
		
		wxDateTime timeKeeper2( timeKeeper.GetTicks() );
		timeKeeper2.Add(wxTimeSpan::Minutes(60 * world_len));
		timeKeeper2.Subtract(wxTimeSpan::Minute()); //If it is 0:00 the next day, its really the same day.
		if (timeKeeper.IsDST() && !timeKeeper2.IsDST())
		{
			timeKeeper.Add(wxTimeSpan::Hour());
			timeKeeper2.Add(wxTimeSpan::Hour());
		}
	
		if(timeKeeper.GetDay() == timeKeeper2.GetDay())
			m_timeLabel = timeKeeper.Format("%b %d");
		else
			m_timeLabel = timeKeeper.Format("%b %d") + "-" + timeKeeper2.Format("%d");
		
		do
		{
			m_tickList.push_back( TickData( time, timeKeeper.Format("%H"), TickData::LARGE ) );

			if ( world_len > 9)
			{
				time += 3.0f;
				timeKeeper.Add(wxTimeSpan::Hours(3));
			}
			else
			{
				time += 1.0f;
				timeKeeper.Add(wxTimeSpan::Hours(1));
			}
		}
		while( time <= m_max );

		if (timeKeeper.IsDST())
			timeKeeper.Add( wxTimeSpan::Hour() );
	}
	else if( world_len < 30 * 24) // less than 30 days
	{	
		//About a month visible.  Just label every day.
		wxDateTime timeKeeper2 = wxDateTime(timeKeeper.GetTicks());

		while(timeKeeper2.GetHour() % 12 != 0)
			timeKeeper2.Add(wxTimeSpan::Hour());

		timeKeeper2.SetMinute(0);

		time += (timeKeeper2 - timeKeeper).GetMinutes() / 60.0f;
		timeKeeper.Add(timeKeeper2 - timeKeeper);

		//Place tick if we are on day start.
		if (timeKeeper.GetHour() == 0)
		{
			m_tickList.push_back( TickData( time, wxEmptyString, TickData::LARGE ) );
			time += 12;
			timeKeeper.Add(wxTimeSpan::Hours(12));
		}
		
		do
		{
			m_tickList.push_back( TickData( time, timeKeeper.Format("%b %d"), TickData::NONE ) );
			time += 12;
			if ( time < m_max )
				m_tickList.push_back( TickData( time, wxEmptyString, TickData::LARGE ) ); // midnight 
			time += 12;
			timeKeeper.Add(wxTimeSpan::Hours(24));
		}
		while( time < m_max );
	}
	else
	{
		//Assume day endpoints.
		//Only Label Months
		//m_timeLabel = "Month";

		wxDateTime timeKeeper2(timeKeeper.GetTicks());
		timeKeeper2.SetHour(0);
		timeKeeper2.SetMinute(0);
		timeKeeper2.Add(wxTimeSpan::Day());
		double daysVisibleInMonth = (timeKeeper2 - timeKeeper).GetMinutes() / 60.0f / 24.0f;
		while(timeKeeper2.GetMonth() == timeKeeper.GetMonth())
		{
			daysVisibleInMonth += 1;
			timeKeeper2.Add(wxTimeSpan::Day());
		}

		m_tickList.push_back( TickData(time + daysVisibleInMonth * 24.0f, wxEmptyString, TickData::LARGE) );
		int endMonthHours = time + daysVisibleInMonth * 24.0f; //00:00 on first of next month.

		if (daysVisibleInMonth >= 7)
		{
			//Label the month if it has more than seven days visible.
			m_tickList.push_back( TickData( time + (daysVisibleInMonth * 24.0f / 2.0f), timeKeeper.Format("%b"), TickData::NONE ) );
		}

		timeKeeper2.SetDay(wxDateTime::GetNumberOfDays(timeKeeper2.GetMonth()) / 2); //Middle of the second month.
		time += timeKeeper2.Subtract(timeKeeper).GetHours();
		endMonthHours += 24 * wxDateTime::GetNumberOfDays(timeKeeper2.GetMonth());

		//Loop, labeling months
		//While end of month is visible.
		do
		{
			m_tickList.push_back( TickData( time, timeKeeper2.Format("%b"), TickData::NONE ) );
			timeKeeper.Set(timeKeeper2.GetTicks()); //timeKeeper is position of last label.
			timeKeeper2.Add(wxDateSpan::Month()); 
			timeKeeper2.SetDay(1); //timeKeeper2 is day 1 of next month.

			m_tickList.push_back( TickData( time + timeKeeper2.Subtract(timeKeeper).GetHours(), wxEmptyString, TickData::LARGE ) ); //Adds tick at this pos. (start-month)
			timeKeeper2.SetDay(wxDateTime::GetNumberOfDays(timeKeeper2.GetMonth()) / 2); //timeKeeper2 mid month
			time += timeKeeper2.Subtract(timeKeeper).GetHours(); //hours midMonth.
			endMonthHours += 24 * wxDateTime::GetNumberOfDays(timeKeeper2.GetMonth());
		}
		while (endMonthHours < m_max);

		//timeKeeper holds the middle of last month we actually labelled.
		//We still need to label the last month if it has more than 7 days showing.
		timeKeeper.Add(wxDateSpan::Month()); 
		timeKeeper.SetDay(1); // First not-yet-labeled month
		time = endMonthHours - 24 * wxDateTime::GetNumberOfDays(timeKeeper.GetMonth()); //00:00 on first of not-yet-labeled month.

		//Take care of fractional days at the max.
		timeKeeper2 = wxDateTime(01, wxDateTime::Jan, 1970, 00, 00, 00);
		timeKeeper2.Add(wxTimeSpan::Minutes(m_max * 60.0f));
		daysVisibleInMonth = timeKeeper2.GetHour() / 24.0f;
		daysVisibleInMonth += timeKeeper2.GetMinute() / 60.0f / 24.0f;
		timeKeeper2.Set(timeKeeper.GetTicks());

		time += daysVisibleInMonth * 24.0f;
		timeKeeper2.Add(wxTimeSpan::Minutes(daysVisibleInMonth * 24.0f * 60.0f));
		while(time < m_max && timeKeeper2.GetMonth() == timeKeeper.GetMonth())
		{
			daysVisibleInMonth += 1;
			timeKeeper2.Add(wxTimeSpan::Day());
			time += 24;
		}

		if (daysVisibleInMonth >= 7)
		{
			//Label the month if it has more than seven days visible.
			m_tickList.push_back( TickData( time - (daysVisibleInMonth * 24.0f / 2.0f), timeKeeper.Format("%b"), TickData::NONE ) );
		}
	}

	// save last calculation min/max to avoid recalculating axis labels & ticks
	// if world range hasn't changed.
	m_lastMin = m_min;
	m_lastMax = m_max;
}
