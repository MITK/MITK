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
#include "mitkCompareImageDataFilter.h"
#include "mitkImageToImageFilter.h"

// struct CompareFilterResults;

namespace mitk
{
  /*! Documentation:
  * \brief Filter for comparing two multi channel mitk::Image objects by channel wise by pixel values
  *
  * The comparison is channel- / pixel-wise.
  */
  class MITKCORE_EXPORT MultiComponentImageDataComparisonFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MultiComponentImageDataComparisonFilter, ImageToImageFilter);
    itkSimpleNewMacro(Self);

    /*! /brief
    */
    void SetTestImage(const Image *_arg);
    const Image *GetTestImage();

    /*! /brief
    */
    void SetValidImage(const Image *_arg);
    const Image *GetValidImage();

    /*! /brief Specify the tolerance of the image data comparison
        /param Tolerance Default is 0.0f. */
    itkSetMacro(Tolerance, double);
    itkGetMacro(Tolerance, double);

    /*! /brief
    */
    void SetCompareFilterResult(CompareFilterResults *results);

    /*! /brief Get the detailed results of the comparison run
    * /sa CompareFilterResults */
    CompareFilterResults *GetCompareFilterResult();

    /*! /brief Get the result of the comparison

    * The method compares only the number of pixels with differences. It returns true if the amount
    * is under the specified threshold. To get the complete results, use the GetCompareResults method.

    * Returns false also if the itk ComparisonImageFilter raises an exception during update.

    * /param threshold Allowed percentage of pixels with differences (between 0.0...1.0) */
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
