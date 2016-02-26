#ifndef __pl_plot_h
#define __pl_plot_h

#include <vector>

#include <wx/string.h>
#include <wx/stream.h>

#include "wex/plot/plaxis.h"
#include "wex/plot/ploutdev.h"

class wxPLPlot;

class wxPLDeviceMapping
{
public:
	wxPLDeviceMapping() { }
	virtual ~wxPLDeviceMapping() { }
	virtual wxRealPoint ToDevice( double x, double y ) const = 0;
	virtual void GetDeviceExtents( wxRealPoint *pos, wxRealPoint *size ) const = 0;
	virtual wxRealPoint GetWorldMinimum() const = 0;
	virtual wxRealPoint GetWorldMaximum() const = 0;
	virtual wxPLAxis* GetXAxis() const = 0;
	virtual wxPLAxis* GetYAxis() const = 0;
	virtual bool IsPrimaryXAxis() const = 0; // true if X_BOTTOM
	virtual bool IsPrimaryYAxis() const = 0; // true if Y_LEFT
	
	inline wxRealPoint ToDevice( const wxRealPoint &p ) const { return ToDevice(p.x, p.y); }
};

class wxPLPlottable
{
protected:
	wxString m_label;
	wxString m_xLabel;
	wxString m_yLabel;
	bool m_showInLegend;
	bool m_antiAliasing;

public:
	wxPLPlottable() : m_label(wxEmptyString), m_showInLegend(true), m_antiAliasing(true) {  }
	wxPLPlottable( const wxString &label ) : m_label(label), m_showInLegend(true), m_antiAliasing(true) {  }
	virtual ~wxPLPlottable() {  }

	// pure virtuals

	virtual wxRealPoint At( size_t i ) const = 0;
	virtual size_t Len() const = 0;
	virtual void Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map ) = 0;
	virtual void DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct ) = 0;

	// properties

	virtual wxString GetLabel() const { return m_label; }
	virtual wxString GetXDataLabel( wxPLPlot *plot=0 ) const;
	virtual wxString GetYDataLabel( wxPLPlot *plot=0 ) const;

	virtual void SetLabel( const wxString &label ) { m_label = label; }
	virtual void SetXDataLabel( const wxString &label ) { m_xLabel = label; }
	virtual void SetYDataLabel( const wxString &label ) { m_yLabel = label; }
	virtual void ShowInLegend( bool b ) { m_showInLegend = b; }
	virtual bool IsShownInLegend() { return m_showInLegend; }

	// display

	void SetAntiAliasing( bool aa ) { m_antiAliasing = aa; }
	bool GetAntiAliasing() { return m_antiAliasing; }


	// helpers

	virtual wxPLAxis *SuggestXAxis() const;
	virtual wxPLAxis *SuggestYAxis() const;

	virtual bool GetMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax) const;
	virtual bool ExtendMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax, bool extendToNice = false) const;
	virtual std::vector<wxString> GetExportableDatasetHeaders( wxUniChar sep, wxPLPlot *plot=0 ) const;
	virtual std::vector<wxRealPoint> GetExportableDataset(double Xmin, double Xmax, bool visible_only) const;
};

class wxPLSideWidgetBase
{
public:
	wxPLSideWidgetBase();
	virtual ~wxPLSideWidgetBase();

	virtual void Render( wxPLOutputDevice &, const wxPLRealRect & ) = 0;
	virtual wxRealPoint CalculateBestSize() = 0;

	void InvalidateBestSize();
	wxRealPoint GetBestSize();

private:
	wxRealPoint m_bestSize;
};

class wxPLPlot
{
public:
	wxPLPlot();
	virtual ~wxPLPlot();
	
	enum AxisPos { X_BOTTOM, X_TOP, Y_LEFT, Y_RIGHT };
	enum PlotPos { PLOT_TOP, PLOT_BOTTOM, NPLOTPOS };
	enum LegendPos { FLOATING, NORTHWEST, SOUTHWEST, NORTHEAST, SOUTHEAST, NORTH, SOUTH, EAST, WEST, BOTTOM, RIGHT  };

	void AddPlot( wxPLPlottable *p, AxisPos xap = X_BOTTOM, AxisPos yap = Y_LEFT, PlotPos ppos = PLOT_TOP, bool update_axes = true );
	wxPLPlottable *RemovePlot(wxPLPlottable *p, PlotPos plotPosition = NPLOTPOS);
	bool ContainsPlot(wxPLPlottable *p, PlotPos plotPosition = NPLOTPOS);
	void DeleteAllPlots();
	size_t GetPlotCount();
	wxPLPlottable *GetPlot( size_t i );
	wxPLPlottable *GetPlotByLabel( const wxString &series );
	bool GetPlotPosition( const wxPLPlottable *p, 
		AxisPos *xap, AxisPos *yap, PlotPos *ppos );

	wxPLAxis *GetXAxis1() { return m_x1.axis; }
	wxPLAxis &X1() { return Axis(X_BOTTOM); }
	void SetXAxis1( wxPLAxis *a ) { m_x1.set( a ); }
	
	wxPLAxis *GetXAxis2() { return m_x2.axis; }
	wxPLAxis &X2() { return Axis(X_TOP); }
	void SetXAxis2( wxPLAxis *a ) { return m_x2.set( a ); }
	
	wxPLAxis *GetYAxis1( PlotPos ppos = PLOT_TOP ) { return m_y1[ppos].axis; }
	wxPLAxis &Y1( PlotPos ppos = PLOT_TOP ) { return Axis(Y_LEFT,ppos); }
	void SetYAxis1( wxPLAxis *a, PlotPos ppos = PLOT_TOP ) { m_y1[ppos].set( a ); }
	
	wxPLAxis *GetYAxis2( PlotPos ppos = PLOT_TOP ) { return m_y2[ppos].axis; }
	wxPLAxis &Y2( PlotPos ppos = PLOT_TOP ) { return Axis(Y_RIGHT,ppos); }
	void SetYAxis2( wxPLAxis *a, PlotPos ppos = PLOT_TOP ) { m_y2[ppos].set( a ); }

	wxPLAxis *GetAxis( AxisPos axispos, PlotPos ppos = PLOT_TOP );
	wxPLAxis &Axis( AxisPos axispos, PlotPos ppos = PLOT_TOP );
	void SetAxis( wxPLAxis *a, AxisPos axispos, PlotPos ppos = PLOT_TOP );
	
	
	void ShowGrid( bool coarse, bool fine ) { m_showCoarseGrid = coarse; m_showFineGrid = fine; }
	void ShowCoarseGrid( bool coarse ) { m_showCoarseGrid = coarse; }
	void ShowFineGrid( bool fine ) { m_showFineGrid = fine; }
	void ShowTitle( bool show ) { m_showTitle = show; }
	void SetTitle( const wxString &title );
	wxString GetTitle() { return m_title; }

	void SetGridColour( const wxColour &col ) { m_gridColour = col; }
	void SetPlotAreaColour( const wxColour &col ) { m_plotAreaColour = col; }
	void SetAxisColour( const wxColour &col ) { m_axisColour = col; }
	void SetTickTextColour( const wxColour &col ) { m_tickTextColour = col; }
	
	void ShowLegend( bool show ) { m_showLegend = show; }
	void SetLegendReversed( bool reverse ) { m_reverseLegend = reverse; }
	void SetLegendLocation( LegendPos pos, double xpercent = -1.0, double ypercent = -1.0 );
	bool SetLegendLocation( const wxString &spos );
	wxRealPoint GetLegendLocation() { return m_legendPosPercent; }
		
	void SetSideWidget( wxPLSideWidgetBase *sw, AxisPos pos = Y_RIGHT );
	wxPLSideWidgetBase *GetSideWidget( AxisPos pos );
	wxPLSideWidgetBase *ReleaseSideWidget( AxisPos pos );

	void WriteDataAsText( wxUniChar sep, wxOutputStream &os, 
		bool visible_only = true, bool include_x = true );
		
	void UpdateAxes( bool recalculate_all = false );
	void RescaleAxes();
	void DeleteAxes();

	void Invalidate(); // erases all cached positions and layouts, but does not issue refresh
	void Render( wxPLOutputDevice &dc, wxPLRealRect geom ); // note: does not draw the background.  DC should be cleared with desired bg color already


	static bool AddPdfFontDir( const wxString &path );
	static wxString LocatePdfFontInfoXml( const wxString &face );
	static wxArrayString ListAvailablePdfFonts();
	static bool SetPdfDefaultFont( const wxString &face, double points );
	bool RenderPdf( const wxString &file, double width, double height );

	class text_layout;
	class axis_layout;
protected:

	void DrawGrid( wxPLOutputDevice &dc, wxPLAxis::TickData::TickSize size );
	void DrawPolarGrid(wxPLOutputDevice &dc, wxPLAxis::TickData::TickSize size);
	void DrawLegend(wxPLOutputDevice &gdc, const wxPLRealRect &geom);

	void UpdateHighlightRegion();
	void DrawLegendOutline();


	bool m_showLegend;
	bool m_showCoarseGrid;
	bool m_showFineGrid;
	bool m_showTitle;
	wxString m_title;
	wxColour m_gridColour;
	wxColour m_axisColour;
	wxColour m_tickTextColour;
	wxColour m_plotAreaColour;
	wxPLRealRect m_legendRect;
	LegendPos m_legendPos;
	wxRealPoint m_legendPosPercent;
	bool m_reverseLegend;
	bool m_moveLegendMode;
	bool m_moveLegendErase;
	wxPoint m_anchorPoint;
	wxPoint m_currentPoint;

	std::vector< wxPLRealRect > m_plotRects;
	wxPLSideWidgetBase *m_sideWidgets[4];

	struct plot_data
	{
		plot_data() : plot(0), ppos(PLOT_TOP), xap(X_BOTTOM), yap(Y_LEFT) {  }
		wxPLPlottable *plot;
		PlotPos ppos;
		AxisPos xap;
		AxisPos yap;
	};

	std::vector<plot_data> m_plots;


	struct legend_item
	{
		legend_item( wxPLOutputDevice &dc, wxPLPlottable *p );
		~legend_item();
		wxPLPlottable *plot;
		wxString text;
		text_layout *layout;
	};

	bool m_legendInvalidated;
	std::vector< legend_item* > m_legendItems;
	void CalcLegendTextLayout( wxPLOutputDevice &dc );

	struct axis_data
	{
		axis_data();
		~axis_data();
		void set( wxPLAxis *a );
		void invalidate();

		wxPLAxis *axis;
		axis_layout *layout;
		text_layout *label;
	};
	
	text_layout *m_titleLayout;
	axis_data m_x1, m_x2, m_y1[NPLOTPOS], m_y2[NPLOTPOS]; // m_x1 used for angular axis on polar plots, m_y1[0] used for radial axis
};


#ifdef TEXTLAYOUT_DEMO
#include <wx/window.h>
class TextLayoutDemo : public wxWindow
{
public:
	TextLayoutDemo( wxWindow *parent );

	std::vector<double> Draw( wxDC &dc, const wxRect &geom );
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );

	DECLARE_EVENT_TABLE();
};
#endif

#endif

