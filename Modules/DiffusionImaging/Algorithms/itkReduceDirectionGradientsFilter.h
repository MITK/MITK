/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkReduceDirectionGradientsFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_ReduceDirectionGradientsFilter_h_
#define _itk_ReduceDirectionGradientsFilter_h_

#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkPointShell.h>

namespace itk
{

  template <class TInputScalarType, class TOutputScalarType>
  class ReduceDirectionGradientsFilter
    : public ImageToImageFilter<itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
  {

  public:

    typedef ReduceDirectionGradientsFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TOutputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
      Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(ReduceDirectionGradientsFilter, ImageToImageFilter);

    typedef TInputScalarType                         InputScalarType;
    typedef itk::VectorImage<InputScalarType,3>      InputImageType;
    typedef typename InputImageType::PixelType       InputPixelType;

    typedef TOutputScalarType                         OutputScalarType;
    typedef itk::VectorImage<OutputScalarType,3>      OutputImageType;
    typedef typename OutputImageType::PixelType       OutputPixelType;

    typedef OutputScalarType                          BaselineScalarType;
    typedef BaselineScalarType                        BaselinePixelType;
    typedef typename itk::Image<BaselinePixelType,3>  BaselineImageType;

    typedef vnl_vector_fixed< double, 3 > GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;

    itkGetMacro(OriginalGradientDirections, GradientDirectionContainerType::Pointer)
    itkSetMacro(OriginalGradientDirections, GradientDirectionContainerType::Pointer)

    itkGetMacro(GradientDirections, GradientDirectionContainerType::Pointer)
    itkSetMacro(GradientDirections, GradientDirectionContainerType::Pointer)

    itkGetMacro(NumGradientDirections, int)
    itkSetMacro(NumGradientDirections, int)

    itkGetMacro(Iterations, unsigned long)
    itkSetMacro(Iterations, unsigned long)

    std::vector< int > GetUsedGradientIndices();
  protected:
    ReduceDirectionGradientsFilter();
    ~ReduceDirectionGradientsFilter() {};

    void GenerateData();
    double Costs();

    GradientDirectionContainerType::Pointer m_GradientDirections;
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;

    std::vector< int > m_UsedGradientIndices;
    std::vector< int > m_UnUsedGradientIndices;
    std::vector< int > m_BaselineImageIndices;

    int m_NumGradientDirections;
    unsigned long m_Iterations;
  };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkReduceDirectionGradientsFilter.txx"
#endif


#endif
