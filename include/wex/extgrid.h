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

#ifndef __wxExtGridctrl_h
#define __wxExtGridctrl_h

#include <wx/grid.h>

class wxExtGridCellAttrProvider : public wxGridCellAttrProvider
{
public:
	wxExtGridCellAttrProvider(bool highlight_r0 = false, bool hide_00 = false, bool highlight_c0 = false);
	virtual ~wxExtGridCellAttrProvider();

	virtual wxGridCellAttr *GetAttr(int row, int col,
		wxGridCellAttr::wxAttrKind  kind) const;

private:
	wxGridCellAttr *m_attrForOddRows;
	wxGridCellAttr *m_attrRow0;
	wxGridCellAttr *m_attrCol0;
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
	size_t NumCellsSelected() const;

	void Copy(bool all = false, bool with_headers = false);

	enum PasteMode { PASTE_CURSOR, PASTE_ALL, PASTE_ALL_RESIZE, PASTE_ALL_RESIZE_ROWS };
	void Paste(PasteMode mode = PASTE_CURSOR);

	void ResizeGrid(int nr, int nc);

private:
	void OnGridKey(wxKeyEvent &evt);
	void OnGridCellChange(wxGridEvent &evt);
	void OnGridCellSelect(wxGridEvent &evt);
	void OnGridEditorHidden(wxGridEvent &evt);
	void OnGridEditorShown(wxGridEvent &evt);
	void OnGridRangeSelect(wxGridRangeSelectEvent &evt);
	void OnGridLabelClick(wxGridEvent &evt);

	bool m_skipSelect;
	int m_selTopRow, m_selBottomRow;
	int m_selLeftCol, m_selRightCol;
	bool m_enableCopyPaste;
	bool m_sendPasteEvent;
	int m_lastSelTopRow, m_lastSelBottomRow;
	int m_lastSelLeftCol, m_lastSelRightCol;

	DECLARE_EVENT_TABLE()
};

#endif
