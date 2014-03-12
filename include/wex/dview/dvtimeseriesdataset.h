#ifndef __DVTimeSeriesDataSet_h
#define __DVTimeSeriesDataSet_h

/*
 * wxDVTimeSeriesDataSet.h
 *
 * This class is an interface to provide time series data.
 *
 * Data units on the x axis are hours and should be passed as double.
 *
 * ASSUMPTIONS: Any subclass of TimeSeriesDataSet should adhere to these.
 * -Ordered Data: if (m > n), then data.At(m).x > data.At(n).x
 *    -This is necessary because of how we iterate through a data set to plot it.
 * -Even Time Step: consecutive points have the same deltaX.
 *    -This is necessary because of how we average in the profile view.
 *
 * Also, we don't handle missing data points yet.
 */

#include <vector>

#include <wx/gdicmn.h>
#include <wx/string.h>

class wxDVTimeSeriesDataSet
{
protected:
	/*Constructors and Destructors*/
	wxDVTimeSeriesDataSet();
public:
	virtual ~wxDVTimeSeriesDataSet();

	/*Pure virtual functions must be implemented by subclass*/
	virtual wxRealPoint At(size_t i) const = 0;
	virtual size_t Length() const = 0;
	virtual double GetTimeStep() const = 0;
	virtual wxString GetSeriesTitle() const = 0;
	virtual wxString GetUnits() const = 0;
	
	/*Helper Functions*/
	wxRealPoint operator[] (size_t i) const;

	wxString GetTitleWithUnits();
	double GetMinHours();
	double GetMaxHours();
	double GetTotalHours();
	void GetMinAndMaxInRange(double* min, double* max, size_t startIndex, size_t endIndex);
	void GetMinAndMaxInRange(double* min, double* max, double startHour, double endHour);
	void GetDataMinAndMax(double* min, double* max);
	std::vector<wxRealPoint> GetDataVector();
};

class wxDVArrayDataSet : public wxDVTimeSeriesDataSet
{
public:
	wxDVArrayDataSet();
	wxDVArrayDataSet( const wxString &var, const std::vector<double> &data );
	wxDVArrayDataSet( const wxString &var, const std::vector<wxRealPoint> &data );
	wxDVArrayDataSet( const wxString &var, const wxString &units, const double &timestep );
	wxDVArrayDataSet( const wxString &var, const wxString &units, const double &timestep, const std::vector<double> &data );
	wxDVArrayDataSet( const wxString &var, const wxString &units, const double &offset, const double &timestep, const std::vector<double> &data );
	
	virtual wxRealPoint At(size_t i) const;
	virtual size_t Length() const;
	virtual double GetTimeStep() const;
	virtual wxString GetSeriesTitle() const;
	virtual wxString GetUnits() const;

	void Copy( const std::vector<double> &data );
	void Clear();
	void Alloc( size_t n );
	void Append( const wxRealPoint &p );
	void Set( size_t i, double x, double y );
	void SetY( size_t i, double y );
	
	void SetSeriesTitle( const wxString &title );
	void SetUnits( const wxString &units );
	void SetTimeStep( double ts, bool recompute_x = true );
	void SetOffset( double off, bool recompute_x = true );
	

protected:
	void RecomputeXData();
private:
	wxString m_varLabel;
	wxString m_varUnits;
	double m_timestep; // timestep in hours - fractional hours okay
	double m_offset; // offset in hours from Jan1 00:00 - fractional hours okay
	std::vector<wxRealPoint> m_pData;
	
};

#endif