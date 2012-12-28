#ifdef WEX_USE_LK

#include <wx/app.h>
#include <wx/thread.h>
#include <wx/html/htmlwin.h>

#include "wex/lkscript.h" // defines LK_USE_WXWIDGETS
#include "wex/mtrand.h"
#include "wex/utils.h"

#include <lk_lex.h>
#include <lk_absyn.h>
#include <lk_eval.h>
#include <lk_stdlib.h>
#include <lk_invoke.h>
#include <lk_math.h>
#include <lk_env.h>
#include <lk_parse.h>


#include "wex/plot/plplotctrl.h"
#include "wex/plot/plbarplot.h"
#include "wex/plot/pllineplot.h"
#include "wex/plot/plscatterplot.h"

class PlotWin;

static int _iplot = 1;
static PlotWin *_curplot = 0;

class PlotWin : public wxFrame
{
private:
	wxPLPlotCtrl *m_plot;
public:
	PlotWin( wxWindow *parent )
		: wxFrame( parent, wxID_ANY, 
			wxString::Format("plot %d", _iplot++),
			wxDefaultPosition, wxSize(500,400) )
	{
		m_plot = new wxPLPlotCtrl( this, wxID_ANY );
		m_plot->SetBackgroundColour(*wxWHITE);
		_curplot = this;
		Show();
	}

	virtual ~PlotWin()
	{
		if ( _curplot == this )
			_curplot = 0;
	}

	enum { BAR, LINE, SCATTER };
	void Add( double *x, double *y, int len, int thick, wxColour &col, int type,
		const wxString &xlab, const wxString &ylab, const wxString &series,
		int xap, int yap)
	{
		if (len <= 0 ) return;
		
		std::vector<wxRealPoint> data;
		data.reserve( len );
		for (int i=0;i<len;i++)
			data.push_back( wxRealPoint( x[i], y[i] ) );

		wxPLPlottable *p = 0;

		switch (type )
		{
		case BAR:
			p = new wxPLBarPlot( data, series, col );
			break;
		case LINE:
			p = new wxPLLinePlot( data, series, col, wxPLLinePlot::SOLID, thick, false );
			break;
		case SCATTER:
			p = new wxPLScatterPlot( data, series, col, thick, false );
			break;
		}

		if (!p)
			return;

		p->SetXDataLabel( xlab );
		p->SetYDataLabel( ylab );
		m_plot->AddPlot( p, (wxPLPlotCtrl::AxisPos)xap, (wxPLPlotCtrl::AxisPos) yap );
		m_plot->RescaleAxes();
		m_plot->Refresh();
	}

	static PlotWin *Current() { return _curplot; }
	static void NewPlot( ) { _curplot = 0; }
	static wxPLPlotCtrl *Surface()
	{
		if ( Current() ) return Current()->m_plot;
		else return 0;
	}
};


void fcall_newplot( lk::invoke_t &cxt )
{
	LK_DOC("newplot", "Switches to a new plotting window on the next call to plot.", "([boolean:remove all]):none");
	PlotWin::NewPlot(  );

	if ( cxt.arg_count() == 1 && cxt.arg(0).as_boolean() )
	{
		wxWindowList wl = ::wxTopLevelWindows;
		for (size_t i=0;i<wl.size();i++)
			if ( PlotWin *p = dynamic_cast<PlotWin*>( wl[i] ))
				p->Destroy();
	}
}

void fcall_plot( lk::invoke_t &cxt )
{
	LK_DOC("plot", "Creates an XY line, bar, or scatter plot. Options include thick, type, color, xap, yap, xlabel, ylabel, series.", "(array:x, array:y, table:options):void");

	wxLKScriptCtrl *lksc = (wxLKScriptCtrl*)cxt.user_data();

	PlotWin *plot = PlotWin::Current();
	if ( plot == 0 )
		plot = new PlotWin( lksc->GetTopLevelWindowForScript() );

	lk::vardata_t &a0 = cxt.arg(0).deref();
	lk::vardata_t &a1 = cxt.arg(1).deref();

	if ( a0.type() == lk::vardata_t::VECTOR
		&& a1.type() == lk::vardata_t::VECTOR
		&& a0.length() == a1.length()
		&& a0.length() > 0 )
	{
		int thick = 1;
		int type = PlotWin::LINE;
		wxColour col = *wxBLUE;
		wxString xlab = "x";
		wxString ylab = "y";
		wxString series = wxEmptyString;
		int xap = wxPLPlotCtrl::X_BOTTOM;
		int yap = wxPLPlotCtrl::Y_LEFT;

		if (cxt.arg_count() > 2 && cxt.arg(2).deref().type() == lk::vardata_t::HASH )
		{
			lk::vardata_t &t = cxt.arg(2).deref();
			if ( lk::vardata_t *arg = t.lookup("thick") )
			{
				thick = arg->as_integer();
				if (thick < 1) thick = 1;
			}

			if ( lk::vardata_t *arg = t.lookup("type") )
			{
				wxString stype = arg->as_string().c_str();
				stype.Lower();
				if (stype == "bar") type = PlotWin::BAR;
				else if (stype == "scatter") type = PlotWin::SCATTER;
			}
			
			if (lk::vardata_t *arg = t.lookup("color") )
			{
				if ( arg->type() == lk::vardata_t::VECTOR
					&& arg->length() == 3 )
				{
					int r = arg->index(0)->as_integer();
					int g = arg->index(1)->as_integer();
					int b = arg->index(2)->as_integer();

					col = wxColour(r,g,b);
				}
				else
					col = wxColour( wxString(arg->as_string().c_str()));
			}

			if (lk::vardata_t *arg = t.lookup("xap"))
			{
				if (arg->as_string() == "top") xap = wxPLPlotCtrl::X_TOP;
			}

			if ( lk::vardata_t *arg = t.lookup("yap"))
			{
				if (arg->as_string() == "right") yap = wxPLPlotCtrl::Y_RIGHT;
			}

			if ( lk::vardata_t *arg = t.lookup("series"))
				series = arg->as_string().c_str();

			if ( lk::vardata_t *arg = t.lookup("xlabel"))
				xlab = arg->as_string().c_str();

			if ( lk::vardata_t *arg = t.lookup("ylabel"))
				ylab = arg->as_string().c_str();
		}
		
		int len = cxt.arg(0).length();
		double *x = new double[len];
		double *y = new double[len];

		for (int i=0;i<len;i++)
		{
			x[i] = a0.index(i)->as_number();
			y[i] = a1.index(i)->as_number();
		}

		plot->Add( x, y, len, thick, col, type, xlab, ylab, series, xap, yap );

		delete [] x;
		delete [] y;
	}
}

void fcall_plotopt( lk::invoke_t &cxt )
{
	LK_DOC("plotopt", "Modifies the current plot properties like title, coarse, fine, legend, legendpos, wpos, wsize", "(table:options):void");
	wxPLPlotCtrl *plot = PlotWin::Surface();
	if (!plot) return;

	bool mod = false;
	if ( lk::vardata_t *arg = cxt.arg(0).lookup("title") )
	{
		plot->SetTitle( arg->as_string() );
		mod = true;
	}
	
	if ( lk::vardata_t *arg = cxt.arg(0).lookup("coarse") )
	{
		plot->ShowCoarseGrid( arg->as_boolean() );
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(0).lookup("fine") )
	{
		plot->ShowFineGrid( arg->as_boolean() );
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(0).lookup("legend") )
	{
		plot->ShowLegend( arg->as_boolean() );
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(0).lookup("legendpos") )
	{
		double xper = 90, yper = 10;
		if (arg->type() == lk::vardata_t::VECTOR && arg->length() == 2 )
		{
			xper = arg->index(0)->as_number();
			yper = arg->index(1)->as_number();
			plot->SetLegendLocation( wxPLPlotCtrl::FLOATING, xper, yper);
			mod = true;
		}
		else if ( arg->type() == lk::vardata_t::STRING )
		{
			mod = plot->SetLegendLocation( arg->as_string() );
		}
	}

	if ( lk::vardata_t *arg = cxt.arg(0).lookup("window") )
	{
		if (PlotWin::Current()
			&& arg->type() == lk::vardata_t::VECTOR 
			&& arg->length() == 4 )
		{
			int x = arg->index(0)->as_integer();
			int y = arg->index(1)->as_integer();
			int w = arg->index(2)->as_integer();
			int h = arg->index(3)->as_integer();
			if ( x >= 0 && y >= 0 )
				PlotWin::Current()->SetPosition( wxPoint(x, y) );

			if ( w > 0 && h > 0 )
				PlotWin::Current()->SetClientSize( wxSize(w,h) );

		}
	}

	if (mod)
		plot->Refresh();
}

void fcall_plotpng( lk::invoke_t &cxt )
{
	LK_DOC( "plotpng", "Export the current plot as rendered on the screen to a PNG image file.", "(string:file name):boolean" );
	wxPLPlotCtrl *plot = PlotWin::Surface();
	if (!plot) return;
	cxt.result().assign( plot->Export( cxt.arg(0).as_string() ) );
}

void fcall_axis( lk::invoke_t &cxt )
{
	LK_DOC("axis", "Modifies axis properties (type, label, showlabel, min, max, ticklabels) on the current plot.", "(string:axis name 'x1' 'y1' 'x2' 'y2', table:options):void");
	lk_string axname = cxt.arg(0).as_string();
	wxPLPlotCtrl *plot = PlotWin::Surface();
	if (!plot) return;
	wxPLAxis *axis = 0;
	if (axname == "x1") axis = plot->GetXAxis1();
	if (axname == "x2") axis = plot->GetXAxis2();
	if (axname == "y1") axis = plot->GetYAxis1();
	if (axname == "y2") axis = plot->GetYAxis2();
	if (!axis) return;

	if (cxt.arg_count() < 2 || cxt.arg(1).type() != lk::vardata_t::HASH ) return;
	bool mod = false;

	
	if ( lk::vardata_t *arg = cxt.arg(1).lookup("type") )
	{
		double min, max;
		axis->GetWorld(&min, &max);

		if ( arg->as_string() == "log" )
		{
			if ( min <= 0 ) min = 0.001;
			if ( max < min ) max = min+10;

			wxPLLogAxis *log = new wxPLLogAxis( min, max, axis->GetLabel() );
			log->ShowTickText( axis->IsTickTextVisible() );
			log->ShowLabel( axis->IsLabelVisible() );

			if ( axname == "x1" ) plot->SetXAxis1( log );
			if ( axname == "x2" ) plot->SetXAxis2( log );
			if ( axname == "y1" ) plot->SetYAxis1( log );
			if ( axname == "y2" ) plot->SetYAxis2( log );

			axis = log;
			mod = true;
		}
		else if ( arg->as_string() == "linear" )
		{
			wxPLLinearAxis *lin = new wxPLLinearAxis( min, max, axis->GetLabel() );
			lin->ShowTickText( axis->IsTickTextVisible() );
			lin->ShowLabel( axis->IsLabelVisible() );

			if ( axname == "x1" ) plot->SetXAxis1( lin );
			if ( axname == "x2" ) plot->SetXAxis2( lin );
			if ( axname == "y1" ) plot->SetYAxis1( lin );
			if ( axname == "y2" ) plot->SetYAxis2( lin );

			axis = lin;
			mod = true;
		}		
	}

	if ( lk::vardata_t *arg = cxt.arg(1).lookup("label") )
	{
		axis->SetLabel( arg->as_string() );
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(1).lookup("min") )
	{
		double min, max;
		axis->GetWorld(&min, &max);
		min = arg->as_number();
		axis->SetWorld(min, max);
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(1).lookup("max") )
	{
		double min, max;
		axis->GetWorld(&min,&max);
		max = arg->as_number();
		axis->SetWorld(min,max);
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(1).lookup("ticklabels") )
	{
		axis->ShowTickText( arg->as_boolean() );
		mod = true;
	}

	if ( lk::vardata_t *arg = cxt.arg(1).lookup("showlabel") )
	{
		axis->ShowLabel( arg->as_boolean() );
		mod = true;
	}

	if (mod) plot->Refresh();
}

void fcall_rand( lk::invoke_t &cxt )
{
	LK_DOC("rand", "Generate a random number between 0 and 1.", "(none):number");
static wxMTRand rng;
	cxt.result().assign( rng.rand() );
}

void fcall_httpget( lk::invoke_t &cxt )
{
	LK_DOC("http_get", "Issue an HTTP GET request and return the text.", "(string:url):string");
	cxt.result().assign( wxWebHttpGet( cxt.arg(0).as_string() ) );
}

void fcall_httpdownload( lk::invoke_t &cxt )
{
	LK_DOC("http_download", "Download a file using HTTP", "(string:url, string:local file):boolean");
	cxt.result().assign( wxWebHttpDownload( cxt.arg(0).as_string(), cxt.arg(1).as_string() ) );
}

void fcall_decompress( lk::invoke_t &cxt )
{
	LK_DOC("decompress", "Decompress a local archive file.", "(string:archive, string:target):boolean");
	cxt.result().assign( wxDecompressFile( cxt.arg(0).as_string(), cxt.arg(1).as_string() ) );
}

void fcall_out( lk::invoke_t &cxt )
{
	LK_DOC("out", "Output data to the console.", "(...):none");

	wxLKScriptCtrl *lksc = (wxLKScriptCtrl*)cxt.user_data();
	wxString output;
	for (size_t i=0;i<cxt.arg_count();i++)
		output += cxt.arg(i).as_string();
	lksc->OnOutput( output );
}

void fcall_outln( lk::invoke_t &cxt )
{
	LK_DOC("outln", "Output data to the console with a newline.", "(...):none");
	
	wxLKScriptCtrl *lksc = (wxLKScriptCtrl*)cxt.user_data();
	wxString output;
	for (size_t i=0;i<cxt.arg_count();i++)
		output += cxt.arg(i).as_string(); 
	output += '\n';
	lksc->OnOutput( output );
}

void fcall_in(  lk::invoke_t &cxt )
{
	LK_DOC("in", "Input text from the user.", "(none):string");
	cxt.result().assign( wxGetTextFromUser("Standard Input:") );	
}

lk::fcall_t* wexlib_lkfuncs()
{
	static const lk::fcall_t vec[] = {
		fcall_in,
		fcall_out,
		fcall_outln,
		fcall_httpget,
		fcall_httpdownload,
		fcall_decompress,
		fcall_newplot,
		fcall_plot,
		fcall_plotopt,
		fcall_plotpng,
		fcall_axis, 
		fcall_rand,
		0 };
		
	return (lk::fcall_t*)vec;
}



static bool eval_callback( int line, void *cbdata )
{
	wxLKScriptCtrl *sc = ((wxLKScriptCtrl*)cbdata);

	if (!sc->OnEval( line )) return false;

	return !sc->IsStopFlagSet();
}

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
	m_env = new lk::env_t;	
	m_scriptRunning = false;
	m_stopScriptFlag = false;

	SetLanguage( LK );
	EnableCallTips( true );
	
	RegisterLibrary( wexlib_lkfuncs(), "I/O and Plotting Functions", this );
	RegisterLibrary( lk::stdlib_basic(), "Standard Operations" );
	RegisterLibrary( lk::stdlib_string(), "String Functions" );
	RegisterLibrary( lk::stdlib_math(), "Math Functions" );
	RegisterLibrary( lk::stdlib_wxui(), "User interface Functions" );
	
	wxFont font( *wxNORMAL_FONT );
	AnnotationSetStyleOffset( 512 );
	StyleSetFont( 512, font );
	StyleSetForeground( 512, *wxBLACK );
	StyleSetBackground( 512, wxColour(255,187,187) );
	
	m_topLevelWindow = 0;
}

wxLKScriptCtrl::~wxLKScriptCtrl()
{
	delete m_env;
}

bool wxLKScriptCtrl::OnEval( int /*line*/ )
{
	return !IsStopFlagSet();
}

void wxLKScriptCtrl::OnOutput( const wxString &output )
{
	wxLogStatus( output ); // default behavior
}

void wxLKScriptCtrl::RegisterLibrary( lk::fcall_t *funcs, const wxString &group, void *user_data )
{
	m_env->register_funcs( funcs, user_data );
	libdata x;
	x.library = funcs;
	x.name = group;
	m_libs.push_back( x );
	UpdateInfo();
}

void wxLKScriptCtrl::UpdateInfo()
{
	ClearCallTips();
	std::vector<lk_string> list = m_env->list_funcs();
	wxString funclist;
	for (size_t i=0;i<list.size();i++)
	{
		lk::doc_t d;
		if (lk::doc_t::info( m_env->lookup_func( list[i] ), d ))
		{
			wxString data = d.func_name + d.sig1 + "\n\n" + d.desc1;
			if (d.has_2) data += "\n\n" + d.func_name + d.sig2 + "\n\n" + d.desc2;
			if (d.has_3) data += "\n\n" + d.func_name + d.sig3 + "\n\n" + d.desc3;

			AddCallTip( d.func_name, data );	
			funclist += d.func_name + " ";
		}
	}

	SetKnownIdentifiers( funclist );
}


wxString wxLKScriptCtrl::GetHtmlDocs()
{
	wxString data;
	for ( size_t i=0;i<m_libs.size();i++)
		data += lk::html_doc( m_libs[i].name, m_libs[i].library );
	return data;
}

void wxLKScriptCtrl::ShowHelpDialog( wxWindow *custom_parent )
{
	if (custom_parent == 0)
		custom_parent = this;

	wxFrame *frm = new wxFrame( custom_parent, wxID_ANY, 
		"Scripting Reference", wxDefaultPosition, wxSize(900, 800) );
	wxHtmlWindow *html = new wxHtmlWindow( frm, wxID_ANY, wxDefaultPosition, wxDefaultSize );
	html->SetPage( GetHtmlDocs() );
	frm->Show();
}


void wxLKScriptCtrl::OnScriptTextChanged( wxStyledTextEvent &evt )
{
	if ( evt.GetModificationType() & wxSTC_MOD_INSERTTEXT 
		|| evt.GetModificationType() & wxSTC_MOD_DELETETEXT )
	{
		AnnotationClearLine( GetCurrentLine() );

		m_timer.Stop();
		m_timer.Start( 900, true );
		evt.Skip();
	}
}

void wxLKScriptCtrl::OnTimer( wxTimerEvent & )
{
	AnnotationClearAll();

			
	int first_error_line = 0;
	wxString output;

	wxString input = GetText();
	lk::input_string p( input );
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

bool wxLKScriptCtrl::IsScriptRunning()
{
	return m_scriptRunning;
}

bool wxLKScriptCtrl::IsStopFlagSet()
{
	return m_stopScriptFlag;
}

void wxLKScriptCtrl::Stop()
{
	m_stopScriptFlag = true;
}

bool wxLKScriptCtrl::Execute( const wxString &run_dir,
							 wxWindow *toplevel )
{
	if (m_scriptRunning)
	{
		wxMessageBox("A script is already running.");
		return false;
	}
		
	m_env->clear_objs();
	m_env->clear_vars();

	m_scriptRunning = true;
	m_stopScriptFlag = false;
	m_topLevelWindow = toplevel;

	//m_stopButton->Show();

	//Layout();

	wxString script = GetText();
	if (!run_dir.IsEmpty())
	{
		wxString fn = run_dir + "/~script";
		FILE *fp = fopen( (const char*)fn.c_str(), "w" );
		if (fp)
		{
			for (size_t i=0;i<script.Len();i++)
				fputc( (char)script[i], fp );
			fclose(fp);
		}
	}

	lk::input_string p( script );
	lk::parser parse( p );
	
	lk::node_t *tree = parse.script();
				
	wxYield();
	bool success = false;

	if ( parse.error_count() != 0 
		|| parse.token() != lk::lexer::END)
	{
		OnOutput("parsing did not reach end of input\n");
	}
	else
	{
		m_env->clear_vars();
		m_env->clear_objs();

		lk::vardata_t result;
		unsigned int ctl_id = lk::CTL_NONE;
		wxStopWatch sw;
		std::vector<lk_string> errors;
		if ( lk::eval( tree, m_env, errors, result, 0, ctl_id, eval_callback, this ) )
		{
			long time = sw.Time();
			OnOutput(wxString::Format("elapsed time: %ld msec\n", time));

			/*
			lk_string key;
			lk::vardata_t *value;
			bool has_more = env.first( key, value );
			while( has_more )
			{
				applog("env{%s}=%s\n", key, value->as_string().c_str());
				has_more = env.next( key, value );
			}
			*/

			success = true;
		}
		else
		{
			OnOutput("eval fail\n");
			for (size_t i=0;i<errors.size();i++)
				OnOutput( wxString(errors[i].c_str()) + "\n");

		}
	}
			
	int i=0;
	while ( i < parse.error_count() )
		OnOutput( parse.error(i++) );

	if (tree) delete tree;
		
	//m_stopButton->Hide();		
	//Layout();
		
	m_env->clear_objs();
	m_env->clear_vars();

	m_scriptRunning = false;
	m_stopScriptFlag = false;
	m_topLevelWindow = 0;

	return success;
}

#endif // WEX_USE_LK
