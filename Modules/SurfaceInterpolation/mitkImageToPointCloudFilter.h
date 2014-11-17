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

#ifndef mitkImageToPointCloudFilter_h_Included
#define mitkImageToPointCloudFilter_h_Included

#include <MitkSurfaceInterpolationExports.h>
#include <mitkImageToSurfaceFilter.h>

#include <itkCastImageFilter.h>
#include <itkLaplacianImageFilter.h>

namespace mitk
{

struct MitkSurfaceInterpolation_EXPORT DetectConstants
{
  static const int LAPLACIAN_STD_DEV2 = 0;
  static const int LAPLACIAN_STD_DEV3 = 1;
  static const int CANNY_EDGE = 2;
  static const int THRESHOLD = 3;
};

class MitkSurfaceInterpolation_EXPORT ImageToPointCloudFilter: public ImageToSurfaceFilter
{

public:

  mitkClassMacro( ImageToPointCloudFilter, ImageToSurfaceFilter);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef itk::Image<short, 3> ImageType;
  typedef itk::Image<double, 3>  DoubleImageType;
  typedef itk::Image<double, 3> FloatImageType;
  typedef itk::CastImageFilter< ImageType, FloatImageType > ImagePTypeToFloatPTypeCasterType;
  typedef itk::LaplacianImageFilter< FloatImageType, FloatImageType > LaplacianFilterType;
  typedef int DetectionMethod;

  void SetDetectionMethod(DetectionMethod method);

protected:

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  ImageToPointCloudFilter();

  virtual ~ImageToPointCloudFilter();

private:

  template<typename TPixel, unsigned int VImageDimension>
  void StdDeviations(itk::Image<TPixel, VImageDimension>* image, double mean, double stdDev, int amount);

  void LaplacianStdDev(itk::Image<short, 3>::Pointer image, int amount);

  mitk::Surface::Pointer m_PointSurface;
  mitk::Image::Pointer m_EdgeImage;

  DetectionMethod m_method;

};

}
#endif
