#include <math.h>
#include "mex.h"
#include "matrix.h"
#define REAL float








void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
   
    if(nrhs!=3) {
  mexPrintf("wrong usage!!\n",nrhs);
    return;
  } else if(nlhs>4) {
  printf("Too many output arguments\n");
    return;
  }

    int pcnt = 0;
    const mxArray *Dim;
    Dim = prhs[pcnt++];       
    REAL *dim = (REAL*) mxGetData(Dim);
    int w = (int) dim[0];
    int h = (int) dim[1];
    int d = (int) dim[2];    

    const mxArray *Pts;
    Pts = prhs[pcnt++];       
    const int numdim = mxGetNumberOfDimensions(Pts);
    const int *pdims = mxGetDimensions(Pts);
    int numPts = pdims[1];
    REAL *pts = (REAL*) mxGetData(Pts);
    
   const mxArray *Weights;
    Weights = prhs[pcnt++];       
    const int wnumdim = mxGetNumberOfDimensions(Weights);
    const int *wpdims = mxGetDimensions(Weights);
    int numW = wpdims[0];
    REAL *weights = (REAL*) mxGetData(Weights);
    

    int dims[3];
    dims[0] = w;
    dims[1] = h;
    dims[2] = d;
    plhs[0] = mxCreateNumericArray(3,dims,mxGetClassID(Dim),mxREAL);
    REAL *accu = (REAL*) mxGetData(plhs[0]);


   for (int i = 0; i < numPts; i++)
  {
    int idx = 3*i;

    int px = (int) (pts[idx]);
    if (px < 0 || px >= w-1)
      continue;
    int py = (int) (pts[idx+1]);
    if (py < 0 || py >= h-1)
      continue;
    int pz = (int) (pts[idx+2]);
    if (pz < 0 || pz >= d-1)
      continue;
    float frac_x = pts[idx  ] - px;
    float frac_y = pts[idx+1] - py;
    float frac_z = pts[idx+2] - pz;


    accu[px + w*(py+h*pz)] += (1-frac_x)*(1-frac_y)*(1-frac_z) * weights[i];

    accu[px+1 + w*(py+h*pz)] += (frac_x)*(1-frac_y)*(1-frac_z) * weights[i];
    accu[px + w*(py+1+h*pz)] += (1-frac_x)*(frac_y)*(1-frac_z) * weights[i];
    accu[px + w*(py+h*pz+h)] += (1-frac_x)*(1-frac_y)*(frac_z) * weights[i];

    accu[px + w*(py+1+h*pz+h)] += (1-frac_x)*(frac_y)*(frac_z) * weights[i];
    accu[px+1 + w*(py+h*pz+h)] += (frac_x)*(1-frac_y)*(frac_z) * weights[i];
    accu[px+1 + w*(py+1+h*pz)] += (frac_x)*(frac_y)*(1-frac_z) * weights[i];

    accu[px+1 + w*(py+1+h*pz+h)] += (frac_x)*(frac_y)*(frac_z) * weights[i];
  
  }


}






