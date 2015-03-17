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

/**
 * @brief Calculates edges and extracts them as an UnstructuredGrid with respect
 * to the given segmentation.
 *
 * At first the statistic of the grey values within the segmentation is
 * calculated. Based on this statistic a thresholding is executed. The
 * thresholded image will be processed by morphological filters. The resulting
 * image will be used for masking the input image. The masked image is used as
 * input for the ImageToPointCloudFilter, which output is an UnstructuredGrid.
 */
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

  /** Sets the segmentation for calculating the statistics within that */
  void SetSegmentationMask(mitk::Image::Pointer);

protected:

  /** This method is called by Update(). */
  virtual void GenerateData();

  /** Initializes the output information */
  virtual void GenerateOutputInformation();

  /** Constructor */
  FeatureBasedEdgeDetectionFilter();

  /** Destructor */
  virtual ~FeatureBasedEdgeDetectionFilter();

  /** Execute a thresholding filter with the given lower and upper bound */
  template <typename TPixel, unsigned int VImageDimension>
  void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, double lower, double upper);

private:

  mitk::UnstructuredGrid::Pointer m_PointGrid;

  /** The used mask given by the segmentation*/
  mitk::Image::Pointer m_SegmentationMask;

  /** The thesholded image */
  mitk::Image::Pointer m_thresholdImage;

  /** The thesholded image after the morphological filters*/
  mitk::Image::Pointer m_morphThreshold;

  /** The masked image */
  mitk::Image::Pointer m_MaskedImage;

  /** The generated image from the laplace filter */
  mitk::Image::Pointer m_EdgeImage;

  /** The extracted pixels/points */
  mitk::Image::Pointer m_EdgePoints;

};

}
#endif
