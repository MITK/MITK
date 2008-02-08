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
 * The resulting image has the same size as the input image. The image is cut using a vtkStencil
 * User can decide if he wants to keep the original values or create a binary image 
 * by setting MakeBinaryOutputOn (default is false). If set to true all voxels inside the 
 * surface are set to one and all outside voxel are set to zero.
 *
 *  The pixel type of the output image is currently always integer
 * 
 * @ingroup SurfaceFilters
 * @ingroup Process
 */
class MITK_CORE_EXPORT SurfaceToImageFilter : public ImageSource
{
public:
  mitkClassMacro(SurfaceToImageFilter, ImageSource);
  itkNewMacro(Self);

  itkSetMacro(MakeOutputBinaryOn, bool);
  itkGetMacro(MakeOutputBinaryOn, bool);

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
  //##ModelId=3EF4A4A70345
  SurfaceToImageFilter();

  //##ModelId=3EF4A4A70363
  virtual ~SurfaceToImageFilter();

  void Stencil3DImage(int time = 0);

  bool m_MakeOutputBinaryOn;

  float m_BackgroundValue;

};

} // namespace mitk

#endif /* MITKCOONSPATCHFILTER_H_HEADER_INCLUDED_C10B22CD */
