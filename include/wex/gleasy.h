#ifndef __gleasy_h
#define __gleasy_h

#include <vector>
#include <wx/window.h>
#include <wx/glcanvas.h>

struct wxGLPoint3D
{
	wxGLPoint3D() : x(0),y(0), z(0) { }
	wxGLPoint3D( const wxGLPoint3D &p ) :x(p.x), y(p.y), z(p.z) { }
	wxGLPoint3D( float _x, float _y, float _z ) : x(_x), y(_y), z(_z) { }
	float x, y, z;
};

class wxGLEasyCanvas : public wxGLCanvas
{
public:
	wxGLEasyCanvas( wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );
	virtual ~wxGLEasyCanvas();

	void SetView( const wxGLPoint3D &min, const wxGLPoint3D &max );
protected:
	virtual void OnRender();

	void Color( const wxColour &c );
	void Point( float x, float y, float z );
	void Point( const wxGLPoint3D &p );
	void BeginPoints(); // optional optimization for rendering lots of points
	void EndPoints();
	void Points( const std::vector<wxGLPoint3D> &list );
	void Line( const wxGLPoint3D &p1, const wxGLPoint3D &p2 );
	void Lines( const std::vector<wxGLPoint3D> &list );
	void Text( const wxGLPoint3D &p, const wxString &text );


	void SetupProjectionMode();


	wxGLContext m_glContext;
	bool m_pointListMode;
	float m_xrot, m_yrot, m_lastX, m_lastY, m_scale;
	wxGLPoint3D m_min, m_max;
	
	void OnChar( wxKeyEvent & );
	void OnMouse( wxMouseEvent & );
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );
	DECLARE_EVENT_TABLE()
};


class wxGLEasyCanvasTest : public wxGLEasyCanvas
{
public:
	wxGLEasyCanvasTest( wxWindow *parent );

	void OnMenu( wxCommandEvent & );
	void OnRightDown( wxMouseEvent &evt );


protected:
	virtual void OnRender();
	std::vector<wxGLPoint3D> m_data;
	DECLARE_EVENT_TABLE();
};

#endif
