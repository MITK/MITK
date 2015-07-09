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

#ifndef itSmoothedClassProbabilites_h
#define itSmoothedClassProbabilites_h

#include "itkImageToImageFilter.h"

namespace itk
{
template< class TImage>
class SmoothedClassProbabilites:public ImageToImageFilter< TImage, TImage >
{
public:
  /** Standard class typedefs. */
  typedef SmoothedClassProbabilites   Self;
  typedef ImageToImageFilter< TImage, TImage > Superclass;
  typedef SmartPointer< Self >        Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Run-time type information (and related methods). */
  itkTypeMacro(SmoothedClassProbabilites, ImageToImageFilter)

  itkSetMacro(Sigma, double)

  void SetMaskImage(TImage * mask){m_MaskImage = mask;}

protected:
  SmoothedClassProbabilites(){}
  ~SmoothedClassProbabilites(){}


  /** Does the real work. */
  virtual void GenerateData();

private:
  //LabelSampler(const Self &); //purposely not implemented
  //void operator=(const Self &);  //purposely not implemented

  double m_Sigma;
  TImage * m_MaskImage;

};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include <../src/Algorithms/itkSmoothedClassProbabilites.cpp>
#endif


#endif // itLabelSampler_h
