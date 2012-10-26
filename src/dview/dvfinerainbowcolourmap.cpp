
#include "dview/dvfinerainbowcolourmap.h"

wxDVFineRainbowColourMap::wxDVFineRainbowColourMap(double min, double max)
	: wxDVColourMap(min, max)
{
	//Color Transition resolution: number of colors to use per transition.
	//In most cases we use this number.
	//In some cases we gorw or shrink the transition lenght to get a better selection of colours.
	//For instance, orange is between red and yellow, and the green transition is too long (loss of contrast).
	int res = 20;

	// Black to Violet
	for (int i=0; i<res; i++)
		mColourList.push_back(wxColour(i * 255 / res, 0, i * 255 / res));
	
	// Violet to Dark Blue
	for (int i=res-1; i>res/2; i--)
		mColourList.push_back(wxColour(i * 255 / (res/2), 0, 255));
	
	// Dark Blue to Light Blue
	for (int i=0; i<=res/2-1; i++)
		mColourList.push_back(wxColour(0, 255 * i / (res/2), 255));
	
	//Light Blue to Green
	for (int i=res/2-1; i>0; i--)
		mColourList.push_back(wxColour(0, 255, i * 255 / (res/2)));
	
	//Green to Yellow
	for (int i=0; i<res/2; i++)
		mColourList.push_back(wxColour(i * 255 / (res/2), 255, 0));

	//Yellow to Orange to Red
	for (int i=2*res; i>=0; i--)
		mColourList.push_back(wxColour(255, i * 255 / (2*res), 0));
}

wxDVFineRainbowColourMap::~wxDVFineRainbowColourMap()
{

}

wxString wxDVFineRainbowColourMap::GetName()
{
	return wxT("Fine Rainbow");
}

wxColour wxDVFineRainbowColourMap::ColourForValue(double val)
{
	if (val <= mMinVal)
		return mColourList[0];
	if (val >= mMaxVal)
		return mColourList[mColourList.size()-1];

	int position = mColourList.size() * (val - mMinVal) / (mMaxVal - mMinVal);

	return mColourList[position];
}

wxSize wxDVFineRainbowColourMap::DrawIn(wxDC& dc, const wxRect& geom)
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
	double colourBarStep = double(colourBarHeight) / double(mColourList.size());

	dc.SetPen(*wxTRANSPARENT_PEN);
	for (int i=0; i<mColourList.size(); i++)
	{
		dc.SetBrush(wxBrush(mColourList[i]));
		dc.DrawRectangle(colourBarX+1, charHeight/2 + 1 + (mColourList.size()-1-i)*colourBarStep, 10, colourBarStep+1);
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawRectangle(colourBarX, charHeight/2, 12, colourBarHeight+2);

	wxCoord totalWidth = maxWidth + 14 + 2;
	wxCoord totalHeight = colourBarHeight + charHeight;

	return wxSize(totalWidth, totalHeight);
}
