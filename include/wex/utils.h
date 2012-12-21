#ifndef __util_h
#define __util_h

#include <vector>
#include <wx/string.h>
#include <wx/arrstr.h>

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

#endif

