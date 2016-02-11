#include <wx/dcmemory.h>
#include "wex/plot/plaxis.h"
#include "wex/plot/plcolourmap.h"

wxPLColourMap::wxPLColourMap(double min, double max)
{
	m_min = min;
	m_max = max;
	m_format = "%lg";
}

wxPLColourMap::wxPLColourMap( const wxPLColourMap &cpy )
{
	Copy( cpy );
}

wxPLColourMap::~wxPLColourMap()
{
	// nothing to do
}

wxPLColourMap &wxPLColourMap::operator=( const wxPLColourMap &cpy )
{
	if ( this != &cpy )
		Copy( cpy );

	return *this;
}

void wxPLColourMap::Copy( const wxPLColourMap &cpy )
{
	m_min = cpy.m_min;
	m_max = cpy.m_max;
	m_format = cpy.m_format;
	m_colourList = cpy.m_colourList;
}


void wxPLColourMap::SetScaleMinMax(double min, double max)
{
	m_min = min;
	m_max = max;
	InvalidateBestSize();
}

void wxPLColourMap::SetScaleMin(double min)
{
	m_min = min;
	InvalidateBestSize();
}

void wxPLColourMap::SetScaleMax(double max)
{
	m_max = max;
	InvalidateBestSize();
}

double wxPLColourMap::GetScaleMin()
{
	return m_min;
}

double wxPLColourMap::GetScaleMax()
{
	return m_max;
}

void wxPLColourMap::ExtendScaleToNiceNumbers()
{
	wxPLAxis::ExtendBoundsToNiceNumber(&m_max, &m_min);
}

wxSize wxPLColourMap::CalculateBestSize()
{
	wxBitmap bit(100, 100);
	wxMemoryDC dc(bit);
	dc.SetFont( *wxNORMAL_FONT );
	
	double range = m_max - m_min;
	double step = range / 10;
	wxCoord maxWidth = 0, temp;
	for (int i=0; i<11; i++)
	{
		dc.GetTextExtent( wxString::Format( wxFormatString(m_format), m_min + i*step), &temp, NULL);
		if (temp > maxWidth)
			maxWidth = temp;
	}

	return wxSize( 17+maxWidth, 300 );
}

void wxPLColourMap::Render( wxPLOutputDevice &dc, const wxRect &geom)
{
	double colourBarHeight = 240;
	if (geom.height < 240)
		colourBarHeight = 120; //Probably not ideal.  Fix this.
	
	
	dc.Font( -1, false );
	double charHeight = dc.CharHeight();
	
	double colourBarX = geom.x+1;
	double colourBarStep = colourBarHeight / ((double)m_colourList.size());
	for (size_t i=0; i<m_colourList.size(); i++)
	{
		dc.Pen( m_colourList[i] );
		dc.Brush( m_colourList[i] );
		dc.Rect(colourBarX+1, geom.y+charHeight/2 + 1 + (m_colourList.size()-1-i)*colourBarStep, 10, colourBarStep+1);
	}
		
	double xTextPos = colourBarX + 14;
	double yTextStep = colourBarHeight / 10;

	double range = m_max - m_min;
	double step = range / 10;
	for (size_t i=0; i<11; i++)
		dc.Text( wxString::Format( wxFormatString(m_format), m_min + i*step), xTextPos, geom.y+wxCoord((10-i)*yTextStep) );	
}

wxColour wxPLColourMap::ColourForValue(double val)
{
	if ( m_colourList.size() == 0 ) return *wxBLACK;
	if ( !(wxFinite(val)) || val <= m_min) return m_colourList[0];
	if (val >= m_max) return m_colourList[m_colourList.size()-1];

	size_t position = (size_t)( ((double)m_colourList.size()) * (val - m_min) / (m_max - m_min) );
	if ( position >= 0 && position < m_colourList.size() )
		return m_colourList[position];
	else
		return m_colourList[0];
}



wxPLCoarseRainbowColourMap::wxPLCoarseRainbowColourMap( double min, double max )
	: wxPLColourMap( min, max )
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

wxString wxPLCoarseRainbowColourMap::GetName()
{
	return _("Coarse Rainbow");
}

wxPLFineRainbowColourMap::wxPLFineRainbowColourMap( double min, double max )
	: wxPLColourMap( min, max )
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

wxString wxPLFineRainbowColourMap::GetName()
{
	return _("Fine Rainbow");
}

wxPLJetColourMap::wxPLJetColourMap( double min, double max )
	: wxPLColourMap( min, max )
{
 static double jet [] = {
	     0,         0,  132.6000,
         0,         0,  142.8000,
         0,         0,  153.0000,
         0,         0,  163.2000,
         0,         0,  173.4000,
         0,         0,  183.6000,
         0,         0,  193.8000,
         0,         0,  204.0000,
         0,         0,  214.2000,
         0,         0,  224.4000,
         0,         0,  234.6000,
         0,         0,  244.8000,
         0,         0,  255.0000,
         0,   10.2000,  255.0000,
         0,   20.4000,  255.0000,
         0,   30.6000,  255.0000,
         0,   40.8000,  255.0000,
         0,   51.0000,  255.0000,
         0,   61.2000,  255.0000,
         0,   71.4000,  255.0000,
         0,   81.6000,  255.0000,
         0,   91.8000,  255.0000,
         0,  102.0000,  255.0000,
         0,  112.2000,  255.0000,
         0,  122.4000,  255.0000,
         0,  132.6000,  255.0000,
         0,  142.8000,  255.0000,
         0,  153.0000,  255.0000,
         0,  163.2000,  255.0000,
         0,  173.4000,  255.0000,
         0,  183.6000,  255.0000,
         0,  193.8000,  255.0000,
         0,  204.0000,  255.0000,
         0,  214.2000,  255.0000,
         0,  224.4000,  255.0000,
         0,  234.6000,  255.0000,
         0,  244.8000,  255.0000,
         0,  255.0000,  255.0000,
   10.2000,  255.0000,  244.8000,
   20.4000,  255.0000,  234.6000,
   30.6000,  255.0000,  224.4000,
   40.8000,  255.0000,  214.2000,
   51.0000,  255.0000,  204.0000,
   61.2000,  255.0000,  193.8000,
   71.4000,  255.0000,  183.6000,
   81.6000,  255.0000,  173.4000,
   91.8000,  255.0000,  163.2000,
  102.0000,  255.0000,  153.0000,
  112.2000,  255.0000,  142.8000,
  122.4000,  255.0000,  132.6000,
  132.6000,  255.0000,  122.4000,
  142.8000,  255.0000,  112.2000,
  153.0000,  255.0000,  102.0000,
  163.2000,  255.0000,   91.8000,
  173.4000,  255.0000,   81.6000,
  183.6000,  255.0000,   71.4000,
  193.8000,  255.0000,   61.2000,
  204.0000,  255.0000,   51.0000,
  214.2000,  255.0000,   40.8000,
  224.4000,  255.0000,   30.6000,
  234.6000,  255.0000,   20.4000,
  244.8000,  255.0000,   10.2000,
  255.0000,  255.0000,         0,
  255.0000,  244.8000,         0,
  255.0000,  234.6000,         0,
  255.0000,  224.4000,         0,
  255.0000,  214.2000,         0,
  255.0000,  204.0000,         0,
  255.0000,  193.8000,         0,
  255.0000,  183.6000,         0,
  255.0000,  173.4000,         0,
  255.0000,  163.2000,         0,
  255.0000,  153.0000,         0,
  255.0000,  142.8000,         0,
  255.0000,  132.6000,         0,
  255.0000,  122.4000,         0,
  255.0000,  112.2000,         0,
  255.0000,  102.0000,         0,
  255.0000,   91.8000,         0,
  255.0000,   81.6000,         0,
  255.0000,   71.4000,         0,
  255.0000,   61.2000,         0,
  255.0000,   51.0000,         0,
  255.0000,   40.8000,         0,
  255.0000,   30.6000,         0,
  255.0000,   20.4000,         0,
  255.0000,   10.2000,         0,
  255.0000,         0,         0,
  244.8000,         0,         0,
  234.6000,         0,         0,
  224.4000,         0,         0,
  214.2000,         0,         0,
  204.0000,         0,         0,
  193.8000,         0,         0,
  183.6000,         0,         0,
  173.4000,         0,         0,
  163.2000,         0,         0,
  153.0000,         0,         0,
  142.8000,         0,         0,
  132.6000,         0,         0 };

	size_t nc = sizeof(jet)/sizeof(double)/3;
	for( size_t i=0;i<nc;i++ )
	{
		int r = (int)jet[3*i];
		int g = (int)jet[3*i+1];
		int b = (int)jet[3*i+2];
		m_colourList.push_back( wxColour(r,g,b) );
	}
}

wxString wxPLJetColourMap::GetName()
{
	return _("Jet");
}


wxPLGrayscaleColourMap::wxPLGrayscaleColourMap( double min, double max )
	: wxPLColourMap( min, max )
{
	const size_t ncolours = 50;
	for ( size_t i=0;i<ncolours;i++ )
	{
		int grey = (int)(((double)i)/ncolours*255.0);
		m_colourList.push_back( wxColour( grey, grey, grey ) );
	}
}

wxString wxPLGrayscaleColourMap::GetName()
{
	return _("Grayscale");
}
