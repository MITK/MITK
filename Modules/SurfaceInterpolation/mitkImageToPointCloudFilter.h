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
#include <mitkImageToUnstructuredGridFilter.h>
#include <mitkUnstructuredGrid.h>

#include <itkCastImageFilter.h>
#include <itkLaplacianImageFilter.h>

namespace mitk
{

enum MitkSurfaceInterpolation_EXPORT DetectConstant
{
  LAPLACIAN_STD_DEV2,
  LAPLACIAN_STD_DEV3,
  CANNY_EDGE,
  THRESHOLD
};

class MitkSurfaceInterpolation_EXPORT ImageToPointCloudFilter:
    public ImageToUnstructuredGridFilter
{

public:

  mitkClassMacro( ImageToPointCloudFilter, ImageToUnstructuredGridFilter)

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef itk::Image<short, 3> ImageType;
  typedef itk::Image<double, 3>  DoubleImageType;
  typedef itk::Image<double, 3> FloatImageType;
  typedef itk::LaplacianImageFilter< FloatImageType, FloatImageType >
          LaplacianFilterType;
  typedef DetectConstant DetectionMethod;

  itkGetMacro(Method,DetectionMethod)
  itkGetMacro(EdgeImage,mitk::Image::Pointer)
  itkGetMacro(EdgePoints,mitk::Image::Pointer)
  itkGetMacro(NumberOfExtractedPoints,int)

  itkSetMacro(Method,DetectionMethod)

protected:

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  ImageToPointCloudFilter();

  virtual ~ImageToPointCloudFilter();

private:

  template<typename TPixel, unsigned int VImageDimension>
  void StdDeviations(itk::Image<TPixel, VImageDimension>* image, int amount);

  void LaplacianStdDev(Image::ConstPointer image, int amount);

  mitk::UnstructuredGrid::Pointer m_PointGrid;

  mitk::BaseGeometry* m_Geometry;

  mitk::Image::Pointer m_EdgeImage;
  mitk::Image::Pointer m_EdgePoints;

  int m_NumberOfExtractedPoints;

  DetectionMethod m_Method;

};

}
#endif
