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

#ifndef ITKB0IMAGEEXTRACTIONTOSEPARATEIMAGEFILTER_H
#define ITKB0IMAGEEXTRACTIONTOSEPARATEIMAGEFILTER_H

#include "itkB0ImageExtractionImageFilter.h"

namespace itk
{
 /** \class B0ImageExtractionToSeparateImageFilter
   \brief This class extends the B0ImageExtractionImageFilter, it returns a time-sliced image containing
   all available b0 images for given DWI Image
   */

template< class TInputImagePixelType,
class TOutputImagePixelType >
class B0ImageExtractionToSeparateImageFilter :
    public B0ImageExtractionImageFilter< TInputImagePixelType, TOutputImagePixelType >
{
public:

  /** basic typedefs */
  typedef B0ImageExtractionImageFilter<
    TInputImagePixelType, TOutputImagePixelType > Superclass;
  typedef B0ImageExtractionToSeparateImageFilter  Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;

  /** typedefs from superclass */
  typedef typename Superclass::GradientDirectionType GradientDirectionType;
  typedef typename Superclass::GradientDirectionContainerType GradientDirectionContainerType;
  typedef typename GradientDirectionContainerType::Iterator GradContainerIteratorType;
  typedef typename Superclass::InputImageType InputImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(B0ImageExtractionToSeparateImageFilter,
    B0ImageExtractionImageFilter);

protected:

  B0ImageExtractionToSeparateImageFilter();
  virtual ~B0ImageExtractionToSeparateImageFilter(){};

  void GenerateData();

  using Superclass::m_Directions;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkB0ImageExtractionToSeparateImageFilter.txx"
#endif

#endif // ITKB0IMAGEEXTRACTIONTOSEPARATEIMAGEFILTER_H
