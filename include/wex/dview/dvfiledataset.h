#ifndef __DVFileDataSet_h
#define __DVFileDataSet_h

/*
 * wxDVFileDataSet.h
 * 
 * For the stand-alone application, we provide the ability to load a data set from a file.
 * Other applications that implement DViewLib will handle wxDVTimeSeriesDataSet in their own way.
 *
 * This class implements wxDVTimeSeriesDataSet.  This class is used to read a 
 * data set from files.  Right now we support csv and txt.
 */

#include "wex/dview/dvtimeseriesdataset.h"
#include "wex/dview/dvplotctrl.h"

class wxDVFileDataSet : public wxDVTimeSeriesDataSet
{
public:
	/* Constructors and Destructors */
	wxDVFileDataSet();
	virtual ~wxDVFileDataSet();

	static void ReadDataFromCSV(wxDVPlotCtrl* plotWin, const wxString& filename, wxChar separator = ',');
	static bool FastRead(wxDVPlotCtrl* plotWin, const wxString& filename, int prealloc_data = 8760, int prealloc_lnchars = 1024);
	static void Read8760WFLines(std::vector<wxDVFileDataSet*> &dataSets, FILE* infile, int wfType);
	static bool ReadWeatherFile(wxDVPlotCtrl* plotWin, const wxString& filename);

	/* We must implement these functions to provide data*/
	wxRealPoint At(size_t i) const;
	size_t Length() const;
	double GetTimeStep() const;
	wxString GetSeriesTitle() const;
	wxString GetUnits() const;

	/* Functions to set data */
	void Alloc(int size);
	void Append(const wxRealPoint& p);
	void SetSeriesTitle(const wxString& title);
	void SetUnits(const wxString& units);
	void SetTimeStep(double timeStep);

private:
	//Time Step units are hours.  Default is 1 for hourly data.
	double mTimeStep;

	wxString mSeriesTitle;
	wxString mUnits;

	std::vector<wxRealPoint> mDataPoints;
};

#endif

