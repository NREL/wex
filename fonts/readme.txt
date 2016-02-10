These are some Computer-Modern (LaTeX) fonts version 0.6.3 
from http://cm-unicode.sourceforge.net/

They have been converted for use with wxPdfDocument using the makefont.exe utility:

  makefont.exe -i -f cmuntt.otf -e iso-8859-1

example usage in code:

	wxPdfDocument doc;

	if ( !doc.AddFont( "Computer Modern", "", "cmunrm.xml" ) )
		wxMessageBox( "Could not load computer-modern font\n\ncwd: " + wxGetCwd() );
	
	if ( !doc.SetFont( "Computer Modern", wxPDF_FONTSTYLE_REGULAR, 12.0 ) )
		wxMessageBox( "Could not set computer-modern font" );

	doc.Text( 10, 10, "Text output with computer modern font!!" );

Note:  the font files need to be in the 'fonts/' subfolder of the 
current working directory of the application, or in folder 
defined by the WXPDF_FONTPATH environment variable if it is defined.