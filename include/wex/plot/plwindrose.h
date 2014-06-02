#ifndef __pl_polarplot_h
#define __pl_polarplot_h

#include "wex/plot/plplotctrl.h"

class wxPLWindRose : public wxPLPlottable
{
public:
	wxPLWindRose();
	wxPLWindRose( const std::vector<wxRealPoint> &data,
		const wxString &label = wxEmptyString,
		const wxColour &col = *wxBLUE,
		int size = 1,
		bool scale = false );

	virtual ~wxPLWindRose();
	
	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	virtual void Draw( wxDC &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxDC &dc, const wxRect &rct);

	void SetColour( const wxColour &col ) { m_colour = col; }
	void SetSize( int radius ) { m_size = radius; }
	void SetIgnoreAngle(bool ignore=true) { m_ignoreAngles = ignore; }

	virtual wxPLAxis *SuggestXAxis() const;
	virtual wxPLAxis *SuggestYAxis() const;

protected:
	wxColour m_colour;
	int m_size;
	bool m_scale;
	bool m_ignoreAngles;
	std::vector<wxRealPoint> m_data;

};

#endif
