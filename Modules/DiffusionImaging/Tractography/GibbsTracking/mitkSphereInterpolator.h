/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _SPHEREINTERPOLATOR
#define _SPHEREINTERPOLATOR

#include <MitkDiffusionImagingExports.h>

class MitkDiffusionImaging_EXPORT SphereInterpolator
{
public:
  float *barycoords;
  int *indices;
  int size;  // size of LUT
  int sN;   // (sizeofLUT-1)/2
  int nverts; // number of data vertices


  float beta;
  float inva;
  float b;


  int *idx;
  float *interpw;

  SphereInterpolator(float *barycoords, int *indices, int numverts, int sizeLUT, float beta)
  {
    this->barycoords = barycoords;
    this->indices = indices;
    this->size = sizeLUT;
    this->sN = (sizeLUT-1)/2;
    this->nverts = numverts;
    this->beta = beta;

    inva = (sqrt(1+beta)-sqrt(beta));
    b = 1/(1-sqrt(1/beta + 1));

  }

  inline void getInterpolation(vnl_vector_fixed<float, 3> N)
  {
    float nx = N[0];
    float ny = N[1];
    float nz = N[2];

    if (nz > 0.5)
    {
      int x = float2int(nx);
      int y = float2int(ny);
      int i = 3*6*(x+y*size);  // (:,1,x,y)
      idx = indices+i;
      interpw = barycoords +i;
      return;
    }
    if (nz < -0.5)
    {
      int x = float2int(nx);
      int y = float2int(ny);
      int i = 3*(1+6*(x+y*size));  // (:,2,x,y)
      idx = indices+i;
      interpw = barycoords +i;
      return;
    }
    if (nx > 0.5)
    {
      int z = float2int(nz);
      int y = float2int(ny);
      int i = 3*(2+6*(z+y*size));  // (:,2,x,y)
      idx = indices+i;
      interpw = barycoords +i;
      return;
    }
    if (nx < -0.5)
    {
      int z = float2int(nz);
      int y = float2int(ny);
      int i = 3*(3+6*(z+y*size));  // (:,2,x,y)
      idx = indices+i;
      interpw = barycoords +i;
      return;
    }
    if (ny > 0)
    {
      int x = float2int(nx);
      int z = float2int(nz);
      int i = 3*(4+6*(x+z*size));  // (:,1,x,y)
      idx = indices+i;
      interpw = barycoords +i;
      return;
    }
    else
    {
      int x = float2int(nx);
      int z = float2int(nz);
      int i = 3*(5+6*(x+z*size));  // (:,1,x,y)
      idx = indices+i;
      interpw = barycoords +i;
      return;
    }

  }


  inline float invrescale(float f)
  {
    float x = (fabs(f)-b)*inva;
    if (f>0)
      return (x*x-beta);
    else
      return beta - x*x;
  }

  inline int float2int(float x)
  {
    return int((invrescale(x)+1)*sN-0.5);

  }


};

#endif
