/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHistogramGenerator_h
#define mitkHistogramGenerator_h

#include "mitkImage.h"
#include <itkHistogram.h>
#include <itkImage.h>
#include <itkObject.h>

namespace mitk
{
  //##Documentation
  //## @brief Provides an easy way to calculate an itk::Histogram for a mitk::Image
  //##
  class MITKCORE_EXPORT HistogramGenerator : public itk::Object
  {
  public:
    mitkClassMacroItkParent(HistogramGenerator, itk::Object);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) typedef itk::Statistics::Histogram<double> HistogramType;

    itkSetMacro(Image, mitk::Image::ConstPointer);
    itkSetMacro(Size, int);
    itkGetConstMacro(Size, int);
    itkGetConstObjectMacro(Histogram, HistogramType);

    // TODO: calculate if needed in GetHistogram()
    void ComputeHistogram();
    float GetMaximumFrequency() const;
    static float CalculateMaximumFrequency(const HistogramType *histogram);

  protected:
    HistogramGenerator();

    ~HistogramGenerator() override;

    mitk::Image::ConstPointer m_Image;
    int m_Size;
    HistogramType::ConstPointer m_Histogram;
  };

} // namespace mitk

#endif
