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

    GradientDirectionContainerType::Pointer GetOriginalGradientDirections(){return m_OriginalGradientDirections;}
    void SetOriginalGradientDirections(GradientDirectionContainerType::Pointer ptr){m_OriginalGradientDirections = ptr;}

    GradientDirectionContainerType::Pointer GetTargetGradientDirections(){return m_TargetGradientDirections;}

    double GetTargetB_Value(){return m_TargetB_Value;}
    double GetB_Value(){return m_B_Value;}
    void SetB_Value(double val){m_B_Value = val;}
    void SetOriginalBValueMap(BValueMap inp){m_B_ValueMap = inp;}
    void SetFunctor(const DWIVoxelFunctor & functor){m_functor = functor;}

  protected:
    RadialMultishellToSingleshellImageFilter();
    ~RadialMultishellToSingleshellImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType NumberOfThreads );

    GradientDirectionContainerType::Pointer m_TargetGradientDirections;   ///< container for the subsampled output gradient directions
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;   ///< input gradient directions

    BValueMap m_B_ValueMap;
    double m_B_Value;
    double m_TargetB_Value;

    std::vector<vnl_matrix< double > > m_ShellInterpolationMatrixVector;
    vnl_vector<double> m_bValueVector;
    std::vector<IndicesVector> m_bZeroIndicesSplitVectors;

    IndicesVector m_allDirectionsIndicies;
    unsigned int m_allDirectionsSize;

    DWIVoxelFunctor m_functor;

   };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRadialMultishellToSingleshellImageFilter.cpp"
#endif


#endif
