#include <wx/window.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>

#include <wex/gleasy.h>


BEGIN_EVENT_TABLE( wxGLEasyCanvas, wxGLCanvas )
    EVT_SIZE(wxGLEasyCanvas::OnSize)
    EVT_PAINT(wxGLEasyCanvas::OnPaint)
    EVT_CHAR(wxGLEasyCanvas::OnChar)
    EVT_MOUSE_EVENTS(wxGLEasyCanvas::OnMouse)
END_EVENT_TABLE()

wxGLEasyCanvas::wxGLEasyCanvas( wxWindow *parent, int id, const wxPoint &pos, const wxSize &size )
	: wxGLCanvas( parent, id, pos, size, wxBORDER_NONE ), m_glContext( this )
{
	m_pointListMode = false;
	m_xrot = m_yrot = 0;
	m_lastX = m_lastY = 0;
	m_scale = 1.0f;
	m_min = wxGLPoint3D(-1,-1,-1);
	m_max = wxGLPoint3D(1,1,1);
	SetupProjectionMode();
}

wxGLEasyCanvas::~wxGLEasyCanvas()
{
	// nothing to do (yet)
}

	
void wxGLEasyCanvas::OnRender()
{
}


void wxGLEasyCanvas::SetView( const wxGLPoint3D &min, const wxGLPoint3D &max )
{
	m_min = min;
	m_max = max;
	SetupProjectionMode();
}

void wxGLEasyCanvas::Color( const wxColour &c )
{
	glColor3f( float(c.Red())/255.0f, 
		float(c.Green())/255.0f, 
		float(c.Blue())/255.0f );
}

void wxGLEasyCanvas::Point( float x, float y, float z )
{
	if ( m_pointListMode ) glVertex3f( x, y, z );
	else
	{
		glBegin( GL_POINT );
		glVertex3f( x, y, z );
		glEnd( );
	}
}

void wxGLEasyCanvas::Point( const wxGLPoint3D &p )
{
	glVertex3f( p.x, p.y, p.z );
}

void wxGLEasyCanvas::BeginPoints()
{
	if ( !m_pointListMode )
	{
		m_pointListMode = true;
		glBegin( GL_POINTS );
	}
}

void wxGLEasyCanvas::EndPoints()
{
	if ( m_pointListMode )
	{
		m_pointListMode = false;
		glEnd();
	}
}

void wxGLEasyCanvas::Points( const std::vector<wxGLPoint3D> &list )
{
	BeginPoints();
	for( std::vector<wxGLPoint3D>::const_iterator it = list.begin();
		it != list.end();
		++it )
		glVertex3f( it->x, it->y, it->z );

	EndPoints();
}

void wxGLEasyCanvas::Line( const wxGLPoint3D &p1, const wxGLPoint3D &p2 )
{
	glBegin( GL_LINES );
	glVertex3f( p1.x, p1.y, p1.z );
	glVertex3f( p2.x, p2.y, p2.z );
	glEnd();
}

void wxGLEasyCanvas::Lines( const std::vector<wxGLPoint3D> &list )
{
	glBegin( GL_LINE_STRIP );
	for( std::vector<wxGLPoint3D>::const_iterator it = list.begin();
		it != list.end();
		++it )
		glVertex3f( it->x, it->y, it->z );
	glEnd();
}


void wxGLEasyCanvas::Text( const wxGLPoint3D &p, const wxString &text )
{
}

void wxGLEasyCanvas::OnChar( wxKeyEvent &evt )
{
	// don't do anything yet.
	evt.Skip();
}


void wxGLEasyCanvas::OnMouse( wxMouseEvent &evt )
{
    // Allow default processing to happen, or else the canvas cannot gain focus
    // (for key events).
    evt.Skip();
	
	if ( evt.Dragging() && evt.ControlDown() )
	{
		// zoom in/out
		int dy = m_lastY - evt.GetY();
		m_scale  += (float)(dy) / 500.0f;
		if ( m_scale < 0.001 ) m_scale = 0.001;
		if ( m_scale > 100.0 ) m_scale = 100.0;
		Refresh(false);
	}
    else if ( evt.Dragging() )
	{
        m_yrot += (evt.GetX() - m_lastX)*1.0;
        m_xrot += (evt.GetY() - m_lastY)*1.0;
        Refresh(false);
    }

    m_lastX = evt.GetX();
    m_lastY = evt.GetY();

	// allow descendents to handle mouse events too
	evt.Skip();
}

void wxGLEasyCanvas::OnPaint( wxPaintEvent & )
{
    // This is a dummy, to avoid an endless succession of paint messages.
    // OnPaint handlers must always create a wxPaintDC.
    wxPaintDC dc(this);

    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.
    SetCurrent( m_glContext );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glPushMatrix();
	glScalef( m_scale, m_scale, m_scale );
    glRotatef( m_yrot, 0.0f, 1.0f, 0.0f );
    glRotatef( m_xrot, 1.0f, 0.0f, 0.0f );
	
		// xyz axes
		Color( *wxRED );
		Line( wxGLPoint3D(0,0,0), wxGLPoint3D( 100, 0, 0 ) );
		Color( *wxGREEN );
		Line( wxGLPoint3D(0,0,0), wxGLPoint3D( 0, 100, 0 ) );
		Color( *wxBLUE );
		Line( wxGLPoint3D(0,0,0), wxGLPoint3D( 0, 0, 100 ) );
	
	OnRender();

    glPopMatrix();

    SwapBuffers();
}

void wxGLEasyCanvas::OnSize( wxSizeEvent &evt )
{
	SetupProjectionMode();
}

void wxGLEasyCanvas::SetupProjectionMode()
{
    if ( !IsShownOnScreen() )
        return;

    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.
    SetCurrent( m_glContext );

    int w, h;
    GetClientSize(&w, &h);

    // It's up to the application code to update the OpenGL viewport settings.
    // In order to avoid extensive context switching, consider doing this in
    // OnPaint() rather than here, though.
    glViewport(0, 0, (GLint) w, (GLint) h);

    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
	glOrtho(
		m_min.x,m_max.x,
		m_min.y,m_max.y,
		m_min.z,m_max.z );
		*/

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}





BEGIN_EVENT_TABLE( wxGLEasyCanvasTest, wxGLEasyCanvas )
	EVT_RIGHT_DOWN( wxGLEasyCanvasTest::OnRightDown )
	EVT_MENU( wxID_OPEN, wxGLEasyCanvasTest::OnMenu )
END_EVENT_TABLE()

wxGLEasyCanvasTest::wxGLEasyCanvasTest( wxWindow *parent )
	: wxGLEasyCanvas( parent, wxID_ANY )
{
}

void wxGLEasyCanvasTest::OnMenu( wxCommandEvent &evt )
{
	switch( evt.GetId() )
	{
	case wxID_OPEN:
	{
		wxFileDialog dlg( this, "Open x,y,z data file" );
		if ( dlg.ShowModal() == wxID_OK )
		{
			if ( FILE *fp = fopen( dlg.GetPath().c_str(), "r" ) )
			{
				wxGLPoint3D min(1e38,1e38,1e38), max(-1e38,-1e38,-1e38);
				char buf[512];
				m_data.clear();
				m_data.reserve( 20000 );
				while( fgets( buf, 512, fp ) )
				{
					wxGLPoint3D p;
					sscanf( buf, "%g,%g,%g", &p.x, &p.y, &p.z );
					m_data.push_back( p );

					if ( p.x < min.x ) min.x = p.x;
					if ( p.y < min.y ) min.y = p.y;
					if ( p.z < min.z ) min.z = p.z;
					if ( p.x > max.x ) max.x = p.x;
					if ( p.y > max.y ) max.y = p.y;
					if ( p.z > max.z ) max.z = p.z;
				}
				fclose(fp);

				SetView( min, max );

				wxMessageBox( wxString::Format("loaded %d points", (int)m_data.size()) );
				
				// NOTE: for some reason under wxGTK the following is required to avoid that
				//       the surface gets rendered in a small rectangle in the top-left corner of the frame
				PostSizeEventToParent();

				Refresh();
			}
		}
	}
		break;
	}
}

void wxGLEasyCanvasTest::OnRender()
{
	if ( m_data.size() > 0 )
	{
		// data
		Color( *wxYELLOW );
		Points( m_data );

	}
}

void wxGLEasyCanvasTest::OnRightDown( wxMouseEvent &evt )
{
	wxMenu menu;
	menu.Append( wxID_OPEN, "Open..." );
	PopupMenu( &menu );
}
