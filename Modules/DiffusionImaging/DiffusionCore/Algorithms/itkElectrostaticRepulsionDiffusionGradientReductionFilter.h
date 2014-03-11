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
#ifndef _itk_ElectrostaticRepulsionDiffusionGradientReductionFilter_h_
#define _itk_ElectrostaticRepulsionDiffusionGradientReductionFilter_h_

#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkPointShell.h>

namespace itk
{

/**
* \brief Select subset of the input vectors equally distributed over the sphere using an iterative electrostatic repulsion strategy.   */

  template <class TInputScalarType, class TOutputScalarType>
  class ElectrostaticRepulsionDiffusionGradientReductionFilter
    : public ImageToImageFilter<itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
  {

  public:

    typedef ElectrostaticRepulsionDiffusionGradientReductionFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TOutputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
      Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ElectrostaticRepulsionDiffusionGradientReductionFilter, ImageToImageFilter)

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
    typedef std::map<unsigned int, IndicesVector>     BValueMap;

    itkGetMacro(OriginalGradientDirections, GradientDirectionContainerType::Pointer)
    itkSetMacro(OriginalGradientDirections, GradientDirectionContainerType::Pointer)

    itkGetMacro(GradientDirections, GradientDirectionContainerType::Pointer)
    itkSetMacro(GradientDirections, GradientDirectionContainerType::Pointer)

    IndicesVector GetUsedGradientIndices(){return m_UsedGradientIndices;}
    void SetOriginalBValueMap(BValueMap inp){m_OriginalBValueMap = inp;}
    void SetShellSelectionBValueMap(BValueMap inp){m_InputBValueMap = inp;}
    void SetNumGradientDirections(std::vector<unsigned int> numDirs){m_NumGradientDirections = numDirs;}

  protected:
    ElectrostaticRepulsionDiffusionGradientReductionFilter();
    ~ElectrostaticRepulsionDiffusionGradientReductionFilter() {}

    void GenerateData();
    double Costs(); ///< calculates electrostatic energy of current direction set

    GradientDirectionContainerType::Pointer m_GradientDirections;   ///< container for the subsampled output gradient directions
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;   ///< input gradient directions

    IndicesVector m_UsedGradientIndices;
    IndicesVector m_UnusedGradientIndices;
    IndicesVector m_BaselineImageIndices;

    BValueMap m_OriginalBValueMap;
    BValueMap m_InputBValueMap;

    std::vector<unsigned int> m_NumGradientDirections;
  };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkElectrostaticRepulsionDiffusionGradientReductionFilter.txx"
#endif


#endif
