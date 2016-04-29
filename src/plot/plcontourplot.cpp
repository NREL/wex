
#include "wex/plot/plcontourplot.h"
#include "wex/plot/plcolourmap.h"

#include <algorithm>

//#include <wex/tri/triangulation.c>
/*

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

/*
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


//* <p>Return the first side that should be used in a CCW traversal.</p>
//*
//* @param cell the Cell to process.
//* @param prev previous side, only used for saddle cells.
//* @return the 1st side of the line segment of the designated cell.
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


//* <p>Return the second side that should be used in a CCW traversal.</p>
//*
//* @param cell the Cell to process.
//* @param prev previous side, only used for saddle cells.
//* @return the 2nd side of the line segment of the designated cell.
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


//* <p>A given contour can be made up of multiple disconnected regions, each
//* potentially having multiple holes. Both regions and holes are captured as
//* individual sub-paths.</p>
//*
//* <p>The process is iterative. It starts w/ an empty GeneralPath instance
//* and continues until all Cells are processed. With every invocation the
//* GeneralPath object is updated to reflect the new sub-path(s).</p>
//*
//* <p>Once a non-saddle cell is used it is cleared so as to ensure it will
//* not be re-used when finding sub-paths w/in the original path.</p>
//*
//* @param grid on input the matrix of cells representing a given contour.
//* Note that the process will alter the Cells, so on output the original
//* Grid instance _will_ be modified. In other words this method is NOT
//* idempotent when using the same object references and values.
//* @param r row index of the start Cell.
//* @param c column index of the start Cell.
//* @param path a non-null GeneralPath instance to update.

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
*/

wxPLContourPlot::wxPLContourPlot()
	: wxPLPlottable()
{
	m_cmap = 0;
}

wxPLContourPlot::wxPLContourPlot( 
	const wxMatrix<double> &x,
	const wxMatrix<double> &y,
	const wxMatrix<double> &z,
	bool filled,
	const wxString &label, double lmin, double lmax, int levels, wxPLColourMap *cmap )
	: wxPLPlottable( label ), m_x(x), m_y(y), m_z(z), m_levelMin(lmin), m_levelMax(lmax), m_filled(filled)
{
	if ( z.Cells() > 0 )
		Update( levels );
	
	m_cmap = cmap;
}

wxPLContourPlot::~wxPLContourPlot()
{
	// nothing to do
}

#include "mplcontour.h"

void wxPLContourPlot::Update( int levels )
{	
	if ( m_z.Cells() == 0 ) return;

	if ( m_x.Rows() != m_y.Rows() 
		|| m_y.Rows() != m_z.Rows() ) return;
	
	if ( m_x.Cols() != m_y.Cols() 
		|| m_y.Cols() != m_z.Cols() ) return;

	m_cPolys.clear();

	size_t ny = m_z.Rows();
	size_t nx = m_z.Cols();
	double min=1e99, max=-1e99, here;
	
	wxMatrix<unsigned int> mask; // no mask currently
	for( size_t i=0;i<ny;i++ )
	{
		for( size_t j=0;j<nx;j++ )
		{
			here = m_z(i,j);
			
			// create the mask if needed (i.e. if NaNs or Infs in the z data)
			if ( !std::isfinite( here ) )
			{
				if ( mask.Empty() )
					mask.ResizeFill( ny, nx, 0 );

				// mask out this cell if not finite
				mask(i,j) = 1;
			}
			else
			{				
				if ( here < min ) min = here;
				if ( here > max ) max = here;
			}
		}
	}

	if ( max <= min ) return;

	if ( m_levelMin < m_levelMax )
	{
		min = m_levelMin;
		max = m_levelMax;
	}

	// note: QCG expects shape(ny,nx) so swap y and x for correct rendering
	QuadContourGenerator qcg( m_x, m_y, m_z, mask, true, 0 );	
	if ( !m_filled )
	{
		double zstep = (max-min)/levels;
		for( int k=0;k<levels;k++ )
		{
			double zval =  min + zstep/2 + k*zstep;
			managed_ptr_list<ContourLine> list;
			qcg.create_contour( zval, list );

		
			for( size_t i=0;i<list.size();i++ )
			{
				m_cPolys.push_back( C_poly() );
				C_poly &CC = m_cPolys.back();
				CC.z = zval;
				std::vector<XY> &xy = *list[i];
				for( size_t j=0;j<xy.size();j++ )
					CC.pts.push_back( C_pt( xy[j].x, xy[j].y ) );
			}
		}
	}
	else
	{
		for( int k=0;k<levels;k++ )
		{
			double zlow =  min + ((double)k)/((double)levels)*(max-min);
			double zhigh =  min + ((double)k+1)/((double)levels)*(max-min);

			managed_ptr_list<QuadContourGenerator::VertexCodes> list;
			qcg.create_filled_contour( zlow, zhigh, list );
					
			for( size_t i=0;i<list.size();i++ )
			{
				QuadContourGenerator::VertexCodes &vc = *list[i];
				if ( vc.vertices.Rows() == vc.codes.Rows() )
				{
					m_cPolys.push_back( C_poly() );
					C_poly &CC = m_cPolys.back();
					CC.z = zlow;
					CC.zmax = zhigh;
					for( size_t j=0;j<vc.vertices.Rows();j++ )
					{
						double x= vc.vertices(j,0);
						double y = vc.vertices(j,1);
						unsigned char code = vc.codes(j,0);
						CC.pts.push_back( C_pt( x, y, (char)code ) );
					}
				}
			}
		}

	}

	//MarchingSquares ms;
	//ms.Contours( &m_contours, m_z, levels );
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
	if ( i < m_x.Cells() && i < m_y.Cells() ) 
		return wxRealPoint( m_x.RawIndex(i), m_y.RawIndex(i) );
	else 
		return wxRealPoint( std::numeric_limits<double>::quiet_NaN(),
			std::numeric_limits<double>::quiet_NaN() );
}

size_t wxPLContourPlot::Len() const
{
	return m_x.Cells();
}

std::vector<double> data_x;
std::vector<double> data_y;
wxMatrix<int> triangles;

static void draw_tri_mesh( wxPLOutputDevice &dc, const wxPLDeviceMapping &map )
{
	dc.Pen( wxColour(240,240,240), 0.75 );
	dc.NoBrush();
	
	for( size_t i=0;i<triangles.Rows();i++ )
	{
		wxRealPoint pt[3];
		double solval = 0;
		for (int k=0;k<3;k++ )
		{
			int idx = triangles(i,k);
			pt[k] = map.ToDevice( data_x[idx], data_y[idx] );
		}
		dc.Polygon( 3, pt );
	}
}
	
void wxPLContourPlot::Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map )
{
	if ( !m_cmap ) return;
	
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
	if ( !m_filled )
	{
		dc.NoBrush();
		for( size_t i=0;i<m_cPolys.size();i++ )
		{
			dc.Pen( m_cmap->ColourForValue( m_cPolys[i].z ), 2 );

			size_t n = m_cPolys[i].pts.size();
			std::vector<wxRealPoint> mapped( n );
			for( size_t j=0;j<n;j++ )
				mapped[j] = map.ToDevice( m_cPolys[i].pts[j].x, m_cPolys[i].pts[j].y );

			dc.Lines( m_cPolys[i].pts.size(), &mapped[0] ); 
		}
	}
	else
	{
		dc.NoPen();

		for( size_t i=0;i<m_cPolys.size();i++ )
		{
			double zmid = 0.5*(m_cPolys[i].z + m_cPolys[i].zmax );
			wxColour color(m_cmap->ColourForValue( zmid ));
			dc.Pen( color, 2 );
			dc.Brush( color );

			size_t n = m_cPolys[i].pts.size();
			std::vector<wxRealPoint> mapped( n );

			size_t idx = 0;
			for( size_t j=0;j<n;j++ )
			{
				C_pt &p = m_cPolys[i].pts[j];
				if ( p.act == MOVETO )
				{
					idx=0;
					mapped[idx++] = map.ToDevice( p.x, p.y );
				}
				else if ( p.act == LINETO )
				{
					mapped[idx++] = map.ToDevice( p.x, p.y );
				}
				else if ( p.act == CLOSEPOLY )
				{
					mapped[idx++] = map.ToDevice( p.x, p.y );
					dc.Polygon( idx, &mapped[0] );
					idx = 0;
				}
			}
		}
	}

	draw_tri_mesh( dc, map );
}

void wxPLContourPlot::DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct)
{
	// currently nothing to show in legend...? 
}


void wxPLContourPlot::MinMax(	const std::vector<double> &v,
		double *minval, double *maxval )
{
	double min = 1e99, max = -1e99;
	for( size_t i=0;i<v.size();i++ )
	{
		if ( std::isfinite( v[i] ) )
		{
			if ( v[i] < min ) min = v[i];
			if ( v[i] > max ) max = v[i];
		}
	}

	if ( minval ) *minval = min;
	if ( maxval ) *maxval = max;
}

bool wxPLContourPlot::MeshGrid( 
		double xmin, double xmax, size_t nx,
		double ymin, double ymax, size_t ny,
	wxMatrix<double> &xmesh,
	wxMatrix<double> &ymesh )
{
	if ( xmax <= xmin || ymax <= ymin || nx < 2  || ny < 2 ) return false;

	double xstep = (xmax-xmin)/(nx-1);
	double ystep = (ymax-ymin)/(ny-1);

	xmesh.Resize( ny, nx );
	ymesh.Resize( ny, nx );

	for( size_t i=0;i<ny;i++ )
	{
		for( size_t j=0;j<nx;j++ )
		{
			xmesh(i,j) = xmin + j*xstep;
			ymesh(i,j) = ymin + i*ystep;
		}
	}

	return true;
}


void wxPLContourPlot::Peaks( size_t n,
	wxMatrix<double> &xx, wxMatrix<double> &yy, wxMatrix<double> &zz, 
	double *min, double *max )
{
	if ( min ) *min = 1e99;
	if ( max ) *max = -1e99;

	xx.Resize( n, n );
	yy.Resize( n, n );
	zz.Resize( n, n );

	for( size_t i=0;i<n;i++ )
	{
		for( size_t j=0;j<n;j++ )
		{
			double y = -3.0 + ((double)i)/((double)n-1)*6.0;
			double x = -3.0 + ((double)j)/((double)n-1)*6.0;

			double z =  3*(1-x)*(1-x)*exp(-(x*x) - (y+1)*(y+1))
			   - 10*(x/5 - x*x*x - y*y*y*y*y)*exp(-x*x-y*y)
			   - 1/3*exp(-(x+1)*(x+1) - y*y);

			if ( min && z < *min ) *min = z;
			if ( max && z > *max ) *max = z;
			
			xx(j,i) = x;
			yy(j,i) = y;
			zz(j,i) = z;
		}
	}
}


//#include "delaunay.h"
extern "C" {
	#include "qhull/qhull_a.h"
}

static int qhull_last_error = 0;
static const char* qhull_error_msg[6] = {
    "no error",             /* 0 = qh_ERRnone */
    "input inconsistency",  /* 1 = qh_ERRinput */
    "singular input data",  /* 2 = qh_ERRsingular */
    "precision error",      /* 3 = qh_ERRprec */
    "insufficient memory",  /* 4 = qh_ERRmem */
    "internal error"};      /* 5 = qh_ERRqhull */


/* Return the indices of the 3 vertices that comprise the specified facet (i.e.
 * triangle). */
static void get_facet_vertices(const facetT* facet, int indices[3])
{
    vertexT *vertex, **vertexp;
    FOREACHvertex_(facet->vertices)
        *indices++ = qh_pointid(vertex->point);
}

/* Return the indices of the 3 triangles that are neighbors of the specified
 * facet (triangle). */
static void get_facet_neighbours(const facetT* facet, const int* tri_indices,
                     int indices[3])
{
    facetT *neighbor, **neighborp;
    FOREACHneighbor_(facet)
        *indices++ = (neighbor->upperdelaunay ? -1 : tri_indices[neighbor->id]);
}

/* Delaunay implementation methyod.  If hide_qhull_errors is 1 then qhull error
 * messages are discarded; if it is 0 then they are written to stderr. */
static bool qhull_delaunay(int npoints, const double* x, const double* y,
			  wxMatrix<int> &triangles,
			  wxMatrix<int> &neighbors )
{
    coordT* points = NULL;
    facetT* facet;
    int i, ntri, max_facet_id;
    int exitcode;               /* Value returned from qh_new_qhull(). */
    int* tri_indices = NULL;    /* Maps qhull facet id to triangle index. */
    int indices[3];
    int curlong, totlong;       /* Memory remaining after qh_memfreeshort. */
    const int ndim = 2;
    int* triangles_ptr;
    int* neighbors_ptr;

    /* Allocate points. */
    points = (coordT*)malloc(npoints*ndim*sizeof(coordT));
    if (points == NULL) {
        fprintf( stderr, "Could not allocate points array in qhull.delaunay" );
        goto error_before_qhull;
    }

    /* Prepare points array to pass to qhull. */
    for (i = 0; i < npoints; ++i) {
        points[2*i  ] = x[i];
        points[2*i+1] = y[i];
    }
    
    /* Perform Delaunay triangulation. */
	/* qhull expects a FILE* to write errors to, use stderr */
    exitcode = qh_new_qhull(ndim, npoints, points, False,
                            "qhull d Qt Qbb Qc Qz", NULL, stderr);
    if (exitcode != qh_ERRnone) {
		qhull_last_error = exitcode;
        fprintf( stderr,
                     "Error in qhull Delaunay triangulation calculation: %s (exitcode=%d)",
                     qhull_error_msg[exitcode], exitcode );
        goto error;
    }

    /* Split facets so that they only have 3 points each. */
    qh_triangulate();

    /* Determine ntri and max_facet_id.
       Note that libqhull uses macros to iterate through collections. */
    ntri = 0;
    FORALLfacets {
        if (!facet->upperdelaunay)
            ++ntri;
    }

    max_facet_id = qh facet_id - 1;

    /* Create array to map facet id to triangle index. */
    tri_indices = (int*)malloc((max_facet_id+1)*sizeof(int));
    if (tri_indices == NULL) {
		fprintf( stderr,"Could not allocate triangle map in qhull.delaunay");
        goto error;
    }

    /* Allocate python arrays to return. */
	triangles.Resize( ntri, 3 );
	neighbors.Resize( ntri, 3 );

    triangles_ptr = triangles.Data();
    neighbors_ptr = neighbors.Data();

    /* Determine triangles array and set tri_indices array. */
    i = 0;
    FORALLfacets {
        if (!facet->upperdelaunay) {
            tri_indices[facet->id] = i++;
            get_facet_vertices(facet, indices);
            *triangles_ptr++ = (facet->toporient ? indices[0] : indices[2]);
            *triangles_ptr++ = indices[1];
            *triangles_ptr++ = (facet->toporient ? indices[2] : indices[0]);
        }
        else
            tri_indices[facet->id] = -1;
    }

    /* Determine neighbors array. */
    FORALLfacets {
        if (!facet->upperdelaunay) {
            get_facet_neighbours(facet, tri_indices, indices);
            *neighbors_ptr++ = (facet->toporient ? indices[2] : indices[0]);
            *neighbors_ptr++ = (facet->toporient ? indices[0] : indices[2]);
            *neighbors_ptr++ = indices[1];
        }
    }

	data_x.resize( npoints );
	data_y.resize( npoints );
	for( int i=0;i<npoints;i++ )
	{
		data_x[i] = x[i];
		data_y[i] = y[i];
	}

    /* Clean up. */
    qh_freeqhull(!qh_ALL);
    qh_memfreeshort(&curlong, &totlong);
    if (curlong || totlong)
        fprintf( stderr, "Qhull could not free all allocated memory", 1);

    free(tri_indices);
    free(points);

	return true;

error:
    qh_freeqhull(!qh_ALL);
    qh_memfreeshort(&curlong, &totlong);
    free(tri_indices);

error_before_qhull:
    free(points);

    return NULL;
}

static int search( const wxMatrix<int> &tri, const std::vector<double> &x, const std::vector<double> &y, double xq, double yq )
{
	// TODO: http://www.geom.uiuc.edu/~bradb/qhull3.1/html/qh-faq.htm#vclosest
	// this is a naive search - faster methods available for delaunay triangulation using the neighbors information
	for( size_t i=0;i<tri.Rows();i++ )
	{
		/*
		Get the vertices of triangle TRIANGLE.
		*/
		int a = tri(i,0);
		int b = tri(i,1);
		int c = tri(i,2);
		/*
		Using vertex C as a base, compute the distances to vertices A and B,
		and the point (X,Y).
		*/
		double dxa = x[a] - x[c];
		double dya = y[a] - y[c];

		double dxb = x[b] - x[c];
		double dyb = y[b] - y[c];

		double dxp = xq - x[c];
		double dyp = yq - y[c];

		double det = dxa * dyb - dya * dxb;
		/*
		Compute the barycentric coordinates of the point (X,Y) with respect
		to this triangle.
		*/
		double alpha = ( dxp * dyb - dyp * dxb ) / det;
		double beta =  ( dxa * dyp - dya * dxp ) / det;
		double gamma = 1.0 - alpha - beta;
		/*
		If the barycentric coordinates are all positive, then the point
		is inside the triangle and we're done.
		*/
		if ( 0.0 <= alpha &&
			0.0 <= beta  &&
			0.0 <= gamma )
		{
			return i;
		}
	}

	return -1;
}

#include <wx/msgdlg.h>

bool wxPLContourPlot::GridData( 
			const std::vector<double> &x, 
			const std::vector<double> &y,
			const std::vector<double> &z,
			const wxMatrix<double> &xq,
			const wxMatrix<double> &yq,
		wxMatrix<double> &zinterp )
{
	if ( x.size() != y.size() || y.size() != z.size() ) return false;
	if ( xq.Rows() != yq.Rows() || xq.Cols() != yq.Cols() ) return false;

	size_t len = x.size();

	wxMatrix<int> neighbors;
	if ( !qhull_delaunay( len, &x[0], &y[0], triangles, neighbors ) )
	{
		wxMessageBox( "Error in qhull delaunay" + wxString(qhull_error_msg[ qhull_last_error ]) );
		return false;
	}

	zinterp.Resize( xq.Rows(), xq.Cols() );

	for( size_t i=0;i<xq.Rows();i++ )
	{
		for( size_t j=0;j<xq.Cols();j++ )
		{
			double xqq = xq(i,j);
			double yqq = yq(i,j);

			int index = search( triangles, x, y, xqq, yqq );
			if ( index >= 0 )
			{
				int a = triangles(index,0);
				int b = triangles(index,1);
				int c = triangles(index,2);

				double d1 = sqrt( pow( xqq - x[a], 2 ) + pow( yqq - y[a], 2 ) );
				double d2 = sqrt( pow( xqq - x[b], 2 ) + pow( yqq - y[b], 2 ) );
				double d3 = sqrt( pow( xqq - x[c], 2 ) + pow( yqq - y[c], 2 ) );
				
				// calculate interpolated Z value
				double zi = ( d1*z[a] + d2*z[b] + d3*z[c] ) / ( d1 + d2 + d3 );
				zinterp(i,j) = zi;

			}
			else
				zinterp(i,j) = std::numeric_limits<double>::quiet_NaN();
		}
	}

	return true;
}