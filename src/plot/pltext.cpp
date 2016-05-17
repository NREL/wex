#include <wx/tokenzr.h>

#include <wex/plot/pltext.h>

struct escape_sequence
{
	wxString code;
	wxUniChar value;
};

static escape_sequence escape_codes[] = {

	{ "Alpha",   L'\x0391' },
	{ "Beta",    L'\x0392' },
	{ "Gamma",   L'\x0393' },
	{ "Delta",   L'\x0394' },
	{ "Epsilon", L'\x0395' },
	{ "Zeta",    L'\x0396' },
	{ "Eta",     L'\x0397' },
	{ "Theta",   L'\x0398' },
	{ "Iota",    L'\x0399' },
	{ "Kappa",   L'\x039a' },
	{ "Lambda",  L'\x039b' },
	{ "Mu",      L'\x039c' },
	{ "Nu",      L'\x039d' },
	{ "Xi",      L'\x039e' },
	{ "Omicron", L'\x039f' },
	{ "Pi",      L'\x03a0' },
	{ "Rho",     L'\x03a1' },
	{ "Sigma",   L'\x03a3' },
	{ "Tau",     L'\x03a4' },
	{ "Upsilon", L'\x03a5' },
	{ "Phi",     L'\x03a6' },
	{ "Chi",     L'\x03a7' },
	{ "Psi",     L'\x03a8' },
	{ "Omega",   L'\x03a9' },

	{ "alpha",   L'\x03b1' },
	{ "beta",    L'\x03b2' },
	{ "gamma",   L'\x03b3' },
	{ "delta",   L'\x03b4' },
	{ "epsilon", L'\x03b5' },
	{ "zeta",    L'\x03b6' },
	{ "eta",     L'\x03b7' },
	{ "theta",   L'\x03b8' },
	{ "iota",    L'\x03b9' },
	{ "kappa",   L'\x03ba' },
	{ "lambda",  L'\x03bb' },
	{ "mu",      L'\x03bc' },
	{ "nu",      L'\x03bd' },
	{ "xi",      L'\x03be' },
	{ "omicron", L'\x03bf' },
	{ "pi",      L'\x03c0' },
	{ "rho",     L'\x03c1' },
	{ "fsigma",  L'\x03c3' },
	{ "sigma",   L'\x03c3' },
	{ "tau",     L'\x03c4' },
	{ "upsilon", L'\x03c5' },
	{ "phi",     L'\x03c6' },
	{ "chi",     L'\x03c7' },
	{ "psi",     L'\x03c8' },
	{ "omega",   L'\x03c9' },

	{ "emph",    L'\x00a1' },
	{ "qmark",   L'\x00bf' },
	{ "cent",    L'\x00a2' },
	{ "pound",   L'\x00a3' },
	{ "euro",    L'\x20ac' },
	{ "section", L'\x00a7' },
	{ "dot",     L'\x00b7' },
	{ "mult",    L'\x00d7' },
	{ "copy",    L'\x00a9' },
	{ "reg",     L'\x00ae' },
	{ "deg",     L'\x00b0' },
	{ "pm",      L'\x00b1' },
	{ "ne",      L'\x2260' },
	{ "approx",  L'\x2248' },

	{ wxEmptyString, 0 },
};

static const int FontPointAdjust = 2;

wxPLTextLayout::wxPLTextLayout( wxPLOutputDevice &dc, const wxString &text, TextAlignment ta )
	: m_bounds(0, 0)
{
	if ( text.IsEmpty() ) return;

	// get current font state for subsequent relative adjustments
	double fontPoints = dc.Font();

	// split text into lines, and parse each one into text pieces after resolving escape sequences
	wxArrayString lines = wxStringTokenize( text, "\r\n" );
	for (size_t i=0;i<lines.Count();i++)
		m_lines.push_back( Parse( Escape(lines[i]) ) );

	if ( m_lines.size() == 0 ) return;
		
	// compute extents of each text piece with the right font
	for ( size_t i=0;i<m_lines.size(); i++ )
	{
		for ( size_t j=0;j<m_lines[i].size(); j++ )
		{
			text_piece &tp = m_lines[i][j];
			double width, height;
			dc.Font( tp.state == text_piece::NORMAL ? fontPoints : fontPoints-FontPointAdjust );
			dc.Measure( tp.text, &width, &height );
			tp.size.x = width;
			tp.size.y = height;
		}
	}

	// obtain the approximate heights for normal and small text
	dc.Font( fontPoints-FontPointAdjust );
	double height_small = fontPoints-FontPointAdjust;
	dc.Measure( "0", NULL, &height_small );

	dc.Font( fontPoints );
	double height_normal = fontPoints;
	dc.Measure( "0", NULL, &height_normal );

	// sequentially calculate the origins of each text piece
		
	const double offset = 0.25*height_small; // amount to raise/lower super/sub-scripts
	double y = 0;
		
	for ( size_t i=0;i<m_lines.size(); i++ )
	{
		double x = 0;
		bool has_sup = false, has_sub = false;
		// layout this line's X positions, keep track of whether it has super/subs
		for ( size_t j=0; j< m_lines[i].size(); j++ )
		{
			text_piece &tp = m_lines[i][j];
			if ( tp.state == text_piece::SUPERSCRIPT ) has_sup = true;
			else if (tp.state == text_piece::SUBSCRIPT ) has_sub = true;
				
			// save original relative alignment from x 
			// allows future realignment of text pieces if needed
			tp.aligned_x = x;

			tp.origin.x = x;
			x += tp.size.x;

		}

		// save the line width as the maximum bounds
		if ( x > m_bounds.x )
			m_bounds.x = x;

		// layout this line's Y positions
		if ( has_sup ) y += offset;
		for ( size_t j=0;j<m_lines[i].size(); j++ )
		{
			text_piece &tp = m_lines[i][j];
			if ( tp.state == text_piece::NORMAL )
				tp.origin.y = y;
			else if (tp.state == text_piece::SUPERSCRIPT )
				tp.origin.y = y - offset;
			else if (tp.state == text_piece::SUBSCRIPT )
				tp.origin.y = y + height_normal - 3*offset;
		}

		y += height_normal + offset/3;
	}

	if ( ta != LEFT )
		Align( ta );

	// save the final y position as the maximum height
	m_bounds.y = y;
}

void wxPLTextLayout::Align( TextAlignment ta )
{
	// realign x offsets for differently aligned text
	// now that we know the total bounds width
	// note: does not require recalculating text sizes since they are cached
	for ( size_t i=0;i<m_lines.size();i++ )
	{
		double line_width = 0;
		for ( size_t j=0;j<m_lines[i].size();j++ )
		{
			// restore original aligned positions for each text piece
			m_lines[i][j].origin.x = m_lines[i][j].aligned_x;
			line_width += m_lines[i][j].size.x;
		}

		double offset = 0;
		if ( ta == CENTER ) offset = 0.5*(m_bounds.x - line_width);
		else if ( ta == RIGHT ) offset = m_bounds.x - line_width;

		if ( offset != 0 ) // only do this for center/right alignments
			for ( size_t j=0;j<m_lines[i].size();j++ )
				m_lines[i][j].origin.x += offset;
	}
}

void wxPLTextLayout::Render( wxPLOutputDevice &dc, double x, double y, double rotationDegrees, bool drawBounds )
{
	if ( m_lines.size() == 0 ) return;

	double fontPoints = dc.Font();

	if ( drawBounds )
	{
		dc.Pen( *wxLIGHT_GREY, 0.5 );
		dc.NoBrush();
	}

	// layout has already been calculated, assuming the same font.
	// render the text directly given the starting coordinates
	if ( rotationDegrees == 0.0 )
	{
		for ( size_t i=0;i<m_lines.size();i++ )
		{
			for ( size_t j=0;j<m_lines[i].size();j++ )
			{
				text_piece &tp = m_lines[i][j];
				dc.Font( tp.state == text_piece::NORMAL ? fontPoints : fontPoints-FontPointAdjust );
				dc.Text( tp.text, x + tp.origin.x, y + tp.origin.y );				
				if ( drawBounds )
					dc.Rect( x+tp.origin.x, y+tp.origin.y, tp.size.x, tp.size.y );
			}
		}
	}
	else
	{
		double theta = -M_PI/180*rotationDegrees;
		double sintheta = sin(theta);
		double costheta = cos(theta);
		for ( size_t i=0;i<m_lines.size();i++ )
		{
			for ( size_t j=0;j<m_lines[i].size();j++ )
			{
				text_piece &tp = m_lines[i][j];
				dc.Font( tp.state == text_piece::NORMAL ? fontPoints : fontPoints-FontPointAdjust );
				double rotx = tp.origin.x*costheta - tp.origin.y*sintheta;
				double roty = tp.origin.x*sintheta + tp.origin.y*costheta;
				dc.Text( tp.text, x + rotx, y + roty, rotationDegrees );
			}
		}
	}

	// restore font state
	dc.Font( fontPoints );
}

wxString wxPLTextLayout::Escape( const wxString &text )
{
	wxString result;
	result.Alloc( text.Length() ); 

	bool last_char_slash = false;
	wxString::const_iterator it = text.begin();
	while( it != text.end() )
	{
		if ( *it == '\\'  && !last_char_slash )
		{
			++it; // skip the slash
			wxString code;
			while ( it != text.end()
				&& (*it) != '\\'
				&& wxIsalpha( (*it) ) )
			{
				code += *it;
				++it;
			}

			last_char_slash = ( it != text.end() 
				&& code.Len() == 0 
				&& *it == '\\' );

			if ( it != text.end()  
				&& (*it == ' ' || *it == '\t' ) )
				it++; // skip next space if it exists.  assume is for delineating codes

								
			wxUniChar value( 0 );
			size_t idx = 0;
			while ( ::escape_codes[idx].value != 0 )
			{
				if ( ::escape_codes[idx].code == code )
				{
					value = ::escape_codes[idx].value;
					break;
				}
				else idx++;
			}

			if ( value != 0 ) result += value;
			else if ( !last_char_slash ) result << L'\x275a';
		}
		else
		{
			last_char_slash = false;
			result += *it;
			++it;
		}
	}

	return result;
}

std::vector<wxPLTextLayout::text_piece> wxPLTextLayout::Parse( const wxString &text )
{
	std::vector<text_piece> list;

	text_piece tp;

	wxString::const_iterator it = text.begin();
	while( it != text.end() )
	{
		if ( *it == '^' || *it == '_' )
		{
			wxUniChar modifier = *it;
			it++; // skip modifier to get the next character
			if ( it == text.end() || *it == modifier )
			{
				// if we have a double modifier,
				// simply add it as normal text
				tp.text += *it;
			}
			else 
			{
				if ( !tp.text.IsEmpty() )
				{
					tp.state = text_piece::NORMAL;
					list.push_back( tp );
					tp.text.clear();
				}
				
				tp.state = (modifier == '^') ? text_piece::SUPERSCRIPT : text_piece::SUBSCRIPT;
				if ( it != text.end() && *it == '{' )
				{
					it++; // skip the {
					while ( it != text.end() && *it != '}' )
					{
						tp.text += *it;
						it++;
					}
					
					// go until } encountered
					if ( it != text.end() && *it == '}' )
						it++;
				}
				else
				{
					while ( it != text.end() 
						&& *it != ' ' 
						&& *it != '/'
						&& *it != '\t'
						&& *it != '^'
						&& *it != '_'
						&& *it != '('
						&& *it != '{'
						&& *it != '['
						&& *it != '='
						&& *it != ','
						&& *it != ';' )
					{
						tp.text += *it;
						it++;
					}

					if ( it != text.end() && *it != ' ' )
						--it; // return the character that ended the current mode
				}

				if ( tp.text.Len() > 0 )
				{
					list.push_back( tp );
					tp.text.clear();
				}
			}
		}
		else
		{
			tp.text += *it;
		}

		if ( it != text.end() )
			++it;
	}

	// push back final text piece if needed		
	if ( !tp.text.IsEmpty() )
	{
		tp.state = text_piece::NORMAL;
		list.push_back( tp );
	}

	return list;
}





#include <wx/dc.h>
#include <wx/dcgraph.h>
#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE( wxPLTextLayoutDemo, wxWindow )
	EVT_PAINT( wxPLTextLayoutDemo::OnPaint )
	EVT_SIZE( wxPLTextLayoutDemo::OnSize )
END_EVENT_TABLE()


wxPLTextLayoutDemo::wxPLTextLayoutDemo( wxWindow *parent )
	: wxWindow( parent, wxID_ANY )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
}

std::vector<double> wxPLTextLayoutDemo::Draw( wxPLOutputDevice &dc, const wxPLRealRect &geom )
{
	std::vector<double> vlines;
	
	// test 1
	{
		dc.Font( 0 );
		wxPLTextLayout t1( dc, "basic text, nothing special" );
		t1.Render( dc, geom.x+20, geom.y+20, 0.0, true );
		vlines.push_back( geom.x+20 );
		vlines.push_back( geom.x+20+t1.Width() );
	}

	// test 2
	{
		dc.Font( 2 );
		//gc->SetFont( wxFont(18, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Century Schoolbook"), *wxBLACK );
		wxPLTextLayout t2( dc, "escape^sup_sub \\\\  \\  \\euro \\badcode \\Omega~\\Phi\\ne\\delta, but\n\\Kappa\\approx\\zeta \\emph\\qmark, and this is the end of the text!. Cost was 10 \\pound, or 13.2\\cent" );
		t2.Render( dc, geom.x+20, geom.y+120, 0.0, true );
	}
	
	// test 3
	{
		dc.Font( -1, *wxRED );
		//gc->SetFont( wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Consolas"), *wxBLACK );
		wxPLTextLayout t3( dc, "super^2 ,not_\\rho\\gamma f hing_{special,great,best}\n\\alpha^^\\beta c^\\delta  efjhijkl__mnO^25 pq_0 r\\Sigma tuvwxyz\nABCDEFGHIJKL^^MNOPQRSTUVWXZY" );
		t3.Render( dc, geom.x+20, geom.y+420, 90, true );
		t3.Render( dc, geom.x+200, geom.y+350, 45.0, true );
		t3.Render( dc, geom.x+400, geom.y+300, 0.0, true );
		vlines.push_back( geom.x+400 );
		vlines.push_back( geom.x+400+t3.Width() );
	}
	
	// test 4
	{
		dc.Font(-2, *wxBLUE );
		//gc->SetFont( wxFont(16, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), *wxBLACK );
		wxPLTextLayout t4( dc, "x_1^2_3 abc=y_2^4" );
		t4.Render( dc, geom.x+200, geom.y+70, 0, false );
	}

	// test 5
	{
		dc.Font(+3 );
		//gc->SetFont( wxFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), *wxBLACK );
		wxPLTextLayout t5( dc, "small (7): x_1^2_3 abc=y_2^4" );
		t5.Render( dc, geom.x+500, geom.y+60, 0, false );
	}
	
	// test 6
	{
		dc.Font(-3 );
		//gc->SetFont( wxFont(8, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), *wxBLACK );
		wxPLTextLayout t6( dc, "small (8): x_1^2_3 abc=y_2^4" );
		t6.Render( dc, geom.x+500, geom.y+80, 0, false );
	}
	
	// test 7
	{
		dc.Font(-2 );
		//gc->SetFont( wxFont(9, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), *wxBLACK );
		wxPLTextLayout t7( dc, "small (9): x_1^2_3 abc=y_2^4" );
		t7.Render( dc, geom.x+500, geom.y+100, 0, false );
	}
	
	// test 8
	{
		dc.Font(-1, *wxGREEN );
		//gc->SetFont( wxFont(10, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), *wxBLACK );
		//wxPLGraphicsOutputDevice dc( gc );
		wxPLTextLayout t8( dc, "small (10): x_1^2_3 abc=y_2^4" );
		t8.Render( dc, geom.x+500, geom.y+120, 0, false );
	}

	vlines.push_back( geom.x+500 );
	
	return vlines;
}

void wxPLTextLayoutDemo::OnPaint( wxPaintEvent & )
{
	wxAutoBufferedPaintDC pdc( this );
	
	int width, height;
	GetClientSize( &width, &height );

	pdc.SetBackground( *wxWHITE_BRUSH );
	pdc.Clear();

	wxGraphicsContext *gc = wxGraphicsContext::Create( pdc );
	wxPLGraphicsOutputDevice dc( gc );
	Draw( dc, wxPLRealRect( 0, 0, width, height/2 ) );
	delete gc;
}

void wxPLTextLayoutDemo::OnSize( wxSizeEvent & )
{
	Refresh();
}
