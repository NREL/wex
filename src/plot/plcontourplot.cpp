
#include "wex/plot/plcontourplot.h"
#include "wex/plot/plcolourmap.h"

#include <algorithm>

/*
#include <wex/tri/triangulation.c>

class MeanderingTriangles
{
public:	
	struct edge { 
		int n1, n2, c1, c2, bc, flip; 
		edge() { n1=n2=c1=c2=-1; bc=-1; flip=0; } 
		edge(int _n1, int _n2, int _c1, int _c2) : n1(_n1), n2(_n2), c1(_c1), c2(_c2), bc(-1), flip(0) { } 
		bool equals( const edge &e ) const { return (n1==e.n1&&n2==e.n2) || (n1==e.n2&&n2==e.n1); }				
		bool lessthan( const edge &e ) const { return (n1<e.n1) || (n1==e.n1 && n2 < e.n2); }
	};
	static bool edgeLessThan( const edge &e1, const edge &e2 ) { return e1.lessthan(e2); }
	static bool edgeEqual( const edge &e1, const edge &e2 ) { return e1.equals(e2); }
	
	// point locations and values: x,y,z
	wxMatrix<double> x, y, z;
	// Nt x 3 triangles referencing point indices
	wxMatrix<int> tri;

	// all edges after processing and triangle-triangle adjacencies
	std::vector<edge> edg;
	// Nt x 3 references to edge indices
	wxMatrix<int> trie;

	void add_tri_edge( int C, int E )
	{
		for (int i=0;i<3;i++ )
		{
			if ( trie( C, i ) == E ) return;

			if ( trie( C, i ) < 0 )
			{
				trie( C, i ) = E;
				return;
			}
		}

		assert( false ); // should never reach this point
	}

	// determine edges and adjacency
	bool determine_edges()
	{
		int i;
		int np = x.Length();
		int nt = tri.Rows();

		if ( y.Length() != np || z.Length() != np ) return false;
		if ( np <= 0 || nt <= 0 ) return false;

		// process edges from input points and triangle data
		// some notes:
		// all mesh triangles are written with nodes in CCW order
		// therefore, 'cell 1' identifier is consistent with CCW mesh assumption
		// 'cell 2' identifier for interior nodes is thus always CW relative to the nodes

		// create a list of all edges of each mesh cell (triangles only)
		edg.clear();
		edg.reserve( 3*nt );

		for( i=0;i<nt;i++ )
		{
			edg.push_back( edge( tri(i,0), tri(i,1), i, -1 ) );
			edg.push_back( edge( tri(i,1), tri(i,2), i, -1 ) );
			edg.push_back( edge( tri(i,2), tri(i,0), i, -1 ) );
		}


		// flip edge nodes for sorting order and mark flipped edges
		for ( i=0;i<edg.size();i++ )
		{
			if ( edg[i].n1 > edg[i].n2 )
			{
				int n1save = edg[i].n1;
				edg[i].n1 = edg[i].n2;
				edg[i].n2 = n1save;
				edg[i].flip = 1;
			}
		}

		// sort edges in ascending order 
		// stable is important so that the first instance of an edge added to the list
		// remains at front with it's CCW .c1 (cell 1) defined
		std::stable_sort( edg.begin(), edg.end(), edgeLessThan );

		// split into boundary and interior edges
		i=0;
		while( i<edg.size() )
		{
			if ( i+1<edg.size() && edg[i].equals(edg[i+1]) )
			{	// multiple same edge from different triangles, so an interior edge
				edg[i].c2 = edg[i+1].c1; // save cell index for other side of edge
				edg.erase( edg.begin() + i + 1 ); // erase the other equivalent edge
			}
			else
			{	 // no duplicate edge, so a boundary edge
				edg[i].bc = 1;
			}

			i++;
		}

		// undo flipped edge nodes to retain proper CCW ordering relative to cell 1 
		for( i=0;i<edg.size();i++ )
		{
			if ( edg[i].flip )
			{
				int n1save = edg[i].n1;
				edg[i].n1 = edg[i].n2;
				edg[i].n2 = n1save;
				edg[i].flip = false;
			}
		}
		
		// create mapping of triangles to their 3 edges
		trie.ResizeFill( nt, 3, -1 );
		for( i=0;i<edg.size();i++ )
		{
			if ( edg[i].c1 >= 0 ) add_tri_edge( edg[i].c1, i );
			if ( edg[i].c2 >= 0 ) add_tri_edge( edg[i].c2, i );
		}

		return true;
	}
	
	void interpz( int n1, int n2, double zval, wxRealPoint *pt )
	{
		double z1 = z[n1];
		double z2 = z[n2];

		double f = 0.5;
		if ( z2 - z1 != 0.0 )
			f = ( zval - z1 ) / ( z2 - z1 );

		pt->x = x[n1] + f*(x[n2]-x[n1]);
		pt->y = y[n1] + f*(y[n2]-y[n1]);
	}

	void follow( double iso, int istart, int dir, std::vector<bool> &visited, wxPLContourPlot::Contour *contour )
	{
		int i = istart;
		int last = -2; // to not be confused with a boundary edge

		while ( !visited[i] )
		{
			wxRealPoint pt;
			interpz( edg[i].n1, edg[i].n2, iso, &pt );
			contour->points.push_back( pt );
			visited[i] = true;

			// pick next cell(triangle) to follow
			int nextcell = dir == 1 ? edg[i].c1 : edg[i].c2;

			if ( last == nextcell )
				nextcell = dir == 1 ? edg[i].c2 : edg[i].c1;

			// contour ends
			if ( nextcell < 0 )
				break;

			int iedg_next = -1;
			// find the two opposite edges of the next cell
			for( int j=0;j<3;j++ )
			{
				int e = trie(nextcell,j);
				if ( e != i )
				{
					double z1 = z[edg[e].n1];
					double z2 = z[edg[e].n2];

					if ( z1 < iso && z2 < iso
						|| z1 > iso && z2 > iso )
						continue; // iso does not cross this edge

					iedg_next = e;
					break;
				}
			}

			// contour ends here...
			if ( iedg_next < 0 )
				break;

			i = iedg_next;
			last = nextcell;
		}
	}

	void process_iso( double iso, std::vector<wxPLContourPlot::Contour> *contours )
	{
		std::vector<bool> visited( edg.size(), false );

		// visit all the edges once for this iso level.
		for( size_t i=0;i<edg.size();i++ )
		{
			if ( visited[i] ) continue; // don't visit cells twice		
			
			double z1 = z[edg[i].n1];
			double z2 = z[edg[i].n2];
			
			// if ends above or below, no contour sits across this edge
			if ( z1 < iso && z2 < iso
				|| z1 > iso && z2 > iso )
			{
				visited[i] = true;
				continue;
			}	

			// otherwise follow a potential contour in both directions for this 
			// starting edge using the cell adjacency table, updating
			// which cells have been visited
			contours->push_back( wxPLContourPlot::Contour() );
			wxPLContourPlot::Contour &C = contours->back();
			C.level = iso;
			follow( iso, i,  1, visited, &C );
			follow( iso, i, -1, visited, &C );
		}
	}
	

	bool load( const wxMatrix<double> &gridded_data )
	{
		size_t nr = gridded_data.Rows();
		size_t nc = gridded_data.Cols();

		size_t np = nr * nc;
		if ( np < 4 ) return false;

		x.Resize( np );
		y.Resize( np );
		z.Resize( np );

		size_t nt = 2 * ( nr-1 ) * ( nc-1 );
		if ( nt < 2 ) return false;

		tri.Resize( nt, 3 );

		// store all the points
		size_t ip = 0; 
		for( size_t i=0;i<nr;i++ )
		{
			for( size_t j=0;j<nc;j++ )
			{
				x[ip] = j;
				y[ip] = i;
				z[ip] = gridded_data(i,j);
				ip++;
			}
		}


		// store all the triangles
		size_t it = 0;
		for( size_t i=0;i<nr-1;i++ )
		{
			for( size_t j=0;j<nc-1;j++ )
			{
				// point indices for four corners of current grid cell
				size_t p1 = (i  )*nc+(j  );
				size_t p2 = (i  )*nc+(j+1);
				size_t p3 = (i+1)*nc+(j  );
				size_t p4 = (i+1)*nc+(j+1);

				tri(it,0) = p2;
				tri(it,1) = p3;
				tri(it,2) = p1;
				it++;
				
				tri(it,0) = p3;
				tri(it,1) = p2;
				tri(it,2) = p4;
				it++;

			}
		}
		
		// find all the edges
		return determine_edges();
	}

	bool load( double *xx, double *yy, double *zz, size_t len )
	{
		double *xy = new double[ 2*len ];
		if ( !xy ) return false;

		double *pxy = xy;
		x.Resize(len);
		y.Resize(len);
		z.Resize(len);
		for( size_t i=0;i<len;i++ )
		{
			*pxy++ = x[i] = xx[i];
			*pxy++ = y[i] = yy[i];
			z[i] = zz[i];
		}

		// now need to do a delaunay triangulation to
		// form a 2D mesh upon which the contours can be calculated
		
		int *nodes = new int[ 2*len*3 ];
		if ( !nodes ) { delete [] xy; return false; }

		int *neighbors = new int[ 2*len*3 ];
		if ( !neighbors ) { delete [] xy; delete [] nodes; return false; }

		int ntri = 0;
		int err = r8tris2( len, xy, &ntri, nodes, neighbors );

		if ( 0 == err )
		{
			tri.Resize( ntri, 3 );
			for( int i=0;i<ntri;i++ )
			{
				tri(i,0) = nodes[i*3+0];
				tri(i,1) = nodes[i*3+1];
				tri(i,2) = nodes[i*3+2];
			}
		}


		delete [] xy;
		delete [] nodes;
		delete [] neighbors;

		if ( 0 != err ) return false;

		return determine_edges();
	}

};
*/

class MarchingSquares
{
public:
	static enum Side { LEFT, RIGHT, TOP, BOTTOM, NONE, ERROR };

	class Cell
	{
	public:
		unsigned char id;
		bool flipped;
		double left, top, right, bottom;
	public:

		Cell()
		{
			id = 0;
			flipped = false;
			left = top = right = bottom = 0;
		}

		Cell( unsigned char idx, bool flip, double L, double T, double R, double B)
		{
			id = (unsigned char) idx;
			if (flipped && id != 5 && id != 10) {
				flipped = false;
			} else {
				flipped = flip;
			}
			left = L;
			top = T;
			right = R;
			bottom = B;
		}

		Cell( const Cell & rhs )
		{
			id = rhs.id;
			left = rhs.left;
			right = rhs.right;
			top = rhs.top;
			bottom = rhs.bottom;
			flipped = rhs.flipped;
		}
	
		void Clear()
		{
			if ( id == 5 || id == 10 )
				return;

			id = 15; // marker for trivial cell
		}


		void Coord( Side edge, double *X, double *Y ) {
			switch (edge)
			{
			case BOTTOM: *X = bottom; *Y = 0.0; return;
			case LEFT: *X = 0.0; *Y = left; return;
			case RIGHT: *X = 1.0; *Y = right; return;
			case TOP: *X = top; *Y = 1.0; return;
			default:
				*X = *Y = std::numeric_limits<double>::quiet_NaN();
			}
		}

		unsigned char Id() const { return id; }
		bool Saddle() { return id == 5 || id == 10; }
		bool Trivial() { return id == 0 || id == 15; }
		bool Flipped() { return flipped; }
	};

   MarchingSquares()
   {
	   // nothing to do!
   }

   	void Generate( wxMatrix<Cell> &grid, double iso, std::vector<wxPLContourPlot::Contour> *contours ) {
		for (size_t r = 0; r < grid.Rows(); r++) {
			for (size_t c = 0; c < grid.Cols(); c++) {
				// find a start node...
				Cell &cell = grid(r, c);
				if ( !cell.Trivial() && !cell.Saddle() )
				{
					contours->push_back( wxPLContourPlot::Contour() );
					wxPLContourPlot::Contour &CC = contours->back();
					Follow(grid, r, c, CC );
					CC.level = iso;
				}
			}
		}
	}

/**
* <p>Return the first side that should be used in a CCW traversal.</p>
*
* @param cell the Cell to process.
* @param prev previous side, only used for saddle cells.
* @return the 1st side of the line segment of the designated cell.
*/
	Side FirstSide( Cell &cell, Side prev ) {
		switch (cell.Id())
		{
		case 1:
		case 3:
		case 7:
			return LEFT;
		case 2:
		case 6:
		case 14:
			return BOTTOM;
		case 4:
		case 11:
		case 12:
		case 13:
			return RIGHT;
		case 8:
		case 9:
			return TOP;
		case 5:
			switch (prev) {
			case LEFT: return RIGHT;
			case RIGHT: return LEFT;
			default:
				return ERROR;
			}

		case 10:
			switch (prev) {
			case BOTTOM: return TOP;
			case TOP: return BOTTOM;
			default:
				return ERROR;
			}

		default:
			return ERROR;
		}
	}

/**
* <p>Return the second side that should be used in a CCW traversal.</p>
*
* @param cell the Cell to process.
* @param prev previous side, only used for saddle cells.
* @return the 2nd side of the line segment of the designated cell.
*/
	Side SecondSide( Cell &cell, Side prev) {
		switch (cell.Id()) {
		case 8:
		case 12:
		case 14:
			return LEFT;
		case 1:
		case 9:
		case 13:
			return BOTTOM;
		case 2:
		case 3:
		case 11:
			return RIGHT;
		case 4:
		case 6:
		case 7:
			return TOP;
		case 5:

			switch (prev) {
			case LEFT: return cell.Flipped() ? BOTTOM : TOP;
			case RIGHT: return cell.Flipped() ? TOP : BOTTOM;
			default:
				return ERROR;
			}

		case 10:
			switch (prev) {
			case BOTTOM: return cell.Flipped() ? RIGHT : LEFT;
			case TOP: return cell.Flipped() ? LEFT : RIGHT;
			default:
				return ERROR;
			}

		default:
			return ERROR;
	}
}

/**
* <p>A given contour can be made up of multiple disconnected regions, each
* potentially having multiple holes. Both regions and holes are captured as
* individual sub-paths.</p>
*
* <p>The process is iterative. It starts w/ an empty GeneralPath instance
* and continues until all Cells are processed. With every invocation the
* GeneralPath object is updated to reflect the new sub-path(s).</p>
*
* <p>Once a non-saddle cell is used it is cleared so as to ensure it will
* not be re-used when finding sub-paths w/in the original path.</p>
*
* @param grid on input the matrix of cells representing a given contour.
* Note that the process will alter the Cells, so on output the original
* Grid instance _will_ be modified. In other words this method is NOT
* idempotent when using the same object references and values.
* @param r row index of the start Cell.
* @param c column index of the start Cell.
* @param path a non-null GeneralPath instance to update.
*/
	void Follow( wxMatrix<Cell> &grid, int r, int c, wxPLContourPlot::Contour & path) {
		Side prevSide = NONE;

		Cell *start = &grid( r, c );		
		Cell *currentCell = &grid(r, c);
		do
		{
			prevSide = SecondSide(*currentCell, prevSide);
			if ( prevSide == ERROR )
				return; // end the path without closing it

			double x, y;
			currentCell->Coord(prevSide, &x, &y );
			x += c;
			y += r;

			path.points.push_back( wxRealPoint(x,y) );
			
			currentCell->Clear();
			
			switch (prevSide) {
			case BOTTOM: r--; break;
			case LEFT: c--; break;
			case RIGHT: c++; break;
			case TOP: r++; break;
			case ERROR: return; // end the path without closing it
			default:
				break;
			}

			if ( r >= grid.Rows() || c >= grid.Cols() )
				return; // end the path without closing it

			currentCell = &grid(r, c);
		}
		while ( start != currentCell && r < grid.Rows() && c < grid.Cols() );
		
		// close the loop
		path.points.push_back( path.points[0] );
	}

   bool Contour1( const wxMatrix<double> &data, double isovalue, wxMatrix<Cell> &cells ) {
		size_t rowCount = data.Rows();
		size_t colCount = data.Cols();

		if( rowCount < 2 || colCount < 2 ) return false;

		// Every 2x2 block of pixels in the binary image forms a contouring cell,
		// so the whole image is represented by a grid of such cells. Note that
		// this contouring grid is one cell smaller in each direction than the
		// original 2D field.
		cells.ResizeFill( rowCount-1, colCount-1, Cell() );

      for (size_t r = 0; r < rowCount - 1; r++) {
         for (size_t c = 0; c < colCount - 1; c++) {
            // Compose the 4 bits at the corners of the cell to build a binary
            // index: walk around the cell in a clockwise direction appending
            // the bit to the index, using bitwise OR and left-shift, from most
            // significant bit at the top left, to least significant bit at the
            // bottom left.  The resulting 4-bit index can have 16 possible
            // values in the range 0-15.
            unsigned char id = 0;
            double tl = data(r + 1,c    );
            double tr = data(r + 1,c + 1);
            double br = data(r    ,c + 1);
            double bl = data(r    ,c    );
            id |= (tl < isovalue ? 0 : 8);
            id |= (tr < isovalue ? 0 : 4);
            id |= (br < isovalue ? 0 : 2);
            id |= (bl < isovalue ? 0 : 1);
            bool flipped = false;
            if (id == 5 || id == 10) {
               // resolve the ambiguity by using the average data value for the
               // center of the cell to choose between different connections of
               // the interpolated points.
               double center = (tl + tr + br + bl) / 4;
               if (id == 5 && center < isovalue) {
                  flipped = true;
               } else if (id == 10 && center < isovalue) {
                  flipped = true;
               }
            }
            // NOTE (rsn) - we only populate the grid w/ non-trivial cells;
            // i.e. those w/ an index different than 0 and 15.
            if (id != 0 && id != 15) {
               // Apply linear interpolation between the original field data
               // values to find the exact position of the contour line along
               // the edges of the cell.
               double left = 0.5F;
               double top = 0.5F;
               double right = 0.5F;
               double bottom = 0.5F;
               switch (id) {
               case 1:
                  left = (double)((isovalue - bl) / (tl - bl));
                  bottom = (double)((isovalue - bl) / (br - bl));
                  break;
               case 2:
                  bottom = (double)((isovalue - bl) / (br - bl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 3:
                  left = (double)((isovalue - bl) / (tl - bl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 4:
                  top = (double)((isovalue - tl) / (tr - tl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 5:
                  left = (double)((isovalue - bl) / (tl - bl));
                  bottom = (double)((isovalue - bl) / (br - bl));
                  top = (double)((isovalue - tl) / (tr - tl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 6:
                  bottom = (double)((isovalue - bl) / (br - bl));
                  top = (double)((isovalue - tl) / (tr - tl));
                  break;
               case 7:
                  left = (double)((isovalue - bl) / (tl - bl));
                  top = (double)((isovalue - tl) / (tr - tl));
                  break;
               case 8:
                  left = (double)((isovalue - bl) / (tl - bl));
                  top = (double)((isovalue - tl) / (tr - tl));
                  break;
               case 9:
                  bottom = (double)((isovalue - bl) / (br - bl));
                  top = (double)((isovalue - tl) / (tr - tl));
                  break;
               case 10:
                  left = (double)((isovalue - bl) / (tl - bl));
                  bottom = (double)((isovalue - bl) / (br - bl));
                  top = (double)((isovalue - tl) / (tr - tl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 11:
                  top = (double)((isovalue - tl) / (tr - tl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 12:
                  left = (double)((isovalue - bl) / (tl - bl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 13:
                  bottom = (double)((isovalue - bl) / (br - bl));
                  right = (double)((isovalue - br) / (tr - br));
                  break;
               case 14:
                  left = (double)((isovalue - bl) / (tl - bl));
                  bottom = (double)((isovalue - bl) / (br - bl));
                  break;
               default: // shouldn't happen
				   return false;
               }

               cells(r,c) = Cell(id, flipped, left, top, right, bottom);
            }
         }
      }
	  return true;
   }

	void Contours( std::vector<wxPLContourPlot::Contour> *cntr,
		const wxMatrix<double> &z, int levels )
	{
		cntr->clear();
		if ( z.Cells() == 0 ) return;

		double min, max, here;
		min = max = z(0,0);
		for( size_t i=0;i<z.Rows();i++ )
		{
			for( size_t j=0;j<z.Cols();j++ )
			{
				here = z(i,j);
				if ( here < min ) min = here;
				if ( here > max ) max = here;
			}
		}
	
		for( int i=0;i<=levels;i++ )
			ContourAtIso( z, min + ((double)i)/((double)levels)*(max-min), cntr );
	}

	void ContourAtIso( const wxMatrix<double> &z, double iso, std::vector<wxPLContourPlot::Contour> *cntr )
	{
		wxMatrix<Cell> cells;
		if( Contour1( z, iso, cells ) )
			Generate( cells, iso, cntr );
	}

};

wxPLContourPlot::wxPLContourPlot()
	: wxPLPlottable()
{
	m_cmap = 0;
}

wxPLContourPlot::wxPLContourPlot( const wxMatrix<double> &z,
	const wxString &label, int levels, wxPLColourMap *cmap )
	: wxPLPlottable( label ), m_z(z)
{
	if ( z.Cells() > 0 )
		Update( levels );
	
	m_cmap = cmap;
}

wxPLContourPlot::~wxPLContourPlot()
{
	// nothing to do
}

void wxPLContourPlot::Update( int levels )
{
	m_contours.clear();
	MarchingSquares ms;
	ms.Contours( &m_contours, m_z, levels );
}

void wxPLContourPlot::SetColourMap( wxPLColourMap *cmap )
{
	m_cmap = cmap;
}

void wxPLContourPlot::SetLevels( int levels )
{
	Update( levels );
}

wxRealPoint wxPLContourPlot::At( size_t i ) const
{
	double f = ((double)i)/((double)m_z.Cells());
	return wxRealPoint( f*m_z.Cols(), f*m_z.Rows() );
}

size_t wxPLContourPlot::Len() const
{
	return m_z.Cells();
}

static void draw_tri_mesh( wxPLOutputDevice &dc, const wxPLDeviceMapping &map, 
	const wxMatrix<double> &x, const wxMatrix<double> &y, const wxMatrix<int> &tri )
{
	dc.Pen( wxColour(240,240,240), 0.75 );
	dc.NoBrush();
	
	for( size_t i=0;i<tri.Rows();i++ )
	{
		wxRealPoint pt[3];
		double solval = 0;
		for (int k=0;k<3;k++ )
		{
			int idx = tri(i,k);
			pt[k] = map.ToDevice( x[idx], y[idx] );
		}
		dc.Polygon( 3, pt );
	}
}
	
void wxPLContourPlot::Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map )
{
	if ( !m_cmap ) return;

	bool m_filled = false;
	if ( m_filled ) dc.NoPen();
	else dc.NoBrush();
	/*
	MeanderingTriangles mt;
	if ( !mt.load( m_z ) )
	{
		dc.Text( "error loading triangulation", wxRealPoint(10,10) );
		return;
	}

	m_contours.clear();
	if ( m_cmap )
	{
		double min = m_cmap->GetScaleMin();
		double max = m_cmap->GetScaleMax();

		int levels = 20;
		for( int i=0;i<=levels;i++ )
			mt.process_iso( min + ((double)i)/((double)levels) * (max-min), &m_contours );

	}

	draw_tri_mesh( dc, map, mt.x, mt.y, mt.tri );
*/
	for( size_t i=0;i<m_contours.size();i++ )
	{
		if ( m_filled ) dc.Brush( m_cmap->ColourForValue( m_contours[i].level ) );
		else dc.Pen( m_cmap->ColourForValue( m_contours[i].level ), 2 );

		size_t n = m_contours[i].points.size();
		std::vector<wxRealPoint> mapped( n );
		for( size_t j=0;j<n;j++ )
			mapped[j] = map.ToDevice( m_contours[i].points[j] );

		if ( m_filled ) dc.Polygon( m_contours[i].points.size(), &mapped[0] );
		else dc.Lines( m_contours[i].points.size(), &mapped[0] ); 
	}
}

void wxPLContourPlot::DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct)
{
	// currently nothing to show in legend...? 
}
