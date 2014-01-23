#include "wex/dview/dvautocolourassigner.h"


wxDVAutoColourAssigner::wxDVAutoColourAssigner()
{
	ResetColourList();
}

wxDVAutoColourAssigner::~wxDVAutoColourAssigner()
{
}

wxColour wxDVAutoColourAssigner::GetColourForIndex(int index)
{
	for (int i=0; i<mAssignedColours.size(); i++)
	{
		if (mAssignedColours[i].index == index)
			return mAssignedColours[i].colour;
	}

	return *wxRED;
}

bool wxDVAutoColourAssigner::IsColourAssigned( int index )
{
	for (int i=0; i<mAssignedColours.size(); i++)
		if (mAssignedColours[i].index == index)
			return true;

	return false;
}

wxColour wxDVAutoColourAssigner::AssignLineColour(int index)
{
	ColourPair cp;
	wxColour tempColor = *wxRED;
	int tempUseCount = 1000000000;	//Start with an impossibly high number of uses.  We'll reduce it until we find the lowest use count.
	int colorIndex;

	for(int i = 0; i < mAvailableColours.size(); i++)
	{
		if(tempUseCount > mAvailableColours[i].useCount)
		{
			colorIndex = i;
			tempUseCount = mAvailableColours[i].useCount;
			tempColor = mAvailableColours[i].colour;
			if(tempUseCount == 0) { break; }
		}
	}

	mAvailableColours[colorIndex].useCount++;

	cp.index = index;
	cp.colour = tempColor;
	
	mAssignedColours.push_back(cp);
	
	return cp.colour;
}

void wxDVAutoColourAssigner::ResetColourList()
{
	ColourCounter cc;

	mAvailableColours.clear();

	cc.colour = "red";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "forest green";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "blue";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "purple";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	//cc.colour = "goldenrod";
	//cc.useCount = 0;
	//mAvailableColours.push_back(cc);

	cc.colour = "salmon";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "magenta";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "grey";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "aquamarine";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);

	cc.colour = "brown";
	cc.useCount = 0;
	mAvailableColours.push_back(cc);
}

void wxDVAutoColourAssigner::DeAssignAll()
{
	mAssignedColours.clear();
	ResetColourList();
}

void wxDVAutoColourAssigner::DeAssignLineColour(int index)
{
	for (int i = 0; i < mAssignedColours.size(); i++)
	{
		if (mAssignedColours[i].index == index)
		{
			for(int j = 0; j < mAvailableColours.size(); j++)
			{
				if(mAvailableColours[j].colour == mAssignedColours[i].colour) 
				{
					mAssignedColours.erase( mAssignedColours.begin() + i );
					mAvailableColours[j].useCount--;
					if(mAvailableColours[j].useCount < 0) { mAvailableColours[j].useCount = 0; }

					return;
				}
			}
		}
	}
}