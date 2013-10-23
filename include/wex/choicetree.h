#ifndef __wex_choicetree_h
#define __wex_choicetree_h

#include <vector>
#include <wx/wx.h>

class wxChoiceTree;

wxDECLARE_EVENT( wxEVT_CHOICETREE_SELCHANGE, wxCommandEvent );
wxDECLARE_EVENT( wxEVT_CHOICETREE_DOUBLECLICK, wxCommandEvent );

#define EVT_CHOICETREE_SELCHANGE( id, fn ) EVT_COMMAND( id, wxEVT_CHOICETREE_SELCHANGE, fn )
#define EVT_CHOICETREE_DOUBLECLICK( id, fn ) EVT_COMMAND( id, wxEVT_CHOICETREE_DOUBLECLICK, fn )

class wxChoiceTreeItem
{
friend class wxChoiceTree;
public:
	wxChoiceTreeItem();
	virtual ~wxChoiceTreeItem();

	wxSize DefaultSize;
	wxString Label;
	wxColour LabelColour;
	wxString Caption;
	wxColour CaptionColour;
	wxBitmap Image;
	wxString MetaTag;
	bool CanSelect;
	std::vector<wxChoiceTreeItem*> Children;

	virtual bool Draw(wxDC &dc, const wxRect &rct,
			const wxFont &labelfont, const wxFont &capfont,
			int line_state, int last_line_y);
	virtual bool LeftClick(int mx, int my);
	virtual bool RightClick(int mx, int my);
	virtual int HeightRequest( int width, 
		const wxFont &labelfont,
		const wxFont &capfont );

protected:
	bool Hover;
	bool Selected;
	bool Expanded;
	bool Visible;
	wxRect Geometry;
};

class wxChoiceTree : public wxScrolledWindow
{
public:
	wxChoiceTree( wxWindow *parent, int id, 
		const wxPoint &pos=wxDefaultPosition, 
		const wxSize &sz = wxDefaultSize);
	virtual ~wxChoiceTree();

	bool ShowTreeLines;
	int IndentPixels;
	int VerticalSpace;
	int XOffset;
	int YOffset;
	bool ExtendFullWidth;

	wxChoiceTreeItem *Add(const wxString &lbl, const wxString &cap, const wxBitmap &bit = wxNullBitmap, wxChoiceTreeItem *parent=NULL);
	void Add(wxChoiceTreeItem *item, wxChoiceTreeItem *parent);
	void Assign(wxChoiceTreeItem *root);
	void Assign( std::vector<wxChoiceTreeItem*> roots);
	
	std::vector<wxChoiceTreeItem*> GetRootItems();
	
	void Delete(wxChoiceTreeItem *item);
	void DeleteAll();

	bool Find(wxChoiceTreeItem *item);
	wxChoiceTreeItem *Find(const wxString &lbl);
	wxChoiceTreeItem *GetParent(wxChoiceTreeItem *item);
	void Select(wxChoiceTreeItem *item);
	wxChoiceTreeItem *GetSelection();
	void Unselect();
	
	void Collapse(wxChoiceTreeItem *item);
	void CollapseAll();
	void Expand(wxChoiceTreeItem *item);
	void ExpandAll();
	void ExpandTo(wxChoiceTreeItem *item);
	void ShowRootItems();


	void Invalidate();
private:
	int MaxDepth(wxChoiceTreeItem *item, int depth);
	int MaxDepth();
	void Unselect(wxChoiceTreeItem *item);
	wxChoiceTreeItem *LocateParent(wxChoiceTreeItem *item, wxChoiceTreeItem *cur_parent);
	wxChoiceTreeItem *Locate(const wxString &label, wxChoiceTreeItem *parent);
	bool Locate(wxChoiceTreeItem *item, wxChoiceTreeItem *parent);

	std::vector<wxChoiceTreeItem*> m_rootItems;
	wxChoiceTreeItem *m_selectedItem;
	wxChoiceTreeItem *m_lastHoverItem;
	
	void OnResize(wxSizeEvent &evt);

	void FireSelectionChangedEvent();

	void PaintBackground( wxDC &dc );
	void PaintItem(wxDC &dc, wxChoiceTreeItem *item, int line_state, int last_line_y,
		const wxFont &, const wxFont &);
	void OnPaint(wxPaintEvent &evt);
	void OnErase(wxEraseEvent &evt);
	void OnLeftDown(wxMouseEvent &evt);
	void OnMouseMove(wxMouseEvent &evt);
	void OnLeave(wxMouseEvent &evt);
	void OnDoubleClick(wxMouseEvent &evt);
	wxChoiceTreeItem *LocateXY(int mx, int my, wxChoiceTreeItem *parent);
	wxChoiceTreeItem *LocateXY(int mx, int my);
	int CalculateGeometry(int win_width,
									 int &curx, int &cury, 
									 int level, int maxlevels,
									 wxChoiceTreeItem *item);

	void GetFonts( wxFont &label, wxFont &cap );

	DECLARE_EVENT_TABLE()
};


#endif

