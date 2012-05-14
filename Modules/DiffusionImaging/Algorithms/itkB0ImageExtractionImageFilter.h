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

#ifndef __itkB0ImageExtractionImageFilter_h_
#define __itkB0ImageExtractionImageFilter_h_

#include "itkImageToImageFilter.h"

namespace itk{
  /** \class B0ImageExtractionImageFilter
  * \brief This class takes as input a T2-weighted image and computes a brainmask.
  *
  */

  template< class TInputImagePixelType, 
  class TOutputImagePixelType >
  class B0ImageExtractionImageFilter :
    public ImageToImageFilter< VectorImage<TInputImagePixelType, 3>, 
    Image< TOutputImagePixelType, 3 > >
  {

  public:

    typedef B0ImageExtractionImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInputImagePixelType, 3 >, 
      Image< TOutputImagePixelType, 3 >  >
      Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);  

    /** Runtime information support. */
    itkTypeMacro(B0ImageExtractionImageFilter, 
      ImageToImageFilter);

    typedef TInputImagePixelType                     InputPixelType;

    typedef TOutputImagePixelType                    OutputPixelType;

    typedef typename Superclass::InputImageType      InputImageType;
    typedef typename Superclass::OutputImageType     OutputImageType;

    typedef typename Superclass::OutputImageRegionType
      OutputImageRegionType;

    typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;

    typedef itk::VectorContainer< unsigned int, GradientDirectionType >
      GradientDirectionContainerType;

    GradientDirectionContainerType::Pointer GetDirections()
    { return m_Directions; }
    void SetDirections( GradientDirectionContainerType::Pointer directions )
    { this->m_Directions = directions; }

  protected:
    
    B0ImageExtractionImageFilter();
    virtual ~B0ImageExtractionImageFilter() {};
    
    void GenerateData();

    GradientDirectionContainerType::Pointer   m_Directions;

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkB0ImageExtractionImageFilter.txx"
#endif

#endif //__itkB0ImageExtractionImageFilter_h_

