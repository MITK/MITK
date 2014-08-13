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

#ifndef MITKCOMPAREIMAGEDATAFILTER_H
#define MITKCOMPAREIMAGEDATAFILTER_H

//MITK
#include "mitkImageToImageFilter.h"
#include "mitkImage.h"

//ITK
#include <itkImage.h>

namespace mitk
{
/**
 * @brief A simple struct to hold the result of the comparison filter.
 */
struct CompareFilterResults
{
  void PrintSelf()
  {
    if( !m_FilterCompleted )
    {
      MITK_INFO << "Comparison filter terminated due to an exception: \n "
                << m_ExceptionMessage;

      return;
    }

    MITK_INFO << "Min. difference: " << m_MinimumDifference <<"\n"
              << "Max. difference: " << m_MaximumDifference <<"\n"
              << "Total difference: " << m_TotalDifference <<"\n"
              << "Mean difference: " << m_MeanDifference <<"\n"
              << "Number of pixels with differences: " << m_PixelsWithDifference;
  }

  double m_MinimumDifference;
  double m_MaximumDifference;

  double m_TotalDifference;
  double m_MeanDifference;
  size_t m_PixelsWithDifference;

  bool m_FilterCompleted;
  std::string m_ExceptionMessage;

};

/**
 * @brief Filter for comparing two mitk::Image objects by pixel values
 *
 * The comparison is pixel-wise, the filter uses the itk::Testing::ComparisonImageFilter
 * to find differences. The filter expects two images as input, provide them by using the SetInput( int, mitk::Image)
 * method.
 */
class MITK_CORE_EXPORT CompareImageDataFilter
    : public ImageToImageFilter
{
public:
  mitkClassMacro(CompareImageDataFilter,
                 ImageToImageFilter)

  itkSimpleNewMacro(Self)

  /**
   * @brief Get the result of the comparison
   *
   * The method compares only the number of pixels with differences. It returns true if the amount
   * is under the specified threshold. To get the complete results, use the GetCompareResults method.
   *
   * Returns false also if the itk ComparisionImageFilter raises an exception during update.
   *
   * @param threshold Allowed amount of pixels with differences
   */
  bool GetResult(size_t threshold = 0);

  /**
   * @brief Get the detailed results of the comparision run
   *
   * @sa CompareFilterResults
   */
  CompareFilterResults GetCompareResults()
  {
    return m_CompareDetails;
  }

  void SetTolerance(double eps){ m_Tolerance=eps; }

protected:
  CompareImageDataFilter();
  virtual ~CompareImageDataFilter() {}

  virtual void GenerateData();

  /*! \brief Method resets the compare detail memeber struct to its initial state */
  void ResetCompareResultsToInitial();

  /** ITK-like method which calls the ComparisionFilter on the two inputs of the filter */
  template< typename TPixel, unsigned int VImageDimension>
  void EstimateValueDifference( const itk::Image< TPixel, VImageDimension>* itkImage1,
                                const mitk::Image* referenceImage);
  bool m_CompareResult;

  CompareFilterResults m_CompareDetails;
  double m_Tolerance;
};
} // end namespace mitk

#endif // MITKCompareImageDataFilter_H
