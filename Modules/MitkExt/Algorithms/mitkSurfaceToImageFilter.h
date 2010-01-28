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
#include "MitkExtExports.h"
#include "mitkImageSource.h"
#include "mitkSurface.h"
//#include "mitkImage.h"

class vtkPolyData;

namespace mitk {

//class Mesh;
//class VectorOfContourLines;

/**
 *
 * @brief Converts surface data to pixel data. Requires a surface and an 
 * image, which header information defines the output image.
 *
 * The resulting image has the same dimension, size, and Geometry3D 
 * as the input image. The image is cut using a vtkStencil.
 * The user can decide if he wants to keep the original values or create a 
 * binary image by setting MakeBinaryOutputOn (default is \a false). If 
 * set to \a true all voxels inside the surface are set to one and all 
 * outside voxel are set to zero.
 *
 * NOTE: Since the reference input image is passed to the vtkStencil in
 * any case, the image needs to be initialized with pixel values greater than
 * the numerical minimum of the used pixel type (e.g. at least -127 for
 * unsigned char images, etc.) to produce a correct binary image
 * representation of the surface in MakeOutputBinary mode.
 * 
 * @ingroup SurfaceFilters
 * @ingroup Process
 */
class MitkExt_EXPORT SurfaceToImageFilter : public ImageSource
{
public:
  mitkClassMacro(SurfaceToImageFilter, ImageSource);
  itkNewMacro(Self);

  itkSetMacro(MakeOutputBinary, bool);
  itkGetMacro(MakeOutputBinary, bool);
  itkBooleanMacro(MakeOutputBinary);

  itkGetConstMacro(BackgroundValue,float);
  itkSetMacro(BackgroundValue,float);

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::Surface *GetInput(void);

  virtual void SetInput(const mitk::Surface *surface);

  void SetImage(const mitk::Image *source);

  const mitk::Image *GetImage(void);

protected:
  SurfaceToImageFilter();

  virtual ~SurfaceToImageFilter();

  void Stencil3DImage(int time = 0);

  bool m_MakeOutputBinary;

  float m_BackgroundValue;

};

} // namespace mitk

#endif /* MITKCOONSPATCHFILTER_H_HEADER_INCLUDED_C10B22CD */
