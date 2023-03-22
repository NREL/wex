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

#ifndef __easycurl_h
#define __easycurl_h

#include <wx/event.h>
#include <wx/datetime.h>
#include <wx/sstream.h>
#include "tpdlg.h"


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEASYCURL_EVENT, 7578)
END_DECLARE_EVENT_TYPES()

class wxEasyCurlEvent : public wxEvent {
public:
    enum {
        STARTED, PROGRESS, FINISHED
    };

    wxEasyCurlEvent(int id, wxEventType type, int code, const wxString &msg = wxEmptyString,
                    const wxString &url = wxEmptyString,
                    double bytes = 0.0, double total = 0.0)
            : wxEvent(id, type) {
        m_code = code;
        m_msg = msg;
        m_dt = wxDateTime::Now();
        m_url = url;
        m_bytes = bytes;
        m_total = total;
    }

    wxEasyCurlEvent(const wxEasyCurlEvent &evt)
            : wxEvent(evt.GetId(), evt.GetEventType()),
              m_code(evt.m_code),
              m_msg(evt.m_msg),
              m_dt(evt.m_dt),
              m_url(evt.m_url),
              m_bytes(evt.m_bytes),
              m_total(evt.m_total) {}

    int GetStatusCode() const { return m_code; }

    wxString GetMessage() const { return m_msg; }

    virtual wxEvent *Clone() const { return new wxEasyCurlEvent(*this); }

    wxString GetUrl() const { return m_url; }

    double GetBytesTransferred() const { return m_bytes; }

    double GetBytesTotal() const { return m_total; }

protected:
    int m_code;
    wxString m_msg;
    wxDateTime m_dt;
    wxString m_url;
    double m_bytes, m_total;
};

typedef void (wxEvtHandler::*wxEasyCurlEventFunction)(wxEasyCurlEvent &);

#define wxEasyCurlEventFunction(func) \
        (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxEasyCurlEventFunction, &func)

#define EVT_EASYCURL(id, fn) \
        wx__DECLARE_EVT1(wxEASYCURL_EVENT, id, wxEasyCurlEventFunction(fn))

class wxEasyCurl : public wxObject {
public:
    // app-wide init and shutdown calls for underlying libcurl initialization
    static void Initialize();

    static void Shutdown();

    //static void SetApiKeys(const wxString &google_key, const wxString &bing_key, const wxString &developer);

    static void SetUrlEscape(const wxString &key, const wxString &value);

    static void SetProxyAddress(const wxString &proxy);

    static wxString GetProxyForURL(const wxString &url);

    static wxArrayString GetProxyAutodetectMessages();

    wxEasyCurl(wxEvtHandler *handler = 0, int id = wxID_ANY);

    virtual ~wxEasyCurl();

    void SetPostData(const wxString &s) { m_postData = s; }

    void AddHttpHeader(const wxString &s) { m_httpHeaders.Add(s); }

    // progress reporting methods
    // send wxEasyCurlEvents to the specified wxEvtHandler, and events have the given id
    // the event handler will be called in the main thread
    void SetEventHandler(wxEvtHandler *hh, int id);

    wxString GetDataAsString();

    wxImage GetDataAsImage(wxBitmapType bittype = wxBITMAP_TYPE_JPEG);

    bool WriteDataToFile(const wxString &file);

    // asynchronous operation
    void Start(const wxString &url);

    bool Wait(bool yield = false); // must be called to finish download
    bool IsStarted();

    bool IsFinished();

    void Cancel(); // returns immediately
    bool Ok();

    wxString GetLastError();

    // synchronous operation
    bool Get(const wxString &url,
             const wxString &progress_dialog_msg = wxEmptyString,
             wxWindow *parent = NULL);

    class DLThread;

    DLThread *GetThread();

protected:
    friend class DLThread;

    DLThread *m_thread;
    wxEvtHandler *m_handler;
    int m_id;

    wxString m_postData;
    wxArrayString m_httpHeaders;
};

class wxEasyCurlDialog {
public:
    wxEasyCurlDialog(const wxString &message = wxEmptyString, int nthreads = 0, wxWindow *parent = NULL);

    ~wxEasyCurlDialog();

    // update the Status title and visiblity of bars.  Calls yield.
    void NewStage(const wxString &title, int nbars_to_show = -1);

    // if messages appeared during EasyCurl,
    // show the dialog as modal
    void Finalize(const wxString &custom_title = wxEmptyString);

    // update progress, calls yield
    void Update(int ThreadNum, float percent, const wxString &label = wxEmptyString);

    // these don't call yield
    void Log(const wxArrayString &list) { m_tpd->Log(list); }

    void Log(const wxString &s) { m_tpd->Log(s); }

    bool Canceled() { return m_tpd->IsCanceled(); }

    wxThreadProgressDialog &Dialog() { return *m_tpd; }

private:
    wxThreadProgressDialog *m_tpd;
    wxFrame *m_transp;
};


class wxEasyCurlThread : public wxThread {
    std::vector<wxEasyCurl *> m_curls;
    wxArrayString m_urls;
    wxArrayString m_names;
    wxMutex m_currentLock, m_cancelLock, m_nokLock, m_logLock, m_percentLock;
    size_t m_current;
    bool m_canceled;
    size_t m_nok;
    wxArrayString m_messages;
    wxString m_update;
    wxString m_curName;
    float m_percent;
    int m_threadId;
public:
    wxEasyCurlThread(int id);

    void Add(wxEasyCurl *curl, wxString &url, wxString &name);

    size_t Size();

    size_t Current();

    float GetPercent(wxString *update = 0);

    void Cancel();

    size_t NOk();

    void Message(const wxString &text);

    virtual void Warn(const wxString &text);

    virtual void Error(const wxString &text);

    virtual void Update(float percent, const wxString &text);

    virtual bool IsCancelled();

    wxString GetDataAsString();

    wxArrayString GetNewMessages();

    virtual void *Entry();
};


#endif
