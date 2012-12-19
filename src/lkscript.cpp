#ifdef WEX_USE_LK

#include <wx/thread.h>

#include "wex/lkscript.h" // defines LK_USE_WXWIDGETS

#include <lk_lex.h>
#include <lk_absyn.h>
#include <lk_eval.h>
#include <lk_stdlib.h>
#include <lk_invoke.h>
#include <lk_math.h>
#include <lk_env.h>
#include <lk_parse.h>


enum { IDT_TIMER = wxID_HIGHEST+213 };

BEGIN_EVENT_TABLE( wxLKScriptCtrl, wxCodeEditCtrl )	
	EVT_STC_MODIFIED( wxID_ANY, wxLKScriptCtrl::OnScriptTextChanged )
	EVT_TIMER( IDT_TIMER, wxLKScriptCtrl::OnTimer )
END_EVENT_TABLE()

wxLKScriptCtrl::wxLKScriptCtrl( wxWindow *parent, int id,
	const wxPoint &pos, const wxSize &size )
	: wxCodeEditCtrl( parent, id, pos, size ),
		m_timer( this, IDT_TIMER )
{
	SetLanguage( LK );
	EnableCallTips( true );

	lk::env_t env;	
	env.register_funcs( lk::stdlib_basic() );
	env.register_funcs( lk::stdlib_string() );
	env.register_funcs( lk::stdlib_math() );
	env.register_funcs( lk::stdlib_wxui() );
	
	std::vector<lk_string> list = env.list_funcs();
	wxString funclist;
	for (size_t i=0;i<list.size();i++)
	{
		lk::doc_t d;
		if (lk::doc_t::info( env.lookup_func( list[i] ), d ))
		{
			wxString data = d.func_name + d.sig1 + "\n\n" + d.desc1;
			if (d.has_2) data += "\n\n" + d.func_name + d.sig2 + "\n\n" + d.desc2;
			if (d.has_3) data += "\n\n" + d.func_name + d.sig3 + "\n\n" + d.desc3;

			AddCallTip( d.func_name, data );	
			funclist += d.func_name + " ";
		}
	}

	SetKnownIdentifiers( funclist );

	wxFont font( *wxNORMAL_FONT );
	AnnotationSetStyleOffset( 512 );
	StyleSetFont( 512, font );
	StyleSetForeground( 512, *wxBLACK );
	StyleSetBackground( 512, wxColour(255,187,187) );

}

wxLKScriptCtrl::~wxLKScriptCtrl()
{
}

void wxLKScriptCtrl::OnScriptTextChanged( wxStyledTextEvent &evt )
{
	if ( evt.GetModificationType() & wxSTC_MOD_INSERTTEXT 
		|| evt.GetModificationType() & wxSTC_MOD_DELETETEXT )
	{
		AnnotationClearLine( GetCurrentLine() );

		m_timer.Stop();
		m_timer.Start( 400, true );
		evt.Skip();
	}
}

void wxLKScriptCtrl::OnTimer( wxTimerEvent & )
{
	AnnotationClearAll();

			
	int first_error_line = 0;
	wxString output;
	lk::input_string p( GetText() );
	lk::parser parse( p );
	lk::node_t *tree = parse.script();
	if (tree) delete tree;

	if ( parse.error_count() == 0 
		&& parse.token() == lk::lexer::END)
	{
		return;
	}
	else
	{
		int i=0;
		while ( i < parse.error_count() )
		{
			int line;
			output += parse.error(i, &line) + '\n';
			if (i == 0)
				first_error_line = line; // adjust for 0 based line in scintilla

			i++;
		}
	}

	if (parse.token() != lk::lexer::END)
		output += "parsing did not reach end of input";
	else
		output.Trim();

	first_error_line--;

	AnnotationSetText( first_error_line, output );
	AnnotationSetStyle( first_error_line, 0 );
	
	if ( first_error_line != GetCurrentLine() 
		&& ( first_error_line < GetFirstVisibleLine() 
			|| first_error_line > GetFirstVisibleLine()+LinesOnScreen()) )
	{
		AnnotationSetText( GetCurrentLine(), output );
		AnnotationSetStyle( GetCurrentLine(), 0 );
	}

	AnnotationSetVisible( wxSTC_ANNOTATION_BOXED );
}


#endif // WEX_USE_LK
