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

#ifndef _itk_RadialMultishellToSingleshellImageFilterr_h_
#define _itk_RadialMultishellToSingleshellImageFilterr_h_

#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include "itkDWIvoxelFunctor.h"


namespace itk
{

/**
* \brief Select subset of the input vectors equally distributed over the sphere using an iterative electrostatic repulsion strategy.   */

  template <class TInputScalarType, class TOutputScalarType>
  class RadialMultishellToSingleshellImageFilter
    : public ImageToImageFilter<itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
  {

    public:

    typedef RadialMultishellToSingleshellImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> > Superclass;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(RadialMultishellToSingleshellImageFilter, ImageToImageFilter)

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

    typedef std::vector<unsigned int>                 IndicesVector;
    typedef std::map<unsigned int, IndicesVector>     BValueMap;

    void SetOriginalGradientDirections(GradientDirectionContainerType::Pointer ptr){m_OriginalGradientDirections = ptr;}
    void SetOriginalBValue(const double & val){m_OriginalBValue = val;}
    void SetOriginalBValueMap(const BValueMap & inp){m_BValueMap = inp;}
    void SetFunctor(DWIVoxelFunctor * functor){m_Functor = functor;}
    GradientDirectionContainerType::Pointer GetTargetGradientDirections(){return m_TargetGradientDirections;}

  protected:
    RadialMultishellToSingleshellImageFilter();
    ~RadialMultishellToSingleshellImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &, ThreadIdType);

    GradientDirectionContainerType::Pointer m_TargetGradientDirections;   ///< container for the subsampled output gradient directions
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;   ///< input gradient directions

    BValueMap m_BValueMap;
    double m_OriginalBValue;
    double m_TargetBValue;

    std::vector<vnl_matrix< double > > m_ShellInterpolationMatrixVector;

    IndicesVector m_TargetDirectionsIndicies;
    unsigned int m_NumberTargetDirections;

    itk::DWIVoxelFunctor * m_Functor;

   };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRadialMultishellToSingleshellImageFilter.cpp"
#endif


#endif
