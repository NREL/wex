///////////////////////////////////////////////////////////////////////////////
// Name:        pdfdc29.h
// Purpose:
// Author:      Ulrich Telle
// Modified by:
// Created:     2010-11-28
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfdc29.h Interface of the wxPdfDC class (wxWidgets 2.9.x)

#ifndef _PDF_DC29_H_
#define _PDF_DC29_H_

/// Class representing a PDF drawing context
class WXDLLIMPEXP_PDFDOC wxPdfDC : public wxDC {
public:
    wxPdfDC();

    // Recommended constructor
    wxPdfDC(const wxPrintData &printData);

    wxPdfDC(wxPdfDocument *pdfDocument, double templateWidth, double templateHeight);

    wxPdfDocument *GetPdfDocument();

    void SetResolution(int ppi);

    int GetResolution() const;

    void SetImageType(wxBitmapType bitmapType, int quality = 75);

    void SetMapModeStyle(wxPdfMapModeStyle style);

    wxPdfMapModeStyle GetMapModeStyle() const;

private:
wxDECLARE_DYNAMIC_CLASS(wxPdfDC);
};

/// Class representing the PDF drawing context implementation
class WXDLLIMPEXP_PDFDOC wxPdfDCImpl : public wxDCImpl {
public:
    wxPdfDCImpl(wxPdfDC *owner);

    wxPdfDCImpl(wxPdfDC *owner, const wxPrintData &data);

    wxPdfDCImpl(wxPdfDC *owner, wxPdfDocument *pdfDocument, double templateWidth, double templateHeight);

    wxPdfDCImpl(wxPdfDC *owner, const wxString &file, int w = 300, int h = 200);

    virtual ~wxPdfDCImpl();

    void Init();

    wxPdfDocument *GetPdfDocument();

    void SetPrintData(const wxPrintData &data);

    wxPrintData &GetPrintData() { return m_printData; }

    void SetResolution(int ppi);

    int GetResolution() const;

    void SetImageType(wxBitmapType bitmapType, int quality = 75);

    // implement base class pure virtuals

    virtual void Clear();

    virtual bool StartDoc(const wxString &message);

    virtual void EndDoc();

    virtual void StartPage();

    virtual void EndPage();

    virtual void SetFont(const wxFont &font);

    virtual void SetPen(const wxPen &pen);

    virtual void SetBrush(const wxBrush &brush);

    virtual void SetBackground(const wxBrush &brush);

    virtual void SetBackgroundMode(int mode);

    virtual void SetPalette(const wxPalette &palette);

    virtual void DestroyClippingRegion();

    virtual wxCoord GetCharHeight() const;

    virtual wxCoord GetCharWidth() const;

    virtual bool CanDrawBitmap() const;

    virtual bool CanGetTextExtent() const;

    virtual int GetDepth() const;

    virtual wxSize GetPPI() const;

    virtual void SetMapMode(wxMappingMode mode);

    virtual void SetUserScale(double x, double y);

    virtual void SetLogicalScale(double x, double y);

    virtual void SetLogicalOrigin(wxCoord x, wxCoord y);

    virtual void SetDeviceOrigin(wxCoord x, wxCoord y);

    virtual void SetAxisOrientation(bool xLeftRight, bool yBottomUp);

    virtual void SetLogicalFunction(wxRasterOperationMode function);

    virtual void SetTextForeground(const wxColour &colour);

    virtual void ComputeScaleAndOrigin();

#if 0
    // RTL related functions
    // ---------------------

    // get or change the layout direction (LTR or RTL) for this dc,
    // wxLayout_Default is returned if layout direction is not supported
    virtual wxLayoutDirection GetLayoutDirection() const
          { return wxLayout_Default; }
    virtual void SetLayoutDirection(wxLayoutDirection WXUNUSED(dir))
         { }
#endif

protected:
    // the true implementations
    virtual bool DoFloodFill(wxCoord x, wxCoord y, const wxColour &col,
                             wxFloodFillStyle style = wxFLOOD_SURFACE);

    virtual void DoGradientFillLinear(const wxRect &rect,
                                      const wxColour &initialColour,
                                      const wxColour &destColour,
                                      wxDirection nDirection = wxEAST);

    virtual void DoGradientFillConcentric(const wxRect &rect,
                                          const wxColour &initialColour,
                                          const wxColour &destColour,
                                          const wxPoint &circleCenter);

    virtual bool DoGetPixel(wxCoord x, wxCoord y, wxColour *col) const;

    virtual void DoDrawPoint(wxCoord x, wxCoord y);

#if wxUSE_SPLINES

    virtual void DoDrawSpline(const wxPointList *points);

#endif

    virtual void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2);

    virtual void DoDrawArc(wxCoord x1, wxCoord y1,
                           wxCoord x2, wxCoord y2,
                           wxCoord xc, wxCoord yc);

    virtual void DoDrawCheckMark(wxCoord x, wxCoord y,
                                 wxCoord width, wxCoord height);

    virtual void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                                   double sa, double ea);

    virtual void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height);

    virtual void DoDrawRoundedRectangle(wxCoord x, wxCoord y,
                                        wxCoord width, wxCoord height,
                                        double radius);

    virtual void DoDrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height);

    virtual void DoCrossHair(wxCoord x, wxCoord y);

    virtual void DoDrawIcon(const wxIcon &icon, wxCoord x, wxCoord y);

    virtual void DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                              bool useMask = false);

    virtual void DoDrawText(const wxString &text, wxCoord x, wxCoord y);

    virtual void DoDrawRotatedText(const wxString &text, wxCoord x, wxCoord y,
                                   double angle);

    virtual bool DoBlit(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
                        wxDC *source, wxCoord xsrc, wxCoord ysrc,
                        wxRasterOperationMode rop = wxCOPY, bool useMask = false,
                        wxCoord xsrcMask = -1, wxCoord ysrcMask = -1);

    virtual void DoGetSize(int *width, int *height) const;

    virtual void DoGetSizeMM(int *width, int *height) const;

#if wxCHECK_VERSION(2, 9, 5)

    virtual void DoDrawLines(int n, const wxPoint points[],
                             wxCoord xoffset, wxCoord yoffset);

    virtual void DoDrawPolygon(int n, const wxPoint points[],
                               wxCoord xoffset, wxCoord yoffset,
                               wxPolygonFillMode fillStyle = wxODDEVEN_RULE);

    virtual void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                                   wxCoord xoffset, wxCoord yoffset,
                                   wxPolygonFillMode fillStyle);

#else
    virtual void DoDrawLines(int n, wxPoint points[],
                             wxCoord xoffset, wxCoord yoffset);
    virtual void DoDrawPolygon(int n, wxPoint points[],
                               wxCoord xoffset, wxCoord yoffset,
                               wxPolygonFillMode fillStyle = wxODDEVEN_RULE);
    virtual void DoDrawPolyPolygon(int n, int count[], wxPoint points[],
                                   wxCoord xoffset, wxCoord yoffset,
                                   int fillStyle);
#endif // wxCHECK_VERSION

    virtual void DoSetClippingRegionAsRegion(const wxRegion &region);

    virtual void DoSetClippingRegion(wxCoord x, wxCoord y,
                                     wxCoord width, wxCoord height);

    virtual void DoSetDeviceClippingRegion(const wxRegion &region);

    virtual void DoGetTextExtent(const wxString &string,
                                 wxCoord *x, wxCoord *y,
                                 wxCoord *descent = NULL,
                                 wxCoord *externalLeading = NULL,
                                 const wxFont *theFont = NULL) const;

    virtual bool DoGetPartialTextExtents(const wxString &text, wxArrayInt &widths) const;

public:
    int GetDrawingStyle();

    bool StretchBlt(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
                    wxBitmap *bitmap);

    int IncreaseImageCounter() { return ++m_imageCount; }

    void SetMapModeStyle(wxPdfMapModeStyle style) { m_mappingModeStyle = style; }

    wxPdfMapModeStyle GetMapModeStyle() const { return m_mappingModeStyle; }

private:
    int FindPdfFont(wxFont *font) const;

    void SetupPen();

    void SetupBrush();

    double ScaleLogicalToPdfX(wxCoord x) const;

    double ScaleLogicalToPdfXRel(wxCoord x) const;

    double ScaleLogicalToPdfY(wxCoord y) const;

    double ScaleLogicalToPdfYRel(wxCoord y) const;

    double ScaleFontSizeToPdf(int pointSize) const;

    int ScalePdfToFontMetric(double metric) const;

    void CalculateFontMetrics(wxPdfFontDescription *desc, int pointSize,
                              int *height, int *ascent, int *descent, int *extLeading) const;

    bool m_templateMode;
    double m_templateWidth;
    double m_templateHeight;
    double m_ppi;
    double m_ppiPdfFont;
    wxPdfDocument *m_pdfDocument;
    int m_imageCount;
    wxPrintData m_printData;
    wxPdfMapModeStyle m_mappingModeStyle;

    bool m_jpegFormat;
    int m_jpegQuality;

wxDECLARE_DYNAMIC_CLASS(wxPdfDCImpl);
};

#endif
