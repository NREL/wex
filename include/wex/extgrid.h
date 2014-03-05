#ifndef __wxExtGridctrl_h
#define __wxExtGridctrl_h

#include <wx/grid.h>

class wxExtGridCellAttrProvider : public wxGridCellAttrProvider
{
public:
    wxExtGridCellAttrProvider(bool highlight_r0c0=false, bool hide_00=false);
    virtual ~wxExtGridCellAttrProvider();

    virtual wxGridCellAttr *GetAttr(int row, int col,
                                    wxGridCellAttr::wxAttrKind  kind) const;

private:
    wxGridCellAttr *m_attrForOddRows;
	wxGridCellAttr *m_attrRow0Col0;
	wxGridCellAttr *m_attrCell00;
};


class wxExtGridCtrl : public wxGrid
{
public:
	wxExtGridCtrl(wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &sz = wxDefaultSize);

	void EnableCopyPaste(bool b);
	bool IsCopyPasteEnabled();

	void EnablePasteEvent(bool b); // sent as GRID_CELL_CHANGE with GetRow() = -1 and GetCol() = -1
	void GetSelRange(int *top, int *bottom, int *left, int *right);
	void GetLastSelRange(int *top, int *bottom, int *left, int *right);
	void ResetLastSelRange();

	void Copy(bool all=false, bool with_headers=false);
	void Paste(bool all=false);

	void ResizeGrid(int nr, int nc);
	
private:
	void OnGridKey( wxKeyEvent &evt );
	void OnGridCellChange(wxGridEvent &evt);
	void OnGridCellSelect(wxGridEvent &evt);
	void OnGridEditorHidden(wxGridEvent &evt);
	void OnGridEditorShown(wxGridEvent &evt);
	void OnGridRangeSelect(wxGridRangeSelectEvent &evt);
	void OnGridLabelClick(wxGridEvent &evt);

	int mSelTopRow, mSelBottomRow;
	int mSelLeftCol, mSelRightCol;
	bool bSkipSelect;
	bool bCopyPaste;
	bool bSendPasteEvent;
	int mLastSelTopRow, mLastSelBottomRow;
	int mLastSelLeftCol, mLastSelRightCol;

	DECLARE_EVENT_TABLE()
};


#endif
