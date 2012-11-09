
#include "dview/dvtimeseriesdataset.h"


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
	size_t endIndex = size_t((endHour - At(0).x)/GetTimeStep() + 1);

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