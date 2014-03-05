#ifndef __pl_axis_h
#define __pl_axis_h

#include <vector>

#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>

class wxPLAxis
{
public:
	wxPLAxis();
	wxPLAxis( double min, double max, const wxString &label = wxEmptyString );
	wxPLAxis( const wxPLAxis &rhs );
	virtual ~wxPLAxis();
	
	// pure virtuals
	struct TickData {
		enum TickSize { NONE, SMALL, LARGE };
		TickData( ) : world(0.0), size(SMALL), label( wxEmptyString ) { }
		TickData( double w, const wxString &l, TickSize s ) : world(w), size(s), label(l) { }

		double world;
		TickSize size;
		wxString label;
	};

	virtual wxPLAxis *Duplicate() = 0;
	virtual void GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list ) = 0;

	// by default assumes a linear axis.  can be overridden to provide logarithmic or other scaling

	virtual wxCoord WorldToPhysical( double coord, wxCoord phys_min, wxCoord phys_max, bool isHistXAxis = false );
	virtual double PhysicalToWorld( wxCoord p, wxCoord phys_min, wxCoord phys_max );

	// get & set axis properties

	virtual void SetWorld( double min, double max );
	virtual void SetWorldMin( double min );
	virtual void SetWorldMax( double max );
	void GetWorld( double *min, double *max ) { if (min) *min = m_min; if (max) *max = m_max; }
	double GetWorldMin() { return m_min; }
	double GetWorldMax() { return m_max; }
	double GetWorldLength() { return m_max - m_min; }
	
	virtual void SetLabel( const wxString &s ) { m_label = s; }
	virtual wxString GetLabel() { return m_label; }
	virtual void SetColour( const wxColour &col ) { m_colour = col; }
	virtual wxColour GetColour() { return m_colour; }
	
	void SetTickSizes( int smallsz, int largesz ) { m_smallTickSize = smallsz; m_largeTickSize = largesz; }
	void GetTickSizes( int *smallsz, int *largesz ) { if (smallsz) *smallsz = m_smallTickSize; if (largesz) *largesz = m_largeTickSize; }

	void ShowLabel( bool label ) { m_showLabel = label; }
	void ShowTickText( bool ticktext ) { m_showTickText = ticktext; }
	bool IsLabelVisible() { return m_showLabel; }
	bool IsTickTextVisible() { return m_showTickText; }
	
	virtual void ExtendBound( wxPLAxis *a );

protected:
	void Init();
	
	wxString m_label;
	wxColour m_colour;
	double m_min;
	double m_max;
	bool m_showLabel;
	bool m_showTickText;
	int m_smallTickSize;
	int m_largeTickSize;
};


class wxPLLinearAxis : public wxPLAxis
{
public:
	wxPLLinearAxis( double min, double max, const wxString &label = wxEmptyString );
	wxPLLinearAxis( const wxPLLinearAxis &rhs );

	virtual wxPLAxis *Duplicate();
	virtual void GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list );
	
protected:
	virtual void CalcTicksFirstPass( wxCoord phys_min, wxCoord phys_max, 
			std::vector<double> &largeticks, std::vector<double> &smallticks) ;
	virtual void CalcTicksSecondPass( wxCoord phys_min, wxCoord phys_max, 
			std::vector<double> &largeticks, std::vector<double> &smallticks) ;
	
	double AdjustedWorldValue( double world );
	double DetermineLargeTickStep( double physical_len, bool &should_cull_middle);
	size_t DetermineNumberSmallTicks( double big_tick_dist );

	double m_scale, m_offset;
	double m_approxNumberLargeTicks;
	std::vector<double> m_mantissas;
	std::vector<size_t> m_smallTickCounts;
};

class wxPLLabelAxis : public wxPLAxis
{
public:
	wxPLLabelAxis( double min, double max, const wxString &label = wxEmptyString );
	wxPLLabelAxis( const wxPLLabelAxis &rhs );
	
	virtual wxPLAxis *Duplicate();
	virtual void GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list );

	void Add( double world, const wxString &text );
	void Clear();

protected:	
	std::vector<TickData> m_tickLabels;
};


class wxPLLogAxis : public wxPLAxis
{
public:
	wxPLLogAxis( double min, double max, const wxString &label = wxEmptyString );
	wxPLLogAxis( const wxPLLogAxis &rhs );

	virtual wxPLAxis *Duplicate();
	virtual void GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list );
	virtual wxCoord WorldToPhysical( double coord, wxCoord phys_min, wxCoord phys_max );
	virtual double PhysicalToWorld( wxCoord p, wxCoord phys_min, wxCoord phys_max );

	virtual void SetWorld( double min, double max );
	virtual void SetWorldMin( double min );
	virtual void ExtendBound( wxPLAxis *a );

protected:
	virtual void CalcTicksFirstPass( std::vector<double> &largeticks, std::vector<double> &smallticks) ;
	virtual void CalcTicksSecondPass( std::vector<double> &largeticks, std::vector<double> &smallticks) ;
	
	double DetermineTickSpacing();
	size_t DetermineNumberSmallTicks( double big_tick_dist );
};

class wxPLTimeAxis : public wxPLAxis
{
public:
	wxPLTimeAxis( double min, double max, const wxString &label = wxEmptyString );
	wxPLTimeAxis( const wxPLTimeAxis &rhs );

	virtual wxString GetLabel();
	virtual wxPLAxis *Duplicate();
	virtual void GetAxisTicks( wxCoord phys_min, wxCoord phys_max, std::vector<TickData> &list );

private:
	std::vector<TickData> m_tickList;
	wxString m_timeLabel;
	double m_lastMin, m_lastMax;
	void RecalculateTicksAndLabel();
};

#endif

