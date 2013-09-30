#ifndef __uiform_h
#define __uiform_h

#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

class wxUIProperty;

class wxUIPropertyUpdateInterface
{
public:
	virtual void OnPropertyChanged( const wxString &id, wxUIProperty *p ) = 0;
};

class wxUIProperty
{
public:
	explicit wxUIProperty();
	explicit wxUIProperty( wxUIProperty *ref );
	explicit wxUIProperty( double d );
	explicit wxUIProperty( int i );
	explicit wxUIProperty( int i, const wxArrayString &named_options );
	explicit wxUIProperty( bool b );
	explicit wxUIProperty( const wxString &s );
	explicit wxUIProperty( const wxColour &c );
	explicit wxUIProperty( const wxImage &img );
	explicit wxUIProperty( const wxArrayString &strlist );

	enum { INVALID, DOUBLE, BOOLEAN, INTEGER, COLOUR, STRING, STRINGLIST, IMAGE };

	int Type();

	void Set( double d );
	void Set( bool b );
	void Set( int i );
	void Set( const wxColour &c );
	void Set( const wxString &s );
	void Set( const wxArrayString &list);
	void Set( const wxImage &img );
	
	void SetNamedOptions( const wxArrayString &opts, int selection = -1 );
	wxArrayString GetNamedOptions();

	int GetInteger();
	bool GetBoolean();
	double GetDouble();
	wxColour GetColour();
	wxString GetString();
	wxArrayString GetStringList();
	wxImage GetImage();

	void Write( wxOutputStream & );
	bool Read( wxInputStream & );
	
	void AddUpdateInterface( const wxString &name, wxUIPropertyUpdateInterface *pui );
	void RemoveUpdateInterface( wxUIPropertyUpdateInterface *pui );
	void ClearUpdateInterfaces();

private:
	void Init();
	wxUint8 m_type;
	wxUIProperty *m_pReference;

	double m_doubleVal;
	bool m_boolVal;
	int m_intVal;
	wxColour m_colour;
	wxString m_string;
	wxImage m_image;
	wxArrayString m_strList;
	wxArrayString m_namedOptions;
	
	void ValueChanged();
	struct puidata { wxUIPropertyUpdateInterface *pui; wxString id; };
	std::vector<puidata> m_updateInterfaceList;
};

class wxUIObject : public wxUIPropertyUpdateInterface
{
public:
	wxUIObject( );
	virtual ~wxUIObject();

	virtual wxString GetTypeName() = 0;
	virtual wxUIObject *Duplicate() = 0;
	virtual bool Copy( wxUIObject *rhs );
	virtual void Draw( wxDC &dc, const wxRect &geom );
	virtual bool IsWithin( int xx, int yy );
	
	/* methods for handling native controls */
	virtual bool CreateNativeWidget( wxWindow *parent );
	virtual void DestroyNativeWidget();
	virtual wxWindow *GetNativeWidget();
	virtual void OnNativeEvent( );
	virtual void OnPropertyChanged( const wxString &id, wxUIProperty *p );
	
	void SetName( const wxString &name );
	wxString GetName();
	void SetGeometry( const wxRect &r );
	wxRect GetGeometry();

	wxUIProperty &Property( const wxString &name );
	wxArrayString Properties();
		
	virtual void Write( wxOutputStream & );
	virtual bool Read( wxInputStream & );
	
	void Show( bool b ) { m_visible = b; }
	bool IsVisible() { return m_visible; }

protected:
	void AddProperty( const wxString &name, wxUIProperty *prop );
	
private:
	void DeleteProperties();
	bool m_visible;
	struct propdata { wxString name, lowered; wxUIProperty *prop; };
	std::vector<propdata> m_properties;
	
};

class wxUIPropertyEditor : public wxPropertyGrid
{
// Note: this class issues the EVT_PG_CHANGED( ) event
// when a property has changed and the editor or UI needs to be updated
// To know which property last changed, use the "GetLastChangedProperty()"
// method, or, query the wxPGProperty's label.
public:
	wxUIPropertyEditor( wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );
	virtual ~wxUIPropertyEditor();
	
	void SetObject( wxUIObject *obj );
	wxUIObject *GetObject() { return m_curObject; }
	wxString GetLastChangedProperty() { return m_lastChangedProperty; }
	void UpdatePropertyValues();
	
private:
    void OnPropertyGridChange(wxPropertyGridEvent &evt);
    void OnPropertyGridChanging(wxPropertyGridEvent &evt);
	
	struct pgpinfo {
		wxString name;
		int type;
		wxPGProperty *pgp;
	};
	
	void ValueToPropGrid( pgpinfo &p );
	void PropGridToValue( pgpinfo &p );
	
	wxUIObject *m_curObject;
	wxString m_lastChangedProperty;
	
	DECLARE_EVENT_TABLE();
};




class wxUIFormEvent : public wxCommandEvent
{
public:
	wxUIFormEvent( wxEventType commandType = wxEVT_NULL, int id = 0);

	UIObject *GetUIObject() { return m_guiobj; }
	void SetUIObject(UIObject *o) { m_guiobj = o; }

private:
	UIObject *m_guiobj;
};

DECLARE_EVENT_TYPE( wxEVT_UIFORM_SELECT, -1 )
DECLARE_EVENT_TYPE( wxEVT_UIFORM_MODIFY, -1 )

typedef void (wxEvtHandler::*wxUIFormEventFunction)(wxUIFormEvent&);

#define EVT_UIFORM_GENERIC(id, type, fn) \
    DECLARE_EVENT_TABLE_ENTRY( type, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
    wxStaticCastEvent( wxUIFormEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_UIFORM_SELECT( id, fn ) EVT_UIFORM_GENERIC( id, wxEVT_UIFORM_SELECT, fn )
#define EVT_UIFORM_MODIFY( id, fn ) EVT_UIFORM_GENERIC( id, wxEVT_UIFORM_MODIFY, fn )

class wxUIObjectCopyBuffer
{
public:
	wxUIObjectCopyBuffer();
	~wxUIObjectCopyBuffer();

	void Clear();
	void Assign( std::vector<wxUIObject*> &objlist);
	std::vector<wxUIObject*> Get();
	int Count();

private:
	std::vector<wxUIObject*> m_copyList;
};



class wxUIFormData
{
public:
	wxUIFormData( );
	virtual ~wxUIFormData();

	/* for descendant interaction */
	virtual void OnNativeEvent( UIObject *obj );
	virtual void OnValueError( VarInfo *v );
	virtual void OnValueChanged( VarInfo *v );
	virtual void Read();
	virtual void Write();

	/* mechanism to override control/variable data transfer 
	   default behavior transfers variable value */
	virtual void HandleVarToCtrl(VarInfo *v, UIObject *o);
	virtual void HandleCtrlToVar(UIObject *o, VarInfo *v);

	void SetCopyBuffer(UIObjectCopyBuffer *cpbuf);

	void VarToCtrl(UIObject *o);
	void CtrlToVar(UIObject *o);
	virtual void AllVarsToCtrls();

	void EnableEditMode(bool b=true);

	void SetName(const wxString &name);
	wxString GetName();

	void SetCaption(const wxString &caption);
	wxString GetCaption();

	void SetPropEditor(PropEditor *p);
	PropEditor *GetPropEditor();

	void SetSymTab(SymTab *stab);
	SymTab *GetSymTab();

	virtual bool EditMode();
	void SetEditMode(bool b = false);

	UIObject *CreateControl(ControlInfo *ctrlinfo, 
		int x=10, int y=10, int w=-1, int h=-1, const char *namebase=NULL);

	void AddChild(UIObject *obj);
	void RemoveChild(UIObject *obj);
	UIObject *FindChild(const wxString &name);
	virtual UIObject **GetChildren(int &count);
	void RaiseChild(UIObject *obj);
	void DeleteChildren();

	void Draw(wxDC &dc);
	virtual void RefreshView() { Refresh(); }

	virtual wxWindow *GetWxParentWindow();

	bool AreObjectsSelected();
	int GetSelectedCount();
	UIObject *GetSelected(int i);
	void ClearSelections();
	
	void WriteDatabase( wxOutputStream &_os );
	bool ReadDatabase( wxInputStream &_is );

	bool ReadDatabase(FILE *fp);

	void ResizePanel();

	void EnableTabOrderMode(bool b);

	bool IsDirty();
	void SetDirty(bool b);

	void Modified(); // sets bDirty = true and sends MODIFY event
private:
	void WriteObject( wxOutputStream &_os, UIObject *obj );
	bool ReadObject( wxInputStream &_is );

	bool ReadObject(FILE *fp);

	void DrawMultiSelBox();
	void DrawMoveResizeOutlines();
	int IsOverResizeBox(int x, int y, UIObject *obj);
	void SetResizeCursor(int pos = -1);

	UIObjectCopyBuffer *mCopyBuffer;

	Array<UIObject*> mSelectedItems;

	wxColour mSelectColour;

	wxCursor mStandardCursor;
	wxCursor mMoveResizeCursor;
	wxCursor mNwSeCursor;
	wxCursor mNSCursor;
	wxCursor mWECursor;
	wxCursor mNeSwCursor;

	int mTabOrderCounter;
	bool bTabOrderMode;
	bool bEditingAllowed;
	bool bEditMode;
	bool bMoveMode;
	bool bMoveModeErase;
	bool bMultiSelMode;
	bool bMultiSelModeErase;
	int mOrigX, mOrigY, mDiffX, mDiffY, mDiffW, mDiffH;
	bool bResizeMode;
	bool bResizeModeErase;
	int mResizeBox;
	bool bDirty;
	
	PropEditor *mPropEdit;
	SymTab *mSymTab;

	wxString mName;
	wxString mCaption;
	wxWindow *mParent;
	Array<UIObject*> mChildren;
	wxMenu *mPopup;
	int mPopupX, mPopupY;

	void OnMouseMove(wxMouseEvent &evt);
	void OnLeftButtonUp(wxMouseEvent &evt);
	void OnLeftButtonDown(wxMouseEvent &evt);
	void OnRightButtonDown(wxMouseEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnPaint(wxPaintEvent &evt);

	void OnPopup(wxCommandEvent &evt);
	void OnAddCtrl(wxCommandEvent &evt);

	/* native control event handler/propagator */
	void NativeCtrlEvent(wxCommandEvent &evt);

	DECLARE_EVENT_TABLE()
};

#endif
