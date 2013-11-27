#ifndef __util_h
#define __util_h

#include <vector>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dc.h>

#ifdef _MSC_VER
#include <unordered_map>
using std::tr1::unordered_map;
#pragma warning(disable: 4290)  // ignore warning: 'C++ exception specification ignored except to indicate a function is not __declspec(nothrow)'
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif

#include <wx/hashmap.h>
#include <wx/stream.h>

class wxGrid;


std::vector<int> wxCommaDashListToIndices(const wxString &value);
wxString wxLimitTextColumns(const wxString &str, size_t numcols);

wxString wxConvertToBase26(unsigned int val);
unsigned int wxConvertFromBase26(const wxString &val);
wxArrayString wxEnumerateAlphaIndex(const wxString &_start, const wxString &_end);

wxString wxWebHttpGet(const wxString &url, 
					  const wxString &addtlhdr_name=wxEmptyString, 
					  const wxString &addtlhdr_value=wxEmptyString);

bool wxWebHttpDownload(const wxString &url, 
					   const wxString &local_file,
					   int timeout = 10 /*seconds*/, 
					   const wxString &mime = "application/binary",
					   bool with_progress_dialog=true, 
					   void (*callback)(int bytes, int total, void *data)=NULL, 
					   void *data=NULL);

bool wxDecompressFile(const wxString &archive, const wxString &target);
bool wxUnzipFile(const wxString &archive, const wxString &target);
bool wxUntarFile(const wxString &archive, const wxString &target);
bool wxGunzipFile(const wxString &archive, const wxString &target);


enum wxArrowType { wxARROW_UP, wxARROW_DOWN, wxARROW_LEFT, wxARROW_RIGHT };

int wxDrawWordWrappedText(wxDC& dc, const wxString &str, int width, bool draw=false, int x=0, int y=0, wxArrayString *lines=NULL);
void wxDrawRaisedPanel(wxDC &dc, int x, int y, int width, int height);
void wxDrawSunkenPanel(wxDC &dc, int x, int y, int width, int height);
void wxDrawEngravedPanel(wxDC &dc, int x, int y, int width, int height, bool fill);
void wxDrawScrollBar(wxDC &dc, bool vertical, int x, int y, int width, int height);
void wxDrawArrowButton(wxDC &dc, wxArrowType type, int x, int y, int width, int height);
void wxDrawArrow(wxDC &dc, wxArrowType type, int x, int y, int width, int height);


void wxShowTextMessageDialog(const wxString &text, const wxString &title = wxEmptyString, wxWindow *parent = 0, const wxSize &size = wxSize(600,400));

class wxCSVData
{
public:
	wxCSVData();
	wxCSVData( const wxCSVData &copy );
	virtual ~wxCSVData();

	void Set( size_t r, size_t c, const wxString &val );
	wxString &operator()(size_t r, size_t c);
	wxString Get( size_t r, size_t c ) const;
	const wxString &operator()(size_t r, size_t c) const;

	size_t NumCells() const;
	size_t NumRows();
	size_t NumCols();

	void Clear();
	void Clear( size_t r, size_t c );
	bool IsEmpty( size_t r, size_t c );
	
	bool Read( wxInputStream &in );
	void Write( wxOutputStream &out );

	bool ReadFile( const wxString &file );
	bool WriteFile( const wxString &file );

	bool ReadString( const wxString &data );
	wxString WriteString();

	int GetErrorLine() { return m_errorLine; }

	void SetSeparator( wxUniChar sep ) { m_sep = sep; }
	wxUniChar GetSeparator();
	
protected:
	wxUniChar m_sep;
	bool m_invalidated;
	size_t m_nrows, m_ncols;
	typedef unordered_map<wxUint64, wxString> cell_hash;
	cell_hash m_cells;
	int m_errorLine;
	
	wxUint64 Encode( size_t r, size_t c ) const;
	void Decode( wxUint64 idx, size_t *r, size_t *c ) const;
	void RecalculateDimensions();
};

#endif

