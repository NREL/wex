/*
BSD 3-Clause License

Copyright (c) Alliance for Sustainable Energy, LLC. See also https://github.com/NREL/wex/blob/develop/LICENSE
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __csvdata_h
#define __csvdata_h

#include <wx/string.h>
#include <wx/stream.h>

#include <unordered_map>

using std::unordered_map;
#pragma warning(disable: 4290)  // ignore warning: 'C++ exception specification ignored except to indicate a function is not __declspec(nothrow)'

#include <wx/hashmap.h>
#include <wx/stream.h>

class wxGrid;

class wxCSVData {
public:
    wxCSVData();

    wxCSVData(const wxCSVData &copy);

    virtual ~wxCSVData();

    void Copy(const wxCSVData &copy);

    wxCSVData &operator=(const wxCSVData &copy);

    void Set(size_t r, size_t c, const wxString &val);

    wxString &operator()(size_t r, size_t c);

    const wxString &Get(size_t r, size_t c) const;

    const wxString &operator()(size_t r, size_t c) const;

    size_t NumCells() const;

    size_t NumRows();

    size_t NumCols();

    void Clear();

    void Clear(size_t r, size_t c);

    bool IsEmpty(size_t r, size_t c);

    bool Read(wxInputStream &in);

    void Write(wxOutputStream &out);

    bool ReadFile(const wxString &file);

    bool WriteFile(const wxString &file);

    bool ReadString(const wxString &data);

    wxString WriteString();

    int GetErrorLine() { return m_errorLine; }

    void SetSeparator(wxUniChar sep) { m_sep = sep; }

    wxUniChar GetSeparator();

protected:
    wxUniChar m_sep;
    bool m_invalidated;
    size_t m_nrows, m_ncols;
    typedef unordered_map<wxUint64, wxString> cell_hash;
    cell_hash m_cells;
    int m_errorLine;
    wxString m_emptyStr;

    wxUint64 Encode(size_t r, size_t c) const;

    void Decode(wxUint64 idx, size_t *r, size_t *c) const;

    void RecalculateDimensions();
};

#endif
