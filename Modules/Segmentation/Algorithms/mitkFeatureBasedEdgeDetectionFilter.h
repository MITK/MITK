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

#ifndef mitkFeatureBasedEdgeDetectionFilter_h_Included
#define mitkFeatureBasedEdgeDetectionFilter_h_Included

#include <MitkSegmentationExports.h>
#include <mitkImageToUnstructuredGridFilter.h>
#include <mitkUnstructuredGrid.h>

#include <itkImage.h>
#include <itkImageIterator.h>

namespace mitk
{

class MITKSEGMENTATION_EXPORT FeatureBasedEdgeDetectionFilter:
    public ImageToUnstructuredGridFilter
{

public:

  mitkClassMacro( FeatureBasedEdgeDetectionFilter, ImageToUnstructuredGridFilter)

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  itkGetMacro(EdgeImage,mitk::Image::Pointer)
  itkGetMacro(EdgePoints,mitk::Image::Pointer)
  itkGetMacro(SegmentationMask, mitk::Image::Pointer)
  itkGetMacro(thresholdImage, mitk::Image::Pointer)
  itkGetMacro(morphThreshold, mitk::Image::Pointer)
  itkGetMacro(MaskedImage, mitk::Image::Pointer)

  void SetSegmentationMask(mitk::Image::Pointer);

protected:

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  FeatureBasedEdgeDetectionFilter();

  virtual ~FeatureBasedEdgeDetectionFilter();

  template <typename TPixel, unsigned int VImageDimension>
  void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, double lower, double upper);

private:

  mitk::UnstructuredGrid::Pointer m_PointGrid;
  mitk::Image::Pointer m_SegmentationMask;
  mitk::Image::Pointer m_thresholdImage;
  mitk::Image::Pointer m_morphThreshold;
  mitk::Image::Pointer m_MaskedImage;

  mitk::Image::Pointer m_EdgeImage;
  mitk::Image::Pointer m_EdgePoints;

};

}
#endif
