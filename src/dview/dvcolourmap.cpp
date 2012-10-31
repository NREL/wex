#include <wx/dcmemory.h>

#include "dview/dvcolourmap.h"

wxDVColourMap::wxDVColourMap(double min, double max)
{
	m_min = min;
	m_max = max;
}

wxDVColourMap::~wxDVColourMap()
{
}


void wxDVColourMap::SetScaleMinMax(double min, double max)
{
	m_min = min;
	m_max = max;
	InvalidateBestSize();
}

void wxDVColourMap::SetScaleMin(double min)
{
	m_min = min;
	InvalidateBestSize();
}

void wxDVColourMap::SetScaleMax(double max)
{
	m_max = max;
	InvalidateBestSize();
}

double wxDVColourMap::GetScaleMin()
{
	return m_min;
}

double wxDVColourMap::GetScaleMax()
{
	return m_max;
}

void wxDVColourMap::ExtendScaleToNiceNumbers()
{
	ExtendToNiceInPosDir(&m_max, true);
	ExtendToNiceInPosDir(&m_min, false);
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

wxSize wxDVColourMap::CalculateBestSize()
{
	wxBitmap bit(100, 100);
	wxMemoryDC dc(bit);
	dc.SetFont( *wxNORMAL_FONT );
	
	double range = m_max - m_min;
	double step = range / 10;
	wxCoord maxWidth = 0, temp;
	for (int i=0; i<11; i++)
	{
		dc.GetTextExtent( wxString::Format("%lg", m_min + i*step), &temp, NULL);
		if (temp > maxWidth)
			maxWidth = temp;
	}

	return wxSize( 17+maxWidth, 300 );
}

void wxDVColourMap::Render(wxDC &dc, const wxRect &geom)
{
	wxCoord colourBarHeight = 240;
	if (geom.height < 240)
		colourBarHeight = 120; //Probably not ideal.  Fix this.
	
	
	wxFont font( *wxNORMAL_FONT );
	font.SetPointSize( font.GetPointSize() - 1 );
	dc.SetFont( font );
	wxCoord charHeight = dc.GetCharHeight();
	
	wxCoord colourBarX = geom.x+1;
	double colourBarStep = colourBarHeight / ((double)m_colourList.size());
	dc.SetPen(*wxTRANSPARENT_PEN);
	for (size_t i=0; i<m_colourList.size(); i++)
	{
		dc.SetBrush(wxBrush(m_colourList[i]));
		dc.DrawRectangle(colourBarX+1, geom.y+charHeight/2 + 1 + (m_colourList.size()-1-i)*colourBarStep, 10, colourBarStep+1);
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawRectangle(colourBarX, geom.y+charHeight/2, 12, colourBarHeight+2);
	
	wxCoord xTextPos = colourBarX + 14;
	double yTextStep = colourBarHeight / 10;

	double range = m_max - m_min;
	double step = range / 10;
	for (size_t i=0; i<11; i++)
		dc.DrawText( wxString::Format("%lg", m_min + i*step), xTextPos, geom.y+wxCoord((10-i)*yTextStep) );	
}

wxColour wxDVColourMap::ColourForValue(double val)
{
	if (val <= m_min) return m_colourList[0];
	if (val >= m_max) return m_colourList[m_colourList.size()-1];

	int position = m_colourList.size() * (val - m_min) / (m_max - m_min);
	return m_colourList[position];
}



wxDVCoarseRainbowColourMap::wxDVCoarseRainbowColourMap( double min, double max )
	: wxDVColourMap( min, max )
{
	m_colourList.push_back(wxColour(0, 0, 0));
	m_colourList.push_back(wxColour(46, 44, 213));
	m_colourList.push_back(wxColour(0, 108, 255));
	m_colourList.push_back(wxColour(0, 245, 245));
	m_colourList.push_back(wxColour(23, 210, 135));
	m_colourList.push_back(wxColour(104, 186, 44));
	m_colourList.push_back(wxColour(226, 241, 6));
	m_colourList.push_back(wxColour(255, 213, 0));
	m_colourList.push_back(wxColour(255, 159, 0));
	m_colourList.push_back(wxColour(255, 0, 0));
}

wxString wxDVCoarseRainbowColourMap::GetName()
{
	return _("Coarse Rainbow");
}

wxDVFineRainbowColourMap::wxDVFineRainbowColourMap( double min, double max )
	: wxDVColourMap( min, max )
{
	//Color Transition resolution: number of colors to use per transition.
	//In most cases we use this number.
	//In some cases we gorw or shrink the transition lenght to get a better selection of colours.
	//For instance, orange is between red and yellow, and the green transition is too long (loss of contrast).
	int res = 20;

	// Black to Violet
	for (int i=0; i<res; i++)
		m_colourList.push_back(wxColour(i * 255 / res, 0, i * 255 / res));
	
	// Violet to Dark Blue
	for (int i=res-1; i>res/2; i--)
		m_colourList.push_back(wxColour(i * 255 / (res/2), 0, 255));
	
	// Dark Blue to Light Blue
	for (int i=0; i<=res/2-1; i++)
		m_colourList.push_back(wxColour(0, 255 * i / (res/2), 255));
	
	//Light Blue to Green
	for (int i=res/2-1; i>0; i--)
		m_colourList.push_back(wxColour(0, 255, i * 255 / (res/2)));
	
	//Green to Yellow
	for (int i=0; i<res/2; i++)
		m_colourList.push_back(wxColour(i * 255 / (res/2), 255, 0));

	//Yellow to Orange to Red
	for (int i=2*res; i>=0; i--)
		m_colourList.push_back(wxColour(255, i * 255 / (2*res), 0));
}

wxString wxDVFineRainbowColourMap::GetName()
{
	return _("Fine Rainbow");
}

wxDVGrayscaleColourMap::wxDVGrayscaleColourMap( double min, double max )
	: wxDVColourMap( min, max )
{
	const size_t ncolours = 50;
	for ( size_t i=0;i<ncolours;i++ )
	{
		int grey = (int)(((double)i)/ncolours*255.0);
		m_colourList.push_back( wxColour( grey, grey, grey ) );
	}
}

wxString wxDVGrayscaleColourMap::GetName()
{
	return _("Grayscale");
}
