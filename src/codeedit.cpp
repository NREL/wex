#include <wx/wx.h>
#include <wx/filename.h>

#include "wex/codeedit.h"


static bool IsBrace( wxUniChar ch )
{
	return ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '{' || ch == '}';
}

static wxString LimitColumnWidth( const wxString &str, int numcols )
{
	wxString buf;
	int len = (int)str.Len();
	int col=0;
	for (int i=0;i<len;i++)
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


BEGIN_EVENT_TABLE (wxCodeEditCtrl, wxStyledTextCtrl)

    // stc
    EVT_STC_MARGINCLICK( wxID_ANY, wxCodeEditCtrl::OnMarginClick )
    EVT_STC_CHARADDED( wxID_ANY, wxCodeEditCtrl::OnCharAdded )
	EVT_STC_UPDATEUI( wxID_ANY, wxCodeEditCtrl::OnUpdateUI )
	
	// find dialog
	EVT_FIND( wxID_ANY, wxCodeEditCtrl::OnFindDialog )
    EVT_FIND_NEXT( wxID_ANY, wxCodeEditCtrl::OnFindDialog )
    EVT_FIND_REPLACE( wxID_ANY, wxCodeEditCtrl::OnFindDialog )
    EVT_FIND_REPLACE_ALL( wxID_ANY, wxCodeEditCtrl::OnFindDialog )
    EVT_FIND_CLOSE( wxID_ANY, wxCodeEditCtrl::OnFindDialog )

END_EVENT_TABLE()


#ifdef __WXMAC__
#define DEFAULT_FONT_SIZE 13
#define DEFAULT_FONT_FACE "Courier New"
#else
#define DEFAULT_FONT_SIZE 10
#define DEFAULT_FONT_FACE "Consolas"
#endif

static char* CWordlist1 =
    "asm auto break case char const "
    "continue default delete do double else enum "
    "extern float for goto if inline int long "
    "mutable register "
    "return short signed sizeof static "
    "struct switch typedef "
    "union unsigned void volatile "
    "while";

static char* CppWordlist1 =
    "asm auto bool break case catch char class const const_cast "
    "continue default delete do double dynamic_cast else enum explicit "
    "export extern false float for friend goto if inline int long "
    "mutable namespace new operator private protected public register "
    "reinterpret_cast return short signed sizeof static static_cast "
    "struct switch template this throw true try typedef typeid "
    "typename union unsigned using virtual void volatile wchar_t "
    "while";

static char *TrnsysWordlist1 = 
	"version assign simulation tolerances limits unit type "
	"parameters inputs equations constants end width "
	"accelerate loop repeat dfq nocheck solver derivatives "
	"trace format nolist list map ";

static char *TrnsysWordlist2 =
	"abs acos and asin atan cos eql exp gt int or ln log lt max min mod "
	"not sin tan";

static char *TrnsysWordlist3 =
	"const time ";

static char *PythonWordlist1 = 
	"and assert break class continue def "
	"del elif else except exec finally "
	"for from global if import in "
	"is lambda not or pass print "
	"raise return try while yield";

static  char *LKWordlist1  =
	"if while for return exit break continue function const enum class else elseif define this typeof common true false null import ";


wxCodeEditCtrl::wxCodeEditCtrl( wxWindow *parent, int id, 
		const wxPoint &pos, const wxSize &size )
		: wxStyledTextCtrl( parent, id, pos, size, wxBORDER_NONE )
{
	m_findDialog = 0;
	m_lastFindPos = m_lastReplacePos = -1;

	m_callTipsEnabled = false;
	m_ctCaseSensitive = false;
	m_ctStart = '(';
	m_ctEnd = ')';
	
	SetScrollWidthTracking( true );
}

wxCodeEditCtrl::~wxCodeEditCtrl()
{
	if ( m_findDialog != 0 )
		m_findDialog->Destroy();
}

void wxCodeEditCtrl::SetLanguage( const wxString &fileName )
{
	SetLanguage( GetLanguage( fileName ) );
}

void wxCodeEditCtrl::SetLanguage( Language lang )
{
	// first, revert to standard style
	m_lang = NONE;
	
	SetStyleBits( 8 );
    SetLayoutCache( wxSTC_CACHE_PAGE );
	SetLexer( wxSTC_LEX_NULL );
		
    wxFont font ( DEFAULT_FONT_SIZE,
		wxFONTFAMILY_MODERN,
		wxFONTSTYLE_NORMAL,
		wxFONTWEIGHT_NORMAL,
		false,
		DEFAULT_FONT_FACE );

	SetFont( font );
	StyleSetFont (wxSTC_STYLE_DEFAULT, font);
		
	wxFont fontslant( font );
	fontslant.SetStyle( wxFONTSTYLE_ITALIC );
		
	wxFont fontsmall( font );
	fontsmall.SetPointSize( fontsmall.GetPointSize() - 1 );

    int lineNrMargin = TextWidth (wxSTC_STYLE_LINENUMBER, _T("_99999"));
    int foldingMargin = 16;

	SetViewEOL( false );
    SetIndentationGuides( false );
    SetEdgeMode( wxSTC_EDGE_NONE );
    SetViewWhiteSpace( wxSTC_WS_INVISIBLE );
    SetOvertype( false );
    SetReadOnly( false );
    SetWrapMode( wxSTC_WRAP_NONE );
	StyleSetForeground( wxSTC_STYLE_DEFAULT, *wxBLACK );
    StyleSetBackground( wxSTC_STYLE_DEFAULT, *wxWHITE );
    StyleSetForeground( wxSTC_STYLE_LINENUMBER, *wxLIGHT_GREY );
    StyleSetBackground( wxSTC_STYLE_LINENUMBER, *wxWHITE );
    StyleSetForeground( wxSTC_STYLE_INDENTGUIDE, *wxLIGHT_GREY );
    SetFoldFlags(0);

	//SetCaretPeriod(0);

    // set spaces and indention
    SetTabWidth( 4 );
    SetUseTabs( true );
    SetTabIndents( true );
    SetBackSpaceUnIndents( true );
    SetIndent( 4 );
	SetEdgeColumn( 80 );
	SetEdgeColour( wxColour(255,187,187) );
	SetEdgeMode( wxSTC_EDGE_LINE );
    
    // set visibility
    SetVisiblePolicy (wxSTC_VISIBLE_STRICT|wxSTC_VISIBLE_SLOP, 1);
    SetXCaretPolicy (wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);
    SetYCaretPolicy (wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);
	
	SetSelForeground( true, *wxWHITE );
	SetSelBackground( true, *wxBLACK );

    // markers
    MarkerDefine( wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_DOTDOTDOT, *wxBLACK, *wxBLACK);
    MarkerDefine( wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_ARROWDOWN, *wxBLACK, *wxBLACK);
    MarkerDefine( wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY,     *wxBLACK, *wxBLACK);
    MarkerDefine( wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_DOTDOTDOT, *wxBLACK, *wxWHITE);
    MarkerDefine( wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, *wxBLACK, *wxWHITE);
    MarkerDefine( wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY,     *wxBLACK, *wxBLACK);
    MarkerDefine( wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY,     *wxBLACK, *wxBLACK);
	
	CallTipUseStyle( 30 );
	wxFont fontnormal (*wxNORMAL_FONT) ;
	StyleSetFont( wxSTC_STYLE_CALLTIP, fontnormal );
	StyleSetForeground( wxSTC_STYLE_CALLTIP, *wxBLACK );
	StyleSetBackground( wxSTC_STYLE_CALLTIP, wxColour(247,240,210) );

	// then, apply language specific changes
	if ( lang == CPP || lang == C || lang == LK )
	{
		SetLexer( wxSTC_LEX_CPP );

		// set up line number margin
		SetMarginType (0, wxSTC_MARGIN_NUMBER);
		StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour(120,120,120));
		StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour(230,230,230));
		SetMarginWidth (0, lineNrMargin); 
	
		// set margin as unused
		SetMarginType (1, wxSTC_MARGIN_SYMBOL);
		SetMarginWidth (1, 1);
		SetMarginSensitive (1, false);
		
		/*
		// folding
		SetMarginType( 2, wxSTC_MARGIN_SYMBOL );
		SetMarginMask( 2, wxSTC_MASK_FOLDERS );
		StyleSetBackground (2, wxColour (_T("WHITE")));
    
		SetMarginWidth (2, 16);
		SetMarginSensitive (2, true);
		SetProperty (_T("fold"), "1");
		SetProperty (_T("fold.comment"), "1");
		SetProperty (_T("fold.compact"), "1");
		SetProperty (_T("fold.preprocessor"), "1");
	
		SetFoldFlags (wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |
					  wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
		*/
 
		StyleSetForeground(wxSTC_C_COMMENT, wxColour(0x00, 0xaf, 0x00));
		StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0x00, 0xaf, 0x00));
		StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0xaf, 0xaf, 0xaf));
	
		StyleSetFont( wxSTC_STYLE_DEFAULT, font );
		StyleSetFont( wxSTC_C_DEFAULT, font );
		StyleSetFont( wxSTC_C_COMMENT, fontslant );
		StyleSetFont( wxSTC_C_COMMENTLINE, fontslant );
		StyleSetFont( wxSTC_C_COMMENTDOC, fontslant );

		StyleSetForeground(wxSTC_C_WORD, wxColour("red"));
		StyleSetForeground(wxSTC_C_WORD2,  wxColour(0,128,192));
		
		StyleSetForeground(wxSTC_C_NUMBER,  wxColour(0x00, 0x7f, 0x7f));

		wxColour cLiteral( "maroon" );
		StyleSetForeground(wxSTC_C_STRING, cLiteral );
		StyleSetForeground(wxSTC_C_STRINGEOL, cLiteral );
		StyleSetForeground(wxSTC_C_VERBATIM, cLiteral );
		StyleSetForeground(wxSTC_C_STRINGRAW, cLiteral );
		StyleSetForeground(wxSTC_C_TRIPLEVERBATIM, cLiteral );
		StyleSetForeground(wxSTC_C_HASHQUOTEDSTRING, cLiteral );
		
		StyleSetForeground(wxSTC_C_CHARACTER,  wxColour(0x7f, 0x00, 0x7f));
		StyleSetForeground(wxSTC_C_UUID,  wxColour(0x00, 0x7f, 0x7f));
		StyleSetForeground(wxSTC_C_PREPROCESSOR,  wxColour(0x7f, 0x7f, 0x7f));
		StyleSetForeground(wxSTC_C_OPERATOR, wxColour("blue"));
		StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(0x00, 0x00, 0x00));

		StyleSetBackground(wxSTC_STYLE_BRACELIGHT, *wxLIGHT_GREY );
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, *wxWHITE );
		

		if ( lang == C ) SetKeyWords(wxSTC_C_DEFAULT, CWordlist1);	
		else if ( lang == LK )
		{
			SetKeyWords(wxSTC_C_DEFAULT, LKWordlist1 );
			SetMarginWidth(2, 0);
			SetProperty(wxT("fold"), "0");
		}
		else SetKeyWords(wxSTC_C_DEFAULT, CppWordlist1);

		m_lang = lang;
	}
	else if ( lang == VBA )
	{
		SetLexer( wxSTC_LEX_VB );

		SetMarginType (0, wxSTC_MARGIN_NUMBER);
		StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour(120,120,120));
		StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour(230,230,230));
		SetMarginWidth (0, lineNrMargin); 
	
		StyleSetForeground(  wxSTC_B_DEFAULT, *wxBLACK );
		StyleSetForeground(  wxSTC_B_COMMENT, wxColour(0,190,0));
		StyleSetForeground(  wxSTC_B_NUMBER, *wxBLUE);
		StyleSetItalic( wxSTC_B_NUMBER, false);

		StyleSetForeground(  wxSTC_B_KEYWORD, *wxRED);
		StyleSetItalic( wxSTC_B_KEYWORD, false);
		StyleSetForeground(  wxSTC_B_STRING, wxColour("maroon"));
		StyleSetForeground(  wxSTC_B_PREPROCESSOR, *wxBLACK); 
		StyleSetForeground(  wxSTC_B_OPERATOR, *wxBLUE);
		StyleSetForeground(  wxSTC_B_IDENTIFIER, *wxBLACK);
		StyleSetForeground(  wxSTC_B_DATE, *wxBLACK);
		StyleSetForeground(  wxSTC_B_STRINGEOL, *wxBLACK);
		StyleSetForeground(  wxSTC_B_KEYWORD2, wxColour(0,128,192));
		StyleSetForeground(  wxSTC_B_KEYWORD3, *wxBLACK);
		StyleSetForeground(  wxSTC_B_KEYWORD4, *wxBLACK);
		StyleSetForeground(  wxSTC_B_CONSTANT, *wxBLACK);
		StyleSetForeground(  wxSTC_B_ASM, *wxBLACK);
		StyleSetForeground(  wxSTC_B_LABEL, *wxLIGHT_GREY);
		StyleSetForeground(  wxSTC_B_ERROR, *wxBLACK);
		StyleSetForeground(  wxSTC_B_HEXNUMBER, *wxBLACK);
		StyleSetForeground(  wxSTC_B_BINNUMBER, *wxBLACK);

	
		StyleSetBackground(wxSTC_STYLE_BRACELIGHT, *wxLIGHT_GREY );
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, *wxWHITE );

		SetMarginWidth(1,10);
		SetMarginWidth(2,0);
		
		m_lang = VBA;
	}
	else if ( lang == HTML )
	{
		SetLexer(wxSTC_LEX_HTML);

		SetMarginType (0, wxSTC_MARGIN_NUMBER);
		StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour(120,120,120));
		StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour(230,230,230));
		SetMarginWidth (0, lineNrMargin); 

		wxColour cPhpFore(0, 0, 0);
		wxColour cPhpBack(253, 255, 223);
		
		StyleSetFont(wxSTC_STYLE_DEFAULT, font);
		StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (_T("DARK GREY")));
		StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour (_T("WHITE")));
		
		StyleSetFont(wxSTC_H_DEFAULT, font);
		StyleSetForeground( wxSTC_H_DEFAULT, wxColour("black"));
		StyleSetForeground( wxSTC_H_TAG, wxColour("blue"));
		StyleSetForeground( wxSTC_H_TAGUNKNOWN, wxColour("blue"));
	
		StyleSetForeground( wxSTC_H_COMMENT, wxColour(0, 128, 0));
		StyleSetFont( wxSTC_H_COMMENT, fontslant );

		StyleSetForeground( wxSTC_H_ATTRIBUTE, wxColour(0, 0, 150));
		StyleSetForeground( wxSTC_H_ATTRIBUTEUNKNOWN, wxColour(0, 0, 150));
		StyleSetForeground( wxSTC_H_NUMBER, wxColour("black"));
		StyleSetForeground( wxSTC_H_DOUBLESTRING, wxColour(128, 0, 64));
		StyleSetForeground( wxSTC_H_SINGLESTRING, wxColour(128, 0, 64));
		StyleSetForeground( wxSTC_H_OTHER, wxColour("black"));
		StyleSetForeground( wxSTC_H_ENTITY, wxColour("black"));

		//StyleSetBackground( wxSTC_H_SCRIPT, cPhpBack );

		StyleSetForeground( wxSTC_HPHP_DEFAULT, cPhpFore);
		//StyleSetBackground( wxSTC_HPHP_DEFAULT, cPhpBack);
		StyleSetFont( wxSTC_HPHP_DEFAULT, fontsmall);

		StyleSetForeground( wxSTC_HPHP_HSTRING, wxColour(128, 0, 64));
		//StyleSetBackground( wxSTC_HPHP_HSTRING, cPhpBack);
		StyleSetFont( wxSTC_HPHP_HSTRING, fontsmall);

		StyleSetForeground( wxSTC_HPHP_SIMPLESTRING, wxColour(128, 0, 64));
		//StyleSetBackground( wxSTC_HPHP_SIMPLESTRING, cPhpBack);
		StyleSetFont( wxSTC_HPHP_SIMPLESTRING, fontsmall);

		StyleSetForeground( wxSTC_HPHP_WORD, wxColour("red"));
		//StyleSetBackground( wxSTC_HPHP_WORD, cPhpBack);
		StyleSetFont( wxSTC_HPHP_WORD, fontsmall);

		StyleSetForeground( wxSTC_HPHP_NUMBER, wxColour(0, 70, 80));
		//StyleSetBackground( wxSTC_HPHP_NUMBER, cPhpBack);
		StyleSetFont( wxSTC_HPHP_NUMBER, fontsmall);

		StyleSetForeground( wxSTC_HPHP_VARIABLE, wxColour(128,128,64));
		//StyleSetBackground( wxSTC_HPHP_VARIABLE, cPhpBack);
		StyleSetFont( wxSTC_HPHP_VARIABLE, fontsmall);

		StyleSetForeground( wxSTC_HPHP_COMMENT, wxColour(0, 128, 0));
		//StyleSetBackground( wxSTC_HPHP_COMMENT, cPhpBack);
		StyleSetFont( wxSTC_HPHP_COMMENT, fontsmall);

		StyleSetForeground( wxSTC_HPHP_COMMENTLINE, wxColour(0, 128, 0));
		//StyleSetBackground( wxSTC_HPHP_COMMENTLINE, cPhpBack);
		StyleSetFont( wxSTC_HPHP_COMMENTLINE, fontsmall);

		StyleSetForeground( wxSTC_HPHP_HSTRING_VARIABLE, wxColour(128,128,64));
		//StyleSetBackground( wxSTC_HPHP_HSTRING_VARIABLE, cPhpBack);
		StyleSetFont( wxSTC_HPHP_HSTRING_VARIABLE, fontsmall);

		StyleSetForeground( wxSTC_HPHP_OPERATOR, wxColour(cPhpFore));
		//StyleSetBackground( wxSTC_HPHP_OPERATOR, cPhpBack);
		StyleSetFont( wxSTC_HPHP_OPERATOR, fontsmall);

		StyleSetForeground( wxSTC_HPHP_COMPLEX_VARIABLE, wxColour(0, 50, 50));
		//StyleSetBackground( wxSTC_HPHP_COMPLEX_VARIABLE, cPhpBack);
		StyleSetFont( wxSTC_HPHP_COMPLEX_VARIABLE, fontsmall);
		
		m_lang = HTML;
	}
	else if ( lang == TEXT )
	{
		SetLexer( wxSTC_LEX_NULL );
		m_lang = TEXT;
	}
	else if ( lang == TRNSYS )
	{
		SetLexer( wxSTC_LEX_SPICE );

		SetMarginType (0, wxSTC_MARGIN_NUMBER);
		StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour(120,120,120));
		StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour(230,230,230));
		SetMarginWidth (0, lineNrMargin); 

		StyleSetForeground(  wxSTC_SPICE_DEFAULT, *wxBLACK );
		StyleSetForeground(  wxSTC_SPICE_COMMENTLINE, wxColour(0,190,0));
		StyleSetForeground(  wxSTC_SPICE_NUMBER, *wxBLACK);
		StyleSetItalic( wxSTC_SPICE_NUMBER, false);

		StyleSetForeground(  wxSTC_SPICE_KEYWORD, *wxRED);
		StyleSetForeground(  wxSTC_SPICE_KEYWORD2, wxColour(0,128,192));
		StyleSetForeground(  wxSTC_SPICE_KEYWORD3, wxColour("maroon"));
		StyleSetItalic( wxSTC_SPICE_KEYWORD, false);
		StyleSetItalic( wxSTC_SPICE_KEYWORD2, false);
		StyleSetForeground(  wxSTC_SPICE_DELIMITER, *wxBLUE);
		StyleSetForeground(  wxSTC_SPICE_VALUE, *wxBLACK); 
		StyleSetForeground(  wxSTC_SPICE_IDENTIFIER, *wxBLACK);
			
		SetKeyWords(0, TrnsysWordlist1);
		SetKeyWords(1, TrnsysWordlist2);
		SetKeyWords(2, TrnsysWordlist3);
		SetMarginWidth(1,3);
		SetMarginWidth(2,0);
			
		StyleSetBackground(wxSTC_STYLE_BRACELIGHT, *wxLIGHT_GREY );
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, *wxWHITE );
		
		m_lang = TRNSYS;
	}
	else if ( lang == PYTHON )
	{	
		SetLexer (wxSTC_LEX_PYTHON);

		// set up line number margin
		SetMarginType (0, wxSTC_MARGIN_NUMBER);
		StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour(120,120,120));
		StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour(230,230,230));
		SetMarginWidth (0, lineNrMargin); 
	
		// set margin as unused
		SetMarginType (1, wxSTC_MARGIN_SYMBOL);
		SetMarginWidth (1, 3);
		SetMarginSensitive (1, false);

		// folding
		/*
		SetMarginType (2, wxSTC_MARGIN_SYMBOL);
		SetMarginMask (2, wxSTC_MASK_FOLDERS);
		StyleSetBackground (2, wxColour (_T("WHITE")));
    
		SetMarginWidth (2, foldingMargin);
		SetMarginSensitive (2, true);
		SetProperty (_T("fold"), "1");
		SetProperty (_T("fold.comment"), "1");
		SetProperty (_T("fold.compact"), "1");
		SetProperty (_T("fold.preprocessor"), "1");
	
		SetFoldFlags (wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |
					  wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
					  */
 
		StyleSetForeground(wxSTC_P_COMMENTLINE, wxColour(0x00, 0xaf, 0x00));
		StyleSetForeground(wxSTC_P_COMMENTBLOCK, wxColour(0xaf, 0xaf, 0xaf));
	
		StyleSetFont(wxSTC_P_COMMENTLINE, fontslant);
		StyleSetFont(wxSTC_P_COMMENTBLOCK, fontslant);

		StyleSetForeground(wxSTC_P_WORD, wxColour("red"));
		SetKeyWords(wxSTC_P_DEFAULT, PythonWordlist1);
	
		StyleSetForeground(wxSTC_P_NUMBER,  wxColour(0x00, 0x7f, 0x7f));
		StyleSetForeground(wxSTC_P_STRING,  wxColour("maroon"));
		StyleSetForeground(wxSTC_P_CHARACTER,  wxColour(0x7f, 0x00, 0x7f));
		StyleSetForeground(wxSTC_P_TRIPLE,  wxColour(0x00, 0x7f, 0x7f));
		StyleSetForeground(wxSTC_P_TRIPLEDOUBLE,  wxColour(0x7f, 0x7f, 0x7f));
		StyleSetForeground(wxSTC_P_OPERATOR, wxColour("blue"));
		StyleSetForeground(wxSTC_P_IDENTIFIER, wxColour(0x00, 0x00, 0x00));

		StyleSetBackground(wxSTC_STYLE_BRACELIGHT, wxColour("light grey"));
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, wxColour("white"));
		
		m_lang = PYTHON;
	}

	Colourise(0, GetLength());
	Refresh();
}

wxCodeEditCtrl::Language wxCodeEditCtrl::GetLanguage()
{
	return m_lang;
}

wxCodeEditCtrl::Language wxCodeEditCtrl::GetLanguage( const wxString &fileName )
{
	wxString ext = fileName.Lower();
	
	wxFileName fn( fileName );
	if (fn.HasExt())
		ext = fn.GetExt().Lower();

	if (ext == "cpp") return CPP;
	if (ext == "c") return C;
	if (ext == "h") return CPP;
	if (ext == "lk") return LK;
	if (ext == "sul" || ext == "samul" || ext == "vb" || ext == "vba") return VBA;
	if (ext == "html" || ext == "htm" || ext == "php") return HTML;
	if (ext == "txt") return TEXT;
	if (ext == "trd" || ext == "dck") return TRNSYS;
	if (ext == "py") return PYTHON;
	
	return NONE;
}

void wxCodeEditCtrl::SetKnownIdentifiers( const wxString &text )
{
	SetKeyWords( 1, text );
}

void wxCodeEditCtrl::EnableCallTips( bool en )
{
	m_callTipsEnabled = en;
}

void wxCodeEditCtrl::ClearCallTips()
{
	m_callTips.clear();
}

void wxCodeEditCtrl::ConfigureCallTips( wxUniChar start, wxUniChar end, bool case_sensitive )
{
	m_ctStart = start;
	m_ctEnd = end;
	m_ctCaseSensitive = case_sensitive;
}

void wxCodeEditCtrl::AddCallTip( const wxString &key, const wxString &value )
{
	wxString lckey = key;
	if ( !m_ctCaseSensitive )
		lckey.MakeLower();

	m_callTips[lckey] = value;
}
	
void wxCodeEditCtrl::ShowFindDialog()
{
	if (m_findDialog && (m_findDialog->GetWindowStyle() & wxFR_REPLACEDIALOG) == 0)
		m_findDialog->SetFocus();
	else
	{
		if (GetSelectionEnd() - GetSelectionStart() > 0)
			m_findData.SetFindString( this->GetSelectedText() );

		if (m_findDialog) delete m_findDialog;
		m_findDialog = new wxFindReplaceDialog(this, &m_findData, "Find Text", wxFR_NOUPDOWN);
	}

	m_lastFindPos = GetCurrentPos();
	m_findDialog->Show();
}

void wxCodeEditCtrl::ShowReplaceDialog()
{
	if (m_findDialog && (m_findDialog->GetWindowStyle() & wxFR_REPLACEDIALOG))
		m_findDialog->SetFocus();
	else
	{
		if (GetSelectionEnd() - GetSelectionStart() > 0)
			m_findData.SetFindString( this->GetSelectedText() );

		if (m_findDialog) delete m_findDialog;
		m_findDialog = new wxFindReplaceDialog(this, &m_findData, "Find & Replace", wxFR_NOUPDOWN|wxFR_REPLACEDIALOG);
	}
	
	m_findDialog->Show();
}

void wxCodeEditCtrl::HideFindReplaceDialog()
{
	if (m_findDialog)
	{
		delete m_findDialog;
		m_findDialog = 0;
	}
}

int	wxCodeEditCtrl::FindNext( int frtxt_len )
{
	wxString text = m_findData.GetFindString();
	if (frtxt_len < 0) frtxt_len = text.Len();

	int start = m_lastFindPos >= 0 ? m_lastFindPos+frtxt_len : 0;
	if (start > GetLength())
		start = 0;

	int flags = 0;
	
	if (m_findData.GetFlags() & wxFR_WHOLEWORD)
		flags |= wxSTC_FIND_WHOLEWORD;
	
	if (m_findData.GetFlags() & wxFR_MATCHCASE)
		flags |= wxSTC_FIND_MATCHCASE;

	m_lastFindPos = FindText(start, GetLength(), text, flags);
	if (m_lastFindPos >= 0)
		SetSelection(m_lastFindPos, m_lastFindPos+text.Len());
	else
	{
		m_lastFindPos = FindText(0, GetLength(), text, flags);
		if (m_lastFindPos >= 0)
			SetSelection(m_lastFindPos, m_lastFindPos+text.Len());
	}

	return m_lastFindPos;
}

int wxCodeEditCtrl::ReplaceNext( bool stop_at_find )
{
	bool cur_selected = false;
	if (m_findData.GetFlags() & wxFR_MATCHCASE)
		cur_selected = (GetSelectedText() == m_findData.GetFindString() );
	else
		cur_selected = (GetSelectedText().Lower() == m_findData.GetFindString().Lower());

	if (!cur_selected)
	{
		cur_selected = (FindNext() >= 0);
		if (stop_at_find)
			return cur_selected? m_lastFindPos : -1;
	}

	if (!cur_selected)
	{
		return -1;
	}

	ReplaceSelection( m_findData.GetReplaceString() );
	return FindNext(m_findData.GetReplaceString().Len());
}

wxString wxCodeEditCtrl::GetFindString()
{
	return m_findData.GetFindString();
}

void wxCodeEditCtrl::SetFindString( const wxString &s )
{
	m_findData.SetFindString(s);
}

void wxCodeEditCtrl::JumpToLine( int line, bool highlight )
{
	line--;
	if (line<0) line = 0;
	GotoLine(line);
		
	if (highlight)
	{
		MarkerDeleteAll(0);
		MarkerAdd(line, 0);
		SetSelection(PositionFromLine(line), GetLineEndPosition(line)+1);
	}
}

void wxCodeEditCtrl::YankLine()
{
	int line = GetCurrentLine();
	wxString text = GetLine(line);
	SetTargetStart( PositionFromLine(line) );
	SetTargetEnd( PositionFromLine(line+1) );
	ReplaceTarget( wxEmptyString );
	m_yankText = text;
}

void wxCodeEditCtrl::PutLine()
{
	if ( !m_yankText.IsEmpty() )
		InsertText( PositionFromLine( GetCurrentLine() ), m_yankText );
}
	
void wxCodeEditCtrl::OnMarginClick( wxStyledTextEvent &evt )
{
    if (evt.GetMargin() == 2)
	{
        int lineClick = LineFromPosition (evt.GetPosition());
        int levelClick = GetFoldLevel (lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
		{
            ToggleFold (lineClick);
        }
    }
	
	evt.Skip();
}

void wxCodeEditCtrl::OnCharAdded( wxStyledTextEvent &evt )
{
	wxUniChar ch = evt.GetKey();

	if (ch == '\n')
	{
		int curline = GetCurrentLine();
		
		if (curline - 1 >= 0 && curline -1 <= GetLineCount())
		{
			wxString prevline = GetLine(curline - 1);
		
			int prevlinelen = prevline.Length();
			static char buf[32];
			int i;
			for (i=0;i<prevlinelen && i < 31 && (prevline[i] == '\t' || prevline[i] == ' '); i++)
			{
				buf[i] = prevline[i];
			}
			buf[i] = '\0';
			
			ReplaceSelection(buf);
		}
	}
	else if (ch == m_ctStart && m_callTipsEnabled )
	{
		wxString buf = GetTextRange( PositionFromLine( GetCurrentLine() ), GetCurrentPos() );
		
		int i = (int)buf.Len()-2;
		while (i >= 0 && (buf[i] == '\t' || buf[i] == ' '))
			i--;

		int len = 0;
		while (i >= 0 && (isalpha(buf[i]) || isdigit(buf[i]) || buf[i] == '_'))
		{
			i--;
			len++;
		}

		wxString func = buf.Mid(i+1, len);
		if ( !m_ctCaseSensitive )
			func.MakeLower();

		if ( m_callTips.find(func) != m_callTips.end() )		
		{
			if (CallTipActive())			
				Cancel();
			else
				m_ctStack.Clear();

			wxString tip = LimitColumnWidth( m_callTips[func], 80 );
			m_ctStack.Add( tip );
			CallTipShow( GetCurrentPos(), tip );
		}
	}
	else if (ch == m_ctEnd && m_callTipsEnabled)
	{
		Cancel();
		if ( m_ctStack.Count() > 0 )
			m_ctStack.RemoveAt( m_ctStack.Count() - 1 );

		if ( m_ctStack.Count() > 0 )
			CallTipShow( GetCurrentPos(), m_ctStack.Last() );
	}
}

void wxCodeEditCtrl::OnUpdateUI( wxStyledTextEvent &evt )
{
	DoBraceMatch();
	evt.Skip();
}

void wxCodeEditCtrl::OnFindDialog( wxFindDialogEvent &evt )
{
    wxEventType type = evt.GetEventType();

    if ( type == wxEVT_COMMAND_FIND || type == wxEVT_COMMAND_FIND_NEXT )
    {
		if ( FindNext() < 0)
			wxMessageBox("Error: '" + m_findData.GetFindString() + "' could not be found.","Notice");
    }
    else if ( type == wxEVT_COMMAND_FIND_REPLACE )
    {
		if ( ReplaceNext(true) < 0)
			wxMessageBox("Error: '" + m_findData.GetFindString() + "' could not be found for replacement.","Notice");
    }
	else if ( type == wxEVT_COMMAND_FIND_REPLACE_ALL )
	{
		bool start_at_top = false;
		int count = 0;

		if (ReplaceNext() >= 0)
		{
			m_lastReplacePos = GetSelectionStart();
			int last_pos = GetSelectionStart();

			while (ReplaceNext() >= 0)
			{
				if (GetSelectionStart() < last_pos)
					start_at_top = true;

				last_pos = GetSelectionStart();

				if (start_at_top && last_pos >= m_lastReplacePos)
					break;

				count++;
			}
		}

		wxMessageBox( wxString::Format( "%d instances replaced.", count ) );
	}
    else if ( type == wxEVT_COMMAND_FIND_CLOSE )
    {
        wxFindReplaceDialog *dlg = evt.GetDialog();

		if ( dlg == m_findDialog )
            m_findDialog = 0;

        dlg->Destroy();
    }
}

void wxCodeEditCtrl::DoBraceMatch()
{
	int braceAtCaret = -1;
	int braceOpposite = -1;

	FindMatchingBracePosition(braceAtCaret, braceOpposite, false);

	if ((braceAtCaret != -1) && (braceOpposite == -1))
	{
		BraceBadLight(braceAtCaret);
		SetHighlightGuide(0);
	}
	else
	{
		BraceHighlight(braceAtCaret, braceOpposite);

		int columnAtCaret = GetColumn(braceAtCaret);
		int columnOpposite = GetColumn(braceOpposite);

		SetHighlightGuide(columnAtCaret < columnOpposite ? columnAtCaret : columnOpposite);
	}
}

bool wxCodeEditCtrl::FindMatchingBracePosition( int &braceAtCaret, int &braceOpposite, bool sloppy )
{
	bool isInside = false;
	
	int caretPos = GetCurrentPos();
	braceAtCaret = -1;
	braceOpposite = -1;
	wxUniChar charBefore = 0;
	wxUniChar styleBefore = 0;
	int lengthDoc = GetLength();

	if ((lengthDoc > 0) && (caretPos > 0))
	{
		// Check to ensure not matching brace that is part of a multibyte character
		if (PositionBefore(caretPos) == (caretPos - 1))
		{
			charBefore = GetCharAt(caretPos - 1);
			styleBefore = (char)(GetStyleAt(caretPos - 1) & 31);
		}
	}
	// Priority goes to character before caret
	if (charBefore && IsBrace(charBefore) )
	{
		braceAtCaret = caretPos - 1;
	}


	bool isAfter = true;
	if (lengthDoc > 0 && sloppy && (braceAtCaret < 0) && (caretPos < lengthDoc))
	{
		// No brace found so check other side
		// Check to ensure not matching brace that is part of a multibyte character
		if (PositionAfter(caretPos) == (caretPos + 1))
		{
			char charAfter = GetCharAt(caretPos);

			if (charAfter && IsBrace(charAfter))
			{
				braceAtCaret = caretPos;
				isAfter = false;
			}
		}
	}

	if (braceAtCaret >= 0)
	{
		braceOpposite = BraceMatch(braceAtCaret);

		if (braceOpposite > braceAtCaret) {
			isInside = isAfter;
		} else {
			isInside = !isAfter;
		}
	}

	return isInside;
}
