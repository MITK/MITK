/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mitkSurfaceToImageFilter_h__
#define _mitkSurfaceToImageFilter_h__

#include "mitkCommon.h"
#include "mitkImageSource.h"
#include "mitkSurface.h"
//#include "mitkImage.h"

class vtkPolyData;

namespace mitk {

//class Mesh;
//class VectorOfContourLines;

/**
 *
 * @brief Converts surface data to pixel data, one needs a surface and an image
 *
 * The resulting image as the same size as the input image with all voxels inside the 
 * surface set to one and all outside voxel set to zero.
 * 
 * @ingroup SurfaceFilters
 * @ingroup Process
 */
class SurfaceToImageFilter : public ImageSource
{
public:
  mitkClassMacro(SurfaceToImageFilter, ImageSource);
  itkNewMacro(Self);

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::Surface *GetInput(void);

  virtual void SetInput(const mitk::Surface *surface);

  void SetImage(const mitk::Image *source);

  const mitk::Image *GetImage(void);

protected:
  //##ModelId=3EF4A4A70345
  SurfaceToImageFilter();

  //##ModelId=3EF4A4A70363
  virtual ~SurfaceToImageFilter();

};

} // namespace mitk

#endif /* MITKCOONSPATCHFILTER_H_HEADER_INCLUDED_C10B22CD */
