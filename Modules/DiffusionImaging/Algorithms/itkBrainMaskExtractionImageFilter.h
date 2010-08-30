/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  itkNewMacro(Self);  

  /** Runtime information support. */
  itkTypeMacro(BrainMaskExtractionImageFilter, 
                                                   ImageToImageFilter);
 
  typedef unsigned short                           InputPixelType;

  typedef TOutputImagePixelType                    OutputPixelType;

  typedef typename Superclass::InputImageType      InputImageType;
  typedef typename Superclass::OutputImageType     OutputImageType;
  
  typedef typename Superclass::OutputImageRegionType
                                                   OutputImageRegionType;

protected:
  BrainMaskExtractionImageFilter();
  ~BrainMaskExtractionImageFilter() {};

  void GenerateData();

  bool CompareImages( typename OutputImageType::Pointer im1, typename OutputImageType::Pointer im2);
  int ComputeHistogram( typename InputImageType::Pointer image);
  void CopyImage( typename OutputImageType::Pointer target, typename OutputImageType::Pointer source);
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBrainMaskExtractionImageFilter.txx"
#endif

#endif //__itkBrainMaskExtractionImageFilter_h_

