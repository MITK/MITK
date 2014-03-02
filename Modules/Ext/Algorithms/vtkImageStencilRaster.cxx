/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageStencilData.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "mitkvtkImageStencilRaster.h"

#include "vtkImageStencilSource.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"

#include <math.h>
#include <vtkstd/algorithm>

//----------------------------------------------------------------------------
// tolerance for float-to-int conversion in stencil operations

#define VTK_STENCIL_TOL 7.62939453125e-06

//----------------------------------------------------------------------------
vtkImageStencilRaster::vtkImageStencilRaster(const int extent[2])
{
  int rsize = extent[1] - extent[0] + 1;

  // the "raster" is a sequence of pointer-pairs, where the first pointer
  // points to the first value in the raster line, and the second pointer
  // points to one location past the last vale in the raster line.  The
  // difference is the number of x values stored in the raster line.
  this->Raster = new double*[2*static_cast<size_t>(rsize)];

  // the extent is the range of y values (one line per y)
  this->Extent[0] = extent[0];
  this->Extent[1] = extent[1];

  // tolerance should be larger than expected roundoff errors
  this->Tolerance = VTK_STENCIL_TOL;

  // no extent is used initially
  this->UsedExtent[0] = 0;
  this->UsedExtent[1] = -1;
}

//----------------------------------------------------------------------------
vtkImageStencilRaster::~vtkImageStencilRaster()
{
  if (this->UsedExtent[1] >= this->UsedExtent[0])
    {
    size_t i = 2*static_cast<size_t>(this->UsedExtent[0] - this->Extent[0]);
    size_t imax = 2*static_cast<size_t>(this->UsedExtent[1] - this->Extent[0]);

    do
      {
      if (this->Raster[i])
        {
        delete [] this->Raster[i];
        }
      i += 2;
      }
    while (i <= imax);
    }
  delete [] this->Raster;
}

//----------------------------------------------------------------------------
void vtkImageStencilRaster::PrepareForNewData(const int allocateExtent[2])
{
  if (this->UsedExtent[1] >= this->UsedExtent[0])
    {
    // reset and re-use the allocated raster lines
    size_t i = 2*static_cast<size_t>(this->UsedExtent[0]-this->Extent[0]);
    size_t imax=2*static_cast<size_t>(this->UsedExtent[1]-this->Extent[0]);
    do
      {
      this->Raster[i+1] = this->Raster[i];
      i += 2;
      }
    while (i <= imax);
    }

  if (allocateExtent && allocateExtent[1] >= allocateExtent[1])
    {
    this->PrepareExtent(allocateExtent[0], allocateExtent[1]);
    }
}

//----------------------------------------------------------------------------
void vtkImageStencilRaster::PrepareExtent(int ymin, int ymax)
{
  // this does not do any allocation, it just initializes any
  // raster lines are not already part of the UsedExtent, and
  // then expands the UsedExtent to include [ymin, ymax]

  if (this->UsedExtent[1] < this->UsedExtent[0])
    {
    size_t i = 2*static_cast<size_t>(ymin - this->Extent[0]);
    size_t imax = 2*static_cast<size_t>(ymax - this->Extent[0]);

    do
      {
      this->Raster[i] = 0;
      }
    while (++i <= imax);

    this->UsedExtent[0] = ymin;
    this->UsedExtent[1] = ymax;

    return;
    }

  if (ymin < this->UsedExtent[0])
    {
    size_t i = 2*static_cast<size_t>(ymin - this->Extent[0]);
    size_t imax = 2*static_cast<size_t>(this->UsedExtent[0]-this->Extent[0]-1);

    do
      {
      this->Raster[i] = 0;
      }
    while (++i <= imax);

    this->UsedExtent[0] = ymin;
    }

  if (ymax > this->UsedExtent[1])
    {
    size_t i = 2*static_cast<size_t>(this->UsedExtent[1]+1 - this->Extent[0]);
    size_t imax = 2*static_cast<size_t>(ymax - this->Extent[0]);

    do
      {
      this->Raster[i] = 0;
      }
    while (++i <= imax);

    this->UsedExtent[1] = ymax;
    }
}

//----------------------------------------------------------------------------
void vtkImageStencilRaster::InsertPoint(int y, double x)
{
  size_t pos = 2*static_cast<size_t>(y - this->Extent[0]);
  double* &rhead = this->Raster[pos];
  double* &rtail = this->Raster[pos+1];

  // current size is the diff between the tail and the head
  size_t n = rtail - rhead;

  // no allocation on this raster line yet
  if (rhead == 0)
    {
    rhead = new double[2];
    rtail = rhead;
    }
  // grow whenever size reaches a power of two
  else if (n > 1 && (n & (n-1)) == 0)
    {
    double *ptr = new double[2*n];
    for (size_t i = 0; i < n; i++)
      {
      ptr[i] = rhead[i];
      }
    delete [] rhead;
    rhead = ptr;
    rtail = ptr + n;
    }

  // insert the value
  *rtail++ = x;
}

//----------------------------------------------------------------------------
void vtkImageStencilRaster::InsertLine(
  const double pt1[2], const double pt2[2],
  bool inflection1, bool inflection2)
{
  double x1 = pt1[0];
  double x2 = pt2[0];
  double y1 = pt1[1];
  double y2 = pt2[1];

  // swap end points if necessary
  if (y1 > y2)
    {
    x1 = pt2[0];
    x2 = pt1[0];
    y1 = pt2[1];
    y2 = pt1[1];
    bool tmp = inflection1;
    inflection1 = inflection2;
    inflection2 = tmp;
    }

  // find min and max of x values
  double xmin = x1;
  double xmax = x2;
  if (x1 > x2)
    {
    xmin = x2;
    xmax = x1;
    }

  // check for parallel to the x-axis
  if (y1 == y2)
    {
    return;
    }

  double ymin = y1;
  double ymax = y2;

  // if an end is an inflection point, include a tolerance
  ymin -= inflection1*this->Tolerance;
  ymax += inflection2*this->Tolerance;

  // Integer y values for start and end of line
  int iy1, iy2;
  iy1 = this->Extent[0];
  iy2 = this->Extent[1];

  // Check for out of bounds
  if (ymax < iy1 || ymin >= iy2)
    {
    return;
    }

  // Guard against extentY
  if (ymin >= iy1)
    {
    iy1 = vtkMath::Floor(ymin) + 1;
    }
  if (ymax < iy2)
    {
    iy2 = vtkMath::Floor(ymax);
    }

  // Expand allocated extent if necessary
  if (iy1 < this->UsedExtent[0] ||
      iy2 > this->UsedExtent[1])
    {
    this->PrepareExtent(iy1, iy2);
    }

  // Precompute values for a Bresenham-like line algorithm
  double grad = (x2 - x1)/(y2 - y1);
  double delta = (iy1 - y1)*grad;

  // Go along y and place each x in the proper raster line
  for (int y = iy1; y <= iy2; y++)
    {
    double x = x1 + delta;
    // incrementing delta has less roundoff error than incrementing x,
    // since delta will typically be smaller than x
    delta += grad;

    // clamp x (because of tolerance, it might not be in range)
    if (x < xmin)
      {
      x = xmin;
      }
    else if (x > xmax)
      {
      x = xmax;
      }

    this->InsertPoint(y, x);
    }
}

//----------------------------------------------------------------------------
void vtkImageStencilRaster::FillStencilData(
  vtkImageStencilData *data, const int extent[6], int xj, int yj)
{
  if (xj != 0)
    {
    // slices are stacked in the x direction
    int xmin = extent[2*xj];
    int xmax = extent[2*xj+1];
    int ymin = this->UsedExtent[0];
    int ymax = this->UsedExtent[1];
    int zmin = extent[0];
    int zmax = extent[1];

    for (int idY = ymin; idY <= ymax; idY++)
      {
      size_t pos = 2*static_cast<size_t>(idY - this->Extent[0]);
      double *rline = this->Raster[pos];
      double *rlineEnd = this->Raster[pos+1];

      if (rline == 0)
        {
        continue;
        }

      vtkstd::sort(rline, rlineEnd);

      int xy[2];
      xy[2-xj] = idY;

      int lastr = VTK_INT_MIN;

      size_t l = rlineEnd - rline;
      l = l - (l & 1); // force l to be an even number
      for (size_t k = 0; k < l; k += 2)
        {
        double x1 = rline[k] - this->Tolerance;
        double x2 = rline[k+1] + this->Tolerance;

        // make sure one of the ends is in bounds
        if (x2 < xmin || x1 >= xmax)
          {
          continue;
          }

        // clip the line segment with the bounds
        int r1 = xmin;
        int r2 = xmax;

        if (x1 >= xmin)
          {
          r1 = vtkMath::Floor(x1) + 1;
          }
        if (x2 < xmax)
          {
          r2 = vtkMath::Floor(x2);
          }

        // ensure no overlap occurs with previous
        if (r1 <= lastr)
          {
          r1 = lastr + 1;
          }
        lastr = r2;

        for (int idX = r1; idX <= r2; idX++)
          {
          xy[xj-1] = idX;
          data->InsertNextExtent(zmin, zmax, xy[0], xy[1]);
          }
        }
      }
    }
  else
    {
    // slices stacked in the y or z direction
    int zj = 3 - yj;
    int xmin = extent[0];
    int xmax = extent[1];
    int ymin = this->UsedExtent[0];
    int ymax = this->UsedExtent[1];
    int zmin = extent[2*zj];
    int zmax = extent[2*zj+1];

    // convert each raster line into extents for the stencil
    for (int idY = ymin; idY <= ymax; idY++)
      {
      size_t pos = 2*static_cast<size_t>(idY - this->Extent[0]);
      double *rline = this->Raster[pos];
      double *rlineEnd = this->Raster[pos+1];

      if (rline == 0)
        {
        continue;
        }

      vtkstd::sort(rline, rlineEnd);

      int lastr = VTK_INT_MIN;

      // go through each raster line and fill the stencil
      size_t l = rlineEnd - rline;
      l = l - (l & 1); // force l to be an even number
      for (size_t k = 0; k < l; k += 2)
        {
        int yz[2];

        yz[yj-1] = idY;
        yz[2-yj] = zmin;

        double x1 = rline[k] - this->Tolerance;
        double x2 = rline[k+1] + this->Tolerance;

        if (x2 < xmin || x1 >= xmax)
          {
          continue;
          }

        int r1 = xmin;
        int r2 = xmax;

        if (x1 >= xmin)
          {
          r1 = vtkMath::Floor(x1) + 1;
          }
        if (x2 < xmax)
          {
          r2 = vtkMath::Floor(x2);
          }

        // ensure no overlap occurs between extents
        if (r1 <= lastr)
          {
          r1 = lastr + 1;
          }
        lastr = r2;

        if (r2 >= r1)
          {
          data->InsertNextExtent(r1, r2, yz[0], yz[1]);
          }
        }
      }

    // copy the result to all other slices
    if (zmin < zmax)
      {
      for (int idY = ymin; idY <= ymax; idY++)
        {
        int r1, r2;
        int yz[2];

        yz[yj-1] = idY;
        yz[2-yj] = zmin;

        int iter = 0;
        while (data->GetNextExtent(r1, r2, xmin, xmax, yz[0], yz[1], iter))
          {
          for (int idZ = zmin + 1; idZ <= zmax; idZ++)
            {
            yz[2-yj] = idZ;
            data->InsertNextExtent(r1, r2, yz[0], yz[1]);
            }
          yz[2-yj] = zmin;
          }
        }
      }
   }
}
