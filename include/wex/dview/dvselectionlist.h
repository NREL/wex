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

#ifndef __DV_SelectionListCtrl_h
#define __DV_SelectionListCtrl_h

/*
 * wxDVDataSelectionList
 *
 * This list provides a list of dataset names with an arbitrary number of check boxes.
 * This is used by other controls to turn data set visibility on or off.
 *
 * Check boxes are stored in a 1D array, but we use modular arithmetic to let this 1D array hold
 * "rows" and "columns" of an arbitrary length.
 *
 * It makes sense to also have this class take care of assigning line colors to data sets.
 */

#include <vector>
#include <wx/wx.h>
#include "wex/dview/dvautocolourassigner.h"

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_DVSELECTIONLIST, 0)
END_DECLARE_EVENT_TYPES()

#define EVT_DVSELECTIONLIST(id, func) EVT_COMMAND(id, wxEVT_DVSELECTIONLIST, func)

#define wxDVSEL_RADIO_FIRST_COL 0x01
#define wxDVSEL_NO_COLOURS 0x02
#define wxDVSEL_RADIO_ALL_COL 0x03

class wxDVSelectionListCtrl : public wxScrolledWindow, public wxDVAutoColourAssigner
{
public:
	wxDVSelectionListCtrl(wxWindow* parent, wxWindowID id, int num_cols,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, unsigned long style = 0);
	virtual ~wxDVSelectionListCtrl();

	int Append(const wxString &name, const wxString &group = wxEmptyString); // returns row index
	int AppendNoUpdate(const wxString &name, const wxString &group);
	void Append(const wxArrayString &names, const wxString &group = wxEmptyString);  // add a whole list with the same group

	void RemoveAt(int row);
	void RemoveAll();
	int Length(); //This is number of rows.

	void ClearColumn(int col);
	int SelectRowWithNameInCol(const wxString& name, int col = 0); //Returns row.
	void SelectRowInCol(int row, int col = 0, bool value = true);
	void Enable(int row, int col, bool enable);
	bool IsRowSelected(int row, int start_col = 0);
	bool IsSelected(int row, int col);
	wxString GetRowLabel(int row);
	wxString GetRowLabelWithGroup(int row);
	wxString GetSelectedNamesInCol(int col = 0);
	std::vector<int> GetSelectionsInCol(int col = 0);
	int GetNumberOfSelections();
	int GetNumSelected(int col = 0);
	int GetUnsortedRowIndex(int SortedIndex = 0);	//Returns the raw index of a row (index of the underlying data element) when passed the sorted index (the displayed position)

	void Filter(const wxString &search);

	void ExpandAll();
	void ExpandSelections();
	void CollapseAll();

	void SetUngroupedLabel(const wxString &l);

	void GetLastEventInfo(int* row, int* col, bool* isNowChecked);

	void Organize();
	void Invalidate();
protected:
	virtual wxSize DoGetBestSize() const;

private:
	static const int NMAXCOLS = 4;

	unsigned long m_style;
	int m_numCols;
	int m_itemHeight;
	int m_groupHeight;
	int m_boxSize;
	int m_xOffset;

	struct row_item
	{
		wxColour color;
		wxString label;
		wxString group;
		bool value[NMAXCOLS];
		bool enable[NMAXCOLS];
		wxRect geom[NMAXCOLS]; // filled in by renderer
		int row_index;
		bool shown;
	};

	std::vector<row_item*> m_itemList;

	struct group
	{
		group(const wxString &l) : label(l), others(false) { }

		wxString label;
		wxRect geom;
		std::vector<row_item*> items;
		bool others;
	};

	std::vector<group> m_groups;
	wxArrayString m_collapsedGroups;
	wxString m_ungroupedLabel;
	int FindGroup(const wxString &label);

	int m_lastEventRow, m_lastEventCol;
	bool m_lastEventValue;

	wxSize m_bestSize;

	void FreeRowItems();

	void RecalculateBestSize();
	void ResetScrollbars();
	void OnResize(wxSizeEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnErase(wxEraseEvent &evt);
	void OnLeftDown(wxMouseEvent &evt);
	void OnRightDown(wxMouseEvent &evt);
	void OnMouseMove(wxMouseEvent &evt);
	void OnLeave(wxMouseEvent &evt);
	void OnPopupMenu(wxCommandEvent &);

	void HandleRadio(int r, int c);
	void HandleLineColour(int row);

	DECLARE_EVENT_TABLE()
};

#endif
