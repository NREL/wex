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

class wxDVSelectionListCtrl : public wxScrolledWindow, public wxDVAutoColourAssigner
{
public:
	wxDVSelectionListCtrl( wxWindow* parent, wxWindowID id, 
		int num_cols, bool radio_first_col,
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize );
	virtual ~wxDVSelectionListCtrl();

	int Append(const wxString& name, const wxString& group); // returns row index
	void RemoveAt(int row);
	void RemoveAll();
	int Length(); //This is number of rows.

	void ClearColumn(int col);
	int SelectRowWithNameInCol(const wxString& name, int col = 0); //Returns row.
	void SelectRowInCol(int row, int col = 0);
	void Enable(int row, int col, bool enable);
	bool IsRowSelected(int row, int start_col = 0);
	bool IsSelected(int row, int col);
	wxString GetRowLabel(int row);
	wxString GetSelectedNamesInCol(int col = 0);
	std::vector<int> GetSelectionsInCol( int col = 0 );
	int GetNumSelected( int col = 0 );

	void GetLastEventInfo(int* row, int* col, bool* isNowChecked);

protected:	
	virtual wxSize DoGetBestSize() const;

private:
	static const int NMAXCOLS  = 4;
	
	int m_numCols;
	bool m_radioFirstCol;
	static const int m_itemHeight = 18;
	static const int m_boxSize = 12;
	static const int m_xOffset = 8;
	static const int m_yOffset = 4;
	
	struct row_item
	{
		wxColour color;
		wxString label;
		wxString group;
		bool value[NMAXCOLS];
		bool enable[NMAXCOLS];
		wxRect geom[NMAXCOLS]; // filled in by renderer
	};

	std::vector<row_item*> m_itemList;
	std::vector<row_item*> m_orderedItems;

	int m_lastEventRow, m_lastEventCol;
	bool m_lastEventValue;

	wxSize m_bestSize;

	void FreeRowItems();

	void Organize();
	void RecalculateBestSize();
	void ResetScrollbars();
	void Invalidate();
	void OnResize(wxSizeEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnErase(wxEraseEvent &evt);
	void OnLeftDown(wxMouseEvent &evt);
	void OnMouseMove(wxMouseEvent &evt);
	void OnLeave(wxMouseEvent &evt);

	void HandleRadio( int r, int c );
	void HandleLineColour(int row);
	
	DECLARE_EVENT_TABLE()
};

#endif

