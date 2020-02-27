/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

#ifndef __wex_radiochoice_h
#define __wex_radiochoice_h

#include <vector>
#include <wx/panel.h>
#include <wx/radiobut.h>

class wxRadioChoice : public wxPanel {
public:
    wxRadioChoice(wxWindow *parent, int id,
                  const wxPoint &pos = wxDefaultPosition,
                  const wxSize &size = wxDefaultSize);

    virtual wxSize DoGetBestSize() const;

    void Add(const wxString &caption, bool arrange = true);

    void Add(const wxArrayString &list);

    int Find(const wxString &caption);

    void Remove(int idx);

    void Remove(const wxString &caption);

    void Clear();

    int GetCount();

    virtual bool Enable(bool b = true);

    void Enable(int idx, bool b = true);

    bool IsEnabled(int idx);

    void SetLabel(int idx, const wxString &lbl);

    wxString GetLabel(int idx);

    int GetSelection();

    wxString GetValue();

    void SetValue(const wxString &sel);

    void SetSelection(int idx);

    void Rearrange();

    void ShowCaptions(bool b);

    bool CaptionsShown();

    void SetHorizontal(bool b);

    bool IsHorizontal();

    void LayoutEvenly(bool b);

private:
    void OnRadio(wxCommandEvent &evt);

    void OnResize(wxSizeEvent &evt);

    bool m_showCaptions;
    bool m_horizontal;
    bool m_evenly;
    wxArrayString m_captions;
    std::vector<wxRadioButton *> m_buttons;

DECLARE_EVENT_TABLE()
};

#endif
