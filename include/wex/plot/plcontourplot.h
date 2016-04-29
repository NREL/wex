#ifndef __pl_contourplot_h
#define __pl_contourplot_h

#include "wex/matrix.h"
#include "wex/plot/plplot.h"

class wxPLColourMap;

class wxPLContourPlot : public wxPLPlottable
{
public:
	wxPLContourPlot();
	wxPLContourPlot( 
		const wxMatrix<double> &x,
		const wxMatrix<double> &y,
		const wxMatrix<double> &z,
		bool filled,
		const wxString &label = wxEmptyString,
		double level_min=0, double level_max=0,
		int levels = 10,
		wxPLColourMap *cmap = 0 );

	virtual ~wxPLContourPlot();

	static void MinMax(	const std::vector<double> &v,
			double *minval, double *maxval );

	static bool MeshGrid( 
			double xmin, double xmax, size_t nx,
			double ymin, double ymax, size_t ny,
		wxMatrix<double> &xmesh,
		wxMatrix<double> &ymesh );

	static bool GridData( 
			const std::vector<double> &x, 
			const std::vector<double> &y,
			const std::vector<double> &z,
			const wxMatrix<double> &xq,
			const wxMatrix<double> &yq,
		wxMatrix<double> &zinterp  );

	
	static void Peaks( size_t n,
		wxMatrix<double> &xx, wxMatrix<double> &yy, wxMatrix<double> &zz,
		double *min, double *max );


	void SetLevels( int levels );
	void SetColourMap( wxPLColourMap *cmap ); // does not take ownership of colour map

	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	
	virtual void Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct);

protected:
	wxMatrix<double> m_x, m_y, m_z;
	wxPLColourMap *m_cmap;
	bool m_filled;
	double m_levelMin, m_levelMax;

	void Update( int levels );
	
	struct C_pt {
		C_pt( double _x, double _y, char _a=0 ) : x(_x), y(_y), act(_a) { }
		double x, y;
		char act;
	};
	
	struct C_poly {
		std::vector< C_pt > pts;
		double z, zmax;
	};

	std::vector<C_poly> m_cPolys;
	
};


#endif
