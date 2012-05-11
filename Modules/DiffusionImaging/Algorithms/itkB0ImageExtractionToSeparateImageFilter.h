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

#include "itkImageToImageFilter.h"

namespace itk
{
/** \class B0ImageExtractionToSeparateImageFilter
   \brief This class has an advanced functionality to the B0ImageExtractionImageFilter, however the b0 images are
    stored in a 3D+t image with each b0 image beeing a separate timestep and not averaged to a single 3D volume
   */

template< class TInputImagePixelType,
          class TOutputImagePixelType >
class B0ImageExtractionToSeparateImageFilter :
    public ImageToImageFilter< VectorImage<TInputImagePixelType, 3>,
    Image< TOutputImagePixelType, 4 > >
{
public:

  typedef B0ImageExtractionToSeparateImageFilter  Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;

  typedef TInputImagePixelType                    InputPixelType;
  typedef TOutputImagePixelType                   OutputPixelType;

  typedef ImageToImageFilter< VectorImage<TInputImagePixelType, 3>,
      Image< TOutputImagePixelType, 4 > >         Superclass;

  /** typedefs from superclass */
  typedef typename Superclass::InputImageType     InputImageType;
  typedef typename Superclass::OutputImageType    OutputImageType;

  typedef typename Superclass::OutputImageRegionType
  OutputImageRegionType;

  typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;

  typedef itk::VectorContainer< unsigned int, GradientDirectionType >
  GradientDirectionContainerType;

  typedef typename GradientDirectionContainerType::Iterator GradContainerIteratorType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(B0ImageExtractionToSeparateImageFilter,
               ImageToImageFilter);

  GradientDirectionContainerType::Pointer GetDirections()
  {
    return m_Directions;
  }

  void SetDirections( GradientDirectionContainerType::Pointer directions )
  {
    this->m_Directions = directions;
  }

protected:

  B0ImageExtractionToSeparateImageFilter();
  virtual ~B0ImageExtractionToSeparateImageFilter(){};

  void GenerateData();

  GradientDirectionContainerType::Pointer m_Directions;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkB0ImageExtractionToSeparateImageFilter.txx"
#endif

#endif // ITKB0IMAGEEXTRACTIONTOSEPARATEIMAGEFILTER_H
