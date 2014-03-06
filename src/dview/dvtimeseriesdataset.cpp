
#include "wex/dview/dvtimeseriesdataset.h"


wxDVTimeSeriesDataSet::wxDVTimeSeriesDataSet()
{
	
}

wxDVTimeSeriesDataSet::~wxDVTimeSeriesDataSet()
{

}

/* Helper Functions */
wxString wxDVTimeSeriesDataSet::GetTitleWithUnits()
{
	return GetSeriesTitle() + " (" + GetUnits() + ")";
}

wxRealPoint wxDVTimeSeriesDataSet::operator[] (size_t i) const
{
	return At(i);
}

double wxDVTimeSeriesDataSet::GetMinHours()
{
	return At(0).x;
}
double wxDVTimeSeriesDataSet::GetMaxHours()
{
	return At(Length()-1).x;
}

double wxDVTimeSeriesDataSet::GetTotalHours()
{
	return GetMaxHours() - GetMinHours();
}

void wxDVTimeSeriesDataSet::GetMinAndMaxInRange(double* min, double* max, 
												  size_t startIndex, size_t endIndex)
{
	if(endIndex > Length())
		endIndex = Length();
	if(startIndex < 0)
		startIndex = 0;

	double myMin = At(startIndex).y;
	double myMax = At(startIndex).y;

	for(size_t i=startIndex+1; i<endIndex; i++)
	{
		if (At(i).y < myMin)
			myMin = At(i).y;
		if (At(i).y > myMax)
			myMax = At(i).y;
	}

	if (min)
		*min = myMin;
	if (max)
		*max = myMax;
}

void wxDVTimeSeriesDataSet::GetMinAndMaxInRange(double* min, double* max, double startHour, double endHour)
{
	if (startHour < At(0).x)
		startHour = At(0).x;
	if (endHour < At(0).x)
		endHour = At(0).x;
	size_t startIndex = size_t((startHour - At(0).x)/GetTimeStep());
	size_t endIndex = size_t((endHour - At(0).x)/GetTimeStep() + 2);

	if (startIndex < 0)
		startIndex = 0;
	if (startIndex > Length())
		return;
	if (endIndex > Length())
		endIndex = Length();

	GetMinAndMaxInRange(min, max, startIndex, endIndex);
}

void wxDVTimeSeriesDataSet::GetDataMinAndMax(double* min, double* max)
{
	GetMinAndMaxInRange(min, max, size_t(0), Length());
}

std::vector<wxRealPoint> wxDVTimeSeriesDataSet::GetDataVector()
{
	std::vector<wxRealPoint> pp;
	pp.reserve( Length() );
	for (size_t i=0;i<Length();i++)
		pp.push_back( At(i) );
	return pp;
}



// ******** Array data set *********** //


wxDVArrayDataSet::wxDVArrayDataSet()
	: m_timestep(1), m_offset(0)
{
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const std::vector<double> &data )
	: m_varLabel(var), m_timestep(1), m_offset(0)
{
	Copy( data );	
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const std::vector<wxRealPoint> &data )
	: m_varLabel(var), m_pData(data), m_timestep(1), m_offset(0)
{
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const wxString &units, const double &timestep )
	: m_varLabel(var), m_varUnits(units), m_timestep(timestep), m_offset(0)
{
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const wxString &units, const double &timestep, const std::vector<double> &data )
	: m_varLabel(var), m_varUnits(units), m_timestep(timestep), m_offset(0)
{
	Copy( data );
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const wxString &units, const double &offset, const double &timestep, const std::vector<double> &data )
	: m_varLabel(var), m_varUnits(units), m_timestep(timestep), m_offset(offset)
{
	Copy( data );
}


wxRealPoint wxDVArrayDataSet::At(size_t i) const
{
	if ((i<m_pData.size())&&(i>=0))
		return wxRealPoint( m_pData[i].x, m_pData[i].y );
	else
		return wxRealPoint(m_offset+i*m_timestep, 0.0 );
}

size_t wxDVArrayDataSet::Length() const
{
	return m_pData.size();
}

double wxDVArrayDataSet::GetTimeStep() const
{
	return m_timestep;
}

wxString wxDVArrayDataSet::GetSeriesTitle() const
{
	return m_varLabel;
}

wxString wxDVArrayDataSet::GetUnits() const
{
	return m_varUnits;
}

void wxDVArrayDataSet::Clear()
{
	m_pData.clear();
}

void wxDVArrayDataSet::Copy( const std::vector<double> &data )
{
	m_pData.clear();
	if ( data.size() > 0 )
	{
		m_pData.resize( data.size() );
		for( size_t i=0;i<data.size();i++ )
			m_pData[i] = wxRealPoint( m_offset+i*m_timestep, data[i] );
	}
}

void wxDVArrayDataSet::Alloc( size_t n )
{
	m_pData.reserve( n );
}

void wxDVArrayDataSet::Append( const wxRealPoint &p )
{
	m_pData.push_back( p );
}

void wxDVArrayDataSet::Set( size_t i, double x, double y )
{
	if ( i < m_pData.size() )
		m_pData[i] = wxRealPoint( x, y );
}

void wxDVArrayDataSet::SetY( size_t i, double y )
{
	if ( i < m_pData.size() )
		m_pData[i].y = y;
}

void wxDVArrayDataSet::SetSeriesTitle( const wxString &title )
{
	m_varLabel = title;
}

void wxDVArrayDataSet::SetUnits( const wxString &units )
{
	m_varUnits = units;
}

void wxDVArrayDataSet::SetTimeStep( double ts, bool recompute_x )
{
	m_timestep = ts;
	if ( recompute_x ) RecomputeXData();
}

void wxDVArrayDataSet::SetOffset( double off, bool recompute_x )
{
	m_offset = off;
	if ( recompute_x ) RecomputeXData();
}

void wxDVArrayDataSet::RecomputeXData()
{
	for( size_t i=0;i<m_pData.size();i++ )
		m_pData[i].x = m_offset+i*m_timestep;
}