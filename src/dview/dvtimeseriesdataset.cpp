
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


wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const std::vector<double> &data )
	: m_varLabel(var), m_pData(data)
{
	m_timestep=1; //hours
	m_offset=0;
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const wxString &units, const double &timestep, const std::vector<double> &data )
	: m_varLabel(var), m_varUnits(units), m_timestep(timestep), m_pData(data)
{
	m_offset=0;
}

wxDVArrayDataSet::wxDVArrayDataSet( const wxString &var, const wxString &units, const double &offset, const double &timestep, const std::vector<double> &data )
	: m_varLabel(var), m_varUnits(units), m_offset(offset), 
	m_timestep(timestep), m_pData(data)
{
}


wxRealPoint wxDVArrayDataSet::At(size_t i) const
{
	if ((i<m_pData.size())&&(i>=0))
		return wxRealPoint(m_offset+i*m_timestep, m_pData[i] );
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

void wxDVArrayDataSet::SetDataValue(size_t i, double newYValue)
{
	m_pData[i] = newYValue;
}

// ******** Point array data set *********** //

wxDVPointArrayDataSet::wxDVPointArrayDataSet( const wxString &var, const wxString &units, const double &timestep )
	: mSeriesTitle(var), m_varUnits(units), m_timestep(timestep)
{
	
}

wxRealPoint wxDVPointArrayDataSet::At(size_t i) const
{
	return mDataPoints[i];
}

size_t wxDVPointArrayDataSet::Length() const
{
	return mDataPoints.size();
}

double wxDVPointArrayDataSet::GetTimeStep() const
{
	return m_timestep;
}

wxString wxDVPointArrayDataSet::GetSeriesTitle() const
{
	return mSeriesTitle;
}

wxString wxDVPointArrayDataSet::GetUnits() const 
{
	return m_varUnits;
}

void wxDVPointArrayDataSet::SetDataValue(size_t i, double newYValue)
{
	mDataPoints[i].y = newYValue;
}

void wxDVPointArrayDataSet::Append(const wxRealPoint& p)
{
	mDataPoints.push_back(p);
}

void wxDVPointArrayDataSet::Alloc(int size)
{
	mDataPoints.reserve(size);
}
