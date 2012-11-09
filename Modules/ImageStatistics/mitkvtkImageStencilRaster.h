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
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageStencilData.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageStencilData - efficient description of an image stencil
// .SECTION Description
// vtkImageStencilData describes an image stencil in a manner which is
// efficient both in terms of speed and storage space.  The stencil extents
// are stored for each x-row across the image (multiple extents per row if
// necessary) and can be retrieved via the GetNextExtent() method.
// .SECTION see also
// vtkImageStencilSource vtkImageStencil

#ifndef __vtkImageStencilRaster_h
#define __vtkImageStencilRaster_h

#include "vtkImageStencilData.h"
#include "vtkDataObject.h"
#include "ImageStatisticsExports.h"


//BTX
// Description:
// This is a helper class for stencil creation.  It is a raster with
// infinite resolution in the X direction (approximately, since it uses
// double precision).  Lines that represent polygon edges can be drawn
// into this raster, and then filled given a tolerance.
class ImageStatistics_EXPORT vtkImageStencilRaster
{
public:
  // Description:
  // Create a raster with the specified whole y extent.
  vtkImageStencilRaster(const int wholeExtent[2]);

  // Description:
  // Destructor.
  ~vtkImageStencilRaster();

  // Description:
  // Reset the raster to its original state, but keep the same whole
  // extent. Pre-allocate the specified 1D allocateExtent, which must be
  // within the whole extent.
  void PrepareForNewData(const int allocateExtent[2] = 0);

  // Description:
  // Insert a line into the raster, given the two end points.
  // The "inflect1" and "inflect2" should be set if you want
  // to add a small vertical tolerance to either endpoints.
  void InsertLine(const double p1[2], const double p2[2],
                  bool inflect1, bool inflect2);

  // Description:
  // Fill the specified extent of a vtkImageStencilData with the raster,
  // after permuting the raster according to xj and yj.
  void FillStencilData(vtkImageStencilData *data, const int extent[6],
                       int xj = 0, int yj = 1);

  // Description:
  // The tolerance for float-to-int conversions.
  void SetTolerance(double tol) { this->Tolerance = tol; }
  double GetTolerance() { return this->Tolerance; }

protected:
  // Description:
  // Ensure that the raster is initialized for the specified range
  // of y values, which must be within the Extent.
  void PrepareExtent(int ymin, int ymax);

  // Description:
  // Insert an x point into the raster.  If the y value is larger
  // than the y extent, the extent will grow automatically.
  void InsertPoint(int y, double x);

  int Extent[2];
  int UsedExtent[2];
  double **Raster;
  double Tolerance;

private:
  vtkImageStencilRaster(const vtkImageStencilRaster&);  // Not implemented.
  void operator=(const vtkImageStencilRaster&);  // Not implemented.
};
//ETX

#endif



