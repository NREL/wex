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

#include "../src/sqlite3.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <tuple>
#include <vector>



#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/time.h>
#include <wx/tokenzr.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include "wex/dview/dvfilereader.h"
#include "wex/dview/dvplotctrl.h"
#include "wex/dview/dvtimeseriesdataset.h"


vector<tuple<string, string, double> > m_unitConversions;

static bool AllocReadLine(FILE *fp, wxString &buf, int prealloc = 256)
{
	char c;

	buf = "";
	if (prealloc > 10)
		buf.Alloc(prealloc);

	// read the whole line, 1 character at a time, no concern about buffer length
	while ((c = fgetc(fp)) != EOF && c != '\n' && c != '\r')
		buf += c;

	// handle windows <CR><LF>
	if (c == '\r')
	{
		if ((c = fgetc(fp)) != '\n')
			ungetc(c, fp);
	}

	// handle a stray <CR>
	if (c == '\n')
	{
		if ((c = fgetc(fp)) != '\r')
			ungetc(c, fp);
	}

	return !(buf.Len() == 0 && c == EOF);
}

struct WFHeader
{
	WFHeader();

	wxString stationID;
	wxString city;
	wxString state;
	double timezone;
	double latitude;
	double longitude;
	double elevation;
	wxString swrfCreationDate;
	wxString swrfCreationTime;
	wxString swrfServer;
	double lat_requested;
	double lon_requested;
	double dist_from_request;
	int time_step_secs;
};

#define WF_ERR 0
#define WF_TM2 2
#define WF_EPW 3
#define WF_TM3 7
#define WF_SWRF 8
#define WF_SMW 9

WFHeader::WFHeader()
{
	city = state = "INVALID";
	timezone = latitude = longitude = elevation = -1.0;
	time_step_secs = 60;
}

static double ConvertDegMinSec(double degrees,
	double minutes,
	double seconds,
	char direction)
{
	double dd = degrees + minutes / 60.0 + seconds / 3600.0;
	if (tolower(direction) == 's' || tolower(direction) == 'w')
		dd = 0 - dd;

	return dd;
}

static bool ParseTM2Header(const wxString &file, WFHeader &info, FILE **wfretfp)
{
	//  0     1                      2    3 4  5 6  7 8    9   10
	//  23183 PHOENIX                AZ  -7 N 33 26 W 112  1   339
	FILE *fp = fopen(file.c_str(), "r");
	if (!fp)
		return false;

	double deg, min;
	char dir;
	wxString line, buf;
	AllocReadLine(fp, line);
	wxStringTokenizer tokens(line, " \t");

	buf = tokens.GetNextToken(); // skip the first one
	// LOCATION
	info.city = tokens.GetNextToken();
	// State is sometimes blank - email from Paul 11/17/09 - need formatted input char by char
	info.state = tokens.GetNextToken();
	if (info.state.IsNumber()) {
		buf = info.state;
		info.state = "";
	}
	else {
		// TIMEZONE
		buf = tokens.GetNextToken();
	}
	sscanf(buf.c_str(), "%lg", &info.timezone);
	// LATITUDE
	buf = tokens.GetNextToken(); if (buf.Len() > 0) dir = buf[0]; else return false;
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &deg);
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &min);
	info.latitude = ConvertDegMinSec(deg, min, 0, dir);
	// LONGITUDE
	buf = tokens.GetNextToken(); if (buf.Len() > 0) dir = buf[0]; else return false;
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &deg);
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &min);
	info.longitude = ConvertDegMinSec(deg, min, 0, dir);
	// ELEVATION
	buf = tokens.GetNextToken();
	sscanf(buf.c_str(), "%lg", &info.elevation);

	// no lines to skip, all remaining lines are data

	if (wfretfp)
		*wfretfp = fp;
	else
		fclose(fp);

	return true;
}

static bool ParseTM3Header(const wxString &file, WFHeader &info, FILE **wfretfp)
{
	//724880,"RENO TAHOE INTERNATIONAL AP",NV,-8.0,39.483,-119.767,1342
	FILE *fp = fopen(file.c_str(), "r");
	if (!fp)
		return false;

	wxString line, buf;
	AllocReadLine(fp, line);
	wxStringTokenizer tokens(line, ",");
	info.stationID = tokens.GetNextToken();
	// LOCATION
	buf = tokens.GetNextToken();
	if (buf.Left(1) == "\"" && buf.Right(1) == "\"")
		buf = buf.Mid(1, buf.Len() - 2);
	info.city = buf;
	info.state = tokens.GetNextToken();
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &info.timezone);
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &info.latitude);
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &info.longitude);
	buf = tokens.GetNextToken(); sscanf(buf.c_str(), "%lg", &info.elevation);

	// skip over column header (units) line
	AllocReadLine(fp, line);

	// remaining lines are data

	if (wfretfp)
		*wfretfp = fp;
	else
		fclose(fp);

	return true;
}

static bool ParseEPWHeader(const wxString &file, WFHeader &info, FILE **wfretfp)
{
	//  LOCATION,PHOENIX,AZ,USA,TMY2-23183,722780,33.43,-112.02,-7.0,339.0
	FILE *fp = fopen(file.c_str(), "r");
	if (!fp)
		return false;

	wxString line, buf;
	AllocReadLine(fp, line);
	wxStringTokenizer tokens(line, ",");

	buf = tokens.GetNextToken(); // skip the first one
	info.city = tokens.GetNextToken();  // CITY
	info.state = tokens.GetNextToken();  // STATE
	buf = tokens.GetNextToken(); // skip country
	buf = tokens.GetNextToken(); // skip source
	buf = tokens.GetNextToken(); // skip id number
	buf = tokens.GetNextToken(); // LATITUDE
	sscanf(buf.c_str(), "%lg", &info.latitude);
	buf = tokens.GetNextToken(); // LONGITUDE
	sscanf(buf.c_str(), "%lg", &info.longitude);
	buf = tokens.GetNextToken(); // TIMEZONE
	sscanf(buf.c_str(), "%lg", &info.timezone);
	buf = tokens.GetNextToken(); // ELEVATION
	sscanf(buf.c_str(), "%lg", &info.elevation);

	// skip over header lines

	AllocReadLine(fp, line); // DESIGN CONDITIONS
	AllocReadLine(fp, line); // TYPICAL/EXTREME PERIODS
	AllocReadLine(fp, line); // GROUND TEMPERATURES
	AllocReadLine(fp, line); // HOLIDAY/DAYLIGHT SAVINGS
	AllocReadLine(fp, line); // COMMENTS 1
	AllocReadLine(fp, line); // COMMENTS 2
	AllocReadLine(fp, line); // DATA PERIODS
	//DATA PERIODS,N periods, N records/hr, A period 1 name, A start day of week, start date, end date
	//DATA PERIODS,1,1,TMY2 Year,Sunday,1,365

	// remaining lines are data

	if (wfretfp)
		*wfretfp = fp;
	else
		fclose(fp);

	return true;
}

static int GetWeatherFileType(const wxString &weather_file)
{
	/*
	FileLines.Add('* WFType indicates the weather file type');
	C     MODE 1: TMY FILE FORMAT
	C     MODE 2: TMY2 FORMAT
	C     MODE 3: ENERGYPLUS FORMAT
	C     MODE 4: IWEC (INTERNATIONAL WEATHER FOR ENERGY CALCULATIONS)
	C     MODE 5: CWEC (CANADIAN WEATHER FOR ENERGY CALCULATIONS)
	C     MODE 6: METEONORM (TMY2 FILE FORMAT)
	C     MODE 7: TMY3 FORMAT
	C     MODE 8: Sam Wind Resource File (tab delimited)
	*/
	wxString ext;
	wxFileName::SplitPath(weather_file, NULL, NULL, NULL, &ext);
	if (ext.Lower() == "tm2")
		return WF_TM2;
	else if (ext.Lower() == "epw")
		return WF_EPW;
	else if (ext.Lower() == "tm3" || ext.Lower() == "csv")
		return WF_TM3;
	else if (ext.Lower() == "swrf")
		return 	WF_SWRF;
	else if (ext.Lower() == "smw")
		return WF_SMW;
	else
		return WF_ERR; // error!;
}

static int cstrlocate(char *buf, char **colidx, int colmax, char delim)
{
	char *p = buf;
	int ncols = 0;
	colidx[0] = p;
	int i = 1;
	while (p && *p && i < colmax)
	{
		p = strchr(p, delim);
		//if (p) colidx[i++] = ++p;
		if ((p) && (*(++p) != delim)) colidx[i++] = p;
	}

	ncols = i;

	while (i < colmax) colidx[i++] = 0;

	return ncols;
}

/*bool ReadWeatherFileLine(FILE *fp, int type,
int &year, int &month, int &day, int &hour,
double &dn, double &df, double &ambt, double &wind)*/
static bool ReadWeatherFileLine(FILE *fp, int type,
	int &year, int &month, int &day, int &hour,
	double &gh, double &dn, double &df,  // Wh/m2, Wh/m2, Wh/m2
	double &wind, double &drytemp, double &wettemp, // m/s, 'C, 'C
	double &relhum, double &pressure, // %, mbar
	double &winddir, double &snowdepth) // deg, cm
{
	char buf[1024];
	char *cols[128], *p;

	if (!fp)
		return false;

	wxString line;
	if (type == WF_TM2)
	{
		/* taken from PVWatts */
		int yr, mn, dy, hr, ethor, etdn;
		int d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21;      // which of these are used? d3, d10, d15 & d20
		int u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14, u15, u16, u17, u18, u19, u20, u21;  // are any of these ever used?? - no!
		int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
		char f1[2], f2[2], f3[2], f4[2], f5[2], f6[2], f7[2], f8[2], f9[2], f10[2], f11[2], f12[2], f13[2], f14[2], f15[2], f16[2], f17[2], f18[2], f19[2], f20[2], f21[2];

		int nread = fscanf(fp,
			"%2d%2d%2d%2d"
			"%4d%4d"
			"%4d%1s%1d%4d%1s%1d%4d%1s%1d%4d%1s%1d%4d%1s%1d%4d%1s%1d%4d%1s%1d"
			"%2d%1s%1d%2d%1s%1d%4d%1s%1d%4d%1s%1d%3d%1s%1d%4d%1s%1d%3d%1s%1d"
			"%3d%1s%1d%4d%1s%1d%5d%1s%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%3d%1s%1d%3d%1s%1d%3d%1s%1d%2d%1s%1d\n",
			&yr, &mn, &dy, &hr,
			&ethor, // extraterrestrial horizontal radiation
			&etdn, // extraterrestrial direct normal radiation
			&d1, f1, &u1, // GH data value 0-1415 Wh/m2, Source, Uncertainty
			&d2, f2, &u2, // DN data value 0-1200 Wh/m2, Source, Uncertainty
			&d3, f3, &u3, // DF data value 0-700 Wh/m2, Source, Uncertainty
			&d4, f4, &u4, // GH illum data value, Source, Uncertainty
			&d5, f5, &u5, // DN illum data value, Source, Uncertainty
			&d6, f6, &u6, // DF illum data value, Source, Uncertainty
			&d7, f7, &u7, // Zenith illum data value, Source, Uncertainty
			&d8, f8, &u8, // Total sky cover
			&d9, f9, &u9, // opaque sky cover
			&d10, f10, &u10, // dry bulb temp -500 to 500 = -50.0 to 50.0 'C
			&d11, f11, &u11, // dew point temp -600 to 300 = -60.0 to 30.0 'C
			&d12, f12, &u12, // relative humidity 0-100
			&d13, f13, &u13, // pressure millibars
			&d14, f14, &u14, // wind direction
			&d15, f15, &u15, // wind speed 0 to 400 = 0.0 to 40.0 m/s
			&d16, f16, &u16, // visibility
			&d17, f17, &u17, // ceiling height
			&w1, &w2, &w3, &w4, &w5, &w6, &w7, &w8, &w9, &w10, // present weather
			&d18, f18, &u18, // precipitable water
			&d19, f19, &u19, // aerosol optical depth
			&d20, f20, &u20, // snow depth 0-150 cm
			&d21, f21, &u21); // days since last snowfall 0-88

		year = yr + 1900;
		month = mn;
		day = dy;
		hour = hr;

		gh = (double)d1*1.0;
		dn = d2*1.0;           /* Direct radiation */
		df = d3*1.0;           /* Diffuse radiation */
		drytemp = d10 / 10.0;       /* Ambient dry bulb temperature(C) */
		wettemp = (double)d11 / 10.0;
		wind = d15 / 10.0;       /* Wind speed(m/s) */
		relhum = (double)d12;
		pressure = (double)d13;
		winddir = (double)d14;
		snowdepth = (double)d20;

		return nread == 79;
	}
	else if (type == WF_TM3)
	{
	  	if (fgets(buf, sizeof(buf), fp) == NULL)
			return false;
		int ncols = cstrlocate(buf, cols, 128, ',');

		if (ncols < 68)
			return false;

		p = cols[0];

		month = atoi(p);
		p = strchr(p, '/');
		if (!p)
			return false;
		p++;
		day = atoi(p);
		p = strchr(p, '/');
		if (!p) return false;
		p++;
		year = atoi(p);

		hour = atoi(cols[1]);

		gh = (double)atof(cols[4]);
		dn = (double)atof(cols[7]);
		df = (double)atof(cols[10]);

		drytemp = (double)atof(cols[31]);
		wettemp = (double)atof(cols[34]);

		wind = (double)atof(cols[46]);

		relhum = (double)atof(cols[37]);
		pressure = (double)atof(cols[40]);

		winddir = (double)atof(cols[43]);
		snowdepth = 999;

		return true;
	}
	else if (type == WF_EPW)
	{
	  	if (fgets(buf, sizeof(buf), fp) == NULL)
			return false;
		int ncols = cstrlocate(buf, cols, 128, ',');

		if (ncols < 32)
			return false;

		year = atoi(cols[0]);
		month = atoi(cols[1]);
		day = atoi(cols[2]);
		hour = atoi(cols[3]);

		dn = (double)atof(cols[14]);
		df = (double)atof(cols[15]);
		drytemp = (double)atof(cols[6]);
		wind = (double)atof(cols[21]);

		gh = (double)atof(cols[13]);
		wettemp = (double)atof(cols[7]);
		relhum = (double)atof(cols[8]);
		pressure = (double)atof(cols[9]) * 0.01; // convert Pa in to mbar

		winddir = (double)atof(cols[20]);
		snowdepth = (double)atof(cols[30]);

		return true;
	}
	else
		return false;
}

bool wxDVFileReader::FastRead(wxDVPlotCtrl *plotWin, const wxString& filename, int prealloc_data, int prealloc_lnchars)
{
	wxString fExtension = filename.Right(3);
	if (fExtension.CmpNoCase("tm2") == 0 ||
		fExtension.CmpNoCase("epw") == 0 ||
		fExtension.CmpNoCase("smw") == 0)
	{
		return ReadWeatherFile(plotWin, filename);
	}
	else if (fExtension.CmpNoCase("sql") == 0)
	{
		return ReadSQLFile(plotWin, filename);
	}

	wxStopWatch sw;
	sw.Start();

	FILE *inFile = fopen(filename.c_str(), "r"); //r is for read mode.
	if (!inFile)
		return false;

	unsigned lnchars = prealloc_lnchars > 0 ? prealloc_lnchars : 1024;
	lnchars *= 2;  //Give ourselves extra room

	std::vector<wxDVArrayDataSet*> dataSets;
	std::vector<wxString> groupNames;
	std::vector<double> timeCounters;
	int columns = 0;
	bool CommaDelimiters = false;

	wxString firstLine;
	AllocReadLine(inFile, firstLine, lnchars); //Read a line from inFile preallocating lnchars length.

	// if the string with header names is really long,
	// make sure the line reading buffer is plenty long
	// assuming here that the text length of a data line
	// is no more than twice the text length of the header line
	if (firstLine.Len() > lnchars)
		lnchars = firstLine.Len() * 2;

	if (firstLine.Left(19) == wxT("wxDVFileHeaderVer.1"))
	{
		wxString titleStr, offsetStr, tStepStr, unitStr;

		AllocReadLine(inFile, titleStr, 1024);
		AllocReadLine(inFile, offsetStr, 1024);
		AllocReadLine(inFile, tStepStr, 1024);
		AllocReadLine(inFile, unitStr, 1024);

		wxStringTokenizer tkz_titles(titleStr, wxT(",")); //Our format only allows comma csv
		wxStringTokenizer tkz_offsets(offsetStr, wxT(","));
		wxStringTokenizer tkz_tStep(tStepStr, wxT(","));
		wxStringTokenizer tkz_units(unitStr, wxT(","));

		double entry;
		while (tkz_titles.HasMoreTokens()
			&& tkz_offsets.HasMoreTokens()
			&& tkz_tStep.HasMoreTokens()
			&& tkz_units.HasMoreTokens())
		{
			wxDVArrayDataSet *ds = new wxDVArrayDataSet();
			wxString titleToken = tkz_titles.GetNextToken();
			ds->SetSeriesTitle(titleToken.AfterLast('|'));
			tkz_offsets.GetNextToken().ToDouble(&entry);
			timeCounters.push_back(entry);
			tkz_tStep.GetNextToken().ToDouble(&entry);
			ds->SetTimeStep(entry);
			ds->SetUnits(tkz_units.GetNextToken());
			//ds->SetXLabel("Hours since 00:00 Jan 1");
			//ds->SetYLabel(ds->GetSeriesTitle() + " (" + ds->GetUnits() + ")");

			dataSets.push_back(ds);
			groupNames.push_back(titleToken.BeforeLast('|'));
			columns++;
		}
	}
	else
	{
		wxString names, units;
		names = firstLine;

		AllocReadLine(inFile, units, lnchars);

		wxStringTokenizer tkz_names(names, wxT(" \t\r\n")); //String tokenizer breaks up cols.
		wxStringTokenizer tkz_units(units, wxT(" \t\r\n"));

		wxStringTokenizer tkz_names_commas(names, wxT(",")); //If it has commas, use them and nothing else.
		wxStringTokenizer tkz_units_commas(units, wxT(","));
		if (tkz_names_commas.CountTokens() > 1)
		{
			tkz_names = tkz_names_commas;
			tkz_units = tkz_units_commas;
			CommaDelimiters = true;
		}

		int count_names, count_units;
		if ((count_names = tkz_names.CountTokens()) != (count_units = tkz_units_commas.CountTokens()))
		{
			//Check if its a tmy3 with a csv extension.
			//We couldn't catch this earlier because not all csvs are tmy3s.
			if (count_names == 7 && count_units == 68 && fExtension.CmpNoCase("csv") == 0) //Its a tmy3.
			{
				fclose(inFile);
				return ReadWeatherFile(plotWin, filename);
			}
			else
			{
				fclose(inFile);
				return false;
			}
		}

		// Try to read header.
		// If first line contains doubles then ignore title and units.
		// If second line contains doubles ignore units.
		dataSets.reserve(count_names);
		bool firstRowContainsTitles = true;
		bool secondRowContainsUnits = true;
		bool isEnergyPlusOutput = true; // Remains true if first row contains titles, second row contains units, and first value in first row is "Date/Time"
		double entry;

		wxDVArrayDataSet *ds = new wxDVArrayDataSet();
		dataSets.push_back(ds);
		wxString title = tkz_names.GetNextToken();
		timeCounters.push_back(0.5);
		if (IsNumeric(title) || IsDate(title))
		{
			firstRowContainsTitles = false;
			isEnergyPlusOutput = false;
			ds->SetSeriesTitle(wxT("-no name-"));
			groupNames.push_back("");
			title.ToDouble(&entry);
			ds->Append(wxRealPoint(timeCounters[0], entry));
			timeCounters[0] += 1.0;
		}
		else
		{
			ds->SetSeriesTitle(title.AfterLast('|'));
			groupNames.push_back(title.BeforeLast('|'));
			//ds->SetXLabel("Hours since 00:00 Jan 1");
			//ds->SetYLabel(title);
			if (title != "Date/Time")
				isEnergyPlusOutput = false;
		}

		if (firstRowContainsTitles)
		{
			wxString units = tkz_units.GetNextToken();
			if (IsNumeric(units) || IsDate(units))
			{
				secondRowContainsUnits = false;
				ds->SetUnits(wxT("-no units-"));
				units.ToDouble(&entry);
				ds->Append(wxRealPoint(timeCounters[0], entry));
				timeCounters[0] += 1.0;
			}
			else
			{
				isEnergyPlusOutput = false;
				ds->SetUnits(units);
				//ds->SetYLabel(title + " (" + units + ")");
			}
		}
		ds->SetTimeStep(1.0, false);

		columns = 1;
		while (columns < count_names)
		{
			timeCounters.push_back(0.5);
			wxDVArrayDataSet *ds = new wxDVArrayDataSet();
			wxString titleToken = tkz_names.GetNextToken();
			if (isEnergyPlusOutput)
			{
				wxString tt = titleToken.AfterLast('|');
				int fstart = tt.First('[');
				int fend = tt.First(']');
				if (fstart != -1 && fend != -1 && fstart < fend)
				{
					// Convert, e.g., "Variable [Units](Hourly)" to "Variable (Hourly)" with units "Units"
					wxString units = tt.SubString(fstart + 1, fend - 1);
					tt.Replace("[" + units + "]", "");
					ds->SetSeriesTitle(tt);
					ds->SetUnits(units);
				}
				else
				{
					ds->SetSeriesTitle(tt);
				}
			}
			else if (firstRowContainsTitles)
				ds->SetSeriesTitle(titleToken.AfterLast('|'));
			else
			{
				titleToken.ToDouble(&entry);
				ds->SetSeriesTitle(wxT("-no name-"));
				ds->Append(wxRealPoint(timeCounters[columns], entry));
				timeCounters[columns] += 1.0;
			}

			if (secondRowContainsUnits)
			{
				wxString next_units = tkz_units.GetNextToken();
				if (next_units.length() > 0)
					ds->SetUnits(next_units);
				else
					ds->SetUnits(wxT("-no units-"));
			}
			else if (!isEnergyPlusOutput)
			{
				tkz_units.GetNextToken().ToDouble(&entry);
				ds->SetUnits(wxT("-no units-"));
				ds->Append(wxRealPoint(timeCounters[columns], entry));
				timeCounters[columns] += 1.0;
			}
			ds->SetTimeStep(1.0, false);
			dataSets.push_back(ds);
			groupNames.push_back(titleToken.BeforeLast('|'));
			columns++;
		}
	}

	if (prealloc_data > 0)
	{
		// preallocate data
		for (size_t i = 0; i < dataSets.size(); i++)
			dataSets[i]->Alloc(prealloc_data);
	}

	int line = 0, ncol, ndbuf;
	char dblbuf[128], *p, *bp; //Position, buffer position
	char *buf = new char[lnchars];
	char *ret = NULL;
	while (true)
	{
		ret = fgets(buf, lnchars - 1, inFile);
		if (ret == NULL)
			break; //EOF
		if (buf[0] == 'E' && buf[1] == 'O' && buf[2] == 'F')
			break;

		p = buf;
		ncol = 0;
		while (*p && ncol < columns)
		{
			bp = dblbuf;
			ndbuf = 0;
			while (*p && (*p == ' ' || *p == '\t')) p++; // skip white space
			while (*p && *p != ',' && (CommaDelimiters || (*p != '\t' && *p != ' ')) && ++ndbuf < 127) *bp++ = *p++; // read in number
			*bp = '\0'; // terminate string
			if (strlen(dblbuf) > 0)
			{
				dataSets[ncol]->Append(wxRealPoint(timeCounters[ncol], atof(dblbuf))); // convert number and add data point.
				timeCounters[ncol] += dataSets[ncol]->GetTimeStep();
			}
			if (*p) p++; // skip the comma or delimiter
			ncol++;
		}
		line++;
	}

	delete[] buf;

	fclose(inFile);

	//Done reading data; add it to the plotCtrl.

	plotWin->Freeze();
	for (size_t i = 0; i < dataSets.size(); i++)
	{
		dataSets[i]->SetGroupName(groupNames[i].size() > 1 ? groupNames[i] : wxFileNameFromPath(filename));
		plotWin->AddDataSet(dataSets[i], (i == dataSets.size() - 1) /* update_ui ? */);
	}
	plotWin->GetStatisticsTable()->RebuildDataViewCtrl();	//We must do this only after all datasets have been added
	plotWin->Thaw();

	plotWin->ReadState(filename.ToStdString());

	wxLogStatus("Read %i lines of data points.\n", line);
	wxLogDebug("wxDVFileReader::FastRead [ncol=%d nalloc = %d lnchars=%d] = %d msec\n", columns, prealloc_data, lnchars, (int)sw.Time());
	return true;
}

void wxDVFileReader::ReadDataFromCSV(wxDVPlotCtrl *plotWin, const wxString& filename, wxChar separator)
{
	//Deprecated.  Use FastRead.  This method may work if for some reason fastread is broken.
	wxFileInputStream infile(filename);
	if (!infile.IsOk())
	{
		wxMessageBox("Could not read file");
		return;
	}

	wxTextInputStream intext(infile);

	std::vector<wxDVArrayDataSet*> dataSets;
	wxString currentLine = intext.ReadLine();
	if (currentLine.Left(19) != wxT("wxDVFileHeaderVer 1"))
	{
		wxMessageBox("Invalid filetype (header is missing)");
		return;
	}

	currentLine = intext.ReadLine(); // Set Titles From 1st Line
	do
	{
		dataSets.push_back(new wxDVArrayDataSet());
		wxString seriesTitle = currentLine.BeforeFirst(separator);
		dataSets[dataSets.size() - 1]->SetSeriesTitle(seriesTitle);
		if (currentLine.size() == seriesTitle.size())
			currentLine = wxT("");
		else
			currentLine = currentLine.Right(currentLine.size() - seriesTitle.size() - 1);
	} while (currentLine.size() > 0);

	std::vector<double> timeCounters;
	currentLine = intext.ReadLine(); //Offsets from second line.
	for (size_t i = 0; i < dataSets.size(); i++)
	{
		wxString offsetStr = currentLine.BeforeFirst(separator);
		double offsetDouble;
		offsetStr.ToDouble(&offsetDouble);
		timeCounters.push_back(offsetDouble);
		currentLine = currentLine.Right(currentLine.size() - offsetStr.size() - 1);
	}

	currentLine = intext.ReadLine(); // Time Steps from third line.
	for (size_t i = 0; i < dataSets.size(); i++)
	{
		wxString timeStepStr = currentLine.BeforeFirst(separator);
		double timeStepDouble;
		timeStepStr.ToDouble(&timeStepDouble);
		dataSets[i]->SetTimeStep(timeStepDouble);
		currentLine = currentLine.Right(currentLine.size() - timeStepStr.size() - 1);
	}

	currentLine = intext.ReadLine(); //Units from 4th line.
	for (size_t i = 0; i < dataSets.size(); i++)
	{
		wxString units = currentLine.BeforeFirst(separator);
		dataSets[i]->SetUnits(units);
		currentLine = currentLine.Right(currentLine.size() - units.size() - 1);
	}

	wxLogStatus("Reading file for %i data sets \n", dataSets.size());

	//Start Reading Data Points.
	wxString dataStr;
	double dataDouble;
	bool keepGoing = true;
	do
	{
		currentLine = intext.ReadLine();
		for (size_t i = 0; i < dataSets.size(); i++)
		{
			dataStr = currentLine.BeforeFirst(separator);
			if (!dataStr.ToDouble(&dataDouble))
			{
				keepGoing = false;
				break;
			}
			dataSets[i]->Append(wxRealPoint(timeCounters[i], dataDouble));
			timeCounters[i] += dataSets[i]->GetTimeStep();
			currentLine = currentLine.Right(currentLine.size() - dataStr.size() - 1);
		}
	} while (!infile.Eof() && keepGoing);

	//Done reading data; add it to the plotCtrl.
	for (size_t i = 0; i < dataSets.size(); i++)
	{
		dataSets[i]->SetGroupName(wxFileNameFromPath(filename));
		plotWin->AddDataSet(dataSets[i], (i == dataSets.size() - 1));
	}
	plotWin->GetStatisticsTable()->RebuildDataViewCtrl();	//We must do this only after all datasets have been added

	plotWin->ReadState(filename.ToStdString());
}

bool wxDVFileReader::ReadWeatherFile(wxDVPlotCtrl* plotWin, const wxString& filename)
{
	int wfType = GetWeatherFileType(filename);

	// Set up data sets for all of the variables that are going to be read.
	std::vector<wxDVArrayDataSet*> dataSets;
	wxDVArrayDataSet *ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Global Horizontal");
	ds->SetUnits("Wh/m2");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Direct Normal");
	ds->SetUnits("Wh/m2");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Diffuse");
	ds->SetUnits("Wh/m2");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Wind");
	ds->SetUnits("m/s");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Dry Temp");
	ds->SetUnits("'C");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Wet Temp");
	ds->SetUnits("'C");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Relative Humidity");
	ds->SetUnits("%");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Pressure");
	ds->SetUnits("mbar");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("WindDir");
	ds->SetUnits("deg");
	dataSets.push_back(ds);

	ds = new wxDVArrayDataSet();
	ds->SetSeriesTitle("Snow Depth");
	ds->SetUnits("cm");
	dataSets.push_back(ds);

	for (size_t i = 0; i < dataSets.size(); i++)
		dataSets.at(i)->SetTimeStep(1.0); //All have 1 hr tstep.

	//int year, month, day, hour;
	//double gh, dn, df, wind, drytemp, wettemp, relhum, pressure, winddir, snowdepth;

	// Loop over lines in file, reading into data sets array.
	WFHeader head_info;
	FILE *wFile;
	switch (wfType)
	{
	case WF_TM2:
		if (!ParseTM2Header(filename, head_info, &wFile)) { return false; }
		if (!Read8760WFLines(dataSets, wFile, wfType)) { return false; }
		break;

	case WF_TM3:
		if (!ParseTM3Header(filename, head_info, &wFile)) { return false; }
		if (!Read8760WFLines(dataSets, wFile, wfType)) { return false; }
		fclose(wFile);
		break;

	case WF_EPW:
		if (!ParseEPWHeader(filename, head_info, &wFile)) { return false; }
		if (!Read8760WFLines(dataSets, wFile, wfType)) { return false; }
		fclose(wFile);
		break;

	case WF_ERR:
	default:
		return false;
	}

	//Done reading data; add it to the plotCtrl.
	for (size_t i = 0; i < dataSets.size(); i++)
	{
		dataSets[i]->SetGroupName(wxFileNameFromPath(filename));
		plotWin->AddDataSet(dataSets[i], (i == dataSets.size() - 1));
	}
	plotWin->GetStatisticsTable()->RebuildDataViewCtrl();	//We must do this only after all datasets have been added

	plotWin->ReadState(filename.ToStdString());

	return true;
}

bool wxDVFileReader::Read8760WFLines(std::vector<wxDVArrayDataSet*> &dataSets, FILE* infile, int wfType)
{
	int year, month, day, hour;
	double gh, dn, df, wind, drytemp, wettemp, relhum, pressure, winddir, snowdepth;

	for (size_t i = 0; i < 8760; i++)
	{
		if (!ReadWeatherFileLine(infile, wfType, year, month, day, hour, gh, dn, df, wind, drytemp, wettemp,
			relhum, pressure, winddir, snowdepth))
		{
			return false;
		}

		double hr = ((double)i) + 0.5;

		dataSets[0]->Append(wxRealPoint(hr, gh));
		dataSets[1]->Append(wxRealPoint(hr, dn));
		dataSets[2]->Append(wxRealPoint(hr, df));
		dataSets[3]->Append(wxRealPoint(hr, wind));
		dataSets[4]->Append(wxRealPoint(hr, drytemp));
		dataSets[5]->Append(wxRealPoint(hr, wettemp));
		dataSets[6]->Append(wxRealPoint(hr, relhum));
		dataSets[7]->Append(wxRealPoint(hr, pressure));
		dataSets[8]->Append(wxRealPoint(hr, winddir));
		dataSets[9]->Append(wxRealPoint(hr, snowdepth));
	}

	return true;
}

bool wxDVFileReader::ReadSQLFile(wxDVPlotCtrl* plotWin, const wxString& filename)
{
	wxFileName fileName(filename);

	if (!fileName.IsFileReadable()){
		wxMessageBox(wxT("File not readable."), wxT("Error"), wxICON_ERROR);
		return false;
	}

	sqlite3 * db;
	int success = sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READONLY | SQLITE_OPEN_EXCLUSIVE, nullptr);

	if (success == SQLITE_OK) {
		int convertUnits = wxMessageBox(wxT("Would you like to display your Energy+ data in IP units?."), wxT("Units Conversion"), wxYES_NO);

		wxStopWatch sw;
		sw.Start();

		struct DataDictionaryItem
		{
			int recordIndex;
			int envPeriodIndex;
			std::string name;
			std::string keyValue;
			std::string envPeriod;
			std::string reportingFrequency;
			std::string units;
			std::string table;
			unsigned intervalMinutes;
			std::vector<wxDateTime> dateTimes;
			std::vector<double> stdValues;

			DataDictionaryItem(int recordIndex_, int envPeriodIndex_, std::string name_, std::string keyValue_, std::string envPeriod_, std::string reportingFrequency_, std::string units_, std::string table_) :recordIndex(recordIndex_), envPeriodIndex(envPeriodIndex_), name(name_), keyValue(keyValue_), envPeriod(envPeriod_), reportingFrequency(reportingFrequency_), units(units_), table(table_) {}
		};

		std::vector<DataDictionaryItem> dataDictionary;

		if (db)	{
			// Verify that this is an e+ SQL schema
			if (!IsEnergyPlus(db)){
				wxMessageBox(wxT("File not valid Energy+ SQL format."), wxT("Error"), wxICON_ERROR);
			}

			std::string table, name, keyValue, units, rf;

			int dictionaryIndex, code;

			std::stringstream s;
			sqlite3_stmt * sqlStmtPtr;
			std::map<int, std::string> envPeriods;
			std::map<int, std::string>::iterator envPeriodsItr;

			s << "SELECT EnvironmentPeriodIndex, EnvironmentName FROM EnvironmentPeriods";
			sqlite3_prepare_v2(db, s.str().c_str(), -1, &sqlStmtPtr, nullptr);
			code = sqlite3_step(sqlStmtPtr);
			while (code == SQLITE_ROW)
			{
				wxString queryEnvPeriod = ColumnText(sqlite3_column_text(sqlStmtPtr, 1));
				queryEnvPeriod = queryEnvPeriod.Upper();
				envPeriods.insert(std::pair<int, std::string>(sqlite3_column_int(sqlStmtPtr, 0), queryEnvPeriod.ToStdString()));
				code = sqlite3_step(sqlStmtPtr);
			}
			sqlite3_finalize(sqlStmtPtr);

			s.str("");
			s << "SELECT ReportVariableDatadictionaryIndex, VariableName, KeyValue, ReportingFrequency, VariableUnits";
			s << " FROM ReportVariableDatadictionary";
			code = sqlite3_prepare_v2(db, s.str().c_str(), -1, &sqlStmtPtr, nullptr);

			table = "ReportVariableData";

			code = sqlite3_step(sqlStmtPtr);
			while (code == SQLITE_ROW)
			{
				dictionaryIndex = sqlite3_column_int(sqlStmtPtr, 0);
				name = ColumnText(sqlite3_column_text(sqlStmtPtr, 1));
				keyValue = ColumnText(sqlite3_column_text(sqlStmtPtr, 2)).length() ? ColumnText(sqlite3_column_text(sqlStmtPtr, 2)) : "Site";
				rf = ColumnText(sqlite3_column_text(sqlStmtPtr, 3));
				units = ColumnText(sqlite3_column_text(sqlStmtPtr, 4));

				for (envPeriodsItr = envPeriods.begin();
					envPeriodsItr != envPeriods.end();
					++envPeriodsItr)
				{
					wxString queryEnvPeriod = envPeriodsItr->second;
					queryEnvPeriod = queryEnvPeriod.Upper();
					std::string str = keyValue;
					if (queryEnvPeriod.Contains("RUN PERIOD")) {
						str += " Run Period " + rf;
					}
					else {
						str += " Design Day " + rf;
					}
					dataDictionary.push_back(DataDictionaryItem(dictionaryIndex, envPeriodsItr->first, name, str, queryEnvPeriod.ToStdString(), rf, units, table));
				}

				// step to next row
				code = sqlite3_step(sqlStmtPtr);
			}
			sqlite3_finalize(sqlStmtPtr);
		}

		for (size_t i = 0; i < dataDictionary.size(); i++) {
			wxString recordIndexString = wxString::Format(wxT("%d"), (int)dataDictionary[i].recordIndex);
			wxString envPeriodIndexString = wxString::Format(wxT("%d"), (int)dataDictionary[i].envPeriodIndex);

			std::stringstream s;
			s << "SELECT dt.VariableValue, Time.Month, Time.Day, Time.Hour, Time.Minute, Time.Interval FROM ";
			s << dataDictionary[i].table;
			s << " dt INNER JOIN Time ON Time.timeIndex = dt.TimeIndex";
			s << " WHERE ";
			if (dataDictionary[i].table == "ReportMeterData")
			{
				s << " dt.ReportMeterDataDictionaryIndex=";
			}
			else if (dataDictionary[i].table == "ReportVariableData")
			{
				s << " dt.ReportVariableDataDictionaryIndex=";
			}
			s << recordIndexString;
			s << " AND Time.EnvironmentPeriodIndex = ";
			s << envPeriodIndexString;

			sqlite3_stmt * sqlStmtPtr;

			int code = sqlite3_prepare_v2(db, s.str().c_str(), -1, &sqlStmtPtr, nullptr);

			code = sqlite3_step(sqlStmtPtr);
			std::stringstream s2;
			s2 << "SQL Query:" << std::endl;
			s2 << s.str();
			s2 << "Return Code:" << std::endl;
			s2 << code;

			//long cumulativeSeconds = 0;

			std::vector<double> stdValues;
			stdValues.reserve(8760);

			int counter = 0;

			while (code == SQLITE_ROW)
			{
				double value = sqlite3_column_double(sqlStmtPtr, 0);
				stdValues.push_back(value);

				unsigned month = sqlite3_column_int(sqlStmtPtr, 1);
				unsigned day = sqlite3_column_int(sqlStmtPtr, 2);
				unsigned hour = sqlite3_column_int(sqlStmtPtr, 3);
				unsigned minute = sqlite3_column_int(sqlStmtPtr, 4);
				unsigned intervalMinutes = sqlite3_column_int(sqlStmtPtr, 5); // used for run periods

				if (counter++ == 0) {
					dataDictionary[i].intervalMinutes = intervalMinutes;
				}

				if (dataDictionary[i].reportingFrequency == "HVAC System Timestep") {
					wxDateTime dateTime;

					// E+ uses months 1 - 12; wxWidget Month is an enum 0 - 11
					--month;

					if (hour == 24) {
						// EnergyPlus deals in a 00:00:01 -> 24:00:00 instead of
						// 00:00:00 -> 23:59:59 hrs that the real world uses, so
						// we are going to adjust for that

						// For E+ hour = 24, we know E+ minutes must = 0
						assert(minute == 0);

						// Rather than 24:00:00, we want 23:59:59
						dateTime = wxDateTime(day, wxDateTime::Month(month), wxDateTime::Inv_Year, 23, 59, 59, 999);
					}
					else {
						dateTime = wxDateTime(day, wxDateTime::Month(month), wxDateTime::Inv_Year, hour, minute);
					}
					dataDictionary[i].dateTimes.push_back(dateTime);
				}

				// Check for varying intervals when they should remain constant
				if (dataDictionary[i].reportingFrequency != "HVAC System Timestep" && dataDictionary[i].reportingFrequency != "Monthly" && intervalMinutes != dataDictionary[i].intervalMinutes) {
					assert(false);
				}

				//	intervalMinutes notes:
				//	1 : 1 / 60 hour
				//	10 : 1 / 6 hour
				//	15 : 1 / 4 hour
				//	60 : 1 hour
				//	1440 : 24 hours
				//	40320 : 28 days
				//	41760 : 29 days ***** leap year if month == 2 *****
				//	43200 : 30 days
				//	44640 : 31 days

				//if (!firstReportDateTime){
				//	if ((month == 0) || (day == 0)){
				//		// gets called for RunPeriod reports
				//		firstReportDateTime = lastDateTime(false, dataDictionary.envPeriodIndex);
				//	}
				//	else{
				//		// DLM: potential leap year problem
				//		// DLM: get standard time zone?
				//		if (intervalMinutes >= 24 * 60){
				//			// Daily or Monthly
				//			OS_ASSERT(intervalMinutes % (24 * 60) == 0);
				//			firstReportDateTime = openstudio::DateTime(openstudio::Date(month, day), openstudio::Time(1, 0, 0, 0));
				//		}
				//		else {
				//			firstReportDateTime = openstudio::DateTime(openstudio::Date(month, day), openstudio::Time(0, 0, intervalMinutes, 0));
				//		}
				//	}
				//}

				//// Use the new way to create the time series with nonzero first entry
				//cumulativeSeconds += 60 * intervalMinutes;
				//stdSecondsFromFirstReport.push_back(cumulativeSeconds);

				//// check if this interval is same as the others
				//if (isIntervalTimeSeries && !reportingIntervalMinutes){
				//	reportingIntervalMinutes = intervalMinutes;
				//}
				//else if (reportingIntervalMinutes && (reportingIntervalMinutes.get() != intervalMinutes)){
				//	isIntervalTimeSeries = false;
				//	reportingIntervalMinutes.reset();
				//}

				// step to next row
				code = sqlite3_step(sqlStmtPtr);
			}

			if (convertUnits == 2 && dataDictionary[i].units.length()) {
				ConvertUnits(dataDictionary[i].units, stdValues);
			}

			dataDictionary[i].stdValues = stdValues;

			// must finalize to prevent memory leaks
			sqlite3_finalize(sqlStmtPtr);
		}

		// Transfer from dataDictionary into DView
		std::vector<wxDVArrayDataSet*> dataSets;
		std::vector<wxString> groupNames;
		std::vector<double> timeCounters;

		for (size_t i = 0; i < dataDictionary.size(); i++) {
			double timeStep = 1;
			if (dataDictionary[i].reportingFrequency == "Hourly") {
				timeStep = 1;
			}
			else if (dataDictionary[i].reportingFrequency == "Daily") {
				timeStep = 24;
			}
			else if (dataDictionary[i].reportingFrequency == "Monthly") {
				//unsigned intervalMinutes = dataDictionary[i].intervalMinutes;
				//timeStep = dataDictionary[i].intervalMinutes / 60.0;
				//timeStep = 30.41667 * 24; // Average days per month, annually
				timeStep = 1.0;
			}
			else if (dataDictionary[i].reportingFrequency == "HVAC System Timestep") {
				// Note: variable frequency, use 1 minute timestep (E+ minimum) and add "missing" data via interpolation;
				NonuniformTimestepInterpolation(dataDictionary[i].dateTimes, dataDictionary[i].stdValues);
				timeStep = (double)1.0 / 60.0;
			}
			else if (dataDictionary[i].reportingFrequency == "Timestep" || dataDictionary[i].reportingFrequency == "Zone Timestep") {
				// Note: constant frequency, usually 10 or 15 minutes, but always less than 1 hour
				timeStep = dataDictionary[i].intervalMinutes / 60.0;
				wxString stringIntervalMinutes = wxString::Format(wxT("%d"), (unsigned)dataDictionary[i].intervalMinutes);
				dataDictionary[i].keyValue += " " + stringIntervalMinutes + " minutes";
			}
			else {
				// Oops, not handled
				assert(false);
			}

			wxDVArrayDataSet * ds = new wxDVArrayDataSet();
			ds->SetSeriesTitle(dataDictionary[i].keyValue);
			ds->SetTimeStep(timeStep);
			ds->SetUnits(dataDictionary[i].units);
			dataSets.push_back(ds);

			timeCounters.push_back(timeStep);

			groupNames.push_back(dataDictionary[i].name);

			for (size_t j = 0; j < dataDictionary[i].stdValues.size(); j++) {
				dataSets[i]->Append(wxRealPoint(timeCounters[i], dataDictionary[i].stdValues[j])); // convert number and add data point.
				timeCounters[i] += dataSets[i]->GetTimeStep(); // convert number and add data point.
			}
		}

		// Done reading data; add it to the plotCtrl.
		plotWin->Freeze();
		for (size_t i = 0; i < dataSets.size(); i++)
		{
			dataSets[i]->SetGroupName(groupNames[i].size() > 1 ? groupNames[i] : wxFileNameFromPath(filename));
			plotWin->AddDataSet(dataSets[i], (i == dataSets.size() - 1) /* update_ui ? */);
		}
		plotWin->GetStatisticsTable()->RebuildDataViewCtrl();	//We must do this only after all datasets have been added
		plotWin->Thaw();

		plotWin->ReadState(filename.ToStdString());

		//wxLogStatus("Read %i lines of data points.\n", line);
		//wxLogDebug("wxDVFileReader::ReadSQLFile [ncol=%d nalloc = %d lnchars=%d] = %d msec\n", columns, prealloc_data, lnchars, (int)sw.Time());
		return true;
	}
	else {
		sqlite3_close(db);

		wxString stringSuccess = wxString::Format(wxT("%d"), (int)success);

		wxString errorMsg("The following error code was returned while trying to read file ");
		errorMsg += filename + ": " + stringSuccess;
		wxMessageBox(errorMsg, wxT("Error"), wxICON_ERROR);

		return false;
	}
}

void wxDVFileReader::NonuniformTimestepInterpolation(const std::vector<wxDateTime> & times, std::vector<double> & values)
{
	assert(times.size() == values.size());

	std::vector<double> valuesByMinute;

	wxTimeSpan timeSpan = times.at(times.size() - 1) - times.at(0);

	for (size_t i = 1; i < times.size(); i++){
		timeSpan = times[i] - times[i - 1];
		wxLongLong intervalInSeconds = timeSpan.GetSeconds();
		int interval = static_cast<int>(intervalInSeconds.ToDouble()) / 60;
		int remainder = static_cast<int>(intervalInSeconds.ToDouble()) % 60;
		if (remainder > 0) {
			// This is necessary to account for the 1 millisecond time shift required
			// previously to convert Energy+'s 1 - 24 hour clock to 0 - 23 hour
			interval += 1;
		}

		// E+ interval must be greater than 0 and less than 60
		if (interval <= 0) {
			// Converting Energy+ to 0 - 23 hour can have corner case failures
			interval += 60;
		}
		else if (interval >= 60) {
			interval -= 60;
		}
		if (interval <= 0 || interval >= 60) {
			// Should not get here
			assert(false);
			continue;
		}

		double valueDeltaPerMinute = (values[i] - values[i - 1]) / interval;
		for (int j = 0; j < interval; j++){
			double value = values.at(i - 1) + valueDeltaPerMinute * j;
			valuesByMinute.push_back(value);
		}
	}

	//int size = valuesByMinute.size();
	//assert(valuesByMinute.size() == static_cast<unsigned>(expectedVectorLength));

	values = valuesByMinute;
}

bool wxDVFileReader::IsEnergyPlus(sqlite3 * db)
{
	bool success = false;
	if (db) {
		wxString eP("EnergyPLus");
		sqlite3_stmt* sqlStmtPtr;
		sqlite3_prepare_v2(db, "SELECT EnergyPlusVersion FROM Simulations", -1, &sqlStmtPtr, nullptr);
		int code = sqlite3_step(sqlStmtPtr);
		if (code == SQLITE_ROW) {
			wxString version_line = ColumnText(sqlite3_column_text(sqlStmtPtr, 0));
			success = (version_line.Find(eP) != 0);
		}
		sqlite3_finalize(sqlStmtPtr);
	}
	return success;
}

wxString wxDVFileReader::ColumnText(const unsigned char* column)
{
	return wxString(reinterpret_cast<const char*>(column));
}

void wxDVFileReader::ExecAndThrowOnError(const std::string &t_stmt, sqlite3 * db)
{
	char * err = nullptr;
	if (sqlite3_exec(db, t_stmt.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
		std::string errstr;
		if (err) {
			errstr = err;
			sqlite3_free(err);
		}
		throw std::runtime_error("Error executing SQL statement: " + t_stmt + " " + errstr);
	}
}

bool wxDVFileReader::IsNumeric(wxString stringToCheck)
{
	double entry;

	if (stringToCheck.ToDouble(&entry)) { return true; }

	return false;
}

bool wxDVFileReader::IsDate(wxString stringToCheck)
{
	char c;
	size_t AMPMposition = 0;
	wxString dummy = stringToCheck.Trim().Trim(false);

	dummy.Replace("\t", " ");
	dummy.Replace("\r", " ");
	dummy.Replace("\n", " ");

	while (dummy.Contains("  "))
	{
		dummy.Replace("  ", " ");
	}

	dummy.Replace(" /", "/");
	dummy.Replace("/ ", "/");
	dummy.Replace(" -", "-");
	dummy.Replace("- ", "-");
	dummy.Replace(" :", ":");
	dummy.Replace(": ", ":");
	dummy.Replace("p", "P");
	dummy.Replace("a", "A");
	dummy.Replace("m", "M");
	dummy.Replace(" P", "P");
	dummy.Replace("P ", "P");
	dummy.Replace(" A", "A");
	dummy.Replace("A ", "A");
	dummy.Replace(" M", "M");
	dummy.Replace("M ", "M");

	if (dummy.length() > 22) { return false; }

	std::string str = dummy.ToStdString();

	for (size_t i = 0; i < str.length(); i++)
	{
		c = str.at(i);

		if (AMPMposition == 0 && (c == 'a' || c == 'p' || c == 'm' || c == 'A' || c == 'P' || c == 'M')) { AMPMposition = i; }
		if (AMPMposition > 0 && i > AMPMposition + 1) { return false; }

		if (i == 0 && c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9') { return false; }
		if (i >= 1 && i <= 2 && c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9' && c != '/' && c != '-') { return false; }
		if (i >= 3 && i <= 4 && c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9' && c != '/' && c != '-' && c != ' ') { return false; }
		if (i >= 5 && i <= 8 && c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9' && c != '/' && c != '-' && c != ' ' && c != ':') { return false; }
		if (i >= 9 && c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9' && c != '/' && c != '-' && c != ' ' && c != ':' && c != 'A' && c != 'P' && c != 'M') { return false; }
	}

	return true;
}

// Conversion factors from Energy+.idd
void wxDVFileReader::InitUnitConversions()
{
	// NOTE: tuple format (SI unit string, IP unit string, SI to IP conversion factor)
	m_unitConversions.emplace_back("$/(m3/s)", "$/(ft3/min)", 0.000472000059660808);
	m_unitConversions.emplace_back("$/(W/K)", "$/(Btu/h-F)", 0.52667614683731);
	m_unitConversions.emplace_back("$/kW", "$/(kBtuh/h)", 0.293083235638921);
	m_unitConversions.emplace_back("$/m2", "$/ft2", 0.0928939733269818);
	m_unitConversions.emplace_back("$/m3", "$/ft3", 0.0283127014102352);
	m_unitConversions.emplace_back("(kg/s)/W", "(lbm/sec)/(Btu/hr)", 0.646078115385742);
	m_unitConversions.emplace_back("1/K", "1/F", 0.555555555555556);
	m_unitConversions.emplace_back("1/m", "1/ft", 0.3048);
	m_unitConversions.emplace_back("A/K", "A/F", 0.555555555555556);
	m_unitConversions.emplace_back("cm", "in", 0.3937);
	m_unitConversions.emplace_back("cm2", "inch2", 0.15500031000062);
	m_unitConversions.emplace_back("deltaC", "deltaF", 1.8);
	m_unitConversions.emplace_back("deltaC/hr", "deltaF/hr", 1.8);
	m_unitConversions.emplace_back("deltaJ/kg", "deltaBtu/lb", 0.0004299);
	m_unitConversions.emplace_back("g/GJ", "lb/MWh", 0.00793664091373665);
	m_unitConversions.emplace_back("g/kg", "grains/lb", 7);
	m_unitConversions.emplace_back("g/MJ", "lb/MWh", 7.93664091373665);
	m_unitConversions.emplace_back("g/mol", "lb/mol", 0.0022046);
	m_unitConversions.emplace_back("g/m-s", "lb/ft-s", 0.000671968949659);
	m_unitConversions.emplace_back("g/m-s-K", "lb/ft-s-F", 0.000373574867724868);
	m_unitConversions.emplace_back("GJ", "ton-hrs", 78.9889415481832);
	m_unitConversions.emplace_back("J", "Wh", 0.000277777777777778);
	m_unitConversions.emplace_back("J/K", "Btu/F", 526.565);
	m_unitConversions.emplace_back("J/kg", "Btu/lb", 0.00042986);
	m_unitConversions.emplace_back("J/kg-K", "Btu/lb-F", 0.000239005736137667);
	m_unitConversions.emplace_back("J/kg-K2", "Btu/lb-F2", 0.000132889924714692);
	m_unitConversions.emplace_back("J/kg-K3", "Btu/lb-F3", 7.38277359526066E-05);
	m_unitConversions.emplace_back("J/m2-K", "Btu/ft2-F", 4.89224766847393E-05);
	m_unitConversions.emplace_back("J/m3", "Btu/ft3", 2.68096514745308E-05);
	m_unitConversions.emplace_back("J/m3-K", "Btu/ft3-F", 1.49237004739337E-05);
	m_unitConversions.emplace_back("K", "R", 1.8);
	m_unitConversions.emplace_back("K/m", "F/ft", 0.54861322767449);
	m_unitConversions.emplace_back("kg", "lb", 2.2046);
	m_unitConversions.emplace_back("kg/J", "lb/Btu", 2325.83774250441);
	m_unitConversions.emplace_back("kg/kg-K", "lb/lb-F", 0.555555555555556);
	m_unitConversions.emplace_back("kg/m", "lb/ft", 0.67196893069637);
	m_unitConversions.emplace_back("kg/m2", "lb/ft2", 0.204794053596664);
	m_unitConversions.emplace_back("kg/m3", "lb/ft3", 0.062428);
	m_unitConversions.emplace_back("kg/m-s", "lb/ft-s", 0.67196893069637);
	m_unitConversions.emplace_back("kg/m-s-K", "lb/ft-s-F", 0.373316072609094);
	m_unitConversions.emplace_back("kg/m-s-K2", "lb/ft-s-F2", 0.207397818116164);
	m_unitConversions.emplace_back("kg/Pa-s-m2", "lb/psi-s-ft2", 1412.00523459398);
	m_unitConversions.emplace_back("kg/s", "lb/s", 2.20462247603796);
	m_unitConversions.emplace_back("kg/s2", "lb/s2", 2.2046);
	m_unitConversions.emplace_back("kg/s-m", "lb/s-ft", 0.67196893069637);
	m_unitConversions.emplace_back("kJ/kg", "Btu/lb", 0.429925);
	m_unitConversions.emplace_back("kPa", "psi", 0.145038);
	m_unitConversions.emplace_back("L/day", "pint/day", 2.11337629827348);
	m_unitConversions.emplace_back("L/GJ", "gal/kWh", 0.000951022349025202);
	m_unitConversions.emplace_back("L/kWh", "pint/kWh", 2.11337629827348);
	m_unitConversions.emplace_back("L/MJ", "gal/kWh", 0.951022349025202);
	m_unitConversions.emplace_back("lux", "foot-candles", 0.092902267);
	m_unitConversions.emplace_back("m", "ft", 3.28083989501312);
	m_unitConversions.emplace_back("m/hr", "ft/hr", 3.28083989501312);
	m_unitConversions.emplace_back("m/s", "ft/min", 196.850393700787);
	m_unitConversions.emplace_back("m/s", "miles/hr", 2.2369362920544);
	m_unitConversions.emplace_back("m/yr", "inch/yr", 39.3700787401575);
	m_unitConversions.emplace_back("m2", "ft2", 10.7639104167097);
	m_unitConversions.emplace_back("m2/m", "ft2/ft", 3.28083989501312);
	m_unitConversions.emplace_back("m2/person", "ft2/person", 10.764961);
	m_unitConversions.emplace_back("m2/s", "ft2/s", 10.7639104167097);
	m_unitConversions.emplace_back("m2-K/W", "ft2-F-hr/Btu", 5.678263);
	m_unitConversions.emplace_back("m3", "ft3", 35.3146667214886);
	m_unitConversions.emplace_back("m3", "gal", 264.172037284185);
	m_unitConversions.emplace_back("m3/GJ", "ft3/MWh", 127.13292);
	m_unitConversions.emplace_back("m3/hr", "ft3/hr", 35.3146667214886);
	m_unitConversions.emplace_back("m3/hr-m2", "ft3/hr-ft2", 3.28083989501312);
	m_unitConversions.emplace_back("m3/hr-person", "ft3/hr-person", 35.3146667214886);
	m_unitConversions.emplace_back("m3/kg", "ft3/lb", 16.018);
	m_unitConversions.emplace_back("m3/m2", "ft3/ft2", 3.28083989501312);
	m_unitConversions.emplace_back("m3/MJ", "ft3/kWh", 127.13292);
	m_unitConversions.emplace_back("m3/person", "ft3/person", 35.3146667214886);
	m_unitConversions.emplace_back("m3/s", "ft3/min", 2118.88000328931);
	m_unitConversions.emplace_back("m3/s-m", "ft3/min-ft", 645.89);
	m_unitConversions.emplace_back("m3/s-m2", "ft3/min-ft2", 196.85);
	m_unitConversions.emplace_back("m3/s-person", "ft3/min-person", 2118.6438);
	m_unitConversions.emplace_back("m3/s-W", "(ft3/min)/(Btu/h)", 621.099127332943);
	m_unitConversions.emplace_back("N-m", "lbf-in", 8.85074900525547);
	m_unitConversions.emplace_back("N-s/m2", "lbf-s/ft2", 0.0208857913669065);
	m_unitConversions.emplace_back("Pa", "psi", 0.000145037743897283);
	m_unitConversions.emplace_back("percent/K", "percent/F", 0.555555555555556);
	m_unitConversions.emplace_back("person/m2", "person/ft2", 0.0928939733269818);
	m_unitConversions.emplace_back("s/m", "s/ft", 0.3048);
	m_unitConversions.emplace_back("V/K", "V/F", 0.555555555555556);
	m_unitConversions.emplace_back("W", "Btu/h", 3.4121412858518);
	m_unitConversions.emplace_back("W/((m3/s)-Pa)", "W/((gal/min)-ftH20)", 0.188582274697355);
	m_unitConversions.emplace_back("W/(m3/s)", "W/(ft3/min)", 0.0004719475);
	m_unitConversions.emplace_back("W/K", "Btu/h-F", 1.89563404769544);
	m_unitConversions.emplace_back("W/m", "Btu/h-ft", 1.04072);
	m_unitConversions.emplace_back("W/m2", "Btu/h-ft2", 0.316957210776545);
	m_unitConversions.emplace_back("W/m2", "W/ft2", 0.09290304);
	m_unitConversions.emplace_back("W/m2-K", "Btu/h-ft2-F", 0.176110194261872);
	m_unitConversions.emplace_back("W/m2-K2", "Btu/h-ft2-F2", 0.097826);
	m_unitConversions.emplace_back("W/m-K", "Btu-in/h-ft2-F", 6.93481276005548);
	m_unitConversions.emplace_back("W/m-K2", "Btu/h-F2-ft", 0.321418310071648);
	m_unitConversions.emplace_back("W/m-K3", "Btu/h-F3-ft", 0.178565727817582);
	m_unitConversions.emplace_back("W/person", "Btu/h-person", 3.4121412858518);
	m_unitConversions.emplace_back("kPa", "inHg", 0.29523);
	m_unitConversions.emplace_back("m", "in", 39.3700787401575);
	m_unitConversions.emplace_back("m3/hr", "gal/hr", 264.172037284185);
	m_unitConversions.emplace_back("m3/hr-m2", "gal/hr-ft2", 24.5423853466941);
	m_unitConversions.emplace_back("m3/hr-person", "gal/hr-person", 264.172037284185);
	m_unitConversions.emplace_back("m3/m2", "gal/ft2", 24.5423853466941);
	m_unitConversions.emplace_back("m3/person", "gal/person", 264.172037284185);
	m_unitConversions.emplace_back("m3/s", "gal/min", 15850.3222370511);
	m_unitConversions.emplace_back("m3/s-m", "gal/min-ft", 4831.17821785317);
	m_unitConversions.emplace_back("m3/s-W", "(gal/min)/(Btu/h)", 4645.27137336702);
	m_unitConversions.emplace_back("Pa", "ftH2O", 0.00033455);
	m_unitConversions.emplace_back("Pa", "inH2O", 0.00401463);
	m_unitConversions.emplace_back("Pa", "inHg", 0.00029613);
	m_unitConversions.emplace_back("Pa", "Pa", 1);
	m_unitConversions.emplace_back("W", "W", 1);
	m_unitConversions.emplace_back("W/(m3/s)", "W/(gal/min)", 0.0000630902);
	m_unitConversions.emplace_back("W/m2", "W/m2", 1);
	m_unitConversions.emplace_back("W/m-K", "Btu/h-ft-F", 0.577796066000163);
	m_unitConversions.emplace_back("W/person", "W/person", 1);
	m_unitConversions.emplace_back("%", "%", 1);
	m_unitConversions.emplace_back("ach", "ach", 1);
	m_unitConversions.emplace_back("ACH", "ACH", 1);
}

bool wxDVFileReader::ConvertUnits(std::string & units, std::vector<double> & values, bool convertSIToIP)
{
	if (m_unitConversions.size() == 0) InitUnitConversions();

	bool success = false;
	double multiplier = 0;

	// Remove whitespace
	std::string::iterator end_pos = std::remove(units.begin(), units.end(), ' ');
	units.erase(end_pos, units.end());

	wxString errors("The following units failed to be converted: ");
	unsigned startSize = errors.size();

	if (convertSIToIP) {
		auto it = find_if(begin(m_unitConversions), end(m_unitConversions), [units](decltype(*begin(m_unitConversions)) e) {
			return get<0>(e) == units;
		});
		if (it != end(m_unitConversions)) {
			units = get<1>(*it);
			multiplier = get<2>(*it);
			for (size_t i = 0; i < values.size(); i++) {
				values.at(i) *= multiplier;
			}
			success = true;
		}
		else if (units == "C") {
			units = "F";
			for (size_t i = 0; i < values.size(); i++) {
				values.at(i) = values.at(i) * 9 / 5 + 32;
			}
			success = true;
		}
		else {
			errors += units + ", ";
		}
	}
	else{
		auto it = find_if(begin(m_unitConversions), end(m_unitConversions), [units](decltype(*begin(m_unitConversions)) e) {
			return get<1>(e) == units;
		});
		if (it != end(m_unitConversions)) {
			units = get<0>(*it);
			multiplier *= 1 / get<2>(*it);
			for (size_t i = 0; i < values.size(); i++) {
				values.at(i) *= multiplier;
			}
			success = true;
		}
		else if (units == "F") {
			units = "C";
			//value = (value - 32) * 5 / 9;
			for (size_t i = 0; i < values.size(); i++) {
				values.at(i) = (values.at(i) - 32) * 5 / 9;
			}
			success = true;
		}
		else {
			errors += units + ", ";
		}
	}

	if (errors.size() > startSize) {
		errors.RemoveLast(2); // remove last ", "
		wxMessageBox(errors, wxT("Units Conversion Error"), wxICON_INFORMATION);
	}

	return success;
}