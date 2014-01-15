
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>

#include "wex/sched.h"
#include "wex/metro.h"

BEGIN_EVENT_TABLE(wxSchedCtrl, wxWindow)
	EVT_PAINT( wxSchedCtrl::OnPaint )
	EVT_ERASE_BACKGROUND( wxSchedCtrl::OnErase )
	EVT_SIZE( wxSchedCtrl::OnResize )
	EVT_CHAR( wxSchedCtrl::OnChar )
	EVT_LEFT_DOWN( wxSchedCtrl::OnMouseDown )
	EVT_LEFT_UP( wxSchedCtrl::OnMouseUp )
	EVT_MOTION( wxSchedCtrl::OnMouseMove )
	EVT_KILL_FOCUS( wxSchedCtrl::OnLostFocus )
END_EVENT_TABLE()

DEFINE_EVENT_TYPE( wxEVT_SCHEDCTRL_CHANGE )

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define SCHED_FONT wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD)

wxSchedCtrl::wxSchedCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &sz)
	: wxWindow(parent, id, pos, sz, wxWANTS_CHARS)
{
	SetBackgroundColour( *wxWHITE );
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	m_colLabelsVertical = true;
	m_autosizeHeaders = true;
	m_mouseDown = false;
	m_rowHeaderSize = 30;
	m_colHeaderSize = 21;
	m_cellSize = 17;
	m_data.resize( 288, 0 ); // 12 rows x 24 cols
	m_cols = 24;
	m_selStartR = m_selStartC = m_selEndR = m_selEndC = -1;
	m_min = 0;
	m_max = 9;
}

wxSchedCtrl::~wxSchedCtrl()
{
	/* nothing to do */
}

void wxSchedCtrl::AddColour(const wxColour &c)
{
	m_colours.push_back(c);
}

bool wxSchedCtrl::GetColour(int i, wxColour &c)
{
	if (i >= 0 && i < (int)m_colours.size())
	{ 
		c = m_colours[i];
		return true;
	}
	else
		return false;
}

void wxSchedCtrl::Set(int r, int c, int val)
{
	if ( r < NRows() && c < NCols() )
		m_data[ r*m_cols + c ] = val;
}

void wxSchedCtrl::SetMinMax(int min, int max, bool clamp)
{
	m_min = min;
	m_max = max;

	if (!clamp)
		return;

	for (size_t i=0;i<m_data.size();i++)
	{
		if (m_data[i] < min) m_data[i] = min;
		if (m_data[i] > max) m_data[i] = max;
	}
}

int wxSchedCtrl::Get(int r, int c) const
{
	if ( r < NRows() && c < NCols() )
		return m_data[ r*m_cols+c ];
	else
		return -1;
}

void wxSchedCtrl::Set(int val)
{
	for ( size_t i=0;i<m_data.size();i++)
		m_data[i] = val;
	Refresh();
}

void wxSchedCtrl::SetGrid(int nr, int nc)
{
	if ( nr*nc > 0 )
	{
		m_data.resize( nr*nc, 0 );
		m_cols = nc;
	}
}

int wxSchedCtrl::NRows() const
{
	return m_data.size()/m_cols;
}

int wxSchedCtrl::NCols() const
{
	return m_cols;
}

void wxSchedCtrl::AddRowLabel(const wxString &s)
{
	m_rowLabels.Add( s );
}

void wxSchedCtrl::AddColLabel(const wxString &s)
{
	m_colLabels.Add( s );
}

void wxSchedCtrl::ClearLabels()
{
	m_rowLabels.Clear();
	m_colLabels.Clear();
}

void wxSchedCtrl::ClearRowLabels()
{
	m_rowLabels.Clear();
}

void wxSchedCtrl::ClearColLabels()
{
	m_colLabels.Clear();
}


void wxSchedCtrl::AutosizeHeaders()
{
	if (!m_autosizeHeaders) return;
	
	wxClientDC dc(this);
	dc.SetFont( SCHED_FONT );

	
	int r, c;
	int rows = NRows();
	int cols = NCols();
	int s;
	m_rowHeaderSize = 0;
	for (r=0;r<rows&&r<(int)m_rowLabels.Count();r++)
	{
		dc.GetTextExtent(m_rowLabels[r], &s, NULL);
		if (s > m_rowHeaderSize)
			m_rowHeaderSize = s;
	}

	m_colHeaderSize = 0;
	for (c=0;c<cols&&c<(int)m_colLabels.Count();c++)
	{
		int textW, textH;
		dc.GetTextExtent(m_colLabels[c], &textW, &textH);
		if ((s = m_colLabelsVertical ? textW : textH) > m_colHeaderSize)
			m_colHeaderSize = s;
	}

	m_rowHeaderSize += 6;
	m_colHeaderSize += 4;
}

void wxSchedCtrl::OnErase( wxEraseEvent & )
{
	/* nothing to do */
}

void wxSchedCtrl::OnPaint( wxPaintEvent & )
{
	wxAutoBufferedPaintDC dc( this );
	wxSize sz( GetClientSize() );
	wxRect geom( 0, 0, sz.GetWidth(), sz.GetHeight() );

	dc.SetBackground( GetBackgroundColour() );
	dc.Clear();

	int r, c;
	int rows = NRows();
	int cols = NCols();

	dc.SetFont( SCHED_FONT );
	
	dc.SetPen( *wxTRANSPARENT_PEN );
	for (r=0;r<rows;r++)
	{
		for (c=0;c<cols;c++)
		{
			int x = m_rowHeaderSize + c*m_cellSize;
			int y = m_colHeaderSize + r*m_cellSize;

			int selrs = MIN( m_selStartR, m_selEndR );
			int selcs = MIN( m_selStartC, m_selEndC );
			int selre = MAX( m_selStartR, m_selEndR );
			int selce = MAX( m_selStartC, m_selEndC );

			bool sel = (r>=selrs && r<=selre && c>=selcs && c<=selce);
			
			if (x >= geom.width || y >= geom.height)
				break;
			
			int val = Get(r,c);
			if (val >= 1 && val-1 < (int) m_colours.size() || sel)
			{
				dc.SetBrush(wxBrush( sel ? *wxBLUE : m_colours[val-1] ));
				dc.DrawRectangle(geom.x+x,geom.y+y, m_cellSize, m_cellSize);
			}
			
			wxString buf;
			buf << val;
			int textW, textH;
			dc.GetTextExtent(buf, &textW, &textH);
			x += m_cellSize/2 - textW/2;
			y += m_cellSize/2 - textH/2;
			
			dc.SetTextForeground( sel ? *wxWHITE : *wxBLACK );
			dc.DrawText(buf, geom.x + x, geom.y + y);
		}
	}


	dc.SetPen(wxPen(wxColour(120, 120, 120)));
	dc.SetTextForeground(wxColour(120, 120, 120));

	for (r=0;r<=rows;r++)
	{
		dc.DrawLine(geom.x, geom.y+m_colHeaderSize + r*m_cellSize, 
			geom.x+m_rowHeaderSize + cols*m_cellSize, geom.y+m_colHeaderSize + r*m_cellSize);

		if (r < (int)m_rowLabels.Count() && r < rows)
		{
			int yoff = m_cellSize/2 - dc.GetCharHeight()/2;
			dc.DrawText(m_rowLabels[r], geom.x + 2, geom.y + m_colHeaderSize + r*m_cellSize + yoff);
		}
	}

	for (c=0;c<=cols;c++)
	{
		dc.DrawLine(geom.x+m_rowHeaderSize + c*m_cellSize, geom.y, 
			geom.x+m_rowHeaderSize + c*m_cellSize, geom.y+m_colHeaderSize + rows*m_cellSize);

		if (c < (int)m_colLabels.Count() && c < cols)
		{
			if (m_colLabelsVertical)
			{
				int xoff = m_cellSize/2 - dc.GetCharHeight()/2;
				dc.DrawRotatedText(m_colLabels[c], geom.x+m_rowHeaderSize+c*m_cellSize+xoff, geom.y+m_colHeaderSize-2,90);
			}
			else
			{
				int textW;
				dc.GetTextExtent(m_colLabels[c], &textW, NULL);
				int xoff = m_cellSize/2 - textW/2;
				dc.DrawText(m_colLabels[c], geom.x+m_rowHeaderSize+c*m_cellSize+xoff, geom.y+2);
			}
		}
	}

}

void wxSchedCtrl::OnResize(wxSizeEvent &)
{
	Refresh();
}

wxSize wxSchedCtrl::DoGetBestSize() const
{
	const_cast<wxSchedCtrl*>(this)->AutosizeHeaders();

	return wxSize( m_rowHeaderSize + NCols()*m_cellSize,
				m_colHeaderSize + NRows()*m_cellSize);
}


void wxSchedCtrl::OnChar(wxKeyEvent &evt)
{
	int selrs = MIN( m_selStartR, m_selEndR );
	int selcs = MIN( m_selStartC, m_selEndC );
	int selre = MAX( m_selStartR, m_selEndR );
	int selce = MAX( m_selStartC, m_selEndC );

	int key = evt.GetKeyCode();
	if (key == 'C')
	{
		if (wxTheClipboard->Open())
		{
		// This data objects are held by the clipboard, 
		// so do not delete them in the app.
			wxTheClipboard->SetData( new wxTextDataObject( Schedule() ) );
			wxTheClipboard->Close();
		}
	}
	else if (key == 'V')
	{
		if (wxTheClipboard->Open())
		{
			wxTextDataObject tobj;
			wxTheClipboard->GetData( tobj );
			wxString sched = tobj.GetText();
			Schedule( sched );
		}
	}
	else if (key == 'T')
	{
		if (wxTheClipboard->Open())
		{
		// This data objects are held by the clipboard, 
		// so do not delete them in the app.
			wxString tsv;
			for (int r=0;r<NRows();r++)
			{

				for (int c=0;c<m_cols;c++)
				{
					tsv += wxString::Format("%d", Get(r,c));
					if (c < m_cols-1)
						tsv += '\t';
				}
				tsv += '\n';
			}

			wxTheClipboard->SetData( new wxTextDataObject( tsv ) );
			wxTheClipboard->Close();
		}
	}
	else if (key >= '0' && key <= '9' &&
		selrs >= 0 && selcs >= 0)
	{
		for (int r=selrs;r<=selre&&r<NRows();r++)
			for (int c=selcs;c<=selce&&c<NCols();c++)
				if (key-'0' <= m_max && key-'0' >= m_min)
					Set(r,c, key - '0');

		Refresh();

		wxCommandEvent change(wxEVT_SCHEDCTRL_CHANGE, this->GetId() );
		change.SetEventObject( this );
		GetEventHandler()->ProcessEvent(change);
	}
}

void wxSchedCtrl::OnMouseDown(wxMouseEvent &evt)
{
	m_selStartC = (evt.GetX()-m_rowHeaderSize)/m_cellSize;
	m_selStartR = (evt.GetY()-m_colHeaderSize)/m_cellSize;

	if (m_selStartC < 0 || m_selStartC >= NCols() ||
		m_selStartR < 0 || m_selStartR >= NRows() ||
		evt.GetX() < m_rowHeaderSize || evt.GetY() < m_colHeaderSize)
	{
		m_selStartC = m_selStartR = -1;
	}
	else
		this->SetFocus();

	m_selEndR = m_selStartR;
	m_selEndC = m_selStartC;
	m_mouseDown = true;
	Refresh();

}

void wxSchedCtrl::OnMouseUp(wxMouseEvent &)
{
	m_mouseDown = false;
}

void wxSchedCtrl::OnMouseMove(wxMouseEvent &evt)
{
	if (!m_mouseDown)
		return;

	int c = (evt.GetX()-m_rowHeaderSize)/m_cellSize;
	int r = (evt.GetY()-m_colHeaderSize)/m_cellSize;

	if (r >= 0 && r < NRows() &&
		c >= 0 && c < NCols())
	{
		m_selEndR = r;
		m_selEndC = c;
		Refresh();
	}
}

void wxSchedCtrl::OnLostFocus(wxFocusEvent &)
{
	m_selEndR = m_selStartR = -1;
	m_selEndC = m_selStartC = -1;
	Refresh();
}

bool wxSchedCtrl::Schedule(const wxString &sched)
{
	if ((int)sched.Len() != NRows()*NCols())
		return false;

	int x;

	for (int r=0;r<NRows();r++)
		for (int c=0;c<NCols();c++)
			if ( (x = sched[ r*NCols() + c ] - '0') <= m_max && x >= m_min)
				Set(r,c, x);

	Refresh();

	return true;
}

wxString wxSchedCtrl::Schedule() const
{
	wxString buf;
	for (int r=0;r<NRows();r++)
		for (int c=0;c<NCols();c++)
			buf << Get(r,c);

	return buf;
}

void wxSchedCtrl::SetupTOUGrid()
{
	SetupDefaultColours();
	SetGrid(12,24);
	SetMinMax(1,9,true);

	m_rowLabels.clear();
	AddRowLabel("Jan");
	AddRowLabel("Feb");
	AddRowLabel("Mar");
	AddRowLabel("Apr");
	AddRowLabel("May");
	AddRowLabel("Jun");
	AddRowLabel("Jul");
	AddRowLabel("Aug");
	AddRowLabel("Sep");
	AddRowLabel("Oct");
	AddRowLabel("Nov");
	AddRowLabel("Dec");

	m_colLabels.clear();
	AddColLabel("12am");
	AddColLabel("1am");
	AddColLabel("2am");
	AddColLabel("3am");
	AddColLabel("4am");
	AddColLabel("5am");
	AddColLabel("6am");
	AddColLabel("7am");
	AddColLabel("8am");
	AddColLabel("9am");
	AddColLabel("10am");
	AddColLabel("11am");
	AddColLabel("12pm");
	AddColLabel("1pm");
	AddColLabel("2pm");
	AddColLabel("3pm");
	AddColLabel("4pm");
	AddColLabel("5pm");
	AddColLabel("6pm");
	AddColLabel("7pm");
	AddColLabel("8pm");
	AddColLabel("9pm");
	AddColLabel("10pm");
	AddColLabel("11pm");

	AutosizeHeaders();
	InvalidateBestSize();
}

void wxSchedCtrl::SetupDefaultColours()
{
	m_colours.clear();
	AddColour( "AQUAMARINE" );
	AddColour( "CADET BLUE" );
	AddColour( "SIENNA" );
	AddColour( "SEA GREEN" );
	AddColour( "GOLDENROD" );
	AddColour( "FIREBRICK" );
	AddColour( "DARK GREEN" );
	AddColour( "ORCHID" );
	AddColour( "ORANGE RED");
}


bool TranslateSchedule(int tod[8760], const char *weekday, const char *weekend, 
					   int min_val, int max_val)
{
static int nday[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	int i=0;
	if (!weekday || !weekend || strlen(weekday) != 288 || strlen(weekend) != 288)
	{
		for (i=0;i<8760;i++)
			tod[i] = min_val;
		return false;
	}

	int wday = 5;
	for (int m=0;m<12;m++)
	{
		for (int d=0;d<nday[m];d++)
		{
			char *sptr = NULL;
			if (wday <= 0)
				sptr = (char*)weekend;
			else
				sptr = (char*)weekday;

			if (wday >= 0) wday--;
			else wday = 5;

			for (int h=0;h<24&&i<8760 && m*24+h<288;h++)
			{
				tod[i] = (int)(sptr[ m*24 + h ]-'1');
				if (tod[i] < min_val) tod[i] = min_val;
				if (tod[i] > max_val) tod[i] = max_val;
				i++;					
			}
		}
	}
	return true;
}
