
#include "dview/dvcoarserainbowcolourmap.h"


wxDVCoarseRainbowColourMap::wxDVCoarseRainbowColourMap(double min, double max)
	: wxDVColourMap(min, max)
{
	mColourList.push_back(wxColour(0, 0, 0));
	mColourList.push_back(wxColour(46, 44, 213));
	mColourList.push_back(wxColour(0, 108, 255));
	mColourList.push_back(wxColour(0, 245, 245));
	mColourList.push_back(wxColour(23, 210, 135));
	mColourList.push_back(wxColour(104, 186, 44));
	mColourList.push_back(wxColour(226, 241, 6));
	mColourList.push_back(wxColour(255, 213, 0));
	mColourList.push_back(wxColour(255, 159, 0));
	mColourList.push_back(wxColour(255, 0, 0));
}


wxDVCoarseRainbowColourMap::~wxDVCoarseRainbowColourMap()
{

}

wxString wxDVCoarseRainbowColourMap::GetName()
{
	return wxT("Coarse Rainbow");
}

wxColour wxDVCoarseRainbowColourMap::ColourForValue(double val)
{
	if (val <= mMinVal)
		return mColourList[0];
	if (val >= mMaxVal)
		return mColourList[mColourList.size()-1];

	int position = mColourList.size() * (val - mMinVal) / (mMaxVal - mMinVal);

	return mColourList[position];
}

wxSize wxDVCoarseRainbowColourMap::DrawIn(wxDC& dc, const wxRect& geom)
{
	wxCoord colourBarHeight = 220;
	if (geom.height < 240)
	{
		colourBarHeight = 100; //Probably not ideal.  Fix this.
	}

	dc.SetFont(*wxNORMAL_FONT);
	wxCoord charHeight = dc.GetCharHeight();


	double range = mMaxVal - mMinVal;
	double step = range / 10;
	wxCoord maxWidth = 0, temp;
	wxString* labels [11];
	for (int i=0; i<11; i++)
	{
		labels[i] = new wxString();
		*labels[i] = wxString::Format("%g", mMinVal + i*step);
		dc.GetTextExtent(*labels[i], &temp, NULL);
		if (temp > maxWidth)
			maxWidth = temp;
	}

	wxCoord xTextPos = geom.x + geom.width - maxWidth;

	double yTextStep = colourBarHeight / 10;

	for (int i=0; i<11; i++)
	{
		dc.DrawText(*labels[i], xTextPos, wxCoord((10-i)*yTextStep));
		delete labels[i];
	}

	wxCoord colourBarX = xTextPos - 2 - 12;
	double colourBarStep = colourBarHeight / 10;

	dc.SetPen(*wxTRANSPARENT_PEN);
	for (int i=0; i<10; i++)
	{
		dc.SetBrush(wxBrush(mColourList[i]));
		dc.DrawRectangle(colourBarX+1, charHeight/2 + 1 + (9-i)*colourBarStep, 10, colourBarStep+1);
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawRectangle(colourBarX, charHeight/2, 12, colourBarHeight+2);

	wxCoord totalWidth = maxWidth + 14 + 2;
	wxCoord totalHeight = colourBarHeight + charHeight;

	return wxSize(totalWidth, totalHeight);
}