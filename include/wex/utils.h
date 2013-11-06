#ifndef __util_h
#define __util_h

#include <vector>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dc.h>

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

#endif

