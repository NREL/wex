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

#ifndef __pl_contourplot_h
#define __pl_contourplot_h

#include "wex/matrix.h"
#include "wex/plot/plplot.h"

class wxPLColourMap;

class wxPLContourPlot : public wxPLPlottable
{
public:
	wxPLContourPlot();
	wxPLContourPlot(
		const wxMatrix<double> &x,
		const wxMatrix<double> &y,
		const wxMatrix<double> &z,
		bool filled,
		const wxString &label = wxEmptyString,
		int levels = 10,
		wxPLColourMap *cmap = 0);

	virtual ~wxPLContourPlot();

	static void MinMax(const std::vector<double> &v,
		double *minval, double *maxval);
	static void MinMax(const wxMatrix<double> &v,
		double *minval, double *maxval);

	static bool MeshGrid(
		double xmin, double xmax, size_t nx,
		double ymin, double ymax, size_t ny,
		wxMatrix<double> &xmesh,
		wxMatrix<double> &ymesh);

	static bool GridData(
		const std::vector<double> &x,
		const std::vector<double> &y,
		const std::vector<double> &z,
		const wxMatrix<double> &xq,
		const wxMatrix<double> &yq,
		wxMatrix<double> &zinterp);

	static void Peaks(size_t n,
		wxMatrix<double> &xx, wxMatrix<double> &yy, wxMatrix<double> &zz,
		double *min, double *max);

	void SetLevels(int levels, double min = 0, double max = 0);
	void SetLevels(const std::vector<double> &lev);
	void SetColourMap(wxPLColourMap *cmap); // does not take ownership of colour map

	virtual wxRealPoint At(size_t i) const;
	virtual size_t Len() const;

	virtual void Draw(wxPLOutputDevice &dc, const wxPLDeviceMapping &map);
	virtual void DrawInLegend(wxPLOutputDevice &dc, const wxPLRealRect &rct);

protected:
	wxMatrix<double> m_x, m_y, m_z;
	wxMatrix<unsigned int> m_mask;
	double m_zMin, m_zMax;
	wxPLColourMap *m_cmap;
	bool m_filled;
	std::vector<double> m_levels;

	void RebuildMask();
	void RebuildLevels(int n, double min = 0, double max = 0);
	void RebuildContours();

	struct C_poly {
		std::vector< wxRealPoint > pts;
		std::vector< unsigned char > act;
		double z, zmax;
	};

	std::vector<C_poly> m_cPolys;
};

#endif
