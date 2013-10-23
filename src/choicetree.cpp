#include <wx/wx.h>
#include <wx/valtext.h>
#include <wx/dcbuffer.h>
#include <wx/grid.h>
#include <wx/imaglist.h>

#include "wex/choicetree.h"
#include "wex/utils.h"
#include "wex/metro.h"


wxDEFINE_EVENT( wxEVT_CHOICETREE_SELCHANGE, wxCommandEvent );
wxDEFINE_EVENT( wxEVT_CHOICETREE_DOUBLECLICK, wxCommandEvent );

/****************************************
   ********* wxChoiceTreeItem *********
****************************************/

wxChoiceTreeItem::wxChoiceTreeItem()
{
	DefaultSize = wxSize(200, 34);
	LabelColour = *wxBLACK;
	CaptionColour = wxColour(120, 120, 120);
	CanSelect = false;
	Hover = false;
	Selected = false;
	Expanded = false;
	Visible = false;
}

wxChoiceTreeItem::~wxChoiceTreeItem()
{
	/* nothing to do 

	children items are NOT deleted automatically
	this is left up to wxChoiceTree
	*/

}

bool wxChoiceTreeItem::Draw(wxDC &dc, const wxRect &rct, 
							const wxFont &labelfont, const wxFont &capfont,
							int line_state, int last_line_y )
{
	wxColour bg = (Selected? wxColour( 212, 212, 212 ):
					(Hover ? wxColour(231,231,231) : *wxWHITE ));
	dc.SetBrush(wxBrush(bg));
	dc.SetPen(wxPen(bg));
	dc.DrawRectangle(rct.x, rct.y, rct.width, rct.height);

	dc.SetFont( labelfont );
	dc.SetTextForeground(LabelColour);
	int textx = 3;
	int texty = 1;

	if (Image.IsOk())
	{
		textx += Image.GetWidth() + 3;
		dc.DrawBitmap(Image, rct.x+3, rct.y+3);
	}
	dc.DrawText(Label, rct.x + textx, rct.y + texty);
	int lh = dc.GetCharHeight();

	dc.SetFont( capfont );
	dc.SetTextForeground(CaptionColour);
	wxDrawWordWrappedText( dc, Caption, rct.width-textx-8, true, rct.x+textx+10, rct.y+texty+lh+1, NULL );

	// line_state:
	// -1=last child line
	// 0=no line
	// 1=full line

	if (line_state != 0)
	{
		wxPen pen( wxPen( *wxLIGHT_GREY, 2, wxSOLID ) );
		pen.SetJoin( wxJOIN_ROUND );
		dc.SetPen( pen );

		dc.DrawLine(rct.x-1, rct.y+rct.height/2, rct.x-10, rct.y+rct.height/2);

		if (line_state < 0)
			dc.DrawLine(rct.x-10, last_line_y, rct.x-10, rct.y+rct.height/2);
	}

	return true;
}

bool wxChoiceTreeItem::LeftClick(int , int )
{
	return false; /* stub for descendant class expansion */
}

bool wxChoiceTreeItem::RightClick(int , int )
{
	return false; /* stub for descendant class expansion */
}

int wxChoiceTreeItem::HeightRequest( int width, const wxFont &labelfont, const wxFont &capfont )
{
	int height = 24;
	if (Image.IsOk())
	{
		width = width - Image.GetWidth() - 13;
		if (height < Image.GetHeight()+6)
			height = Image.GetHeight()+6;
	}

	if (!Caption.IsEmpty())
	{
		wxBitmap bit(24,24);
		wxMemoryDC memdc;
		memdc.SelectObject( bit );

		memdc.SetFont( labelfont );
		int label_height = memdc.GetCharHeight();

		memdc.SetFont( capfont );
		int text_height = 4 + label_height + wxDrawWordWrappedText(memdc, Caption, width);
		if (text_height > height)
			height = text_height;

		memdc.SelectObject(wxNullBitmap);
	}

	return height;
}


/****************************************
     ********* wxChoiceTree *********
****************************************/

BEGIN_EVENT_TABLE(wxChoiceTree, wxScrolledWindow)
	EVT_SIZE(wxChoiceTree::OnResize)
	EVT_ERASE_BACKGROUND(wxChoiceTree::OnErase)
	EVT_PAINT(wxChoiceTree::OnPaint)
	EVT_LEFT_DOWN( wxChoiceTree::OnLeftDown )
	EVT_MOTION( wxChoiceTree::OnMouseMove )
	EVT_LEAVE_WINDOW( wxChoiceTree::OnLeave )
	EVT_LEFT_DCLICK( wxChoiceTree::OnDoubleClick)
END_EVENT_TABLE()

wxChoiceTree::wxChoiceTree( wxWindow *parent, int id, const wxPoint &pos, const wxSize &sz)
	: wxScrolledWindow(parent, id, pos, sz, wxCLIP_CHILDREN)
{
	SetBackgroundStyle(::wxBG_STYLE_CUSTOM );
	SetBackgroundColour(*wxWHITE);
	
	wxFont font( *wxNORMAL_FONT );
	font.SetWeight( wxFONTWEIGHT_BOLD );
	SetFont( font );

	m_selectedItem = NULL;
	m_lastHoverItem = NULL;

	ShowTreeLines = true;
	IndentPixels = 30;
	VerticalSpace = 0;
	XOffset = 0;
	YOffset = 0;
	ExtendFullWidth = true;
}

wxChoiceTree::~wxChoiceTree()
{
	DeleteAll();
}

void wxChoiceTree::OnResize(wxSizeEvent &)
{
	Invalidate();
}

void wxChoiceTree::Invalidate()
{
	int hpos, vpos;
	GetViewStart( &hpos, &vpos );
	wxSize sz = GetClientSize();

	int cx = XOffset;
	int cy = YOffset;
	int maxlevels = MaxDepth();
	int height = 0;
	for (size_t i=0;i<m_rootItems.size();i++)
		height += CalculateGeometry( sz.GetWidth(), cx, cy, 0, maxlevels, m_rootItems[i]);

	if (height < sz.GetHeight())
		vpos = 0;

	SetScrollbars(1,1,sz.GetWidth(),height+VerticalSpace,0,vpos);
}

wxChoiceTreeItem *wxChoiceTree::Add(const wxString &lbl, const wxString &cap, 
									const wxBitmap &bit, wxChoiceTreeItem *parent)
{
	wxChoiceTreeItem *item = new wxChoiceTreeItem;
	item->Label = lbl;
	item->Caption = cap;
	item->Image = bit;

	Add(item, parent);

	return item;
}

void wxChoiceTree::Add(wxChoiceTreeItem *item, wxChoiceTreeItem *parent)
{
	if (!item || Find(item)) return;

	if (parent)
		parent->Children.push_back(item);
	else
		m_rootItems.push_back(item);

	item->Visible = true;
}

void wxChoiceTree::Assign(wxChoiceTreeItem *root)
{
	DeleteAll();
	m_rootItems.push_back(root);
}

void wxChoiceTree::Assign(std::vector<wxChoiceTreeItem*> roots)
{
	DeleteAll();
	m_rootItems = roots;
}

std::vector<wxChoiceTreeItem*> wxChoiceTree::GetRootItems()
{
	return m_rootItems;
}

void wxChoiceTree::Delete(wxChoiceTreeItem *item)
{
	if (!item) return;

	std::vector<wxChoiceTreeItem*> arr = item->Children;
	for (size_t i=0;i<arr.size();i++)
		Delete( arr[i] );

	// remove this item from parent
	wxChoiceTreeItem *parent = GetParent(item);
	if (parent)
	{
		std::vector<wxChoiceTreeItem*>::iterator it = std::find( parent->Children.begin(), parent->Children.end(), item );
		if ( it != parent->Children.end() )
			parent->Children.erase( it );
	}
	
	// make sure its not a root item
	std::vector<wxChoiceTreeItem*>::iterator it = std::find( m_rootItems.begin(), m_rootItems.end(), item );
	if ( it != m_rootItems.end() )
		m_rootItems.erase( it );

	if (m_selectedItem == item)
		m_selectedItem = NULL;

	// release memory	
	delete item;
}

void wxChoiceTree::DeleteAll()
{
	std::vector<wxChoiceTreeItem*> arr = m_rootItems;
	for (size_t i=0;i<arr.size();i++)
		Delete( arr[i] );

	m_rootItems.clear();
	m_selectedItem = NULL;
}

bool wxChoiceTree::Find(wxChoiceTreeItem *item)
{
	for (size_t i=0;i<m_rootItems.size();i++)
		if (Locate(item, m_rootItems[i]))
			return true;

	return false;
}

wxChoiceTreeItem *wxChoiceTree::Find(const wxString &lbl)
{
	wxChoiceTreeItem *x = NULL;
	for (size_t i=0;i<m_rootItems.size();i++)
		if ( (x=Locate(lbl, m_rootItems[i])) != NULL )
			return x;

	return NULL;
}

void wxChoiceTree::Select(wxChoiceTreeItem *item)
{
	Unselect();
	if (item) item->Selected = true;
	m_selectedItem = item;
}

wxChoiceTreeItem *wxChoiceTree::GetSelection()
{
	return m_selectedItem;
}

void wxChoiceTree::Unselect()
{
	for (size_t i=0;i<m_rootItems.size();i++)
		Unselect(m_rootItems[i]);
}

void wxChoiceTree::Collapse(wxChoiceTreeItem *item)
{
	if (!item) return;
	item->Expanded = false;
	for (size_t i=0;i<item->Children.size();i++)
	{
		Collapse( item->Children[i] );
		item->Children[i]->Visible = false;
	}
}

void wxChoiceTree::CollapseAll()
{
	for (size_t i=0;i<m_rootItems.size();i++)
		Collapse( m_rootItems[i] );
}

void wxChoiceTree::ExpandTo(wxChoiceTreeItem *item)
{
	while (item != NULL)
	{
		item->Expanded = true;
		item->Visible = true;
		for (size_t i=0;i<item->Children.size();i++)
			item->Children[i]->Visible = true;
		item = GetParent(item);
	}

	ShowRootItems();
}

void wxChoiceTree::Expand(wxChoiceTreeItem *item)
{
	if (!item) return;
	item->Expanded = true;
	item->Visible = true;
	for (size_t i=0;i<item->Children.size();i++)
		Expand( item->Children[i] );
}

void wxChoiceTree::ExpandAll()
{
	for (size_t i=0;i<m_rootItems.size();i++)
		Expand( m_rootItems[i] );
}

void wxChoiceTree::ShowRootItems()
{
	for (size_t i=0;i<m_rootItems.size();i++)
		m_rootItems[i]->Visible = true;
}

void wxChoiceTree::Unselect(wxChoiceTreeItem *item)
{
	if (!item) return;
	item->Selected = false;
	for (size_t i=0;i<item->Children.size();i++)
		Unselect( item->Children[i] );
}

wxChoiceTreeItem *wxChoiceTree::GetParent(wxChoiceTreeItem *item)
{
	if (!item) return NULL;
	if ( std::find( m_rootItems.begin(), m_rootItems.end(), item ) != m_rootItems.end() ) return NULL;
	wxChoiceTreeItem *x = NULL;
	for (size_t i=0;i<m_rootItems.size();i++)
		if ( (x=LocateParent(item, m_rootItems[i]))!=NULL )
			return x;
	return NULL;
}

wxChoiceTreeItem *wxChoiceTree::LocateParent(wxChoiceTreeItem *item, wxChoiceTreeItem *cur_parent)
{
	if (!item||!cur_parent) return NULL;

	if ( std::find( cur_parent->Children.begin(), cur_parent->Children.end(), item ) != cur_parent->Children.end() )
		return cur_parent;

	wxChoiceTreeItem *x = NULL;
	for (size_t i=0;i<cur_parent->Children.size();i++)
		if ( (x=LocateParent(item, cur_parent->Children[i]))!=NULL )
			return cur_parent->Children[i];

	return NULL;
}

wxChoiceTreeItem *wxChoiceTree::Locate(const wxString &label, wxChoiceTreeItem *parent)
{
	if (!parent)
		return NULL;

	if (parent->Label == label)
		return parent;

	wxChoiceTreeItem *x = NULL;
	for (size_t i=0;i<parent->Children.size();i++)
	{
		if (parent->Children[i]->Label == label)
			return parent->Children[i];
		else if ((x=Locate(label, parent->Children[i]))!=NULL)
			return x;
	}

	return NULL;
}

bool wxChoiceTree::Locate(wxChoiceTreeItem *item, wxChoiceTreeItem *parent)
{
	if (!item || !parent)
		return false;

	if (parent == item)
		return true;

	for (size_t i=0;i<parent->Children.size();i++)
		if (parent->Children[i] == item || Locate(item, parent->Children[i]))
			return true;

	return false;
}

int wxChoiceTree::MaxDepth(wxChoiceTreeItem *item, int depth)
{
	if (!item) return depth;

	int max = depth+1;

	for (size_t i=0;i<item->Children.size();i++)
	{
		int x = MaxDepth(item->Children[i], max);
		if (x > max)
			max = x;
	}

	return max;

}

int wxChoiceTree::MaxDepth()
{
	int max = 0;
	for (size_t i=0;i<m_rootItems.size();i++)
	{
		int x = MaxDepth( m_rootItems[i], 0 );
		if (x > max)
			max = x;
	}

	return max;
}


void wxChoiceTree::FireSelectionChangedEvent()
{
	// emit selection change event
	wxCommandEvent e( wxEVT_CHOICETREE_SELCHANGE, GetId() );
	e.SetEventObject( this );
	ProcessWindowEvent( e );
}



void wxChoiceTree::PaintBackground( wxDC &dc )
{
	wxColour bg = GetBackgroundColour();
	dc.SetBrush(wxBrush(bg));
	dc.SetPen(wxPen(bg,1));
	wxRect windowRect( wxPoint(0,0), GetClientSize() );
	CalcUnscrolledPosition(windowRect.x, windowRect.y,
		&windowRect.x, &windowRect.y);
	dc.DrawRectangle(windowRect);
}

void wxChoiceTree::PaintItem(wxDC &dc, wxChoiceTreeItem *item, int line_state, int last_line_y, 
							 const wxFont &flab, const wxFont &fcap )
{
	if (!item || !item->Visible) return;

	item->Draw(dc, item->Geometry, flab, fcap, line_state, last_line_y );

	// draw children
	for (size_t i=0;i<item->Children.size();i++)
	{
		if ( item->Children[i]->Visible )
		{
			if (ShowTreeLines) line_state = (i==item->Children.size()-1) ? -1 : 1;
			else line_state = 0;

			PaintItem(dc, item->Children[i], 
				line_state,
				item->Geometry.y+item->Geometry.height, flab, fcap);
		}
	}
}

void wxChoiceTree::OnErase(wxEraseEvent &)
{
	/* nothing to do */
}

void wxChoiceTree::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC pdc(this);
	DoPrepareDC(pdc);
	PaintBackground(pdc);
	wxFont label, cap;
	GetFonts( label, cap );
	for (size_t i=0;i<m_rootItems.size();i++)
		PaintItem(pdc, m_rootItems[i], 0, 0, label, cap);
}

void wxChoiceTree::OnLeftDown(wxMouseEvent &evt)
{
	int vsx, vsy;
	GetViewStart(&vsx,&vsy);
	wxChoiceTreeItem *cur_item = LocateXY(vsx+evt.GetX(), vsy+evt.GetY());
	if (cur_item != m_selectedItem)
	{
		bool sel_change = false;
		bool geom_invalid = false;

		if (m_selectedItem)
		{
			m_selectedItem->Selected = false;
			sel_change = true;		
		}

		if (cur_item && cur_item->Children.size() == 0)
		{
			m_selectedItem = cur_item;
			if (cur_item)
			{
				cur_item->Selected = true;
				sel_change = true;
			}
		}
		else
		{
			m_selectedItem = NULL;
		}

		if (sel_change)
		{
			FireSelectionChangedEvent();
		}

		if (cur_item && !cur_item->Expanded)
		{
			CollapseAll();
			ExpandTo(cur_item);
			geom_invalid = true;
		}
		else if (cur_item && cur_item->Expanded)
		{
			Collapse(cur_item);
			geom_invalid = true;
		}
		
		if (geom_invalid)
			Invalidate();
		else if (sel_change)
			Refresh();
	}
}

void wxChoiceTree::OnMouseMove(wxMouseEvent &evt)
{
	int vsx, vsy;
	GetViewStart(&vsx,&vsy);
	wxChoiceTreeItem *cur_item = LocateXY(vsx+evt.GetX(), vsy+evt.GetY());
	if (cur_item != m_lastHoverItem)
	{
		if (m_lastHoverItem) m_lastHoverItem->Hover = false;
		m_lastHoverItem = cur_item;
		if (cur_item) cur_item->Hover = true;

		Refresh();
	}
}

void wxChoiceTree::OnLeave(wxMouseEvent &)
{
	if (m_lastHoverItem)
	{
		m_lastHoverItem->Hover = false;
		m_lastHoverItem = NULL;
		Refresh();
	}
}

void wxChoiceTree::OnDoubleClick( wxMouseEvent & )
{
	wxCommandEvent e(wxEVT_CHOICETREE_DOUBLECLICK, GetId());
	e.SetEventObject( this );
	ProcessWindowEvent( e );
}

wxChoiceTreeItem *wxChoiceTree::LocateXY(int mx, int my, wxChoiceTreeItem *parent)
{
	if (!parent || !parent->Visible) return NULL;
	if (parent->Geometry.Contains(mx,my)) return parent;

	wxChoiceTreeItem *x = NULL;
	for (size_t i=0;i<parent->Children.size();i++)
		if ( (x=LocateXY(mx,my, parent->Children[i])) != NULL)
			return x;

	return NULL;
}

wxChoiceTreeItem *wxChoiceTree::LocateXY(int mx, int my)
{
	wxChoiceTreeItem *x = NULL;
	for (size_t i=0;i<m_rootItems.size();i++)
		if ( (x=LocateXY(mx,my, m_rootItems[i]))!=NULL)
			return x;
	return NULL;
}

int wxChoiceTree::CalculateGeometry(int win_width,
								 int &curx, int &cury, 
								 int level, int maxlevels,
								 wxChoiceTreeItem *item)
{
	if (!item || !item->Visible) return 0;

	// compute this item's size
	int x, y, w, h;
	x = curx + level * IndentPixels;
	y = cury+VerticalSpace;
	wxSize sz;
	w = win_width - ( ExtendFullWidth ? (x) : ((maxlevels-level)*IndentPixels) ) - XOffset;

	wxFont label, cap;
	GetFonts( label, cap );	
	h = item->HeightRequest( w, label, cap );

	item->Geometry = wxRect(x,y,w,h);
	cury += h+VerticalSpace;

	int total_height = h + VerticalSpace;

	// now children sizes
	for (size_t i=0;i<item->Children.size();i++)
		total_height += CalculateGeometry(win_width, curx, cury, level+1, maxlevels, item->Children[i]);

	return total_height;
}

void wxChoiceTree::GetFonts( wxFont &label, wxFont &cap )
{
	label = GetFont();
	cap = label;
	cap.SetPointSize( cap.GetPointSize() - 2 );
	cap.SetWeight( wxFONTWEIGHT_NORMAL );
}
