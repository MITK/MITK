/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCompareImageDataFilter_h
#define mitkCompareImageDataFilter_h

// MITK
#include <mitkImage.h>
#include <mitkImageToImageFilter.h>

// ITK
#include <itkImage.h>

namespace mitk
{
  /**
   * \brief Holds the detailed results of a CompareImageDataFilter comparison.
   * \sa CompareImageDataFilter
   */
  struct CompareFilterResults
  {
    /** \brief Print the comparison results to MITK_INFO. */
    void PrintSelf()
    {
      if (!m_FilterCompleted)
      {
        MITK_INFO << "Comparison filter terminated due to an exception: \n " << m_ExceptionMessage;

        return;
      }

      MITK_INFO << "Min. difference: " << m_MinimumDifference << "\n"
                << "Max. difference: " << m_MaximumDifference << "\n"
                << "Total difference: " << m_TotalDifference << "\n"
                << "Mean difference: " << m_MeanDifference << "\n"
                << "Number of pixels with differences: " << m_PixelsWithDifference;
    }

    double m_MinimumDifference;  ///< Smallest per-pixel difference found.
    double m_MaximumDifference;  ///< Largest per-pixel difference found.

    double m_TotalDifference;  ///< Sum of all per-pixel differences.
    double m_MeanDifference;  ///< Mean per-pixel difference.
    size_t m_PixelsWithDifference;  ///< Number of pixels that differ beyond tolerance.

    bool m_FilterCompleted;  ///< Whether the comparison finished without exception.
    std::string m_ExceptionMessage;  ///< Exception message if the filter did not complete.
  };

  /**
   * \brief Filter for pixel-wise comparison of two mitk::Image objects.
   *
   * Uses itk::Testing::ComparisonImageFilter internally to find per-pixel
   * differences. Provide two images via SetInput(0, image1) and
   * SetInput(1, image2). For multi-component images the
   * MultiComponentImageDataComparisonFilter is used instead.
   *
   * \ingroup Algorithms
   * \sa CompareFilterResults ImageToImageFilter
   */
  class MITKCORE_EXPORT CompareImageDataFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CompareImageDataFilter, ImageToImageFilter);
    itkSimpleNewMacro(Self);

    /**
     * \brief Check whether the comparison passes a given threshold.
     *
     * Returns true if the number of pixels with differences is at or below
     * the specified threshold. Returns false if the ITK ComparisonImageFilter
     * raised an exception during Update().
     *
     * \param threshold Maximum allowed number of pixels with differences (default 0).
     * \return \c true if the comparison passes, \c false otherwise.
     *
     * \sa GetCompareResults
     */
    bool GetResult(size_t threshold = 0);

    /**
     * \brief Get the detailed results of the comparison run.
     * \return A CompareFilterResults struct with all comparison statistics.
     * \sa CompareFilterResults
     */
    CompareFilterResults GetCompareResults() { return m_CompareDetails; }

    /**
     * \brief Set the tolerance for per-pixel difference comparison.
     * \param eps Allowed absolute difference per pixel.
     */
    void SetTolerance(double eps) { m_Tolerance = eps; }

  protected:
    /** \brief Constructor. Sets the number of required inputs to 2. */
    CompareImageDataFilter();
    /** \brief Destructor. */
    ~CompareImageDataFilter() override {}

    /** \brief Perform the pixel-wise comparison. */
    void GenerateData() override;

    /** \brief Reset the comparison detail struct to its initial state. */
    void ResetCompareResultsToInitial();

    /**
     * \brief Run itk::Testing::ComparisonImageFilter on the two inputs.
     * \tparam TPixel The pixel type of the input image.
     * \tparam VImageDimension The dimension of the input image.
     */
    template <typename TPixel, unsigned int VImageDimension>
    void EstimateValueDifference(const itk::Image<TPixel, VImageDimension> *itkImage1,
                                 const mitk::Image *referenceImage);

    bool m_CompareResult;  ///< Overall pass/fail result.
    CompareFilterResults m_CompareDetails;  ///< Detailed comparison statistics.
    double m_Tolerance;  ///< Per-pixel tolerance value.
  };
} // end namespace mitk

#endif
