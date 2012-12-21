#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <cmath>

#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/tokenzr.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/uri.h>
#include <wx/progdlg.h>
#include <wx/tarstrm.h>
#include <wx/zstream.h>

#include "wex/utils.h"
/*
bool AllocReadLine(FILE *fp, wxString &buf, int prealloc)
{
	char c;

	buf = "";
	if (prealloc > 10)
		buf.Alloc( prealloc );

	// read the whole line, 1 character at a time, no concern about buffer length
	while ( (c=fgetc(fp)) != EOF && c != '\n' && c != '\r')
		buf += c;

	// handle windows <CR><LF>
	if (c == '\r')
	{
		if ( (c=fgetc(fp)) != '\n')
			ungetc(c,fp);
	}

	// handle a stray <CR>
	if (c == '\n')
	{
		if ( (c=fgetc(fp)) != '\r')
			ungetc(c,fp);
	}

	return !(buf.Len() == 0 && c == EOF);
}
*/

wxString wxLimitTextColumns(const wxString &str, size_t numcols)
{
	wxString buf;
	size_t len = (int)str.Len();
	size_t col=0;
	for (size_t i=0;i<len;i++)
	{
		if (col == numcols)
		{
			while (i < len && str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
			{
				buf += str[i];
				i++;
			}
			
			while (i < len && (str[i] == ' ' || str[i] == '\t'))
				i++;

			if (i<len)
				buf += '\n';
			col = 0;
			i--;
		}
		else
		{
			buf += str[i];

			if (str[i] == '\n')
				col = 0;
			else
				col++;
		}
	}

	return buf;
}
/*
void SortByLabels(wxArrayString &names, wxArrayString &labels)
{
	// sort the selections by labels
	wxString buf;
	int count = (int)labels.Count();
	for (int i=0;i<count-1;i++)
	{
		int smallest = i;

		for (int j=i+1;j<count;j++)
			if ( labels[j] < labels[smallest] )
				smallest = j;

		// swap
		buf = labels[i];
		labels[i] = labels[smallest];
		labels[smallest] = buf;

		buf = names[i];
		names[i] = names[smallest];
		names[smallest] = buf;

	}
}

void SortByValues(wxArrayString &names, Vector<double> &values, bool useabs)
{
	if (names.Count() != values.length())
		return;

	// sort the selections by values
	wxString buf;
	double dval;
	int count = (int)values.length();
	for (int i=0;i<count-1;i++)
	{
		int smallest = i;

		for (int j=i+1;j<count;j++)
		{
			if (useabs)
			{
				if ( fabs(values[j]) < fabs(values[smallest]) )
					smallest = j;
			}
			else
			{
				if ( values[j] < values[smallest] )
					smallest = j;
			}
		}

		// swap
		dval = values[i];
		values[i] = values[smallest];
		values[smallest] = dval;

		buf = names[i];
		names[i] = names[smallest];
		names[smallest] = buf;

	}
}

void SortByValues(Vector<double> &sort_also, Vector<double> &values, bool useabs)
{
	if (sort_also.length() != values.length())
		return;

	// sort the selections by values
	double buf;
	double dval;
	int count = (int)values.length();
	for (int i=0;i<count-1;i++)
	{
		int smallest = i;

		for (int j=i+1;j<count;j++)
		{
			if (useabs)
			{
				if ( fabs(values[j]) < fabs(values[smallest]) )
					smallest = j;
			}
			else
			{
				if ( values[j] < values[smallest] )
					smallest = j;
			}
		}

		// swap
		dval = values[i];
		values[i] = values[smallest];
		values[smallest] = dval;

		buf = sort_also[i];
		sort_also[i] = sort_also[smallest];
		sort_also[smallest] = buf;

	}
}

*/


	/*
bool DeleteDirectory(const wxString &path, void (*cb)(const wxString &fn, void *data), void *data)
{
	// Can't use a stack allocated copy b/c 
	// wxWidgest doesn't provide a CloseDir method.
	// Must close the dir by deleting object
	// at end of function to be able to actually
	// remove the directory
	wxDir *dir = new wxDir( path );
	
	//applog("deldir: %s\n", path.c_str());
	if (!dir->IsOpened())
	{
		delete dir;
		return false;
	}
	
	// preload the file names - otherwise GetNext fails on unix/mac
	// because deleting stuff changes the dir contents
	wxString f;
	wxArrayString flist;
	bool hasmore = dir->GetFirst(&f);
	while (hasmore)
	{
		flist.Add( path + "/" + f );
		hasmore = dir->GetNext(&f);
	}
	
	delete dir; // closes the directory so we can remove at the end of the function

	for (int i=0;i<(int)flist.Count();i++)
	{
		f = flist[i];
		if ( wxDirExists(f) )
		{
			if (cb)	(*cb)( f, data );
			DeleteDirectory( f, cb, data);
		}
		else
		{
			if (cb)	(*cb)( f, data );
			wxRemoveFile( f );
		}
	}

	
	return wxRmdir( path );
}

wxULongLong DirectorySize(const wxString &path, int *count)
{
	wxDir dir( path );
	
	if (!dir.IsOpened())
		return 0;
	
	wxULongLong total=0,cur = 0;
	wxString f,item;
	bool hasmore = dir.GetFirst(&f);
	while (hasmore)
	{
		item = path + "/" + f;

		if ( wxDirExists(item) )
		{
			total += DirectorySize( item, count );
			if (count) (*count)++;
		}
		else
		{
			cur = wxFileName::GetSize( item );
			if (count) (*count)++;
			if (cur != wxInvalidSize)
				total += cur;
		}

		hasmore = dir.GetNext(&f);
	}
	
	return total;
}
*/

#define ALPHA_MIN 'a'
#define ALPHA_MAX 'z'
#define LASTCHAR(x) tolower(x[x.Len()-1])

wxString wxConvertToBase26(unsigned int val)
{
	wxString result;
	do
	{
		result.Prepend((char)( (val-1) % 26 + 'A'));
		val = (val-1)/26;
	}while(val>0);
	return result;
}

unsigned int wxConvertFromBase26(const wxString &val)
{
	unsigned int result = 0;
	const char *cval = val.c_str();
	
	while (cval && *cval)
		result = result*26 + toupper(*cval++)-'A'+1;

	return result;
}

wxArrayString wxEnumerateAlphaIndex(const wxString &_start, const wxString &_end)
{
	unsigned int istart = wxConvertFromBase26(_start);
	unsigned int iend = wxConvertFromBase26(_end);
	
	wxArrayString values;
	while (istart <= iend)
	{
		values.Add( wxConvertToBase26(istart) );
		istart++;
	}
	return values;
}

wxString wxWebHttpGet(const wxString &url, const wxString &addtlhdr_name, const wxString &addtlhdr_value)
{
	wxString server, file;

	wxURI uri(url);

	if (uri.GetScheme().Lower() != "http") return wxEmptyString;

	server = uri.GetServer();
	file = uri.GetPath();

	if (uri.HasQuery())
		file += "?" + uri.GetQuery();

	if (uri.HasFragment())
		file += "#" + uri.GetFragment();

	int port = 80;

	if (uri.HasPort())
		port = atoi( uri.GetPort().c_str() );

	wxHTTP get;
	get.SetHeader("Content-type", "text/plain");
	if (!addtlhdr_name.IsEmpty() && !addtlhdr_value.IsEmpty())
		get.SetHeader( addtlhdr_name, addtlhdr_value );

	get.SetTimeout(9);

	int ntries_left = 3;
	while (!get.Connect( server, port ) && ntries_left)
	{
		ntries_left--;
		wxSleep(1);
	}

	//if (!get.IsConnected()) return wxEmptyString;

	if (!wxApp::IsMainLoopRunning())
		return wxEmptyString;

	wxString result;

	wxInputStream *httpStream = get.GetInputStream( file );
	if (get.GetError() == wxPROTO_NOERR)
	{
		wxStringOutputStream out_stream(&result);
		httpStream->Read(out_stream);
	}

	wxDELETE(httpStream);
	get.Close();

	return result;
}

#define NRWBUFBYTES 4096

bool wxWebHttpDownload(const wxString &url, const wxString &local_file, 
					   int timeout,
				const wxString &mime,
				  bool with_progress_dialog, 
				  void (*callback)(int bytes, int total, void *data), void *data)
{
	wxString server, file;

	wxURI uri(url);

	if (uri.GetScheme().Lower() != "http") return false;

	server = uri.GetServer();
	file = uri.GetPath();

	if (uri.HasQuery())
		file += "?" + uri.GetQuery();

	if (uri.HasFragment())
		file += "#" + uri.GetFragment();

	int port = 80;

	if (uri.HasPort())
		port = atoi( uri.GetPort().c_str() );

	wxHTTP get;
	get.SetHeader("Content-type", mime);
	get.SetTimeout( timeout );

	int ntries_left = 3;
	while (!get.Connect( server, port ) && ntries_left)
	{
		ntries_left--;
		wxSleep(1);
	}

	//if (!get.IsConnected()) return false;

	if (!wxApp::IsMainLoopRunning())
		return false;

	wxInputStream *httpStream = get.GetInputStream(file);
	bool ok = false;
	if (get.GetError() == wxPROTO_NOERR)
	{
		int ntotal = httpStream->GetSize();

		if (ntotal < 1) ntotal = -1000;

		char rwbuf[NRWBUFBYTES];

		wxFileOutputStream fout( local_file );
		if (fout.IsOk())		
		{
			wxProgressDialog *prog = NULL;

			if (with_progress_dialog)
			{
				prog = new wxProgressDialog( "HTTP Download", url, abs(ntotal) );
				prog->Show();
				wxSafeYield( prog, true );
			}

			int ndown = 0;
			int yieldreq = 0;
			while(!httpStream->Eof())
			{
				int nread;
				httpStream->Read(rwbuf, NRWBUFBYTES);
				nread = httpStream->LastRead();
				fout.Write(rwbuf, nread);

				if (nread > 0) ndown += nread;

				if (callback) (*callback)(nread, ntotal, data);
				if (prog) 
				{
					if (ntotal > 0)
						prog->Update( ndown );
					else
						prog->Pulse();

					if (++yieldreq % 5 == 0) wxSafeYield( prog, true );
				}
			}

			ok = true;

			if (prog) prog->Destroy();
		}
	}

	return ok;

}

bool wxUnzipFile(const wxString &archive, const wxString &target)
{
	char rwbuf[NRWBUFBYTES];
	wxFFileInputStream in( archive );
	if (!in.IsOk()) return false;

	wxZipInputStream zip(in);
	if (!zip.IsOk()) return false;

	wxZipEntry *zf = zip.GetNextEntry();
	while (zf)
	{
		wxString fn = target + "/" + zf->GetName();
		if (!zf->IsDir())
		{
			// create the directory if needed
			wxString dirpath = wxPathOnly(fn);
			if (!wxDirExists(dirpath))
				wxFileName::Mkdir( dirpath, 511, wxPATH_MKDIR_FULL );

			wxFFileOutputStream out( fn );
			if (!out.IsOk())
			{
				wxDELETE(zf);
				return false;
			}

			while (!zip.Eof())
			{
				zip.Read(rwbuf, NRWBUFBYTES);
				out.Write(rwbuf, zip.LastRead());
			}				
		}
		wxDELETE(zf);
		zf = zip.GetNextEntry();
	}

	return true;
}

bool wxUntarFile(const wxString &archive, const wxString &target)
{
	char rwbuf[NRWBUFBYTES];
	wxFFileInputStream in( archive );
	if (!in.IsOk()) return false;

	wxTarInputStream tar(in);
	if (!tar.IsOk()) return false;

	wxTarEntry *te = tar.GetNextEntry();
	while (te)
	{
		wxString fn = target + "/" + te->GetName();
		if (!te->IsDir())
		{
			// create the directory if needed
			wxString dirpath = wxPathOnly(fn);
			if (!wxDirExists(dirpath))
				wxFileName::Mkdir( dirpath, 511, wxPATH_MKDIR_FULL );

			wxFFileOutputStream out( fn );
			if (!out.IsOk())
			{
				wxDELETE(te);
				return false;
			}

			while (!tar.Eof())
			{
				tar.Read(rwbuf, NRWBUFBYTES);
				out.Write(rwbuf, tar.LastRead());
			}				
		}
		wxDELETE(te);
		te = tar.GetNextEntry();
	}

	return true;
}

bool wxGunzipFile(const wxString &archive, const wxString &target)
{
	char rwbuf[NRWBUFBYTES];

	wxFFileInputStream in( archive );
	if (!in.IsOk()) return false;

	wxZlibInputStream gzp( in );
	if (!gzp.IsOk()) return false;

	wxFFileOutputStream out( target );
	if (!out.IsOk()) return false;

	while (!gzp.Eof())
	{
		gzp.Read( rwbuf, NRWBUFBYTES );
		out.Write( rwbuf, gzp.LastRead() );
	}

	return true;
}

bool wxDecompressFile(const wxString &archive, const wxString &target)
{

	if (archive.Right(4).Lower() == ".zip")
	{
		return wxUnzipFile(archive, target);
	}
	else if (archive.Right(4).Lower() == ".tar")
	{
		return wxUntarFile(archive, target);
	}
	else if (archive.Right(7).Lower() == ".tar.gz")
	{
		wxString tempfile;
		if (!wxGetTempFileName("gunzip", tempfile)) return false;
		if (!wxGunzipFile( archive, tempfile )) return false;
		return wxUntarFile(tempfile, target);
	}
	else if (archive.Right(3).Lower() == ".gz")
	{
		return wxGunzipFile(archive, target);
	}
	else
	{
		return false;
	}
}


// precalculate an array of ray numbers that we want to plot
std::vector<int> wxCommaDashListToIndices(const wxString &value)
{
	std::vector<int> list;
	wxArrayString parts = wxStringTokenize( value, "," );
	for (int i=0;i<(int)parts.Count();i++)
	{
		wxString s = parts[i];
		int hpos = s.Find('-');
		if (hpos < 0)
		{
			long num = 0;
			if (s.ToLong(&num))
				list.push_back( num );
		}
		else
		{
			long start=0, end=0;
			if (s.Mid(0,hpos).ToLong(&start)
				&& s.Mid(hpos+1).ToLong(&end)
				&& end >= start )
			{
				for (int j=start;j<=end;j++)
					list.push_back( j );
			}
		}

	}
	return list;
}


/*
// this will interpolate or extrapolate as necessary
// if slope is infinite (x1 = x2), it will just return the first Y value
double Interpolate(double x1, double y1, double x2, double y2, double xValueToGetYValueFor)
{
	if (x1 == x2) return y1;
	if (y1 == y2) return y1;

	double slope = (y2 - y1)/(x2 - x1);
	double inter = y1 - (slope * x1);
	return (slope*xValueToGetYValueFor) + inter;
}

*/

/*
struct csvcell
{
	size_t r, c;
	wxString text;
};
 
int CSVRead( const wxString &file, Mat2D<wxString> &csv )
{
	FILE *fp = fopen( (const char*)file.c_str(), "r" );
	if ( !fp ) return 0;
	
	size_t size = BUFSIZ;
	char *buf = new char[size];

	size_t max_row = 0;
	size_t max_col = 0;

	std::vector< csvcell* > cells;
	int errflag = 0;

	while ( !feof(fp) )
	{
		// read a full line, extending buf as needed
		size_t last = 0;
		size_t len = 0;
		bool has_line = false;
		while ( !feof(fp) )
		{
			char *line = fgets( buf + last, size - last, fp );
			len = strlen(buf);
			last = len - 1;

			if ( buf[last] != '\n' ) // buffer was too short for the line
			{
				// reallocate it, copying over old buffer contents
				char *pnew = new char[size+BUFSIZ];
				for (size_t k=0;k<size;k++)
					pnew[k] = buf[k];
				delete [] buf;
				buf = pnew;
				size += BUFSIZ;
				continue;
			}
			else
			{
				has_line = ( line != 0 && len > 0 && buf[last] == '\n' );
				break; // full line read into buf.
			}
		}

		if ( !has_line ) break;

		// at this point, buf contains a null terminated string representing 
		// one full line of the CSV file.
		// scan through one character at a time, determine
		
		size_t cur_col = 0;
		char *p = buf;

		while( len > 0 && p && *p && *p != '\n' )
		{
			csvcell *cell = new csvcell;
			cell->r = max_row;
			cell->c = cur_col;
			cells.push_back( cell );

			if (*p == '"')
			{
				p++; // skip the initial quote

				// read a quoted cell
				while ( p && *p )
				{
					if ( *p == '"' )
					{
						if ( *(p+1) == '"')
						{
							cell->text += '"';
							p++; p++;
						}
						else
						{
							p++; // skip current quote
							break; // end of quoted cell
						}
					}
					else
					{
						cell->text += *p++;
					}
				}
			}
			else
			{
				// read a normal cell
				char prev = 0;
				while ( p && *p && *p != ',' && *p != '\n' )
				{
					if ( *p == '"' && prev == '"' )
					{
						cell->text += '"';
						prev = *p;
						p++;
					}
					else
					{
						cell->text += *p;
						prev = *p;
						p++;
					}
				}
			}

			if (*p == ',' || *p == '\n')
			{
				p++; // skip over the comma
				cur_col++;
				if ( cur_col > max_col )
					max_col = cur_col;
			}
			else
			{
				// flag error on the first row with a formatting problem
				if (errflag == 0)
					errflag = -( (int)max_row+1 );
			}
		}

		max_row++;
	}
	
	fclose(fp);
	delete [] buf;

	// copy csv cells into matrix
	if (max_row > 0 && max_col > 0)
		csv.resize( max_row, max_col, wxEmptyString, false );

	for ( std::vector< csvcell* >::iterator it = cells.begin();
		it != cells.end();
		++it )
	{
	
		size_t r = (*it)->r;
		size_t c = (*it)->c;
		if (r < max_row && c < max_col)
			csv.at( r, c ) = (*it)->text;

		delete (*it);
	}


	return (0 == errflag);
}

*/