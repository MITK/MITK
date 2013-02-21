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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_MultiShellAdcAverageReconstructionImageFilter_h_
#define _itk_MultiShellAdcAverageReconstructionImageFilter_h_

#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkPointShell.h>

namespace itk
{

/**
* \brief Select subset of the input vectors equally distributed over the sphere using an iterative electrostatic repulsion strategy.   */

  template <class TInputScalarType, class TOutputScalarType>
  class MultiShellAdcAverageReconstructionImageFilter
    : public ImageToImageFilter<itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
  {

  public:

    typedef MultiShellAdcAverageReconstructionImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TOutputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
      Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(MultiShellAdcAverageReconstructionImageFilter, ImageToImageFilter)

    typedef TInputScalarType                         InputScalarType;
    typedef itk::VectorImage<InputScalarType,3>      InputImageType;
    typedef typename InputImageType::PixelType       InputPixelType;

    typedef TOutputScalarType                         OutputScalarType;
    typedef itk::VectorImage<OutputScalarType,3>      OutputImageType;
    typedef typename OutputImageType::PixelType       OutputPixelType;

    typedef OutputScalarType                          BaselineScalarType;
    typedef BaselineScalarType                        BaselinePixelType;
    typedef typename itk::Image<BaselinePixelType,3>  BaselineImageType;

    typedef vnl_vector_fixed< double, 3 >                               GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;

    typedef std::vector<unsigned int>           IndicesVector;
    typedef std::map<double, IndicesVector>     BValueMap;

    itkGetMacro(OriginalGradientDirections, GradientDirectionContainerType::Pointer)
    itkSetMacro(OriginalGradientDirections, GradientDirectionContainerType::Pointer)

    itkGetMacro(GradientDirections, GradientDirectionContainerType::Pointer)
    itkSetMacro(GradientDirections, GradientDirectionContainerType::Pointer)

    inline void SetOriginalBValueMap(BValueMap inp){m_OriginalBValueMap = inp;}

  protected:
    MultiShellAdcAverageReconstructionImageFilter();
    ~MultiShellAdcAverageReconstructionImageFilter() {}

    void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, int threadId);

    GradientDirectionContainerType::Pointer m_GradientDirections;   ///< container for the subsampled output gradient directions
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;   ///< input gradient directions

    IndicesVector m_UsedGradientIndices;
    IndicesVector m_UnusedGradientIndices;
    IndicesVector m_BaselineImageIndices;

    BValueMap m_BValueMap;

    };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiShellAdcAverageReconstructionImageFilter.cpp"
#endif


#endif
