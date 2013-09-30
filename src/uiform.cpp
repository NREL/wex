
#include <wx/dcbuffer.h>

wxUIProperty::wxUIProperty()
{
	Init();
}

wxUIProperty::wxUIProperty( wxUIProperty *ref )
{
	Init();
	m_type = INVALID;
	m_pReference = ref;
}

wxUIProperty::wxUIProperty( double dp )
{
	Init();
	m_type = DOUBLE;
	m_doubleVal = dp;
}

wxUIProperty::wxUIProperty( bool bp )
{
	Init();
	m_type = BOOLEAN;
	m_boolVal = bp;
}

wxUIProperty::wxUIProperty( int ip )
{
	Init();
	m_type = INTEGER;
	m_intVal = ip;
}

wxUIProperty::wxUIProperty( int i, const wxArrayString &named_options )
{
	Init();
	m_type = INTEGER,
	m_intVal = i;
	m_namedOptions = named_options;
}

wxUIProperty::wxUIProperty( const wxColour &cp )
{
	Init();
	m_type = COLOUR;
	m_colour = cp;
}

wxUIProperty::wxUIProperty( const wxString &sp )
{
	Init();
	m_type = STRING;
	m_string = sp;
}

wxUIProperty::wxUIProperty( const wxImage &img )
{
	Init();
	m_type = IMAGE;
	m_image = img;
}

wxUIProperty::wxUIProperty( const wxArrayString &list )
{
	Init();
	m_type = STRINGLIST;
	m_strList = list;
}
int wxUIProperty::GetType()
{
	if (m_pReference != 0) return m_pReference->GetType();
	else return m_type;
}

void wxUIProperty::Set( double d )
{
	if ( m_pReference ) m_pReference->Set( d );	
	else { m_doubleVal = d; ValueChanged(); }
}

void wxUIProperty::Set( bool b )
{
	if (m_pReference ) m_pReference->Set( b );
	else { m_boolVal = b; ValueChanged(); }
}

void wxUIProperty::Set( int i )
{
	if ( m_pReference ) m_pReference->Set( i );
	else { m_intVal = i; ValueChanged(); }
}

void wxUIProperty::Set( const wxColour &c )
{
	if ( m_pReference ) m_pReference->Set( c );
	else { m_colour = c; ValueChanged(); }
}

void wxUIProperty::Set( const wxString &s )
{
	if ( m_pReference ) m_pReference->Set( s );
	else m_string = s;
}

void wxUIProperty::Set( const wxArrayString &list)
{
	if ( m_pReference ) m_pReference->Set( list );
	else { m_strList = list; ValueChanged(); }
}

void wxUIProperty::Set( const wxImage &img )
{
	if ( m_pReference ) m_pReference->Set( img );
	else { m_image = img; ValueChanged(); }
}

void wxUIProperty::SetNamedOptions( const wxArrayString &opts, int selection )
{
	if ( m_pReference ) m_pReference->SetNamedOptions( opts, selection );
	else
	{
		m_namedOptions = opts;
		if ( selection >= 0 && selection < (int)opts.Count() )
		{
			m_intVal = selection;
			ValueChanged();
		}
	}
}

wxArrayString wxUIProperty::GetNamedOptions()
{
	if ( m_pReference ) return m_pReference->GetNamedOptions();
	else return m_namedOptions;
}


int wxUIProperty::GetInteger()
{
	if ( m_pReference ) return m_pReference->GetInteger();
	else return m_intVal;
}

bool wxUIProperty::GetBoolean()
{
	if ( m_pReference ) return m_pReference->GetBoolean();
	else return m_boolVal;
}

double wxUIProperty::GetDouble()
{
	if ( m_pReference ) return m_pReference->GetDouble();
	else return m_doubleVal;
}

wxColour wxUIProperty::GetColour()
{
	if ( m_pReference ) return m_pReference->GetColour();
	else return m_colour;
}

wxString wxUIProperty::GetString()
{
	if ( m_pReference ) return m_pReference->GetString();
	else return m_string;
}

wxArrayString wxUIProperty::GetStringList()
{
	if ( m_pReference ) return m_pReference->GetStringList();
	else return m_strList;
}

wxImage wxUIProperty::GetImage()
{
	if ( m_pReference ) return m_pReference->GetImage();
	else return m_image;
}

void wxUIProperty::Write( wxOutputStream &_o )
{
	wxDataOutputStream out( _o );
	int type = GetType();
	out.Write8( 0x1d );
	out.Write16( (wxUint16)type );
	switch( type )
	{
	case DOUBLE: out.WriteDouble( GetDouble() ); break;
	case BOOLEAN: out.Write8( GetBoolean() ? 1 : 0 ); break;
	case INTEGER: out.Write32( GetInteger() ); break;
	case STRING: out.WriteString( GetString() ); break;
	case COLOUR:
		{
			wxColour c = GetColour();
			out.Write8( c.Red() );
			out.Write8( c.Green() );
			out.Write8( c.Blue() );
			out.Write8( c.Alpha() );
		}
		break;
	case STRINGLIST:
		{
			wxArrayString list = GetStringList();
			out.Write32( list.Count() );
			for ( size_t i=0;i<list.Count(); i++ )
				out.WriteString( list[i] );
		}
		break;
	case IMAGE:
		{
			wxImage img = GetImage();
			wxPNGHandler().SaveFile( &img, _o, false );			
		}
		break;
	}
	
	out.Write8(0x1d);
}

bool wxUIProperty::Read( wxInputStream &_i )
{
	wxDataInputStream in(_i);

	wxUint8 code = in.Read8();
	wxUint16 type = in.Read16();

	if ( m_pReference )
		m_pReference->m_type = type;
	else
		m_type = type;

	wxUint8 r,g,b,a;
	switch( type )
	{
	case DOUBLE: Set(in.ReadDouble()); break;
	case BOOLEAN: Set( in.Read8() != 0 ? true : false ); break;
	case INTEGER: Set( (int)in.Read32()); break;
	case STRING: Set(in.ReadString()); break;
	case COLOUR: 
		r = in.Read8();
		g = in.Read8();
		b = in.Read8();
		a = in.Read8();
		Set( wxColour(r,g,b,a) );
		break;
	case STRINGLIST:
		{
			wxArrayString list;
			size_t count = in.Read32();
			for (size_t i=0;i<count;i++ )
				list.Add( in.ReadString() );
			Set( list );
		}
		break;
	case IMAGE:
		{
			wxImage img;
			wxPNGHandler().LoadFile( &img, istrm, false );
			Set( img );
		}
		break;
	}

	return ( code == in.Read8() );
}

void wxUIProperty::ValueChanged()
{
	for( size_t i=0;i<m_updateInterfaceList.size(); i++ )
		m_updateInterfaceList[i]->OnPropertyChanged( m_updateInterfaceIds[i], this );
}

void wxUIProperty::AddUpdateInterface( const wxString &id, wxUIPropertyUpdateInterface *pui )
{
	puidata x;
	x.pui = pui;
	x.id = id;
	m_updateInterfaceList.push_back( x );
}

void wxUIProperty::RemoveUpdateInterface( wxUIPropertyUpdateInterface *pui )
{
	size_t i=0;
	while ( i < m_updateInterfaceList.size() )
	{
		if ( m_updateInterfaceList[i].pui == pui )
			m_updateInterfaceList.erase( m_updateInterfaceList.begin() + i );
		else
			i++;
	}
}

void wxUIProperty::ClearUpdateInterfaces()
{
	m_updateInterfaceList.clear();
}

void wxUIProperty::Init()
{
	m_updateInterfaceRef = 0;
	m_type = INVALID;
	m_pReference = 0;
	m_doubleVal = 0.0;
	m_intVal = 0;
	m_boolVal = false;
}

wxUIObject::wxUIObject( )
{
static int g_idCounter = 0;

	m_visible = true;
	AddProperty( "Name", new wxUIProperty( wxString::Format("object %d", ++g_idCounter ) ) );
	AddProperty( "X", new wxUIProperty( (int) 10 ) );
	AddProperty( "Y", new wxUIProperty( (int) 10 ) );
	AddProperty( "Width", new wxUIProperty( (int) 100 ) );
	AddProperty( "Height", new wxUIProperty( (int) 23 ) );	
}

wxUIObject::~wxUIObject()
{
	DeleteProperties();
}

void wxUIObject::DeleteHandles()
{
	for( size_t i=0;i<m_handles.size();i++ )
		delete m_handles[i];
	m_handles.clear();
}

void wxUIObject::DeleteProperties()
{
	for( size_t i=0;i<m_properties.size();i++ )
		delete m_properties[i].prop;
	m_properties.clear();
}

bool wxUIObject::IsWithin( int xx, int yy )
{
	int x = Property("X").GetInteger();
	int y = Property("Y").GetInteger();
	int width = Property("Width").GetInteger();
	int height = Property("Height").GetHeight();
	return ( xx >= x && xx <= x+width
		&& yy >= y && yy < y+height) ;
}

void wxUIObject::Draw( wxDC &dc, const wxRect &geom )
{
	dc.SetPen( *wxBLACK_PEN );
	dc.SetBrush( *wxLIGHT_GREY_BRUSH );
	dc.DrawRectangle( geom );
	
	dc.SetFont( *wxNORMAL_FONT );
	dc.DrawText( geom.x + 2, geom.y + 2, Property("Name").GetString() + " (" + GetTypeName() + ")" );
}

bool wxUIObject::Copy( wxUIObject *rhs )
{
	DeleteProperties();
	for( size_t i=0;i<rhs->m_properties.size();i++ )
	{
		propdata x;
		x.name = rhs->m_properties[i].name;
		x.lowered = rhs->m_properties[i].lowered;
		x.prop = new wxUIProperty( *rhs->m_properties[i].prop );
		x.prop->AddUpdateInterface( x.name, this );
		m_properties.push_back( x );
	}

	return true;
}

void wxUIObject::SetName( const wxString &name )
{
	Property("Name").Set( name );
}

wxString wxUIObject::GetName()
{
	return Property("Name").GetString();
}

void wxUIObject::SetGeometry( const wxRect &r )
{
	Property("X").Set( r.x );
	Property("Y").Set( r.y );
	Property("Width").Set( r.width );
	Property("Height").Set( r.height );
}

wxRect wxUIObject::GetGeometry()
{
	return wxRect(
		Property("X").Integer(),
		Property("Y").Integer(),
		Property("Width").Integer(),
		Property("Height").Integer() );
}

wxUIProperty &wxUIObject::Property( const wxString &name )
{
	static wxUIProperty s_nullProp;

	wxString lowered = name.Lower();
	for( size_t i=0;i<m_properties.size();i++ )
		if ( lowered == m_properties[i].lowered )
			return *m_properties[i].prop;

	return s_nullProp;
}

wxArrayString wxUIObject::Properties()
{
	wxArrayString list;
	for( size_t i=0;i<m_properties.size();i++ )
		list.Add( m_properties[i].name );

	return list;
}


void wxUIObject::Write( wxOutputStream &_o )
{
	wxDataOutputStream out(_o);
	out.Write8( 0xaf ); // start code
	out.Write8( 1 ); // version

	out.Write8( m_visible ? 1 : 0 );

	out.Write32( m_properties.size() );
	for( size_t i=0;i<m_properties.size(); i++ )
	{
		out.WriteString( m_properties[i].name );
		m_properties[i].prop->Write( _o );
	}

	out.Write8( 0xaf );
}

bool wxUIObject::Read( wxInputStream &_i )
{
	wxDataInputStream in(_i);
	wxUint8 code = in.Read8();
	in.Read8(); // version

	m_visible = in.Read8() != 0;

	size_t n = in.Read32();
	for( size_t i=0;i<n;i++)
	{
		wxString name = in.ReadString();
		Property(name).Read( _i );
	}

	return in.Read8() == code;
}


void wxUIObject::AddProperty( const wxString &name, wxUIProperty *prop )
{
	prop->AddUpdateInterface( name, this );
	
	propdata x;
	x.name = name;
	x.lowered = name.Lower();
	x.prop = prop;
	m_properties.push_back( x );
}

void wxUIObject::OnPropertyChanged( const wxString &id, wxUIProperty *p )
{
	/* nothing to do here */
}



BEGIN_EVENT_TABLE( wxUIPropertyEditor, wxPropertyGrid )
    EVT_PG_CHANGED( wxID_ANY, wxUIPropertyEditor::OnPropertyGridChange )
    EVT_PG_CHANGING( wxID_ANY, wxUIPropertyEditor::OnPropertyGridChanging )
END_EVENT_TABLE()


wxUIPropertyEditor::wxUIPropertyEditor( wxWindow *parent, int id, const wxPoint &pos, const wxSize &size )
	: wxPropertyGrid( parent, id, pos, size )
{
	m_curObject = 0;
}

wxUIPropertyEditor::~wxUIPropertyEditor()
{
	m_curObject = 0;
}

void wxUIPropertyEditor::SetObject( wxUIObject *obj )
{	
	m_curObject = 0;

	// clear the property grid
	m_curProps.clear();
	m_propGrid->Clear();

	if ( obj == 0 ) return;

	m_curObject = obj;

	wxArrayString list = obj->Properties();
	for ( size_t i=0; i<list.Count();i++ )
	{
		wxUIProperty &p = obj->Property( list[i] );
		
		wxPGProperty *pg = 0;
		switch( p.Type() )
		{
		case wxUIProperty::DOUBLE:
			pg = new wxFloatProperty( list[i], wxPG_LABEL );
			break;
		case wxUIProperty::INTEGER:
			if ( p.GetNamedOptions().Count() > 0 )
			{
				wxArrayString items = p.GetNamedOptions();
				// TODO
			}
			else
				pg = new wxIntProperty( list[i], wxPG_LABEL );				
			break;
		case wxUIProperty::STRING:
			pg =  new wxStringProperty( list[i], wxPG_LABEL);
			break;
		case wxUIProperty::BOOLEAN:
			pg =  new wxBoolProperty( list[i], wxPG_LABEL );
			break;
		case wxUIProperty::COLOUR:
			pg = new wxColourProperty( list[i], wxPG_LABEL );
			//pg->SetAttribute( "HasAlpha", true );
			break;
		case wxUIProperty::STRINGLIST:
			// TODO
			break;
		case wxUIProperty::IMAGE:
			// TODO
			break;
		}

		if ( pg != 0 )
		{
			m_propGrid->Append( pg );
			pgpinfo x;
			x.name = list[i];
			x.type = p.Type();
			x.pgp = pg;
			m_curProps.push_back( x );
			ValueToPropGrid( x );
		}
	}


}

void wxUIPropertyEditor::OnPropertyGridChange(wxPropertyGridEvent &evt)
{
    wxPGProperty* p = evt.GetProperty();
	for ( size_t i=0;i<m_curProps.size();i++ )
	{
		if ( m_curProps[i].pgp == p )
		{
			PropGridToValue( m_curProps[i] );
			m_lastChangedProperty = m_curProps[i].name;
			evt.Skip();
		}
	}
}

void wxUIPropertyEditor::OnPropertyGridChanging(wxPropertyGridEvent &evt)
{
	// nothing to do here at the moment
}

void wxUIPropertyEditor::UpdatePropertyValues()
{
	for ( size_t i=0;i<m_curProps.size(); i++ )
		ValueToPropGrid( m_curProps[i] );
}

void wxUIPropertyEditor::ValueToPropGrid( pgpinfo &p )
{
	if ( !m_curObject ) return;

	wxUIProperty &vp = m_curObject->Property( p.name );

	switch( vp.GetType() )
	{
	case wxUIProperty::BOOLEAN:
		m_propGrid->SetPropertyValue( p.name,  wxVariant( vp.GetBoolean() ) );
		break;
	case wxUIProperty::DOUBLE:
		m_propGrid->SetPropertyValue( p.name,  wxVariant( vp.GetDouble() ) );
		break;
	case wxUIProperty::INTEGER:
		if ( vp.GetNamedOptions().Count() > 0 )
		{
			// TODO
		}
		else
			m_propGrid->SetPropertyValue( p.name,  wxVariant( vp.GetInteger() ) );
		break;
	case wxUIProperty::STRING:
		m_propGrid->SetPropertyValue( p.name,  wxVariant( vp.GetString() ) );
		break;
	case wxUIProperty::COLOUR:
		m_propGrid->SetPropertyValue( p.name,  wxVariant( vp.GetColour() ) );
		break;
	case wxUIProperty::STRINGLIST:
		// TODO
		break;
	case wxUIProperty::IMAGE:
		// TODO
		break;
	}
}

void wxUIPropertyEditor::PropGridToValue( pgpinfo &p )
{
	if ( !m_curObject ) return;
		
	wxUIProperty &vp = m_curObject->Property(p.name);
	
	wxAny value = p.pgp->GetValue();

	if ( vp.GetType() == wxUIProperty::INVALID ) return;

	switch( vp.GetType() )
	{
	case wxUIProperty::BOOLEAN:
		vp.Set( wxANY_AS(value, bool) );
		break;
	case wxUIProperty::DOUBLE:
		vp.Set( wxANY_AS(value, double) );
		break;
	case wxUIProperty::INTEGER:
		vp.Set( wxANY_AS(value, int) );
		break;
	case wxUIProperty::STRING:
		vp.Set( wxANY_AS(value, wxString) );
		break;
	case wxUIProperty::COLOUR:
		vp.Set( wxANY_AS(value, wxColour) );
		break;
	case wxUIProperty::STRINGLIST:
		// TODO
		break;
	case wxUIProperty::IMAGE:
		// TODO
		break;
	}
}


DEFINE_EVENT_TYPE( wxEVT_UIFORM_SELECT )
DEFINE_EVENT_TYPE( wxEVT_UIFORM_MODIFY )

wxUIFormEvent::wxUIFormEvent(int commandType, int id)
	: wxCommandEvent(commandType, id)
{
	m_guiobj = NULL;
}


UIObjectCopyBuffer::UIObjectCopyBuffer()
{
	/* nothing to do */
}

UIObjectCopyBuffer::~UIObjectCopyBuffer()
{
	Clear();
}

void UIObjectCopyBuffer::Clear()
{
	for (int i=0;i<mCopyList.count();i++)
		delete mCopyList[i];
	mCopyList.clear();
}
void UIObjectCopyBuffer::Assign(Array<UIObject*> &objlist)
{
	Clear();
	mCopyList = objlist;
}

Array<UIObject*> UIObjectCopyBuffer::Get()
{
	return mCopyList;
}

int UIObjectCopyBuffer::Count()
{
	return mCopyList.count();
}


enum {	IDIP_RESIZE_PANEL = 3344, 
		IDIP_DELETE,
		IDIP_COPY,
		IDIP_PASTE,
		IDIP_CLEARALL,
		IDIP_WRITE,
		IDIP_READ,
		IDIP_TABORDERMODE,
		IDIP_DUPLICATE,
		IDIP_ALIGNTOP,
		IDIP_ALIGNLEFT,
		IDIP_ALIGNRIGHT,
		IDIP_ALIGNBOTTOM
};

BEGIN_EVENT_TABLE( UIForm, wxPanel )
	EVT_BUTTON( IDUI_CTRL_NATIVE_BUTTON, UIForm::NativeCtrlEvent)
	EVT_CHECKBOX( IDUI_CTRL_NATIVE_CHECKBOX, UIForm::NativeCtrlEvent)
	EVT_COMBOBOX( IDUI_CTRL_NATIVE_CHOICE, UIForm::NativeCtrlEvent)
	EVT_LISTBOX( IDUI_CTRL_NATIVE_LISTBOX, UIForm::NativeCtrlEvent)
	EVT_CHECKLISTBOX( IDUI_CTRL_NATIVE_CHECKLIST, UIForm::NativeCtrlEvent)
	EVT_TEXT_ENTER( IDUI_CTRL_NATIVE_TEXTENTRY, UIForm::NativeCtrlEvent)
	EVT_NUMERIC( IDUI_CTRL_NATIVE_NUMERIC, UIForm::NativeCtrlEvent)
	EVT_RADIOBUTTON( IDUI_CTRL_NATIVE_RADIO, UIForm::NativeCtrlEvent)
	EVT_SLIDER( IDUI_CTRL_NATIVE_SLIDER, UIForm::NativeCtrlEvent)
	EVT_SCHEDCTRL( IDUI_CTRL_NATIVE_SCHEDCTRL, UIForm::NativeCtrlEvent)
	EVT_TEXT_ENTER( IDUI_CTRL_NATIVE_SCHEDNUMERIC, UIForm::NativeCtrlEvent)
	EVT_TEXT_ENTER( IDUI_CTRL_NATIVE_HOURLYDATA, UIForm::NativeCtrlEvent)
	EVT_PTLAYOUT( IDUI_CTRL_NATIVE_PTLAYOUT, UIForm::NativeCtrlEvent)
	EVT_SHADINGCTRL( IDUI_CTRL_NATIVE_SHADINGCTRL, UIForm::NativeCtrlEvent)
	EVT_MATPROPCTRL( IDUI_CTRL_NATIVE_MATPROPCTRL, UIForm::NativeCtrlEvent)
	EVT_TRLOOP( IDUI_CTRL_NATIVE_TRLOOP, UIForm::NativeCtrlEvent)
	EVT_DATAGRIDBUTTON( IDUI_CTRL_NATIVE_DATAGRIDBUTTON, UIForm::NativeCtrlEvent )
	EVT_MONTHLYSCHEDULE( IDUI_CTRL_NATIVE_MONTHLYSCHEDULE, UIForm::NativeCtrlEvent )
	EVT_DATAARRAYBUTTON( IDUI_CTRL_NATIVE_DATAARRAYBUTTON, UIForm::NativeCtrlEvent )
	EVT_DOUBLEMATRIXCTRL( IDUI_CTRL_NATIVE_DOUBLEMATRIXCTRL, UIForm::NativeCtrlEvent )
	EVT_MONTHLYFACTOR( IDUI_CTRL_NATIVE_MONTHLYFACTOR, UIForm::NativeCtrlEvent )
	EVT_LISTBOX( IDUI_CTRL_NATIVE_SEARCHLISTBOX, UIForm::NativeCtrlEvent )
	EVT_SHADINGBUTTON( IDUI_CTRL_NATIVE_SHADINGBUTTON, UIForm::NativeCtrlEvent )

	EVT_MENU_RANGE( UI_ADD_CONTROL_ID, UI_ADD_CONTROL_ID+40, UIForm::OnAddCtrl )

	EVT_MENU( IDIP_TABORDERMODE, UIForm::OnPopup)
	EVT_MENU( IDIP_RESIZE_PANEL, UIForm::OnPopup)
	EVT_MENU( IDIP_DELETE, UIForm::OnPopup )
	EVT_MENU( IDIP_CLEARALL, UIForm::OnPopup )
	EVT_MENU( IDIP_WRITE, UIForm::OnPopup )
	EVT_MENU( IDIP_READ, UIForm::OnPopup )
	EVT_MENU( IDIP_DUPLICATE, UIForm::OnPopup )
	EVT_MENU( IDIP_COPY, UIForm::OnPopup )
	EVT_MENU( IDIP_PASTE, UIForm::OnPopup )
	EVT_MENU( IDIP_ALIGNTOP, UIForm::OnPopup )
	EVT_MENU( IDIP_ALIGNLEFT, UIForm::OnPopup )
	EVT_MENU( IDIP_ALIGNRIGHT, UIForm::OnPopup )
	EVT_MENU( IDIP_ALIGNBOTTOM, UIForm::OnPopup )

	EVT_LEFT_DOWN( UIForm::OnLeftButtonDown )
	EVT_LEFT_UP( UIForm::OnLeftButtonUp )
	EVT_RIGHT_DOWN( UIForm::OnRightButtonDown )
	EVT_MOTION( UIForm::OnMouseMove )
	EVT_PAINT( UIForm::OnPaint )
	EVT_SIZE( UIForm::OnResize )
END_EVENT_TABLE()

#define RSZBOXW 6
#define BOX_NONE 0
#define BOX_TOPLEFT 1
#define BOX_TOPRIGHT 2
#define BOX_BOTTOMLEFT 3
#define BOX_BOTTOMRIGHT 4
#define BOX_TOP 5
#define BOX_LEFT 6
#define BOX_RIGHT 7
#define BOX_BOTTOM 8


UIForm::UIForm( wxWindow *parent, int id, const wxString &name )
	: wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, 
		wxTAB_TRAVERSAL|wxWANTS_CHARS|wxCLIP_CHILDREN)
{
	mCopyBuffer = NULL;

	bDirty = false;
	mTabOrderCounter = 1;

	bTabOrderMode = false;
	bEditingAllowed = false;
	mPropEdit = NULL;
	mSymTab = NULL;
	bEditMode = false;
	mParent = parent;
	mName = name;
	mChildren.clear();
	
	SetBackgroundColour( *wxWHITE );

	mSelectColour = "magenta";
	
	mStandardCursor = wxCursor( wxCURSOR_ARROW ); 
	mMoveResizeCursor = wxCursor( wxCURSOR_SIZING );
	mNwSeCursor = wxCursor( wxCURSOR_SIZENWSE );
	mNSCursor = wxCursor( wxCURSOR_SIZENS );
	mWECursor = wxCursor( wxCURSOR_SIZEWE );
	mNeSwCursor = wxCursor( wxCURSOR_SIZENESW );

	SetCursor( mStandardCursor );
	
	bMultiSelMode = false;
	bMultiSelModeErase = false;

	bMoveMode = false;
	bMoveModeErase = false;
	mOrigX = mOrigY = mDiffX = mDiffY = 0;

	bResizeMode = false;
	bResizeModeErase = false;
	mResizeBox = BOX_NONE;

	SetClientSize(650,250);

	int i,ct;
	ControlInfo *ctrls = UIGetControls(ct);

	mPopupX = mPopupY = 0;
	mPopup = new wxMenu;
	for (i=0;i<ct;i++)
		mPopup->Append( ctrls[i].wxaddid, "Create '" + wxString(ctrls[i].name) + "'");

	wxMenu *align_menu = new wxMenu;
	align_menu->Append( IDIP_ALIGNTOP, "Top Edges");
	align_menu->Append( IDIP_ALIGNLEFT, "Left Edges");
	align_menu->Append( IDIP_ALIGNRIGHT, "Right Edges");
	align_menu->Append( IDIP_ALIGNBOTTOM, "Bottom Edges");

	mPopup->AppendSeparator();
	mPopup->AppendCheckItem(IDIP_TABORDERMODE, "Tab Order mode");
	mPopup->AppendSeparator();
	mPopup->AppendSubMenu( align_menu, "Align");
	mPopup->Append( IDIP_RESIZE_PANEL, "Resize panel");
	mPopup->Append( IDIP_DUPLICATE, "Duplicate");
	mPopup->Append( IDIP_DELETE, "Delete");
	mPopup->Append( IDIP_CLEARALL, "Clear all");
	mPopup->AppendSeparator();
	mPopup->Append( IDIP_COPY, "Copy");
	mPopup->Append( IDIP_PASTE, "Paste");
	mPopup->AppendSeparator();
	mPopup->Append( IDIP_WRITE, "Write to disk");
	mPopup->Append( IDIP_READ, "Read from disk");
	
}

void UIForm::Modified()
{
	bDirty = true;
	wxUIFormEvent evt(wxEVT_UIFORM_MODIFY, GetId());
	evt.SetEventObject(this);
	ProcessEvent(evt);
}

bool UIForm::IsDirty()
{
	return bDirty;
}

void UIForm::SetDirty(bool b)
{
	bDirty = b;
}

UIForm::~UIForm()
{
	delete mPopup;

	mSymTab = NULL;

	if (mPropEdit)
		mPropEdit->Unlink();

	DeleteChildren();
}

wxWindow *UIForm::GetWxParentWindow()
{
	return this;
}

bool UIForm::Destroy()
{
	mSymTab = NULL;

	if (mPropEdit)
		mPropEdit->Unlink();

	mPropEdit = NULL;

	return wxPanel::Destroy();
}

void UIForm::EnableTabOrderMode(bool b)
{
	if (b)
	{
		wxString result = wxGetTextFromUser("Enter starting tab index number:","","1",this);
		if (!result.IsEmpty())
		{
			mTabOrderCounter = atoi(result.c_str());
			if (mTabOrderCounter < 1) mTabOrderCounter = 1;
		}
		else		
			mTabOrderCounter = 1;
	}
	mSelectedItems.clear();
	bTabOrderMode = b;
	Refresh();
}

void UIForm::EnableEditMode(bool b)
{
	this->bEditingAllowed = b;
	if (EditMode() && !b)
		this->SetEditMode(false);
}


void UIForm::SetPropEditor(PropEditor *p)
{
	if (mPropEdit != NULL)
		mPropEdit->SetObject(NULL);

	mPropEdit = p;

	if (mPropEdit)
		mPropEdit->SetObject(NULL);
}

PropEditor *UIForm::GetPropEditor()
{
	return mPropEdit;
}

void UIForm::SetSymTab(SymTab *stab)
{
	mSymTab = stab;
}

SymTab *UIForm::GetSymTab()
{
	return mSymTab;
}

bool UIForm::EditMode()
{
	return bEditMode;
}

void UIForm::SetEditMode(bool b)
{
	if (bEditMode != b)
	{

		for (int i=0; i<mChildren.count();i++)
			mChildren[i]->Show( bEditingAllowed ? !b : true );

		bEditMode = bEditingAllowed ? b : false ;
		Refresh();

		ClearSelections();
		if (mPropEdit)
			mPropEdit->SetObject(NULL);
	}
}

UIObject *UIForm::CreateControl(ControlInfo *ctrlinfo, 
					int x, int y, int w, int h, const char *namebase)
{
	bool name_ok = (namebase!=NULL) ? (FindChild(namebase)==NULL) : false;
	wxString name = (namebase!=NULL) ? namebase : "";
	int index = 1;

	// create a new control
	while (!name_ok)
	{
		name.Printf("%s%d", namebase?namebase:ctrlinfo->name, index++);
		if (FindChild(name) == NULL)
			name_ok = true;
	}

	UIObject *obj1 = new UIObject( ctrlinfo, name, this);
	mChildren.append(obj1);

	if (w<0)
		w = ctrlinfo->width;
	if (h<0)
		h = ctrlinfo->height;

	obj1->SetGeom(x,y,w,h);
	obj1->ShowNative( !EditMode() );

	Modified();

	return obj1;
}


void UIForm::ClearSelections()
{
	if (!bEditMode)
		return;

	mSelectedItems.clear();
	if (mPropEdit)
		mPropEdit->SetObject(NULL);
	Refresh();
}

bool UIForm::AreObjectsSelected()
{
	return mSelectedItems.count() > 0;
}

int UIForm::GetSelectedCount()
{
	return mSelectedItems.count();
}

UIObject *UIForm::GetSelected(int i)
{
	return mSelectedItems[i];
}

void UIForm::WriteObject( wxOutputStream &_os, UIObject *obj)
{	
	wxDataOutputStream os(_os);

	os.Write8( 0xd3 ); // identifier code
	os.Write8( 1 ); // version

	os.WriteString( obj->GetName() );
	os.WriteString( obj->GetControlInfo()->name );

	wxRect rct = obj->GetGeom();
	
	os.Write16( rct.x );
	os.Write16( rct.y );
	os.Write16( rct.width );
	os.Write16( rct.height );

	int nprop = obj->NumProps();
	os.Write16( nprop );

	for ( int i=0;i<nprop;i++ )
		obj->GetCtrlProp(i)->Write( _os );

	os.Write8( 0xd3 );
}

bool UIForm::ReadObject( wxInputStream &_is )
{
	wxDataInputStream is(_is);

	wxUint8 code = is.Read8();
	wxUint8 ver = is.Read8();

	wxString name = is.ReadString();
	wxString type = is.ReadString();


	ControlInfo *kls = UIFindControlInfo( type );
	if ( !kls )
		return false;

	UIObject *obj = new UIObject( kls, name, this );
	mChildren.append( obj );

	wxUint16 x = is.Read16();
	wxUint16 y = is.Read16();
	wxUint16 width = is.Read16();
	wxUint16 height = is.Read16();
	
	obj->SetGeom( x, y, width, height );


	if (EditMode())
		obj->ShowNative(false);
	
	wxUint16 nprop = is.Read16();
	for (size_t i=0;i<nprop;i++)
	{
		CtrlProp mprop;
		mprop.Read(_is);
	
		CtrlProp *op = obj->FindProp(mprop.name);
		if (op)
		{
			*op = mprop;

			if (kls->f_nativesetprop)
				(*kls->f_nativesetprop)(obj, op);
		}
	}


	return ( code == is.Read8() );
}

bool UIForm::ReadObject(FILE *fp)
{
	wxRect geom;
	char namebuf[256];
	char typebuf[128];
	int nprops;
		
	fscanf(fp, "start %s type %s\n", namebuf, typebuf);
	fscanf(fp, "geom %d %d %d %d\n", &geom.x, &geom.y, &geom.width, &geom.height);
	fscanf(fp, "properties %d\n", &nprops);

	UIObject *obj = NULL;

	ControlInfo *kls = UIFindControlInfo( typebuf );
	if (kls)
	{
		obj = new UIObject( kls, wxString(namebuf), this);
		mChildren.append(obj);
	}
	else
		return false;
			
	if (!obj)
		return false;
	
	obj->SetGeom( geom );

	if (EditMode())
		obj->ShowNative(false);
	
	for (int i=0;i<nprops;i++)
	{
		CtrlProp mprop;
		mprop.Read(fp);
	
		CtrlProp *op = obj->FindProp(mprop.name);
		if (op)
		{
			*op = mprop;

			if (kls->f_nativesetprop)
				(*kls->f_nativesetprop)(obj, op);
		}
	}
	
	return true;	
}

void UIForm::WriteDatabase( wxOutputStream &_os)
{
	wxDataOutputStream os(_os);

	os.Write8( 0xd2 ); // pseudo-unique start identifier
	os.Write8( 1 ); // version
	
	wxSize sz = GetSize();
	
	os.Write16( sz.GetWidth() );
	os.Write16( sz.GetHeight() );

	os.Write16( mChildren.count() );

	// generate the code for widgets that are lowest on the 
	// stacking order, since the need to be added to the dialog first.
	// currently the only widget generated is a 'StaticBox'
	int count;
							
	Array<UIObject*> tabchildren; // to hold widgets with tab information
	Array<UIObject*> nontabchildren; // for all the other widgets
	Array<UIObject*> radiobuttons; // special list for radiobuttons since order matters for them
			
	int j;
	UIObject **children = GetChildren(count);
	for (j=0;j<count;j++)
	{
		children[j]->SetWxrDone(false);
			
		int curorder = -1;
		if ( (curorder = children[j]->GetTabOrder()) > 0 )
		{
			/* insert the current tab child in sorted order into the tabchildren array */
			int index = 0;
			int k;
			for (k=0;k<tabchildren.count();k++)
			{
				if (tabchildren[k]->GetTabOrder() >= curorder)
					break;

				index++;
			}				
			tabchildren.insert(children[j], index);
		}
		else
		{
			/* this widget does not support tabordering */
			nontabchildren.append(children[j]);
		}
	}
	
	/* now append all the non tab children to the end of the tabchildren 
	to get one correct list of widgets to output the code for */
	
	for (j=0;j<nontabchildren.count();j++)
		tabchildren.append( nontabchildren[j] );

	
	/* now put all the radiobuttons into the list with the correct taborder */

	for (j=0;j<tabchildren.count();j++)
	{
		if (strcmp(tabchildren[j]->GetControlInfo()->name, "RadioButton") == 0)
			radiobuttons.append( tabchildren[j] );
	}

	/* write out all the lowered widgets first
	to make sure the they're the lowest on the 
	stacking order in the final form (only GroupBoxes here) */
	for (j=0;j<tabchildren.count();j++)
	{
		UIObject *obj = tabchildren[j];
		ControlInfo *kls = obj->GetControlInfo();
		
		// it's a lowered widget and it hasn't been written yet	
		if (!obj->IsWxrDone() && strcmp(kls->name, "GroupBox") == 0)
		{		
			WriteObject( _os, obj);				
			obj->SetWxrDone(true);
		}
	}
	
	// generate code for all the other widgets,
	// but not the radio buttons, since
	// we need to generate those separately to get
	// the groups right

	for (j=0;j<tabchildren.count();j++)
	{
		UIObject *obj = tabchildren[j];
		ControlInfo *kls = obj->GetControlInfo();
		wxRect rct = obj->GetGeom();
		
		// make sure it's raised widget that hasn't been written yet
		if (obj->IsWxrDone())
			continue;

		if (strcmp(kls->name, "RadioButton") == 0)
		{
			WriteObject( _os, obj );
			obj->SetWxrDone(true);
			
			wxString curgroup = obj->GetPropString("group");
			// generate the code for all the radiobuttons in the same group
			for (int k=0;k<radiobuttons.count();k++)
			{
				UIObject *radio = radiobuttons[k];
				CtrlProp *pg = radio->FindProp("group");
				if (pg && pg->string == curgroup && !radio->IsWxrDone())
				{
					WriteObject(_os, radio);
					radio->SetWxrDone(true);
				}
				
			}
		}
		else
		{
			WriteObject( _os, obj );
			obj->SetWxrDone(true);
		}		
	}

	bDirty = false;
		
	os.Write8( 0xd2 ); // pseudo-unique start identifier
}

bool UIForm::ReadDatabase( wxInputStream &_is )
{
	wxDataInputStream is(_is);

	wxUint8 code = is.Read8();
	wxUint8 ver = is.Read8();

	ClearSelections();
	DeleteChildren();

	wxUint32 width = is.Read16();
	wxUint32 height = is.Read16();
	wxUint32 nc = is.Read16();

	if ( width < 10 ) width = 10;
	if ( width > 3000 ) width = 3000;
	if ( height < 10 ) height = 10;
	if ( height > 3000 ) height = 3000;

	SetSize( width, height );

	bool ok = true;
	for ( size_t i=0;i<nc;i++ )
		ok = ok && ReadObject( _is );

	return ( code == is.Read8() );// check code
}

bool UIForm::ReadDatabase(FILE *fp)
{
	if (!fp)
		return false;

	ClearSelections();
	DeleteChildren();

	int width=400, height=50, nchildren = 0;
	fscanf(fp, "form width %d height %d nguiobjects %d\n", &width, &height, &nchildren);

	if (width < 10)
		width = 10;
	if (width > 2200)
		width = 2200;

	if (height < 10)
		height = 10;
	if (height > 2200)
		height = 2200;

	SetSize(width, height);

	for (int i=0;i<nchildren;i++)
		ReadObject(fp);
	
	mSelectedItems.clear();
	Refresh();

	bDirty = false;
	return true;
}

void UIForm::SetName(const wxString &name)
{
	mName = name;
}

wxString UIForm::GetName()
{
	return mName;
}

void UIForm::SetCaption(const wxString &caption)
{
	mCaption = caption;
}

wxString UIForm::GetCaption()
{
	return (mCaption=="") ? mName : mCaption;
}

void UIForm::AddChild(UIObject *obj)
{
	if (mChildren.find(obj) < 0)
	{
		mChildren.prepend(obj);
		Modified();
	}
}

void UIForm::RemoveChild(UIObject *obj)
{
	mChildren.remove(obj);
	Modified();
}

void UIForm::DeleteChildren()
{

	int nremoved = 0;
	while (mChildren.count() > 0)
	{
		UIObject *child = mChildren[0];
		mChildren.remove(0);
		nremoved++;
		delete child;
	}

	if (nremoved > 0)
		Modified();
}

UIObject *UIForm::FindChild(const wxString &name)
{
	int ct = mChildren.count();
	for (int i=0;i<ct;i++)
	{
		if (mChildren[i]->GetName() == name)
			return mChildren[i];
	}
	return NULL;
}

UIObject** UIForm::GetChildren(int &count)
{
	count = mChildren.count();
	return mChildren.data();
}

void UIForm::RaiseChild(UIObject *child)
{
	int selfindex = -1;
	int i, count;

	count = mChildren.count();
	for (i=0;i<count;i++)
	{
		if (mChildren[i] == child)
		{
			selfindex = i;
			break;
		}
	}

	if (selfindex < 0)
		return;

	for (i=selfindex; i > 0; i--)
		mChildren[i] = mChildren[i-1];

	mChildren[0] = child;

	if (selfindex > 0)
		Modified();
}

int UIForm::IsOverResizeBox(int x, int y, UIObject *obj)
{
	if (!bEditMode)
		return BOX_NONE;

	wxRect rct = obj->GetGeom();
	
	if (x >= rct.x-RSZBOXW && x <= rct.x &&
			y >= rct.y-RSZBOXW && y <= rct.y)
		return BOX_TOPLEFT;	
	
	if (x >= rct.x-RSZBOXW && x <= rct.x &&
			y >= rct.y + rct.height/2 - RSZBOXW/2 && y <= rct.y + rct.height/2 + RSZBOXW/2)
		return BOX_LEFT;

	if (x >= rct.x-RSZBOXW && x <= rct.x &&
			y >= rct.y + rct.height && y <= rct.y + rct.height + RSZBOXW)
		return BOX_BOTTOMLEFT;

	
	if (x >= rct.x+rct.width/2-RSZBOXW/2 && x <= rct.x+rct.width/2+RSZBOXW/2 &&
			y >= rct.y-RSZBOXW && y <= rct.y)
		return BOX_TOP;
	
	if (x >= rct.x+rct.width/2-RSZBOXW/2 && x <= rct.x+rct.width/2+RSZBOXW/2 &&
			y >= rct.y + rct.height && y <= rct.y + rct.height + RSZBOXW)
		return BOX_BOTTOM;


	if (x >= rct.x+rct.width && x <= rct.x+rct.width+RSZBOXW &&
			y >= rct.y-RSZBOXW && y <= rct.y)
		return BOX_TOPRIGHT;	
	
	if (x >= rct.x+rct.width && x <= rct.x+rct.width+RSZBOXW &&
			y >= rct.y + rct.height/2 - RSZBOXW/2 && y <= rct.y + rct.height/2 + RSZBOXW/2)
		return BOX_RIGHT;

	if (x >= rct.x+rct.width && x <= rct.x+rct.width+RSZBOXW &&
			y >= rct.y + rct.height && y <= rct.y + rct.height + RSZBOXW)
		return BOX_BOTTOMRIGHT;

	return BOX_NONE;
}

void UIForm::OnLeftButtonDown(wxMouseEvent &evt)
{
	if (!bEditMode)
	{
		// propagate event to widget if non-native
	}
	else
	{	// edit mode, enable selections and moving
		int mx = evt.GetX();
		int my = evt.GetY();

		mOrigX = mx;
		mOrigY = my;

		ClientToScreen(&mOrigX, &mOrigY);
		if (bTabOrderMode)
		{
			int count = 0;
			UIObject **objs = GetChildren(count);

			for (int i=0;i<count;i++)
			{
				wxRect rct = objs[i]->GetGeom();
				if (mx >= rct.x && mx < rct.x+15 && my >= rct.y && my < rct.y+15)
				{
					objs[i]->SetCtrlProp("taborder", mTabOrderCounter++);
					Modified();
					Refresh();
					break;
				}
			}
			
			if (mTabOrderCounter > count)
			{
				bTabOrderMode = false;
				Refresh();
				return;
			}
		}
		else if (evt.AltDown())
		{
			mDiffX = 0;
			mDiffY = 0;
			bMultiSelMode = true;
			bMultiSelModeErase = true;
		}
		else if (mSelectedItems.count() == 1 && 
			IsOverResizeBox(mx, my, mSelectedItems[0]) > 0 )
		{
			mResizeBox = IsOverResizeBox(mx,my,mSelectedItems[0]);
			// start a resize
			mDiffX = 0;
			mDiffY = 0;
			mDiffW = 0;
			mDiffH = 0;
			bResizeMode = true;
			bResizeModeErase = false;
		}
		else
		{
			// handle a selection procedure	
			UIObject *select_obj = NULL;
			UIObject *move_obj = NULL;
			wxRect rct;
			int count = 0;
			UIObject **objs = GetChildren(count);

			for (int i=0;i<count;i++)
			{
				if ( (*objs[i]->GetControlInfo()->f_iswithin)(mx, my, objs[i]))
				{
					select_obj = objs[i];
					move_obj = objs[i];
					RaiseChild( select_obj );
					break;
				}
			}

			bool redraw = false;

			if (select_obj != NULL && mSelectedItems.find(select_obj) < 0)
			{
				if (!evt.ShiftDown())
				{
					mSelectedItems.clear();

					// callback for single item selection
					wxUIFormEvent evt( wxEVT_UIFORM_SELECT, this->GetId() );
					evt.SetEventObject( this );
					evt.SetUIObject( select_obj );
					ProcessEvent( evt );
				}

				mSelectedItems.append(select_obj);
				redraw = true;
			}
			else if (evt.ShiftDown() && select_obj != NULL && mSelectedItems.find(select_obj) >= 0)
			{
				mSelectedItems.remove(select_obj);
				move_obj = NULL;
				redraw = true;
			}
			else if (select_obj == NULL)
			{
				if (mSelectedItems.count() > 0)
					redraw = true;
				mSelectedItems.clear();
			}


			if (select_obj)
			{
				bMoveMode = true;
				bMoveModeErase = false;
				mDiffX = 0;
				mDiffY = 0;
				mDiffW = 0;
				mDiffH = 0;
			}

			if (redraw)
				Refresh();
			
			if (mPropEdit)
				mPropEdit->SetObject( GetSelectedCount() == 1 ? select_obj : NULL); 
		}
	}
}

void UIForm::OnLeftButtonUp(wxMouseEvent &evt)
{
	if (!bEditMode)
	{
		// propagate button up event to non-native control
	}
	else
	{
		// allow interface editing
		if (bMoveMode)
		{
			int i, count;
			count = mSelectedItems.count();
			for (i=0;i<count;i++)
			{
				wxRect rct = mSelectedItems[i]->GetGeom();
				rct.x += mDiffX;
				rct.y += mDiffY;
				UISnapPoint(&rct.x, &rct.y);
				mSelectedItems[i]->SetGeom(rct);
				if (mPropEdit)
					mPropEdit->UpdateView();
			}

			bMoveMode = false;
			bMoveModeErase = false;

			Modified();
			Refresh();
		}
		else if (bMultiSelMode)
		{
			wxRect selbox;	
			selbox.x = mDiffX<0 ? mOrigX + mDiffX : mOrigX;
			selbox.width = mDiffX<0 ? -mDiffX : mDiffX;
			selbox.y = mDiffY<0 ? mOrigY + mDiffY : mOrigY;
			selbox.height = mDiffY<0 ? -mDiffY : mDiffY;

			ScreenToClient(&selbox.x, &selbox.y);

			int count = mChildren.count();
			for (int i=0;i<count;i++)
			{
				wxRect rct = mChildren[i]->GetGeom();
				if (selbox.Contains(rct))
				{
					if (mSelectedItems.find(mChildren[i]) < 0)
						mSelectedItems.append( mChildren[i] );
					else
						mSelectedItems.remove( mChildren[i] );
				}
			}

			bMultiSelMode = false;
			bMultiSelModeErase = false;
			Modified();
			Refresh();
		}
		else if (bResizeMode && mSelectedItems.count() == 1)
		{	
			wxRect rct = mSelectedItems[0]->GetGeom();
			rct.x += mDiffX;
			rct.y += mDiffY;
			rct.width  += mDiffW;
			rct.height += mDiffH;

			if (rct.width < 5)
				rct.width = 5;
			if (rct.height < 5)
				rct.height = 5;

			mSelectedItems[0]->SetGeom(rct);
			if (mPropEdit)
				mPropEdit->UpdateView();

			bResizeMode = false;
			bResizeModeErase = false;
			Modified();
			Refresh();
		}
		
		SetCursor(mStandardCursor);
	}
}

void UIForm::DrawMultiSelBox()
{
	if (!bMultiSelMode)
		return;

	wxClientDC dc(this);
	dc.SetLogicalFunction( wxINVERT );
	wxBrush brush( *wxWHITE, wxTRANSPARENT );
	wxPen pen(*wxBLACK, 2, wxSOLID);
	pen.SetCap(wxCAP_BUTT);
	pen.SetJoin(wxJOIN_MITER);
	dc.SetBrush(brush);
	dc.SetPen(pen);

	wxRect selbox;	
	selbox.x = mDiffX<0 ? mOrigX + mDiffX : mOrigX;
	selbox.width = mDiffX<0 ? -mDiffX : mDiffX;
	selbox.y = mDiffY<0 ? mOrigY + mDiffY : mOrigY;
	selbox.height = mDiffY<0 ? -mDiffY : mDiffY;

	ScreenToClient(&selbox.x, &selbox.y);
	
	dc.DrawRectangle(selbox);

}


void UIForm::DrawMoveResizeOutlines()
{
	if (!bEditMode)
		return;

	wxClientDC dc(this);
	dc.SetLogicalFunction( wxINVERT );
	wxBrush brush( *wxWHITE, wxTRANSPARENT );
	wxPen pen(*wxBLACK, 2, wxSOLID);
	pen.SetCap(wxCAP_BUTT);
	pen.SetJoin(wxJOIN_MITER);
	dc.SetBrush(brush);
	dc.SetPen(pen);

	int i, count;
	count = mSelectedItems.count();
	for (i=0;i<count;i++)
	{
		wxRect rct = mSelectedItems[i]->GetGeom();

		rct.x = rct.x + mDiffX;
		rct.y = rct.y + mDiffY;
		rct.width = rct.width + mDiffW;
		rct.height = rct.height + mDiffH;
		UISnapPoint(&rct.x, &rct.y);

		dc.DrawRectangle(rct);
	}

}

void UIForm::SetResizeCursor(int pos)
{
	if (!bEditMode)
		return;

	if (pos < 0)
		pos = mResizeBox;

	switch(pos)
	{
	case BOX_TOPLEFT:
	case BOX_BOTTOMRIGHT:
		SetCursor( mNwSeCursor );
		break;
	case BOX_TOPRIGHT:
	case BOX_BOTTOMLEFT:
		SetCursor( mNeSwCursor );
		break;
	case BOX_TOP:
	case BOX_BOTTOM:
		SetCursor( mNSCursor );
		break;
	case BOX_LEFT:
	case BOX_RIGHT:
		SetCursor( mWECursor );
		break;
	default:
		SetCursor( mMoveResizeCursor );
	}
}

void UIForm::OnMouseMove(wxMouseEvent &evt)
{
	if (!bEditMode)
	{
		// propagate move event to non-native widget
	}
	else
	{
		int mx = evt.GetX();
		int my = evt.GetY();

		int xroot = mx;
		int yroot = my;
		ClientToScreen(&xroot, &yroot);

		if (bMoveMode)
		{
			if (bMoveModeErase)
				DrawMoveResizeOutlines();

			mDiffX = xroot - mOrigX;
			mDiffY = yroot - mOrigY;

			UISnapPoint(&mDiffX, &mDiffY);

			DrawMoveResizeOutlines();
			bMoveModeErase = true;
		}
		else if (bResizeMode)
		{
			SetResizeCursor();

			if (bResizeModeErase)
				DrawMoveResizeOutlines();

			int diffx = xroot - mOrigX;
			int diffy = yroot - mOrigY;

			switch(mResizeBox)
			{
			case BOX_TOPLEFT:
				mDiffX = diffx;
				mDiffY = diffy;
				mDiffW = -diffx;
				mDiffH = -diffy;
				break;
			case BOX_TOPRIGHT:
				mDiffX = 0;
				mDiffY = diffy;
				mDiffW = diffx;
				mDiffH = -diffy;
				break;
			case BOX_BOTTOMLEFT:
				mDiffX = diffx;
				mDiffY = 0;
				mDiffW = -diffx;
				mDiffH = diffy;
				break;
			case BOX_BOTTOMRIGHT:
				mDiffX = 0;
				mDiffY = 0;
				mDiffW = diffx;
				mDiffH = diffy;
				break;
			case BOX_TOP:
				mDiffX = 0;
				mDiffY = diffy;
				mDiffW = 0;
				mDiffH = -diffy;				
				break;
			case BOX_LEFT:
				mDiffX = diffx;
				mDiffY = 0;
				mDiffW = -diffx;
				mDiffH = 0;
				break;
			case BOX_RIGHT:
				mDiffX = 0;
				mDiffY = 0;
				mDiffW = diffx;
				mDiffH = 0;
				break;
			case BOX_BOTTOM:
				mDiffX = 0;
				mDiffY = 0;
				mDiffW = 0;
				mDiffH = diffy;
				break;
			default:
				break;
			}

			UISnapPoint(&mDiffX, &mDiffY);
			UISnapPoint(&mDiffW, &mDiffH);

			DrawMoveResizeOutlines();
			bResizeModeErase = true;
		}
		else if (bMultiSelMode)
		{
			if (bMultiSelModeErase)
				DrawMultiSelBox();

			mDiffX = xroot - mOrigX;
			mDiffY = yroot - mOrigY;

			DrawMultiSelBox();
			bMultiSelModeErase = true;
		}
		else if (mSelectedItems.count() == 1)
		{
			int box = IsOverResizeBox(mx, my, mSelectedItems[0]);
			if (box)
				SetResizeCursor(box);
			else
				SetCursor( mStandardCursor );
		}
	}
}

void UIForm::OnResize(wxSizeEvent &evt)
{
}


void UIForm::OnRightButtonDown(wxMouseEvent &evt)
{
	if (evt.ShiftDown() )
	{
		SetEditMode(!EditMode());
		return;
	}
	
	if (EditMode())
	{
		mPopupX = evt.GetX();
		mPopupY = evt.GetY();
		this->PopupMenu( mPopup, mPopupX, mPopupY );
	}
}

void UIForm::OnPaint(wxPaintEvent &evt)
{
	//wxAutoBufferedPaintDC dc(this);
	wxPaintDC dc(this);
	
	wxSize sz = GetSize();

	if (bEditMode)
	{
		dc.SetBrush(wxBrush(wxColour(230, 230, 230)));
		dc.SetPen(wxPen(wxColour(230, 230, 230)));
		dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
		
		dc.SetBrush(*wxLIGHT_GREY_BRUSH);
		dc.SetPen(*wxLIGHT_GREY_PEN);
		//gfx.DrawRectangle(0,0,sz.GetWidth(), sz.GetHeight(), false);

		int spacing = UIGetSnapSpacing() * 3;

		for (int i=spacing;i<sz.GetWidth();i+=spacing)
			for(int j=spacing;j<sz.GetHeight();j+=spacing)
				dc.DrawPoint(i , j);
	}
		
		// paint the children
	int count = 0;
	int i;
	wxRect rct;
	UIObject **objs = GetChildren(count);
	for (i=count-1;i>=0;i--)
	{
		if (bEditMode || (objs[i]->GetControlInfo()->type == CTRL_CUSTOM && objs[i]->IsShown()))
		{
			rct = objs[i]->GetGeom();

			dc.SetClippingRegion(rct);
			
			if (bEditMode && objs[i]->GetControlInfo()->draw_dotted_box)
			{
				wxPen p = wxPen(*wxBLACK, 1, wxSHORT_DASH);
				p.SetCap(wxCAP_BUTT);
				p.SetJoin(wxJOIN_MITER);
				dc.SetPen(p);
				dc.SetBrush(*wxTRANSPARENT_BRUSH);
				dc.DrawRectangle(rct.x, rct.y, rct.width, rct.height);
			}

			(*objs[i]->GetControlInfo()->f_paint)(dc, rct, objs[i]);

			dc.DestroyClippingRegion();
		}

		VarInfo *v = NULL;

//		if (mSymTab != NULL && (v=mSymTab->Lookup( objs[i]->GetName() )) != NULL && !v->GetAttr(VATTR_HIDELABELS))
// Allows for UICB function show_item to show or hide labels and controls and allows for IDE editor to show labels in edit mode
		if (mSymTab != NULL && (v=mSymTab->Lookup( objs[i]->GetName() )) != NULL && !v->GetAttr(VATTR_HIDELABELS) && (objs[i]->IsShown() || bEditMode))
		{
			/*
#ifdef __WXMAC__
			wxFont f = gfx.GetNormalFont(false);
			f.SetPointSize( f.GetPointSize()-1 );
			gfx.SetFont(f);
#else
*/
			dc.SetFont(*wxNORMAL_FONT);
//#endif
			rct = objs[i]->GetGeom();
			wxString buf;
			int sw, sh;

			if (bEditMode && v->GetDataSource() == VDSRC_CALCULATED)
				dc.SetTextForeground(*wxBLUE);
			/*else if ( !objs[i]->Enabled() )
				dc.SetTextForeground( *wxLIGHT_GREY ); */
			else
				dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));

			buf = v->GetLabel();
			dc.GetTextExtent(buf, &sw, &sh);
			dc.DrawText(buf, rct.x - sw - 3, rct.y+ rct.height/2-sh/2);

			buf = v->GetUnits();
			dc.GetTextExtent(buf, &sw, &sh);
			dc.DrawText(buf, rct.x + rct.width + 2, rct.y+ rct.height/2-sh/2);
		}
	}

	if (bEditMode)
	{
		// paint any selection handles
		dc.SetBrush(wxBrush(mSelectColour));
		dc.SetPen(wxPen(mSelectColour));
		count = mSelectedItems.count();
		for (i=0;i<count;i++)
		{
			wxRect rct = mSelectedItems[i]->GetGeom();
			
			// left side
			dc.DrawRectangle(rct.x - RSZBOXW, rct.y - RSZBOXW, RSZBOXW, RSZBOXW);
			dc.DrawRectangle(rct.x - RSZBOXW, rct.y + rct.height/2 - RSZBOXW/2, RSZBOXW, RSZBOXW);
			dc.DrawRectangle(rct.x - RSZBOXW, rct.y + rct.height, RSZBOXW, RSZBOXW);

			// right side
			dc.DrawRectangle(rct.x + rct.width, rct.y - RSZBOXW, RSZBOXW, RSZBOXW);
			dc.DrawRectangle(rct.x + rct.width, rct.y + rct.height/2 - RSZBOXW/2, RSZBOXW, RSZBOXW);
			dc.DrawRectangle(rct.x + rct.width, rct.y + rct.height, RSZBOXW, RSZBOXW);
			
			// bottom
			dc.DrawRectangle(rct.x + rct.width/2 - RSZBOXW/2, rct.y + rct.height, RSZBOXW, RSZBOXW);

			// top
			dc.DrawRectangle(rct.x + rct.width/2 - RSZBOXW/2, rct.y - RSZBOXW, RSZBOXW, RSZBOXW);
		}
	}

	if (bTabOrderMode)
	{
		wxFont f = *wxNORMAL_FONT;
		f.SetWeight(wxFONTWEIGHT_BOLD);
		dc.SetFont(f);
		objs = GetChildren(count);
		for (i=0;i<count;i++)
		{
			if ( objs[i]->FindProp("taborder") != NULL)
			{
				rct = objs[i]->GetGeom();
				
				int tw, th;
				wxString tabnum = wxString::Format("%d",objs[i]->GetTabOrder());
				dc.GetTextExtent(tabnum, &tw, &th);

				dc.SetBrush(*wxLIGHT_GREY_BRUSH);
				dc.SetPen(*wxLIGHT_GREY_PEN);
				dc.DrawRectangle(rct.x, rct.y, tw+4, th+4);

				dc.SetTextForeground(*wxRED);
				dc.DrawText(tabnum, rct.x+2, rct.y+2);

				dc.SetBrush(*wxTRANSPARENT_BRUSH);
				dc.SetPen(wxPen(*wxBLUE));
				dc.DrawRectangle(rct.x, rct.y, tw+4, th+4);
				dc.DrawRectangle(rct.x-2, rct.y-2, rct.width+4, rct.height+4);
			}
		}
	}

}

void UIForm::SetCopyBuffer(UIObjectCopyBuffer *cpbuf)
{
	mCopyBuffer = cpbuf;
}

void UIForm::OnPopup(wxCommandEvent &evt)
{

	switch (evt.GetId())
	{
	case IDIP_COPY:
	case IDIP_PASTE:
		if (!mCopyBuffer)
		{
			wxMessageBox("No copy buffer available to form editor.");
			return;
		}
		else if (evt.GetId() == IDIP_COPY)
		{
			Array<UIObject*> list;
			int i, count = mSelectedItems.count();
			for (i=0;i<count;i++)
			{
				UIObject *tocopy = mSelectedItems[i];
				UIObject *obj = new UIObject( tocopy->GetControlInfo(), tocopy->GetName(), NULL );
				obj->SetGeom( tocopy->GetGeom() );
				obj->CopyProps( tocopy );
				list.append(obj);
			}

			mCopyBuffer->Assign( list );
		}
		else if (evt.GetId() == IDIP_PASTE)
		{
			Array<UIObject*> topaste = mCopyBuffer->Get();
			int i;
			Array<UIObject*> added;
			for (i=0;i<topaste.count();i++)
			{
				wxRect geom = topaste[i]->GetGeom();
				UIObject *obj = CreateControl( topaste[i]->GetControlInfo(), geom.x, geom.y, geom.width, geom.height, topaste[i]->GetName().c_str());
				obj->CopyProps( topaste[i] );
				Modified();
				added.append(obj);
			}
			mSelectedItems = added;
			Refresh();
		}
		break;
	case IDIP_DUPLICATE:
		if (AreObjectsSelected())
		{
			Array<UIObject*> added;
			
			int x0,y0;
			int dx,dy;
			int i, count = mSelectedItems.count();
			for (i=0;i<count;i++)
			{
				UIObject *tocopy = mSelectedItems[i];
				wxRect geom = tocopy->GetGeom();
				if (i==0)
				{
					x0 = geom.x;
					y0 = geom.y;
				}

				dx = geom.x - x0;
				dy = geom.y - y0;
						

				UIObject *obj = CreateControl( tocopy->GetControlInfo(),
					mPopupX+dx, mPopupY+dy, geom.width, geom.height, tocopy->GetName().c_str());

				obj->CopyProps(tocopy);

				added.append( obj );
				Modified();

			}

			mSelectedItems = added;
			Refresh();
		}
		break;

	case IDIP_TABORDERMODE:
		EnableTabOrderMode( !bTabOrderMode );
		Refresh();
		break;

	case IDIP_DELETE:
		if (AreObjectsSelected())
		{
			int i,count;
			count = this->GetSelectedCount();
			for (i=0;i<count;i++)
			{
				RemoveChild( GetSelected(i) );
				delete GetSelected(i);
			}
			ClearSelections();
		}
		break;
	case IDIP_CLEARALL:
		if (wxMessageBox("Really clear the form?", "Query", wxYES_NO|wxICON_EXCLAMATION) == wxYES)
		{
			ClearSelections();
			DeleteChildren();
			Refresh();
		}
		break;
	case IDIP_RESIZE_PANEL:
		ResizePanel();
		break;
	case IDIP_WRITE:
		Write();
		break;
	case IDIP_READ:
		Read();
		break; 
	case IDIP_ALIGNTOP:
	case IDIP_ALIGNLEFT:
	case IDIP_ALIGNRIGHT:
	case IDIP_ALIGNBOTTOM:
		if (GetSelectedCount() > 1)
		{
			int i,count;
			count = GetSelectedCount();
			wxRect geom = GetSelected(0)->GetGeom();
			for (i=1;i<count;i++)
			{
				wxRect objgeom = GetSelected(i)->GetGeom();

				if (evt.GetId() == IDIP_ALIGNTOP)
					objgeom.y = geom.y;
				else if (evt.GetId() == IDIP_ALIGNLEFT)
					objgeom.x = geom.x;
				else if (evt.GetId() == IDIP_ALIGNRIGHT)
					objgeom.x = (geom.x+geom.width)-objgeom.width;
				else if (evt.GetId() == IDIP_ALIGNBOTTOM)
					objgeom.y = (geom.y+geom.height)-objgeom.height;

				GetSelected(i)->SetGeom(objgeom);
				Modified();
			}

			Refresh();
		}
		break;
	}
}

enum { ID_numWidth = wxID_HIGHEST + 134,
		ID_numHeight,
		ID_sldrWidth,
		ID_sldrHeight };

class PanelResizeDialog : public wxDialog
{
	UIForm *m_guiForm;
	AFNumeric *numWidth, *numHeight;
	wxSlider *sldrWidth, *sldrHeight;


public:
	PanelResizeDialog( UIForm *parent, const wxString &title )
		: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize(500,200), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
		m_guiForm(parent)
	{
		
		numWidth = new AFNumeric(this, ID_numWidth, 0, true);
		numWidth->SetFormat( "%d pix");
		numWidth->SetInt( (int) 0 );
		numWidth->SetEditable( false );
		numHeight = new AFNumeric(this, ID_numHeight, 0, true);
		numHeight->SetFormat( "%d pix");
		numHeight->SetInt( (int) 0 );
		numHeight->SetEditable( false );

		sldrWidth = new wxSlider(this, ID_sldrWidth,  600, 30, 2100 );
		sldrHeight = new wxSlider(this, ID_sldrHeight,  100, 30, 2100 );

		wxFlexGridSizer *sz = new wxFlexGridSizer(3, 3);

		sz->AddGrowableCol(2);
		sz->Add( new wxStaticText( this, wxID_ANY, "Width" ) );
		sz->Add( numWidth );
		sz->Add( sldrWidth, 1, wxALL|wxEXPAND );
		sz->Add( new wxStaticText( this, wxID_ANY, "Height" ) );
		sz->Add( numHeight );
		sz->Add( sldrHeight, 1, wxALL|wxEXPAND );

		wxBoxSizer *szv = new wxBoxSizer(wxVERTICAL );
		szv->Add(sz, 1, wxALL|wxEXPAND, 10);
		szv->Add( CreateButtonSizer(wxCANCEL), 0, wxALL|wxEXPAND, 10 );
		SetSizer(szv);

		
		wxSize size = m_guiForm->GetClientSize();
		sldrWidth->SetValue( size.GetWidth() );
		sldrHeight->SetValue( size.GetHeight() );
		numWidth->SetInt( size.GetWidth() );
		numHeight->SetInt( size.GetHeight() );
	}

	
	void OnScroll(wxScrollEvent &evt)
	{
		if (evt.GetId() == ID_sldrWidth)
			numWidth->SetInt( sldrWidth->GetValue() );
		else if (evt.GetId() == ID_sldrHeight)
			numHeight->SetInt( sldrHeight->GetValue() );
	
		if (m_guiForm)
		{
			m_guiForm->SetClientSize( sldrWidth->GetValue(), sldrHeight->GetValue() );
			m_guiForm->Refresh();
		}
	}

	DECLARE_EVENT_TABLE();
};


BEGIN_EVENT_TABLE( PanelResizeDialog, wxDialog )
	EVT_COMMAND_SCROLL( ID_sldrWidth, PanelResizeDialog::OnScroll )
	EVT_COMMAND_SCROLL( ID_sldrHeight, PanelResizeDialog::OnScroll )
END_EVENT_TABLE()


void UIForm::ResizePanel()
{
	PanelResizeDialog dlg(this, "Resize Panel");
	dlg.ShowModal();
	Modified();
}

void UIForm::OnAddCtrl(wxCommandEvent &evt)
{
	ControlInfo *kls = UIFindControlInfo( evt.GetId() );
	if (kls)
	{
		int x = mPopupX - kls->width/2;
		int y = mPopupY - kls->height/2;
		UIObject *obj = CreateControl( kls, x, y );
		
		mSelectedItems.clear();
		if (mPropEdit)
			mPropEdit->SetObject(NULL);

		Refresh();
	}
}

void UIForm::OnNativeEvent(UIObject *obj)
{
	/* nothing to do */
}

void UIForm::NativeCtrlEvent(wxCommandEvent &evt)
{
	UIObject *obj = NULL;
	int i;
	for (i=0;i<mChildren.count();i++)
	{
		if (evt.GetEventObject() == mChildren[i]->GetNativeObj())
		{
			obj = mChildren[i];
			break;
		}
	}

	if (obj)
	{
		// first allow the native object to
		// update the virtual wrapper
		if (!obj->OnNativeEvent( evt ))
			CtrlToVar( obj );// update the symbol table value

		// send notifcation of native event to descendant classes
		OnNativeEvent( obj );
	}

	evt.StopPropagation();
}

void UIForm::HandleVarToCtrl(VarInfo *v, UIObject *o)
{
	ControlInfo *kls = o->GetControlInfo();
	if (kls->f_vartoctrl)
		(*kls->f_vartoctrl)(v,o);
}

void UIForm::HandleCtrlToVar(UIObject *o, VarInfo *v)
{
	ControlInfo *kls = o->GetControlInfo();
	if (kls->f_ctrltovar)
	{
		if ((*kls->f_ctrltovar)(o,v))
		{
			OnValueChanged(v);
		}
		else
		{
			HandleVarToCtrl(v,o); // change back to value before indicating error
			OnValueError(v);
		}
	}
}

void UIForm::VarToCtrl(UIObject *o)
{
	if (!this->mSymTab || !o)
		return;

	VarInfo *v = mSymTab->Lookup( o->GetName() );
	if (!v)
		return;

	HandleVarToCtrl(v, o);
}

void UIForm::CtrlToVar(UIObject *o)
{
	if ( !mSymTab || !o ) return;
	if (VarInfo *v = mSymTab->Lookup( o->GetName() ))
		HandleCtrlToVar(o, v);
}

void UIForm::AllVarsToCtrls()
{
	Freeze();
	for (int i=0;i<mChildren.count();i++)
		VarToCtrl( mChildren[i] );
	Thaw();
}

void UIForm::OnValueChanged(VarInfo *v)
{
	/* nothing to do here */
}

void UIForm::OnValueError(VarInfo *v)
{
	/* nothing to do here */
}

void UIForm::Read()
{
	wxMessageBox("UIForm::Read not implemented.");
}

void UIForm::Write()
{
	wxMessageBox("UIForm::Write not implemented.");
}
