/*
BSD 3-Clause License

Copyright (c) Alliance for Sustainable Energy, LLC. See also https://github.com/NREL/wex/blob/develop/LICENSE
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
#include <math.h>

class wxDVTimeSeriesDataSet {
    wxString m_metaData, m_groupName;
protected:
    /*Constructors and Destructors*/
    wxDVTimeSeriesDataSet();

public:
    virtual ~wxDVTimeSeriesDataSet();

    /*Pure virtual functions must be implemented by subclass*/
    virtual wxRealPoint At(size_t i) const = 0;

    virtual size_t Length() const = 0;

    virtual double GetTimeStep() const = 0;

    virtual double GetOffset() const = 0;

    virtual wxString GetSeriesTitle() const = 0;

    virtual wxString GetUnits() const = 0;

    virtual wxString GetLabel() const;

    /*Helper Functions*/
    wxRealPoint operator[](size_t i) const;

    wxString GetTitleWithUnits() const;

    double GetMinHours();

    double GetMaxHours();

    double GetTotalHours();

    void GetMinAndMaxInRange(double *min, double *max, size_t startIndex, size_t endIndex);

    void GetMinAndMaxInRange(double *min, double *max, double startHour, double endHour);

    void GetDataMinAndMax(double *min, double *max);

    std::vector<wxRealPoint> GetDataVector();

    virtual void SetMetaData(const wxString &meta) { m_metaData = meta; }

    virtual wxString GetMetaData() { return m_metaData; }

    virtual wxString GetGroupName() const { return m_groupName; }

    virtual void SetGroupName(const wxString &g) { m_groupName = g; }
};

class wxDVArrayDataSet : public wxDVTimeSeriesDataSet {
public:
    wxDVArrayDataSet();

    wxDVArrayDataSet(const wxString &var, const std::vector<double> &data);

    wxDVArrayDataSet(const wxString &var, const std::vector<wxRealPoint> &data);

    wxDVArrayDataSet(const wxString &var, const wxString &units, const double &timestep);

    wxDVArrayDataSet(const wxString &var, const wxString &units, const double &timestep,
                     const std::vector<double> &data);

    wxDVArrayDataSet(const wxString &var, const wxString &units, const double &offset, const double &timestep,
                     const std::vector<double> &data);

    virtual wxRealPoint At(size_t i) const;

    virtual size_t Length() const;

    virtual double GetTimeStep() const;

    virtual double GetOffset() const;

    virtual wxString GetSeriesTitle() const;

    virtual wxString GetUnits() const;

    void Copy(const std::vector<double> &data);

    void Clear();

    void Alloc(size_t n);

    void Append(const wxRealPoint &p);

    void Set(size_t i, double x, double y);

    void SetY(size_t i, double y);

    void SetSeriesTitle(const wxString &title);

    void SetUnits(const wxString &units);

    void SetTimeStep(double ts, bool recompute_x = true);

    void SetOffset(double off, bool recompute_x = true);

    void RecomputeXData();

private:
    wxString m_varLabel;
    wxString m_varUnits;
    double m_timestep; // timestep in hours - fractional hours okay
    double m_offset; // offset in hours from Jan1 00:00 - fractional hours okay
    std::vector<wxRealPoint> m_pData;
};

enum StatisticsType {
    MEAN = 0, MIN, MAX, SUMMATION, STDEV, AVGDAILYMIN, AVGDAILYMAX
};

struct StatisticsPoint {
    wxString name;
    double x;
    double Sum;
    double Min;
    double Max;
    double Mean;
    double AvgDailyMin;
    double AvgDailyMax;
    double StDev;
};

class wxDVStatisticsDataSet {
public:
    wxDVStatisticsDataSet(wxDVTimeSeriesDataSet *d);

    double RoundSignificant(double ValueToRound, size_t NumSignifDigits = 4);

    StatisticsPoint At(size_t i) const;

    size_t Length() const;

    void Clear();

    void Alloc(size_t n);

    void Append(const StatisticsPoint &p);

    bool IsSourceDataset(wxDVTimeSeriesDataSet *d);

    //We need the methods below because we can't return a reference to baseDataset itself because of its pure virtual methods
    double GetTimeStep() const;

    double GetOffset() const;

    wxString GetSeriesTitle() const;

    wxString GetUnits() const;

    double GetMinHours();

    double GetMaxHours();

    void GetDataMinAndMax(double *min, double *max);

    void GetMinAndMaxInRange(double *min, double *max, double startHour, double endHour);

private:
    std::vector<StatisticsPoint> m_sData;
    wxDVTimeSeriesDataSet *baseDataset;
};

#endif
