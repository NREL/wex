/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

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
	for (size_t i = 0; i < mAssignedColours.size(); i++)
	{
		if (mAssignedColours[i].index == index)
			return mAssignedColours[i].colour;
	}

	return *wxRED;
}

bool wxDVAutoColourAssigner::IsColourAssigned(int index)
{
	for (size_t i = 0; i < mAssignedColours.size(); i++)
		if (mAssignedColours[i].index == index)
			return true;

	return false;
}

wxColour wxDVAutoColourAssigner::AssignLineColour(int index)
{
	ColourPair cp;
	wxColour tempColor = *wxRED;
	int tempUseCount = 1000000000;	//Start with an impossibly high number of uses.  We'll reduce it until we find the lowest use count.
	int colorIndex = 0;

	for (size_t i = 0; i < mAvailableColours.size(); i++)
	{
		if (tempUseCount > mAvailableColours[i].useCount)
		{
			colorIndex = i;
			tempUseCount = mAvailableColours[i].useCount;
			tempColor = mAvailableColours[i].colour;
			if (tempUseCount == 0) { break; }
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

	/*
	mAvailableColours.push_back( ColourCounter("red") );
	mAvailableColours.push_back( ColourCounter("forest green") );
	mAvailableColours.push_back( ColourCounter("blue") );
	mAvailableColours.push_back( ColourCounter("purple") );
	mAvailableColours.push_back( ColourCounter("salmon") );
	mAvailableColours.push_back( ColourCounter("magenta") );
	mAvailableColours.push_back( ColourCounter("grey") );
	mAvailableColours.push_back( ColourCounter("aquamarine") );
	mAvailableColours.push_back( ColourCounter("brown") );
	*/

	mAvailableColours.push_back(ColourCounter(wxColour(0, 114, 198)));
	mAvailableColours.push_back(ColourCounter(wxColour(255, 150, 64)));
	mAvailableColours.push_back(ColourCounter("FIREBRICK"));
	mAvailableColours.push_back(ColourCounter("DARK SLATE GREY"));
	mAvailableColours.push_back(ColourCounter("PALE GREEN"));
	mAvailableColours.push_back(ColourCounter("MEDIUM VIOLET RED"));
	mAvailableColours.push_back(ColourCounter("GOLDENROD"));
	mAvailableColours.push_back(ColourCounter("dark orchid"));
}

void wxDVAutoColourAssigner::DeAssignAll()
{
	mAssignedColours.clear();
	ResetColourList();
}

void wxDVAutoColourAssigner::DeAssignLineColour(int index)
{
	for (size_t i = 0; i < mAssignedColours.size(); i++)
	{
		if (mAssignedColours[i].index == index)
		{
			for (size_t j = 0; j < mAvailableColours.size(); j++)
			{
				if (mAvailableColours[j].colour == mAssignedColours[i].colour)
				{
					mAssignedColours.erase(mAssignedColours.begin() + i);
					mAvailableColours[j].useCount--;
					if (mAvailableColours[j].useCount < 0) { mAvailableColours[j].useCount = 0; }

					return;
				}
			}
		}
	}
}