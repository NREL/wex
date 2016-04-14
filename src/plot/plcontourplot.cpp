
#include "wex/plot/plcontourplot.h"
#include "wex/plot/plcolourmap.h"

void Write( const wxMatrix<double> &data, const char *file )
{
	if( FILE *fp = fopen(file, "w") )
	{
		for( size_t i=0;i<data.Rows();i++ )
			for( size_t j=0;j<data.Cols();j++ )
				fprintf(fp, "%lg%c", data(data.Rows()-i-1,j), j<data.Cols()-1 ? ',' : '\n');

		fclose(fp);
	}
}


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

class Grid : public wxMatrix<Cell>
{
public:	
	double threshold;
	Grid() {
		threshold = 0;
	}

	Grid( size_t rows, size_t cols, double thres )
		: wxMatrix<Cell>( rows, cols ), threshold(thres)
	{
	}
};


#define EPSILON 1E-7

class PathGenerator
{
public:
	PathGenerator() {
		// nothing to do
	}

	void Generate( Grid &grid, std::vector<wxPLContourPlot::Contour> *contours ) {
		for (size_t r = 0; r < grid.Rows(); r++) {
			for (size_t c = 0; c < grid.Cols(); c++) {
				// find a start node...
				Cell &cell = grid(r, c);
				if ( !cell.Trivial() && !cell.Saddle() )
				{
					contours->push_back( wxPLContourPlot::Contour() );
					wxPLContourPlot::Contour &CC = contours->back();
					Follow(grid, r, c, CC );
					CC.points.push_back( CC.points[0] ); // to close the line
					CC.level = grid.threshold;
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
	void Follow(Grid &grid, int r, int c, wxPLContourPlot::Contour & path) {
		Side prevSide = NONE;

		Cell *start = &grid( r, c );

		double x, y;
		start->Coord(FirstSide(*start, prevSide), &x, &y);
		x += c;
		y += r;

		// prepare a new subpath
		path.points.push_back( wxRealPoint( x, y ) );

		double xPrev, yPrev;
		prevSide = SecondSide(*start, prevSide);
		start->Coord(prevSide, &xPrev, &yPrev);
		xPrev += c;
		yPrev += r;
		
		switch (prevSide) {
		case BOTTOM: r--; break;
		case LEFT: c--; break;
		case RIGHT: c++; break;
		case TOP: r++; // fall through
		default: // keeps compiler happy + handle NONE case which should never happen
			break;
		}

		start->Clear();

		Cell *currentCell = &grid(r, c);
		while ( start != currentCell )
		{
			prevSide = SecondSide(*currentCell, prevSide);

			currentCell->Coord(prevSide, &x, &y );
			x += c;
			y += r;

			if (fabs(x - xPrev) > EPSILON && fabs(y - yPrev) > EPSILON) {
				path.points.push_back( wxRealPoint(x,y) );
			}

			xPrev = x;
			yPrev = y;
			
			currentCell->Clear();
			
			switch (prevSide) {
			case BOTTOM: r--; break;
			case LEFT: c--; break;
			case RIGHT: c++; break;
			case TOP: r++; break;
			default:
				break;
			}

			currentCell = &grid(r, c);
		}

	}
};

class MarchingSquares
{
public:
   MarchingSquares()
   {
   }

   bool Contour1( const wxMatrix<double> &data, double isovalue, Grid &cells ) {
		size_t rowCount = data.Rows();
		size_t colCount = data.Cols();

		if( rowCount < 2 || colCount < 2 ) return false;

		// Every 2x2 block of pixels in the binary image forms a contouring cell,
		// so the whole image is represented by a grid of such cells. Note that
		// this contouring grid is one cell smaller in each direction than the
		// original 2D field.
		cells.ResizeFill( rowCount-1, colCount-1, Cell() );
		cells.threshold = isovalue;

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

   /**
    * <p>Pad data with a given 'guard' value.</p>
    *
    * @param data matrix to pad.
    * @param guard the value to use for padding. It's expected to be less than
    * the minimum of all data cell values.
    * @return the resulting padded matrix which will be larger by 2 in both
    * directions.
    */
   void Pad2( const wxMatrix<double> &data, double zmin, wxMatrix<double> &result ) {
	   int rowCount = data.Rows();
	   int colCount = data.Cols();
	  result.Resize( data.Rows()+2, data.Cols()+2 );


      // the middle
	  for( int i=0;i<rowCount;i++ )
		  for( int j=0;j<colCount;j++ )
			  result(i+1,j+1) = data(i,j);
      // top and bottom rows
      for (int j = 0; j < colCount + 2; j++)
	  {
         result(0,j) = result(1,j);
		 result(rowCount+1,j) = result(rowCount,j);
	  }

      // left- and right-most columns excl. top and bottom rows
      for (int i = 1; i < rowCount + 1; i++)
	  {
         result(i,0) = result(i,1);
		result(i,colCount+1) = result(i,colCount);
	  }

	  result(0,0) = 0.5*(result(0,1) + result(1,0));
	  result(0,colCount+1) = 0.5*(result(0,colCount) + result(1,colCount+1));
	  result(rowCount+1,0) = 0.5*(result(rowCount,0) + result(rowCount+1,1));
	  result(rowCount+1,colCount+1) = 0.5*(result(rowCount,colCount+1) + result(rowCount+1,colCount));


	  Write( result, "padded.csv");
   }

   void Pad( const wxMatrix<double> &data, double zmin, wxMatrix<double> &result ) {
	   int rowCount = data.Rows();
	   int colCount = data.Cols();
	  result.Resize( data.Rows()+2, data.Cols()+2 );

      // top and bottom rows
      for (int j = 0; j < colCount + 2; j++)
         result(0,j) = result(rowCount+1,j) = zmin;

      // left- and right-most columns excl. top and bottom rows
      for (int i = 1; i < rowCount + 1; i++)
         result(i,0) = result(i,colCount+1) = zmin;

      // the middle
	  for( int i=0;i<rowCount;i++ )
		  for( int j=0;j<colCount;j++ )
			  result(i+1,j+1) = data(i,j);

	  Write( result, "padded.csv");
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
	
		// IMPORTANT: pad data to ensure resulting linear strings are closed
		double guard = min - 0.1;

		wxMatrix<double> data;
		Pad( z, guard, data );

		for( int i=0;i<=levels;i++ )
		{
			double iso = min + ((double)i)/((double)levels)*(max-min);
			ContourAtIso( data, iso, cntr );
		}

	}

	void ContourAtIso( wxMatrix<double> &padded, double iso, std::vector<wxPLContourPlot::Contour> *cntr )
	{
		Grid cells;
		if( Contour1( padded, iso, cells ) )
		{
			if ( FILE *fp = fopen("grid.csv", "w") )
			{
				for( size_t i=0;i<cells.Rows();i++ )
					for( size_t j=0;j<cells.Cols();j++ )
						fprintf( fp, "%d%c", (int) cells(cells.Rows()-i-1,j).Id(), j<cells.Cols()-1 ? ',' : '\n' );
				fclose(fp);
			}
			PathGenerator gen;
			gen.Generate( cells, cntr );
		}
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

	//wxMatrix<double> padded;
	//ms.Pad( m_z, -10, padded );
	//ms.ContourAtIso( padded, 0.0, &m_contours );
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
	return wxRealPoint( std::numeric_limits<double>::quiet_NaN(),
		std::numeric_limits<double>::quiet_NaN() );
}

size_t wxPLContourPlot::Len() const
{
	return m_z.Cells();
}
	
void wxPLContourPlot::Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map )
{
	if ( !m_cmap ) return;

	bool m_filled = false;
	if ( m_filled ) dc.NoPen();
	else dc.NoBrush();

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
