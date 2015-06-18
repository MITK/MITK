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

#ifndef HISTOGRAM_GENERATOR_H_HEADER_INCLUDED
#define HISTOGRAM_GENERATOR_H_HEADER_INCLUDED

#include <itkObject.h>
#include <itkHistogram.h>
#include <itkImage.h>
#include "mitkImage.h"

namespace mitk {

//##Documentation
//## @brief Provides an easy way to calculate an itk::Histogram for a mitk::Image
//##
class MITKCORE_EXPORT HistogramGenerator : public itk::Object
{
public:
  mitkClassMacroItkParent(HistogramGenerator,itk::Object);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  typedef itk::Statistics::Histogram<double> HistogramType;

  itkSetMacro(Image,mitk::Image::ConstPointer);
  itkSetMacro(Size,int);
  itkGetConstMacro(Size,int);
  itkGetConstObjectMacro(Histogram,HistogramType);

  // TODO: calculate if needed in GetHistogram()
  void ComputeHistogram();
  float GetMaximumFrequency() const;
  static float CalculateMaximumFrequency(const HistogramType* histogram);
protected:
  HistogramGenerator();

  virtual ~HistogramGenerator();

  mitk::Image::ConstPointer m_Image;
  int m_Size;
  HistogramType::ConstPointer m_Histogram;
};

} // namespace mitk

#endif /* HISTOGRAM_GENERATOR_H_HEADER_INCLUDED */
