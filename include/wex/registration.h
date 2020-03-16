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

#ifndef __wex_registration_h
#define __wex_registration_h

#include <wx/dialog.h>

class wxTextCtrl;

class wxMetroButton;

class wxOnlineRegistrationData {
public:
    wxOnlineRegistrationData();

    virtual ~wxOnlineRegistrationData();

    virtual wxString GetAppName() = 0;

    virtual wxString GetOrganization() = 0;

    virtual bool IsAnOverrideKey(const char *) = 0;

    virtual wxString GetLocalRegistrationFile() = 0;

    virtual wxString ReadSetting(const wxString &) = 0;

    virtual void WriteSetting(const wxString &, const wxString &) = 0;

    virtual wxString GetVersionAndPlatform() = 0;

    virtual wxString GetNoticeText() = 0;

    enum Endpoint {
        CHECK_IN, RESEND_KEY, REGISTER_NEW
    };

    virtual bool GetApi(Endpoint ept, wxString *url, wxString *post) = 0;

    virtual void ShowHelp() = 0;

    virtual wxString ReadProxy() = 0;

    virtual bool WriteProxy(const wxString &) = 0;
};

class wxOnlineRegistration : public wxDialog {
public:
    wxOnlineRegistration(wxWindow *parent);

    static void Init(wxOnlineRegistrationData *);

    static bool CheckRegistration();

    static void EnableDebugMessages(bool b);

    static wxString GetEmail();

    static wxString GetKey();

    static wxString GetVersionAndPlatform();

    static bool IncrementUsage();

    static void DecrementUsage();

    static bool CheckInWithServer(int *total_usage_count = 0);

    static int CountSinceLastVerify();

    static bool CanStart();

    static int AllowedStartsRemaining();

    static bool ShowDialog(
            const wxString &msg = wxEmptyString,
            const wxString &btn = wxEmptyString); // returns false on cancel
    static bool ShowNotice();

private:

    wxTextCtrl *m_email;
    wxTextCtrl *m_key;
    wxTextCtrl *m_output;
    wxMetroButton *m_close, *m_register;

    void OnRegister(wxCommandEvent &);

    void OnConfirm(wxCommandEvent &);

    void OnHelp(wxCommandEvent &);

    void OnEmail(wxCommandEvent &);

    void OnProxySetup(wxCommandEvent &);

DECLARE_EVENT_TABLE();
};

#endif
