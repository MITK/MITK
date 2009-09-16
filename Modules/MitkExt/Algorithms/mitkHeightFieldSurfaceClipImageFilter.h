/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKHEIGHTFIELDSURFACECLIPIMAGEFILTER_H_HEADER_INCLUDED
#define MITKHEIGHTFIELDSURFACECLIPIMAGEFILTER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkSurface.h"
#include "mitkGeometry3D.h"

namespace itk {
template <class TPixel, unsigned int VImageDimension> class ITK_EXPORT Image;
}

namespace mitk {


/**
 * \brief Filter for clipping an image with an height-field represented by
 * an mitk::Surface.
 *
 * The filter assumes that the surface represents a height field as seen from
 * the plane defined by (1.0/0.0/0.0), (0.0/1.0/0.0). All voxels lying on one
 * side of the height field will be clipped, i.e., set to 0. In addition, any
 * voxels which are out of bounds when projected on this plane will be clipped
 * as well.
 *
 * \ingroup Process
 */
class MITKEXT_CORE_EXPORT HeightFieldSurfaceClipImageFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(HeightFieldSurfaceClipImageFilter, ImageToImageFilter);

  itkNewMacro(Self);

  void SetClippingSurface( Surface *clippingSurface );
  const Surface *GetClippingSurface() const;

  enum
  {
    CLIPPING_MODE_CONSTANT = 0,
    CLIPPING_MODE_MULTIPLYBYFACTOR,
  };

  void SetClippingMode( int mode );
  int GetClippingMode();
  void SetClippingModeToConstant();
  void SetClippingModeToMultiplyByFactor();

  itkSetMacro( ClippingConstant, ScalarType );
  itkGetConstMacro( ClippingConstant, ScalarType );

  itkSetMacro( MultiplicationFactor, ScalarType );
  itkGetConstMacro( MultiplicationFactor, ScalarType );

protected:
  HeightFieldSurfaceClipImageFilter();

  ~HeightFieldSurfaceClipImageFilter();

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  template < typename TPixel, unsigned int VImageDimension >
  void _InternalComputeClippedImage(
    itk::Image< TPixel, VImageDimension > *itkImage, 
    HeightFieldSurfaceClipImageFilter *clipImageFilter, 
    vtkPolyData *clippingPolyData,
    AffineTransform3D *imageToPlaneTransform );

  ImageTimeSelector::Pointer m_InputTimeSelector;
  ImageTimeSelector::Pointer m_OutputTimeSelector;

  int m_ClippingMode;
  
  ScalarType m_ClippingConstant;
  ScalarType m_MultiplicationFactor;

  int m_HeightFieldResolutionX;
  int m_HeightFieldResolutionY;

  double m_MaxHeight;


  /** \brief Time when Header was last initialized */
  itk::TimeStamp m_TimeOfHeaderInitialization;
};

} // namespace mitk

#endif /* MITKHEIGHTFIELDSURFACECLIPIMAGEFILTER_H_HEADER_INCLUDED */


