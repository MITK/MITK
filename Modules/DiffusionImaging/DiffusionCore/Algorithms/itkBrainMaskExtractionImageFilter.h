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

#ifndef __itkBrainMaskExtractionImageFilter_h_
#define __itkBrainMaskExtractionImageFilter_h_

#include "itkImageToImageFilter.h"

namespace itk{
/** \class BrainMaskExtractionImageFilter
 * \brief This class takes as input a T2-weighted image and computes a brainmask.
 *
 */

template< class TOutputImagePixelType >
class BrainMaskExtractionImageFilter :
  public ImageToImageFilter< Image< unsigned short, 3 >,
                             Image< TOutputImagePixelType, 3 > >
{

public:

  typedef BrainMaskExtractionImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< Image< unsigned short, 3 >,
                             Image< TOutputImagePixelType, 3 >  >
                          Superclass;

   /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(BrainMaskExtractionImageFilter,
                                                   ImageToImageFilter);

  typedef unsigned short                           InputPixelType;

  typedef TOutputImagePixelType                    OutputPixelType;

  typedef typename Superclass::InputImageType      InputImageType;
  typedef typename Superclass::OutputImageType     OutputImageType;

  typedef typename Superclass::OutputImageRegionType
                                                   OutputImageRegionType;

  itkSetMacro( MaxNumIterations, int )
  itkGetMacro( MaxNumIterations, int )

protected:
  BrainMaskExtractionImageFilter();
  ~BrainMaskExtractionImageFilter() {};

  void GenerateData();

  bool CompareImages( typename OutputImageType::Pointer im1, typename OutputImageType::Pointer im2);
  int ComputeHistogram( typename InputImageType::Pointer image);
  void CopyImage( typename OutputImageType::Pointer target, typename OutputImageType::Pointer source);

  int m_MaxNumIterations;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBrainMaskExtractionImageFilter.txx"
#endif

#endif //__itkBrainMaskExtractionImageFilter_h_

