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


/* time/date helper functions for working with generic years */

// number of days in each month
extern int wxNDay[12];

/* month: 1-12 time: hours, starting 0=jan 1st 12am, returns 1..nday*/
int wxDayOfMonth(int month, double time);

/* returns month number 1..12 given 
	time: hour index in year 0..8759 */
int wxMonthOf(double time);

/* converts 'time' (hours since jan 1st 12am, 0 index) to M(1..12), D(1..N), H(0..23), M(0..59) */
void wxTimeToMDHM( double time, int *mo, int *dy, int *hr, int *min = 0 );

/* converts M(1..12), D(1..N), H(0..23) M(0..59) to time in hours since jan 1st 12am, 0 index ) */
double wxMDHMToTime( int mo, int dy, int hr, int min = 0);

/* format a MDHM time into a pretty string */
wxString wxFormatMDHM( int mo, int dy, int hr, int min = 0, bool use_12_hr = true );
wxString wxFormatTime( double time, bool use_12_hr = true );


// sort (n^2) names and labels together
void wxSortByLabels(wxArrayString &names, wxArrayString &labels);

#endif

