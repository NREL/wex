/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

#ifndef __DVFileDataSet_h
#define __DVFileDataSet_h

/*
 * wxDVFileDataSet.h
 *
 * For the stand-alone application, we provide the ability to load a data set from a file.
 * Other applications that implement DViewLib will handle wxDVTimeSeriesDataSet in their own way.
 *
 * This class implements wxDVTimeSeriesDataSet.  This class is used to read a
 * data set from files.  Right now we support csv and txt and Energy+ sql and
 * can convert the sql input from its native IP to SI units and values.
 */
#include <stdio.h>
#include <vector>
#include <wx/string.h>

struct sqlite3;

class wxDVPlotCtrl;
class wxDVArrayDataSet;
class wxDateTime;

using namespace std;

class wxDVFileReader
{
public:
	static void ReadDataFromCSV(wxDVPlotCtrl* plotWin, const wxString& filename, wxChar separator = ',');
	static bool FastRead(wxDVPlotCtrl* plotWin, const wxString& filename, int prealloc_data = 8760, int prealloc_lnchars = 1024);
	static bool Read8760WFLines(std::vector<wxDVArrayDataSet*> &dataSets, FILE* infile, int wfType);
	static bool ReadWeatherFile(wxDVPlotCtrl* plotWin, const wxString& filename);
	static bool ReadSQLFile(wxDVPlotCtrl* plotWin, const wxString& filename);
	static bool IsNumeric(wxString stringToCheck);
	static bool IsDate(wxString stringToCheck);

private:
	static wxString ColumnText(const unsigned char* column);
	static bool IsEnergyPlus(sqlite3 * db);
	static void ExecAndThrowOnError(const std::string &t_stmt, sqlite3 * db);
	// Interpolates to synthesize data at the 1 minute timeStep
	static void NonuniformTimestepInterpolation(const std::vector<wxDateTime> & times, std::vector<double> & values);
	static void InitUnitConversions();
	// Converts both the units and the values
	static bool ConvertUnits(std::string & units, std::vector<double> & values, bool convertSIToIP = true);
};

#endif
