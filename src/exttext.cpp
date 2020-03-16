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

#include "wex/exttext.h"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(wxExtTextCtrl, wxTextCtrl)
                EVT_TEXT_ENTER(wxID_ANY, wxExtTextCtrl::OnTextEnter)
                EVT_KILL_FOCUS(wxExtTextCtrl::OnLoseFocus)
                EVT_SET_FOCUS(wxExtTextCtrl::OnSetFocus)
END_EVENT_TABLE()

wxExtTextCtrl::wxExtTextCtrl(wxWindow *parent, int id,
                             const wxString &text,
                             const wxPoint &pos,
                             const wxSize &size,
                             long style)
        : wxTextCtrl(parent, id, text, pos, size,
                     style | wxTE_PROCESS_ENTER) {
    /* nothing to do */
}

void wxExtTextCtrl::OnTextEnter(wxCommandEvent &evt) {
    if (m_focusStrVal != GetValue()) {
        m_focusStrVal = GetValue();
        evt.Skip();
    }
}

void wxExtTextCtrl::OnSetFocus(wxFocusEvent &evt) {
    m_focusStrVal = GetValue();
    SetSelection(0, m_focusStrVal.Len());
    evt.Skip();
}

void wxExtTextCtrl::OnLoseFocus(wxFocusEvent &evt) {
    if (m_focusStrVal != GetValue()) {
        wxCommandEvent enterpress(wxEVT_COMMAND_TEXT_ENTER, this->GetId());
        enterpress.SetEventObject(this);
        enterpress.SetString(GetValue());
        GetEventHandler()->ProcessEvent(enterpress);
    }
    evt.Skip();
}
