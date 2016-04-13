#ifndef __pl_contourplot_h
#define __pl_contourplot_h

#include "wex/matrix.h"
#include "wex/plot/plplot.h"

class wxPLColourMap;

class wxPLContourPlot : public wxPLPlottable
{
public:
	wxPLContourPlot();
	wxPLContourPlot( const wxMatrix<double> &z,
		const wxString &label = wxEmptyString,
		int levels = 10,
		wxPLColourMap *cmap = 0 );

	virtual ~wxPLContourPlot();

	void SetLevels( int levels );
	void SetColourMap( wxPLColourMap *cmap ); // does not take ownership of colour map

	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	
	virtual void Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct);

	
	struct Contour
	{
		double level;
		std::vector<wxRealPoint> points;
	};

protected:
	wxMatrix<double> m_z;
	wxPLColourMap *m_cmap;
	
	void Update( int levels );
	std::vector<Contour> m_contours;	
};


#endif
