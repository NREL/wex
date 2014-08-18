#include <wx/app.h>
#include <wx/thread.h>
#include <wx/html/htmlwin.h>

#include "wex/lkscript.h" // defines LK_USE_WXWIDGETS
#include "wex/mtrand.h"
#include "wex/utils.h"
#include "wex/jsonreader.h"

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
#include "wex/plot/plwindrose.h"

enum { BAR, HBAR, LINE, SCATTER, WINDROSE };
static void CreatePlot( wxPLPlotCtrl *plot, double *x, double *y, int len, int thick, wxColour &col, int type,
	const wxString &xlab, const wxString &ylab, const wxString &series,
	int xap, int yap, double base_x, const wxString &stackon)
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
	{
		wxPLBarPlot *bar = new wxPLBarPlot( data, series, col );

		if ( !stackon.IsEmpty() )
			if ( wxPLBarPlot *pp = dynamic_cast<wxPLBarPlot*>( plot->GetPlotByLabel( stackon ) ) )
				bar->SetStackedOn( pp );
		
		p = bar;
	}
		break;
	case HBAR:
	{
		wxPLHBarPlot *hbar = new wxPLHBarPlot( data, base_x, series, col );
		
		if ( !stackon.IsEmpty() )
			if ( wxPLHBarPlot *pp = dynamic_cast<wxPLHBarPlot*>( plot->GetPlotByLabel( stackon ) ) )
				hbar->SetStackedOn( pp );

		p = hbar;
	}
		break;
	case LINE:
		p = new wxPLLinePlot( data, series, col, wxPLLinePlot::SOLID, thick, false );
		break;
	case SCATTER:
		p = new wxPLScatterPlot( data, series, col, thick, false );
		break;
	case WINDROSE:
		p = new wxPLWindRose(data, series, col, thick, false);
		break;
	}

	if (!p)
		return;

	p->SetXDataLabel( xlab );
	p->SetYDataLabel( ylab );
	plot->AddPlot( p, (wxPLPlotCtrl::AxisPos)xap, (wxPLPlotCtrl::AxisPos) yap );
	plot->Invalidate();
	plot->Refresh();
}


class PlotWin;

static wxWindow *s_curToplevelParent = 0;
static int _iplot = 1;
static PlotWin *s_curPlotWin = 0;
static wxPLPlotCtrl *s_curPlot = 0;

static wxWindow *GetCurrentTopLevelWindow()
{
	wxWindowList &wl = ::wxTopLevelWindows;
	for( wxWindowList::iterator it = wl.begin(); it != wl.end(); ++it )
		if ( wxTopLevelWindow *tlw = dynamic_cast<wxTopLevelWindow*>( *it ) )
			if ( tlw->IsShown() && tlw->IsActive() )
				return tlw;

	return 0;
}

class PlotWin : public wxFrame
{
public:
	wxPLPlotCtrl *m_plot;

	PlotWin( wxWindow *parent )
		: wxFrame( parent, wxID_ANY, 
			wxString::Format("plot %d", _iplot++),
			wxDefaultPosition, wxSize(500,400),
			(parent != 0 ? wxFRAME_FLOAT_ON_PARENT : 0 )|wxCLOSE_BOX|wxCLIP_CHILDREN|wxCAPTION|wxRESIZE_BORDER )
	{
		m_plot = new wxPLPlotCtrl( this, wxID_ANY );
		m_plot->SetBackgroundColour(*wxWHITE);
		Show();
	}

	virtual ~PlotWin()
	{
		if ( s_curPlot == m_plot )
			s_curPlot = 0;

		if ( s_curPlotWin == this )
			s_curPlotWin = 0;

	}
};

static void ClearPlotSurface()
{
	s_curPlot = 0;
	s_curPlotWin = 0;
}

static wxPLPlotCtrl *GetPlotSurface( wxWindow *parent )
{
	// somebody externally defined the plot target?
	if ( s_curPlot != 0 ) 
		return s_curPlot;

	// there is current window
	if ( s_curPlotWin != 0 ) {
		s_curPlot = s_curPlotWin->m_plot;
		return s_curPlot;
	}

	// create a new window
	s_curPlotWin = new PlotWin( parent );
	s_curPlot = s_curPlotWin->m_plot;
	return s_curPlot;
}

void wxLKSetToplevelParent( wxWindow *parent )
{
	s_curToplevelParent = parent;
}

void wxLKSetPlotTarget( wxPLPlotCtrl *plot )
{
	s_curPlot = plot;
}

wxPLPlotCtrl *wxLKGetPlotTarget()
{
	return s_curPlot;
}

void fcall_newplot( lk::invoke_t &cxt )
{
	LK_DOC("newplot", "Switches to a new plotting window on the next call to plot.", "([boolean:remove all]):none");
	ClearPlotSurface();

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
	LK_DOC("plot", "Creates an XY line, bar, horizontal bar, or scatter plot. Options include thick, type, color, xap, yap, xlabel, ylabel, series, baseline, stackon.", "(array:x, array:y, table:options):void");
	
	wxPLPlotCtrl *plot = GetPlotSurface( 
		(s_curToplevelParent!=0)
			? s_curToplevelParent 
			: GetCurrentTopLevelWindow() );

	lk::vardata_t &a0 = cxt.arg(0).deref();
	lk::vardata_t &a1 = cxt.arg(1).deref();

	if ( a0.type() == lk::vardata_t::VECTOR
		&& a1.type() == lk::vardata_t::VECTOR
		&& a0.length() == a1.length()
		&& a0.length() > 0 )
	{
		int thick = 1;
		double base_x = 0.0; // used for horizontal bar plots
		int type = LINE;
		wxColour col = *wxBLUE;
		wxString xlab = "x";
		wxString ylab = "y";
		wxString series = wxEmptyString;
		int xap = wxPLPlotCtrl::X_BOTTOM;
		int yap = wxPLPlotCtrl::Y_LEFT;
		wxString stackon;

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
				if (stype == "bar") type = BAR;
				if (stype == "hbar") type = HBAR;
				else if (stype == "scatter") type = SCATTER;
				else if (stype == "windrose") type = WINDROSE;
			}

			if ( lk::vardata_t *arg = t.lookup("baseline") )
			{
				base_x = arg->as_number();
			}

			if ( lk::vardata_t *arg = t.lookup("stackon") )
			{
				stackon = arg->as_string();
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

		CreatePlot( plot, x, y, len, thick, col, type, xlab, ylab, series, xap, yap, base_x, stackon );

		delete [] x;
		delete [] y;
	}
}

void fcall_plotopt( lk::invoke_t &cxt )
{
	LK_DOC("plotopt", "Modifies the current plot properties like title, coarse, fine, legend, legendpos, wpos, wsize, autoscale", "(table:options):void");
	wxPLPlotCtrl *plot = s_curPlot;
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
		if ( s_curPlotWin
			&& arg->type() == lk::vardata_t::VECTOR 
			&& arg->length() == 4 )
		{
			int x = arg->index(0)->as_integer();
			int y = arg->index(1)->as_integer();
			int w = arg->index(2)->as_integer();
			int h = arg->index(3)->as_integer();
			if ( x >= 0 && y >= 0 )
				s_curPlotWin->SetPosition( wxPoint(x, y) );

			if ( w > 0 && h > 0 )
				s_curPlotWin->SetClientSize( wxSize(w,h) );

		}
	}
	
	if (mod)
	{
		plot->Invalidate();
		plot->Refresh();
	}
}

void fcall_plotout( lk::invoke_t &cxt )
{
	LK_DOC( "plotout", "Output the current plot to a file. If the optional format parameter is not given, a PNG format image file is generated.  Valid formats are png, bmp, jpg, pdf, svg.", "(string:file name, [string:format]):boolean" );
	wxPLPlotCtrl *plot = s_curPlot;
	if (!plot) return;
	
	wxString file( cxt.arg(0).as_string() );
	wxString format( "png" ); // default format is PNG
	if ( cxt.arg_count() > 1 )
		format = cxt.arg(1).as_string().Lower();

	bool ok = false;
	if ( format == "pdf" ) 
		ok = plot->ExportPdf( file );
	else if ( format == "svg" )
		ok = plot->ExportSvg( file );
	else if ( format == "bmp" )
		ok = plot->Export( file, wxBITMAP_TYPE_BMP );
	else if ( format == "jpg" )
		ok = plot->Export( file, wxBITMAP_TYPE_JPEG );
	else if ( format == "png" )
		ok = plot->Export( file, wxBITMAP_TYPE_PNG );

	cxt.result().assign( ok );
}

void fcall_axis( lk::invoke_t &cxt )
{
	LK_DOC("axis", "Modifies axis properties (type, label, labels[2D array for 'label' axis type], showlabel, min, max, ticklabels) on the current plot.", "(string:axis name 'x1' 'y1' 'x2' 'y2', table:options):void");
	lk_string axname = cxt.arg(0).as_string();
	wxPLPlotCtrl *plot = s_curPlot;
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

		wxPLAxis *axis_new = 0;

		if ( arg->as_string() == "log" )
		{
			if ( min <= 0 ) min = 0.001;
			if ( max < min ) max = min+10;

			axis_new = new wxPLLogAxis( min, max, axis->GetLabel() );
		}
		else if ( arg->as_string() == "linear" )
		{
			axis_new = new wxPLLinearAxis( min, max, axis->GetLabel() );
		}
		else if ( arg->as_string() == "time" )
		{
			axis_new = new wxPLTimeAxis( min, max, axis->GetLabel() );
		}
		else if ( arg->as_string() == "label" )
		{
			if (lk::vardata_t *_tx = cxt.arg(1).lookup("labels"))
			{
				lk::vardata_t &tx = _tx->deref();
				if ( tx.type() == lk::vardata_t::VECTOR )
				{
					wxPLLabelAxis *axl = new wxPLLabelAxis( min, max, axis->GetLabel() );					
					for( size_t i=0;i<tx.length();i++ )
					{
						lk::vardata_t &item = tx.index(i)->deref();
						if ( item.type() == lk::vardata_t::VECTOR && item.length() == 2 )
							axl->Add( item.index(0)->as_number(), item.index(1)->as_string() );
					}

					axis_new = axl;
				}
			}

		}

		if ( axis_new != 0 )
		{
			axis_new->ShowTickText( axis->IsTickTextVisible() );
			axis_new->ShowLabel( axis->IsLabelVisible() );
			mod = true;
			axis = axis_new;
			if (axname == "x1") plot->SetXAxis1( axis_new );
			if (axname == "x2") plot->SetXAxis2( axis_new );
			if (axname == "y1") plot->SetYAxis1( axis_new );
			if (axname == "y2") plot->SetYAxis2( axis_new );
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
	
	if (mod) {
		plot->Invalidate();
		plot->Refresh();
	}
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




static void jsonval_to_lkval( const wxJSONValue &jv, lk::vardata_t &lv )
{
	switch( jv.GetType() )
	{
    case wxJSONTYPE_INT:        /*!< the object contains an integer           */
		lv.assign( (double)jv.AsInt() );
		break;
    case wxJSONTYPE_UINT:       /*!< the object contains an unsigned integer  */
		lv.assign( (double)jv.AsUInt() );
		break;
    case wxJSONTYPE_DOUBLE:     /*!< the object contains a double             */
		lv.assign( jv.AsDouble() );
		break;
    case wxJSONTYPE_STRING:     /*!< the object contains a wxString object    */
		lv.assign( jv.AsString() );
		break;
    case wxJSONTYPE_CSTRING:    /*!< the object contains a static C-string    */
		lv.assign( wxString(jv.AsCString()) );
		break;
    case wxJSONTYPE_BOOL:       /*!< the object contains a boolean            */
		lv.assign( (double)( jv.AsBool() ? 1.0 : 0.0 ) );
		break;
    case wxJSONTYPE_ARRAY:      /*!< the object contains an array of values   */
		lv.empty_vector();
		lv.vec()->resize( jv.Size() );
		for( size_t i=0;i<(size_t)jv.Size();i++ )
			jsonval_to_lkval( jv.ItemAt( i ), lv.vec()->at( i ) );
		break;
    case wxJSONTYPE_OBJECT:     /*!< the object contains a map of keys/values */
	{
		lv.empty_hash();
		wxArrayString keys = jv.GetMemberNames();
		for( size_t i=0;i<keys.size();i++ )
		{
			lk::vardata_t &val = lv.hash_item( keys[i] );
			jsonval_to_lkval( jv.ItemAt( keys[i] ), val );
		}
	}
		break;
    case wxJSONTYPE_LONG:       /*!< the object contains a 32-bit integer     */
		lv.assign( (double)jv.AsInt32() );
		break;
    case wxJSONTYPE_INT64:      /*!< the object contains a 64-bit integer     */
		lv.assign( (double)jv.AsInt64() );
		break;
    case wxJSONTYPE_ULONG:      /*!< the object contains an unsigned 32-bit integer  */
		lv.assign( (double)jv.AsUInt32() );
		break;
    case wxJSONTYPE_UINT64:     /*!< the object contains an unsigned 64-bit integer  */
		lv.assign( (double)jv.AsUInt64() );
		break;
    case wxJSONTYPE_SHORT:      /*!< the object contains a 16-bit integer            */
		lv.assign( (double)jv.AsShort() );
		break;
    case wxJSONTYPE_USHORT:     /*!< the object contains a 16-bit unsigned integer   */
		lv.assign( (double)jv.AsUShort() );
		break;

	case wxJSONTYPE_MEMORYBUFF:  /*!< the object contains a binary memory buffer   */
    case wxJSONTYPE_INVALID:  /*!< the object is not uninitialized        */
    case wxJSONTYPE_NULL:       /*!< the object contains a NULL value         */
	default:
		lv.nullify();
		return;
	}
}

static void fcall_jsonparse( lk::invoke_t &cxt )
{
	LK_DOC( "jsonparse", "Parse a JSON string and return an LK object hierarchy.  On error, returns null.", "(string):variant" );
	wxJSONReader reader;
	wxJSONValue root;
	if (reader.Parse( cxt.arg(0).as_string(), &root )!=0)
	{
		cxt.result().nullify();
		return;
	}

	jsonval_to_lkval( root, cxt.result() );
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

lk::fcall_t* wxLKHttpFunctions()
{
	static const lk::fcall_t vec[] = {
		fcall_httpget,
		fcall_httpdownload,
		0 };
		
	return (lk::fcall_t*)vec;
}

lk::fcall_t* wxLKPlotFunctions()
{
	static const lk::fcall_t vec[] = {
		fcall_newplot,
		fcall_plot,
		fcall_plotopt,
		fcall_plotout,
		fcall_axis, 
		0 };
		
	return (lk::fcall_t*)vec;
}

lk::fcall_t* wxLKMiscFunctions()
{
	static const lk::fcall_t vec[] = {
		fcall_decompress, 
		fcall_rand,
		fcall_jsonparse,
		0 };
		
	return (lk::fcall_t*)vec;
}

lk::fcall_t* wxLKStdOutFunctions()
{
	static const lk::fcall_t vec[] = {
		fcall_out, 
		fcall_outln,
		0 };
		
	return (lk::fcall_t*)vec;
}



enum { IDT_TIMER = wxID_HIGHEST+213 };

BEGIN_EVENT_TABLE( wxLKScriptCtrl, wxCodeEditCtrl )	
	EVT_STC_MODIFIED( wxID_ANY, wxLKScriptCtrl::OnScriptTextChanged )
	EVT_TIMER( IDT_TIMER, wxLKScriptCtrl::OnTimer )
END_EVENT_TABLE()

wxLKScriptCtrl::wxLKScriptCtrl( wxWindow *parent, int id,
	const wxPoint &pos, const wxSize &size, unsigned long libs )
	: wxCodeEditCtrl( parent, id, pos, size ),
		m_timer( this, IDT_TIMER )
{
	m_tree = 0;
	m_env = new lk::env_t;	
	m_scriptRunning = false;
	m_stopScriptFlag = false;

	SetLanguage( LK );
	EnableCallTips( true );
	
	if( libs & wxLK_STDLIB_BASIC ) RegisterLibrary( lk::stdlib_basic(), "Standard Operations" );
	if( libs & wxLK_STDLIB_STRING ) RegisterLibrary( lk::stdlib_string(), "String Functions" );
	if( libs & wxLK_STDLIB_MATH ) RegisterLibrary( lk::stdlib_math(), "Math Functions" );
	if( libs & wxLK_STDLIB_WXUI ) RegisterLibrary( lk::stdlib_wxui(), "User interface Functions" );
	if( libs & wxLK_STDLIB_PLOT ) RegisterLibrary( wxLKPlotFunctions(), "Plotting Functions", this );
	if( libs & wxLK_STDLIB_HTTP ) RegisterLibrary( wxLKHttpFunctions(), "HTTP Functions", this );
	if( libs & wxLK_STDLIB_MISC ) RegisterLibrary( wxLKMiscFunctions(), "Misc Functions", this );
	if( libs & wxLK_STDLIB_SOUT ) RegisterLibrary( wxLKStdOutFunctions(), "BIOS Functions", this );
	
	wxFont font( *wxNORMAL_FONT );
	AnnotationSetStyleOffset( 512 );
	StyleSetFont( 512, font );
	StyleSetForeground( 512, *wxBLACK );
	StyleSetBackground( 512, wxColour(255,187,187) );
	
	m_topLevelWindow = 0;
}

wxLKScriptCtrl::~wxLKScriptCtrl()
{
	if ( m_tree ) delete m_tree;
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
		m_timer.Start( 1700, true );
	}
	
	evt.Skip();
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
	if ( first_error_line < 0 ) first_error_line = 0;

	AnnotationSetText( first_error_line, output );
	AnnotationSetStyle( first_error_line, 0 );
	
	int curline = GetCurrentLine();
	if ( curline < 0 )
		curline = GetFirstVisibleLine();

	if ( first_error_line != curline
		&& ( first_error_line < GetFirstVisibleLine() 
			|| first_error_line > GetFirstVisibleLine()+LinesOnScreen()) )
	{
		AnnotationSetText( curline, output );
		AnnotationSetStyle( curline, 0 );
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


class my_eval : public lk::eval
{
	wxLKScriptCtrl *m_lcs;
public:
	my_eval( lk::node_t *tree, lk::env_t *env, wxLKScriptCtrl *lcs ) 
		: lk::eval( tree, env ), m_lcs(lcs)
	{
	}

	virtual bool on_run( int line )
	{
		if ( !m_lcs->OnEval( line ) ) return false;

		return !m_lcs->IsStopFlagSet();
	}
};


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
	s_curToplevelParent = toplevel;

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
	
	if ( m_tree != 0 ) delete m_tree;
	m_tree = parse.script();
				
	wxYield();
	bool success = false;

	if ( parse.error_count() != 0 
		|| parse.token() != lk::lexer::END)
	{
		OnOutput("Parsing did not reach end of input.\n");
	}
	else
	{
		m_env->clear_vars();
		m_env->clear_objs();

		wxStopWatch sw;
		my_eval e( m_tree, m_env, this );

		if ( e.run() )
		{
			double time = sw.Time();
			OnOutput(wxString::Format("Elapsed time: %.1lf seconds.\n", 0.001*time));

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
			OnOutput("Script did not finish.\n");
			for (size_t i=0;i<e.error_count();i++)
				OnOutput( e.get_error(i) + "\n");

		}
	}
			
	int i=0;
	while ( i < parse.error_count() )
		OnOutput( parse.error(i++) );
			
	m_env->clear_objs();

	m_scriptRunning = false;
	m_stopScriptFlag = false;
	m_topLevelWindow = 0;

	return success;
}

