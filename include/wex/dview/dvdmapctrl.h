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

#ifndef __DVMapCtrl_h
#define __DVMapCtrl_h

/*
 * wxDVDmapCtrl.h
 *
 * This control contains a DMap plot surface and the controls
 * necessary to select a channel.  This control handles data sets and laying things out.
 * The plot surface takes care of the actual drawing.
 */

#include <wx/panel.h>

#include <vector>

class wxCheckBox;
class wxChoice;
class wxDVDMapPlot;
class wxDVSelectionListCtrl;
class wxDVTimeSeriesDataSet;
class wxPLColourMap;
class wxPLLinearAxis;
class wxPLPlotCtrl;
class wxPLTimeAxis;
class wxScrollBar;
class wxSearchCtrl;
class wxTextCtrl;
class wxTimer;
class wxTimerEvent;

class wxDVDMapCtrl : public wxPanel
{
public:
	wxDVDMapCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~wxDVDMapCtrl();

	//Does not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet* d, bool update_ui);
	void RemoveDataSet(wxDVTimeSeriesDataSet* d); //releases ownership, does not delete.
	void RemoveAllDataSets(); //clear all data sets from graphs and memory. (delete plottables.  Never took ownership.

	wxString GetCurrentDataName();
	bool SetCurrentDataName(const wxString& name);
	wxPLColourMap* GetCurrentColourMap();
	void SetColourMapName(const wxString& name);
	void SetReverseColours(bool b);
	bool IsReversedColours();
	void SelectDataSetAtIndex(int index);
	int GetNumberOfSelections();

	void ChangePlotDataTo(wxDVTimeSeriesDataSet* d);

	//These just set the value.
	//They do not check that it is within the limits or adjust it at all.
	double GetZMin();
	void SetZMin(double min);
	double GetZMax();
	void SetZMax(double max);
	double GetXMin();
	void SetXMin(double min);
	double GetXMax();
	void SetXMax(double max);
	double GetYMin();
	void SetYMin(double min);
	double GetYMax();
	void SetYMax(double max);
	void SetXViewRange(double min, double max);
	void SetYViewRange(double min, double max);
	void SetViewWindow(double xMin, double yMin, double xMax, double yMax);
	void PanXByPercent(double p);
	void PanYByPercent(double p);

	//These functions will move the bounds if they need to be moved.
	void KeepXBoundsWithinLimits(double* xMin, double* xMax);
	void KeepYBoundsWithinLimits(double* yMin, double* yMax);
	void KeepBoundsWithinLimits(double*xMin, double* yMin, double* xMax, double* yMax);
	void KeepNewBoundsWithinLimits(double* newMin, double* newMax);

	//Keep in limits and round to nice number.
	void MakeXBoundsNice(double* xMin, double* xMax);
	void MakeYBoundsNice(double* yMin, double* yMax);
	void MakeAllBoundsNice(double* xMin, double* yMin, double* xMax, double* yMax);

	void UpdateScrollbarPosition();
	void UpdateXScrollbarPosition();
	void UpdateYScrollbarPosition();

	void ZoomFactorAndUpdate(double factor, double shiftPercent = 0.0);

	void ReadState(std::string filename);
	void WriteState(std::string filename);

	/*Event Handlers*/
	void OnDataChannelSelection(wxCommandEvent& e);
	void OnSearch(wxCommandEvent& e);

	void OnColourMapSelection(wxCommandEvent& e);
	void OnColourMapMinChanged(wxCommandEvent& e);
	void OnColourMapMaxChanged(wxCommandEvent& e);
	void OnZoomIn(wxCommandEvent& e);
	void OnZoomOut(wxCommandEvent& e);
	void OnZoomFit(wxCommandEvent& e);
	void OnHighlight(wxCommandEvent& e);
	void OnMouseWheel(wxMouseEvent& e);
	void OnScroll(wxScrollEvent& e);
	void OnScrollLineUp(wxScrollEvent& e);
	void OnScrollLineDown(wxScrollEvent& e);
	void OnScrollPageUp(wxScrollEvent& e);
	void OnScrollPageDown(wxScrollEvent& e);
	void OnYScroll(wxScrollEvent& e);
	void OnYScrollLineUp(wxScrollEvent& e);
	void OnYScrollLineDown(wxScrollEvent& e);
	void OnYScrollPageUp(wxScrollEvent& e);
	void OnYScrollPageDown(wxScrollEvent& e);

	void OnResetColourMapMinMax(wxCommandEvent& e);
	void OnReverseColours(wxCommandEvent &);

	void Invalidate(); // recalculate and rerender plot

private:
	void ColourMapSelection();
	void ReverseColours();
	void ColourMapMinChanged();
	void ColourMapMaxChanged();
	void wxDVDMapCtrl::OnTimer(wxTimerEvent&);

	wxDVSelectionListCtrl *m_selector;
	wxSearchCtrl *m_srchCtrl;
	wxChoice *m_colourMapSelector;
	wxCheckBox *m_reverseColours;

	wxTextCtrl *m_minTextBox;
	wxTextCtrl *m_maxTextBox;

	std::vector<wxDVTimeSeriesDataSet*> m_dataSets;
	wxDVTimeSeriesDataSet* m_currentlyShownDataSet;

	wxPLPlotCtrl *m_plotSurface;
	wxPLColourMap *m_colourMap;
	wxDVDMapPlot *m_dmap;
	wxPLTimeAxis *m_xAxis; // axes are owned by plot surface
	wxPLLinearAxis *m_yAxis;

	wxScrollBar *m_xGraphScroller;
	wxScrollBar *m_yGraphScroller;

	std::string m_filename;

	/*
	double mXWorldMin, mXWorldMax;
	double mYWorldMin, mYWorldMax;

	double mOrigXMin, mOrigXMax, mOrigYMin, mOrigYMax;
	*/

	wxTimer * m_timer;

	double m_xAxixWorldMin;
	double m_xAxixWorldMax;

	DECLARE_EVENT_TABLE()
};

#endif
