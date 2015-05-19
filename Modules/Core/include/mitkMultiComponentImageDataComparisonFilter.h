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

#ifndef MITKMULTICOMPONENTIMAGEDATACOMPARISONFILTER_H
#define MITKMULTICOMPONENTIMAGEDATACOMPARISONFILTER_H

// mitk includes
#include "mitkImageToImageFilter.h"
#include "mitkCompareImageDataFilter.h"

//struct CompareFilterResults;

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
    void SetTestImage( const Image *_arg);
    const Image* GetTestImage();

    /*! /brief
    */
    void SetValidImage( const Image *_arg);
    const Image* GetValidImage();

    /*! /brief Specify the tolerance of the image data comparison
        /param Tolerance Default is 0.0f. */
    itkSetMacro(Tolerance, double);
    itkGetMacro(Tolerance, double);

    /*! /brief
    */
    void SetCompareFilterResult( CompareFilterResults* results);

    /*! /brief Get the detailed results of the comparison run
    * /sa CompareFilterResults */
    CompareFilterResults* GetCompareFilterResult();

    /*! /brief Get the result of the comparison

    * The method compares only the number of pixels with differences. It returns true if the amount
    * is under the specified threshold. To get the complete results, use the GetCompareResults method.

    * Returns false also if the itk ComparisionImageFilter raises an exception during update.

    * /param threshold Allowed percentage of pixels with differences (between 0.0...1.0) */
    bool GetResult( double threshold = 0.0f);

  protected:

    MultiComponentImageDataComparisonFilter();

    ~MultiComponentImageDataComparisonFilter();

    virtual void GenerateData() override;

    template < typename TPixel >
    void CompareMultiComponentImage( const Image* testImage, const Image* validImage);

    double m_Tolerance;
    bool m_CompareResult;

    CompareFilterResults* m_CompareDetails;
  };
} // end namespace mitk

#endif // MITKMULTICOMPONENTIMAGEDATACOMPARISONFILTER_H
