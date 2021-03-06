/*
  A simple 2D hydro code
  (C) Romain Teyssier : CEA/IRFU           -- original F90 code
  (C) Pierre-Francois Lavallee : IDRIS      -- original F90 code
  (C) Guillaume Colin de Verdiere : CEA/DAM -- for the C version
  (C) Jeffrey Poznanovic : CSCS             -- for the OpenACC version
*/


/*

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

*/

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#include "parametres.h"
#include "utils.h"
#include "slope.h"

#ifndef HMPP

#define DABS(x) (double) fabs((x))
#define IDX(i,j,k) ( (i*Hstep*Hnxyt) + (j*Hnxyt) + k )

void
slope(const int n,
      const int Hnvar,
      const int Hnxyt,
      const double Hslope_type,
      const int slices, const int Hstep, double *q, double *dq) {
  //const int slices, const int Hstep, double q[Hnvar][Hstep][Hnxyt], double dq[Hnvar][Hstep][Hnxyt]) {
  int nbv, i, ijmin, ijmax, s;
  double dlft, drgt, dcen, dsgn, slop, dlim;
  // long ihvwin, ihvwimn, ihvwipn;
  // #define IHVW(i, v) ((i) + (v) * Hnxyt)

  WHERE("slope");
  ijmin = 0;
  ijmax = n;

#pragma acc parallel pcopyin(q[0:Hnvar*Hstep*Hnxyt]) pcopy(dq[0:Hnvar*Hstep*Hnxyt])
#pragma acc loop gang collapse(2)
  for (nbv = 0; nbv < Hnvar; nbv++) {
    for (s = 0; s < slices; s++) {
#pragma acc loop vector
      for (i = ijmin + 1; i < ijmax - 1; i++) {
        dlft = Hslope_type * (q[IDX(nbv,s,i)] - q[IDX(nbv,s,i - 1)]);
        drgt = Hslope_type * (q[IDX(nbv,s,i + 1)] - q[IDX(nbv,s,i)]);
        dcen = half * (dlft + drgt) / Hslope_type;
        dsgn = (dcen > 0) ? (double) 1.0 : (double) -1.0;       // sign(one, dcen);
        slop = fmin(fabs(dlft), fabs(drgt));
        dlim = slop;
        if ((dlft * drgt) <= zero) {
          dlim = zero;
        }
        dq[IDX(nbv,s,i)] = dsgn * fmin(dlim, fabs(dcen));
#ifdef FLOPS
        flops += 8;
#endif
      }
    }
  }
}                               // slope

#undef IHVW
#undef IDX

#endif /* HMPP */
//EOF
