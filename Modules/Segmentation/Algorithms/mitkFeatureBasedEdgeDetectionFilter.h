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
  class MITKSEGMENTATION_EXPORT FeatureBasedEdgeDetectionFilter : public ImageToUnstructuredGridFilter
  {
  public:
    mitkClassMacro(FeatureBasedEdgeDetectionFilter, ImageToUnstructuredGridFilter)

      itkFactorylessNewMacro(Self)

      /** Sets the segmentation for calculating the statistics within that */
      void SetSegmentationMask(mitk::Image::Pointer);

  protected:
    /** This method is called by Update(). */
    void GenerateData() override;

    /** Initializes the output information */
    void GenerateOutputInformation() override;

    /** Constructor */
    FeatureBasedEdgeDetectionFilter();

    /** Destructor */
    ~FeatureBasedEdgeDetectionFilter() override;

    /** Execute a thresholding filter with the given lower and upper bound */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding(const itk::Image<TPixel, VImageDimension> *originalImage,
                         double lower,
                         double upper,
                         mitk::Image::Pointer &result);

    template <typename TPixel, unsigned int VImageDimension>
    void ContourSearch(itk::Image<TPixel, VImageDimension> *originalImage, mitk::Image::Pointer &result);

    template <typename TPixel, unsigned int VImageDimension>
    void ThreadedClosing(itk::Image<TPixel, VImageDimension> *originalImage, mitk::Image::Pointer &result);

  private:
    mitk::UnstructuredGrid::Pointer m_PointGrid;

    /** The used mask given by the segmentation*/
    mitk::Image::Pointer m_SegmentationMask;
  };
}
#endif
