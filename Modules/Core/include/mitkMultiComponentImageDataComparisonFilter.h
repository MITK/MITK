/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiComponentImageDataComparisonFilter_h
#define mitkMultiComponentImageDataComparisonFilter_h

// mitk includes
#include <mitkCompareImageDataFilter.h>
#include <mitkImageToImageFilter.h>

// struct CompareFilterResults;

namespace mitk
{
  /**
   * \brief Filter for comparing two multi-channel mitk::Image objects by pixel values.
   *
   * The comparison is performed channel-wise and pixel-wise. The filter supports
   * all common pixel component types (char, short, int, long, float, double and
   * their unsigned variants).
   *
   * \ingroup Process
   * \sa CompareImageDataFilter
   * \sa ImageToImageFilter
   */
  class MITKCORE_EXPORT MultiComponentImageDataComparisonFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MultiComponentImageDataComparisonFilter, ImageToImageFilter);
    itkSimpleNewMacro(Self);

    /**
     * \brief Set the test image to compare against the valid (reference) image.
     *
     * This image is set as the second input (index 1) of the filter.
     */
    void SetTestImage(const Image *_arg);

    /** \brief Get the test image. */
    const Image *GetTestImage();

    /**
     * \brief Set the valid (reference) image for comparison.
     *
     * This image is set as the first input (index 0) of the filter.
     */
    void SetValidImage(const Image *_arg);

    /** \brief Get the valid (reference) image. */
    const Image *GetValidImage();

    /**
     * \brief Specify the tolerance for per-pixel comparison.
     *
     * Pixel differences at or below this tolerance are not counted as differences.
     * Default is 0.0.
     */
    itkSetMacro(Tolerance, double);

    /** \brief Get the tolerance for per-pixel comparison. */
    itkGetMacro(Tolerance, double);

    /**
     * \brief Set a CompareFilterResults struct to receive detailed comparison results.
     * \param results Pointer to the results struct that will be populated during GenerateData().
     */
    void SetCompareFilterResult(CompareFilterResults *results);

    /**
     * \brief Get the detailed results of the comparison run.
     * \return Pointer to the CompareFilterResults struct, or nullptr if none was set.
     * \sa CompareFilterResults
     */
    CompareFilterResults *GetCompareFilterResult();

    /**
     * \brief Get the overall result of the comparison.
     *
     * Compares the number of pixels with differences against the given threshold.
     * Returns true if the number of differing pixels is at or below the threshold.
     * Returns false if the comparison found differences above the threshold or if
     * the filter raised an exception during update.
     *
     * \param threshold Allowed number of pixels with differences. Default is 0.0.
     * \return true if the images are considered equal within tolerance and threshold.
     *
     * \sa GetCompareFilterResult
     */
    bool GetResult(double threshold = 0.0f);

  protected:
    MultiComponentImageDataComparisonFilter();

    ~MultiComponentImageDataComparisonFilter() override;

    void GenerateData() override;

    template <typename TPixel>
    void CompareMultiComponentImage(const Image *testImage, const Image *validImage);

    double m_Tolerance;
    bool m_CompareResult;

    CompareFilterResults *m_CompareDetails;
  };
} // end namespace mitk

#endif
