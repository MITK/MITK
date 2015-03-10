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


#ifndef MITKHEIGHTFIELDSURFACECLIPIMAGEFILTER_H_HEADER_INCLUDED
#define MITKHEIGHTFIELDSURFACECLIPIMAGEFILTER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "MitkAlgorithmsExtExports.h"
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
class MITKALGORITHMSEXT_EXPORT HeightFieldSurfaceClipImageFilter : public ImageToImageFilter
{
public:

    typedef std::vector<mitk::Surface*> ClippingPlaneList;

  mitkClassMacro(HeightFieldSurfaceClipImageFilter, ImageToImageFilter);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** \brief Set/Get the surface defining a height field as a triangle mesh */
  void SetClippingSurface( Surface *clippingSurface );

  /** \brief Set/Get the surfaces defining a height field as a triangle mesh */
  void SetClippingSurfaces( ClippingPlaneList planeList );

  /** \brief Set/Get the surface defining a height field as a triangle mesh */
  const Surface *GetClippingSurface() const;

  enum
  {
    CLIPPING_MODE_CONSTANT = 0,
    CLIPPING_MODE_MULTIPLYBYFACTOR,
    CLIPPING_MODE_MULTIPLANE
  };

  /** \brief Specifies whether clipped part of the image shall be replaced
   * by a constant or multiplied by a user-set factor */
  void SetClippingMode( int mode );

  /** \brief Specifies whether clipped part of the image shall be replaced
  * by a constant or multiplied by a user-set factor */
  int GetClippingMode();

  /** \brief Specifies whether clipped part of the image shall be replaced
  * by a constant or multiplied by a user-set factor */
  void SetClippingModeToConstant();

  /** \brief Specifies whether clipped part of the image shall be replaced
  * by a constant or multiplied by a user-set factor */
  void SetClippingModeToMultiplyByFactor();


  void SetClippingModeToMultiPlaneValue();

  /** \brief Set/Get constant gray-value for clipping in CONSTANT mode */
  itkSetMacro( ClippingConstant, ScalarType );

  /** \brief Set/Get constant gray-value for clipping in CONSTANT mode */
  itkGetConstMacro( ClippingConstant, ScalarType );

  /** \brief Set/Get multiplaction factor for clipping in MULTIPLYBYFACTOR mode */
  itkSetMacro( MultiplicationFactor, ScalarType );

  /** \brief Set/Get multiplaction factor for clipping in MULTIPLYBYFACTOR mode */
  itkGetConstMacro( MultiplicationFactor, ScalarType );

  /** \brief Set/Get x-resolution of height-field sampling (default: 256). */
  itkSetMacro( HeightFieldResolutionX, unsigned int );

  /** \brief Set/Get x-resolution of height-field sampling (default: 256). */
  itkGetConstMacro( HeightFieldResolutionX, unsigned int );

  /** \brief Set/Get y-resolution of height-field sampling (default: 256). */
  itkSetMacro( HeightFieldResolutionY, unsigned int );

  /** \brief Set/Get y-resolution of height-field sampling (default: 256). */
  itkGetConstMacro( HeightFieldResolutionY, unsigned int );

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
  unsigned m_MultiPlaneValue;

  unsigned int m_HeightFieldResolutionX;
  unsigned int m_HeightFieldResolutionY;

  double m_MaxHeight;


  /** \brief Time when Header was last initialized */
  itk::TimeStamp m_TimeOfHeaderInitialization;
};

} // namespace mitk

#endif /* MITKHEIGHTFIELDSURFACECLIPIMAGEFILTER_H_HEADER_INCLUDED */
