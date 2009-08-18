#include "propagate.h"

/* -------------------------------------------------------------------------
Implementation of the Voronoi-based segmentation on image manifolds [1]

The code below is based on the 'IdentifySecPropagateSubfunction.cpp'
module (revision 3667) of CellProfiler [2]. CellProfiler is released
under the terms of GPL, however the LGPL license was granted by T. Jones
on Feb 7, 07 to use the code in the above file for this project.

[1] T. Jones, A. Carpenter and P. Golland,
    "Voronoi-Based Segmentation of Cells on Image Manifolds"
    CVBIA05 (535-543), 2005

[2] CellProfiler: http://www.cellprofiler.org

Copyright (C) of the original CellProfiler code:
 - Anne Carpenter <carpenter@wi.mit.edu>
 - Thouis Jones <thouis@csail.mit.edu>
 - In Han Kang <inthek@mit.edu>

Copyright (C) of the implementation below:
 - Oleg Sklyar <osklyar@ebi.ac.uk>
 - Wolfgang Huber <huber@ebi.ac.uk>

See: ../LICENSE for license, LGPL.

------------------------------------------------------------------------- */
#include <R_ext/Error.h>
#include <queue>
#include "tools.h"

using namespace std;

#define INDEX(i,j) ((i) + (j) * nx)

class Pixel {
  public:
    double distance;
    int i, j;
    double seed;
    Pixel (double ds, int ini, int inj, double s) :
      distance(ds), i(ini), j(inj), seed(s) {}
};

struct Pixel_compare {
  bool operator() (const Pixel& a, const Pixel& b) const {
    return a.distance > b.distance;
  }
};

typedef priority_queue<Pixel, vector<Pixel>, Pixel_compare> PixelQueue;

/* forward declaration: distance calculation function */
inline double
deltaG (double *, int, int, int, int, int, int, double, int);

/* these will form indexes for a neighbourhood of surrounding pixels, going
 * firsth through the ones left-right and top-bottom and then diagonals */
static int ix[8] = {-1, 0, +1,  0, -1, +1, -1, +1};
static int jy[8] = { 0, 1,  0, -1, -1, +1, +1, -1};

/* ----  R Interface entry point  --------------------------------------- */
SEXP
propagate (SEXP x, SEXP seeds_, SEXP mask_, SEXP ext_, SEXP lambda_) {
  SEXP res;
  int i, ii, j, jj, cntr, nprotect = 0;

  int nx = INTEGER(GET_DIM(x))[0];
  int ny = INTEGER(GET_DIM(x))[1];
  int nz = getNumberOfFrames(x, 0);

  int ext = INTEGER( ext_ )[0];
  double lambda = REAL( lambda_ )[0];

  PROTECT ( res = Rf_duplicate(x) );
  nprotect++;
 
  /* we will keep distances here, reset for every new image */
  double * dists = new double[ nx * ny ];

  for ( int im = 0; im < nz; im++ ) {
    double * src =   &( REAL( x )[im * nx * ny] );
    double * tgt =   &( REAL( res )[im * nx * ny] );
    double * seeds = &( REAL( seeds_ )[im * nx * ny] );

    double * mask;
    if ( mask_ != R_NilValue )
      mask = &( REAL( mask_ )[im * nx * ny] );
    else
      mask = NULL;

    PixelQueue pixel_queue;
    double seed, d;
    int index;
    bool masked;
    /* main algorithm */

    /* res initialization */
    for ( index = 0; index < nx * ny; index++ )
      tgt[ index ] = 0.0;

    /* initialization */
    for ( j = 0; j < ny; j++ )
      for ( i = 0; i < nx; i++ ) {
        index = i + nx * j;
        masked = false;
        if ( mask )
          if ( mask[ index] == 0 )
              masked = true;
        /* initialize distances */
        dists[ index ] = R_PosInf;

        /* mark seed in returns, indexing should start at 1; */
        /* 0.5 is reserved to mark contact regions (EBImage) */
        if ( (seed = seeds[index]) <= 0.9 || masked ) continue;
        tgt[ index ] = seed;
        dists[ index ] = 0.0;
        /* push neighbours onto the queue */
        for ( cntr = 0; cntr < 8; cntr++ ) {
          ii = i + ix[cntr];
          jj = j + jy[cntr];
          if ( ii < 0 || ii >= nx || jj < 0 || jj >= ny ) continue;
          /* at this initialization step we do not push pixels on the queue which are
            * already assigned, i.e. which are seeds because their distance is 0 and cannot be smaller */
          if ( tgt[ INDEX(ii, jj) ] > 0.9 ) continue;
          pixel_queue.push( Pixel(deltaG(src, i, j, ii, jj, nx, ny, lambda, ext), ii, jj, seed) );
        }
      }

    /* propagation */
    /* the queue only has pixels around the seeds originally, but it
      * gets new pixels dynamically as the assigned regions extend */
    while ( !pixel_queue.empty() ) {
      /* get the topmost pixel as its distance is largerst */
      Pixel px = pixel_queue.top();
      index = INDEX(px.i, px.j);
      /* and remove it from the queue */
      pixel_queue.pop();

      masked = false;
      if ( mask )
        if ( mask[ index] <= 0 )
          masked = true;

      /* if currently assigned distance for this pixel (PosInf)
        * is larger than the one stored in the pixel when pushing
        * onto the queue, then reassign distance and seed */
      if ( dists[ index ] <= px.distance || masked ) continue;
      dists[ index ] = px.distance;
      tgt[ index ] = px.seed;
      /* push neighbours onto the queue */
      for ( cntr = 0; cntr < 8; cntr++ ) {
        ii = px.i + ix[cntr];
        jj = px.j + jy[cntr];
        if ( ii < 0 || ii >= nx || jj < 0 || jj >= ny ) continue;
        index = INDEX(ii, jj) ;
        /* now we do not want to push onto the queue pixels that are already
          * assigned to the same seed, we only update their distance. pixels assigned
          * to other seeds are pushed up as their distance to this seed can be
          * smaller later on (in L.163 above). anyway this should be much faster than
          * CellProfile'r original algorithm as we do not resize the queue on
          * pixels that we are not going to reassign */
        d = px.distance + deltaG(src, px.i, px.j, ii, jj, nx, ny, lambda, ext);
        if ( tgt[ index ] == px.seed ) {
          if ( dists[ index ] > d )
            dists[ index ] = d;
          continue;
        }
        pixel_queue.push( Pixel(d, ii, jj, px.seed) );
      }
    }
  }

  delete[] dists;
  UNPROTECT( nprotect );
  return res;
}

inline double
clamped_fetch (double * data, int i, int j, int nx, int ny) {
  if (i < 0) i = 0;
  if (i >= nx) i = nx - 1;
  if (j < 0) j = 0;
  if (j >= ny) j = ny - 1;
  return data[ INDEX(i, j) ];
}

/* this is the distance evaluation in the modified metric */
inline double
deltaG ( double * src, int x1,  int y1, int x2,  int y2,
        int nx, int ny, double lambda, int ext) {
  int x, y;
  double dI = 0.0;

  for (x=-ext; x<=ext; x++)
    for (y=-ext; y<=ext; y++)
      dI += fabs(clamped_fetch(src, x1+x, y1+y, nx, ny) -
                 clamped_fetch(src, x2+x, y2+y, nx, ny));
  /* this is not because we calculate dI/dx as it is dx*(dI/dx)=dI, this is
  * because we want the mean value of the gradient in the region */
  dI /= (2.0*ext+1.0)*(2.0*ext+1.0);

  double dEucl = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);

  // Gradient described in the original paper [1]
  return sqrt((dI*dI + lambda*dEucl)/(1.0 + lambda));

  /* CellProfiler's gradient
     double dManh = abs(x2 - x1) + abs(y2 - y1);
     return sqrt( dI*dI + dManh * lambda * lambda);
  */
  // return sqrt(dI) + 1e-3*lambda*dEucl*dEucl;
}

