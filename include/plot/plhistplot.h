#ifndef __pl_histplot_h
#define __pl_histplot_h

#include "plot/plplotctrl.h"

class wxPLHistogramPlot : public wxPLPlottable
{
public:
	wxPLHistogramPlot();
	wxPLHistogramPlot( const std::vector<wxRealPoint> &data,
		const wxString &label );

	void Init();

	enum NormalizeType {NO_NORMALIZE = 0, NORMALIZE, NORMALIZE_PDF};
	
	void SetData( const std::vector<wxRealPoint> &data );

	//Getters and Setters
	void SetLineStyle( const wxColour &c, int width );
	void SetFillColour( const wxColour &c );
	void SetNumberOfBins( size_t n );
	void SetNormalize( NormalizeType n );
	NormalizeType GetNormalize();
	int GetNumberOfBins();
	double GetNiceYMax();

	bool GetIgnoreZeros();
	void SetIgnoreZeros(bool value = true);

	static int GetSturgesBinsFor(int nDataPoints);
	static int GetSqrtBinsFor(int nDataPoints);

	
	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxDC &dc, const wxRect &rct);
	virtual wxPLAxis *SuggestXAxis();
	virtual wxPLAxis *SuggestYAxis();
	virtual bool GetMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax) const;

private:	
	bool m_normalize;
	bool m_normalizeToPdf;
	bool m_ignoreZeros;

	wxColour m_lineColour;
	int m_lineThickness;
	wxColour m_fillColour;

	size_t m_numberOfBins;

	std::vector<double> m_histData;

	void RecalculateHistogram();

	double m_niceMax;
	double m_dataMin, m_dataMax;

	std::vector<wxRealPoint> m_data;
};

#endif
