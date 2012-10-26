
#include "dview/dvcolourmap.h"


wxDVColourMap::wxDVColourMap(double min, double max)
{
	mMinVal = min;
	mMaxVal = max;
}


wxDVColourMap::~wxDVColourMap()
{
}


void wxDVColourMap::SetScaleMinMax(double min, double max)
{
	mMinVal = min;
	mMaxVal = max;
}

void wxDVColourMap::SetScaleMin(double min)
{
	mMinVal = min;
}

void wxDVColourMap::SetScaleMax(double max)
{
	mMaxVal = max;
}

double wxDVColourMap::GetScaleMin()
{
	return mMinVal;
}

double wxDVColourMap::GetScaleMax()
{
	return mMaxVal;
}

void wxDVColourMap::ExtendScaleToNiceNumbers()
{
	ExtendToNiceInPosDir(&mMaxVal, true);
	ExtendToNiceInPosDir(&mMinVal, false);
}

void wxDVColourMap::ExtendToNiceInPosDir(double* d, bool posDir)
{
	//If dir is true move in positive direction.  Otherwise negative.
	bool neg = false;
	if (*d < 0)
	{
		neg = true;
		*d = -*d; //d must be > 0 for log10.
	}
	if (*d == 0)
		return; //Already a nice number.

	int exp = floor(log10(*d));
	double ratio = *d / pow(double(10), exp);

	double niceMultiplier;
	if ((!neg && posDir) || (neg && !posDir))
	{
		if (ratio <= 1)
			niceMultiplier = 1;
		else if (ratio <= 2)
			niceMultiplier = 2;
		else if (ratio <= 5)
			niceMultiplier = 5;
		else if (ratio <= 8)
			niceMultiplier = 8;
		else
			niceMultiplier = 10;
	}
	else //negative number.  must go closer to 0.
	{
		if (ratio >= 8)
			niceMultiplier = 8;
		else if (ratio >= 5)
			niceMultiplier = 5;
		else if (ratio >= 2)
			niceMultiplier = 2;
		else if (ratio >= 1)
			niceMultiplier = 1;
		else 
		{
			exp --;
			niceMultiplier = 8;
		}
	}

	*d = niceMultiplier * pow(double(10), exp); //return this.

	if (neg)
		*d = -*d;
}
