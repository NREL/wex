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
	cp.index = index;
	cp.colour = *wxRED;

	if (mAvailableColours.size() > 0)
	{
		cp.colour = mAvailableColours[0];
		mAvailableColours.erase( mAvailableColours.begin() );
	}
	
	mAssignedColours.push_back(cp);
	
	return cp.colour;
}

void wxDVAutoColourAssigner::ResetColourList()
{
	mAvailableColours.clear();
	mAvailableColours.push_back("red");
	mAvailableColours.push_back("forest green");
	mAvailableColours.push_back("blue");
	mAvailableColours.push_back("purple");
	mAvailableColours.push_back("goldenrod");
	mAvailableColours.push_back("salmon");
	mAvailableColours.push_back("magenta");
	mAvailableColours.push_back("grey");
	mAvailableColours.push_back("aquamarine");
	mAvailableColours.push_back("brown");
}

void wxDVAutoColourAssigner::DeAssignAll()
{
	mAssignedColours.clear();
	ResetColourList();
}

void wxDVAutoColourAssigner::DeAssignLineColour(int index)
{
	for (int i=0; i<mAssignedColours.size(); i++)
	{
		if (mAssignedColours[i].index == index)
		{
			mAvailableColours.push_back(mAssignedColours[i].colour);
			mAssignedColours.erase( mAssignedColours.begin() + i );
			return;
		}
	}
}