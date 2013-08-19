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
    typedef ImageToImageFilter< itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> > Superclass;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

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
    typedef std::map<unsigned int, IndicesVector>     BValueMap;

    GradientDirectionContainerType::Pointer GetOriginalGradientDirections(){return m_OriginalGradientDirections;}
    void SetOriginalGradientDirections(GradientDirectionContainerType::Pointer ptr){m_OriginalGradientDirections = ptr;}

    GradientDirectionContainerType::Pointer GetTargetGradientDirections(){return m_TargetGradientDirections;}

    double GetTargetB_Value(){return m_TargetB_Value;}

    double GetB_Value(){return m_B_Value;}
    void SetB_Value(double val){m_B_Value = val;}

    void SetOriginalBValueMap(BValueMap inp){m_B_ValueMap = inp;}

  protected:
    MultiShellAdcAverageReconstructionImageFilter();
    ~MultiShellAdcAverageReconstructionImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType NumberOfThreads );
    void S_S0Normalization( vnl_vector<double> & vec, const double & S0 );
    void calculateAdcFromSignal( vnl_vector<double> & vec, const double & bValue);
    void calculateSignalFromAdc( vnl_vector<double> & vec, const double & bValue, const double & referenceSignal);



    GradientDirectionContainerType::Pointer m_TargetGradientDirections;   ///< container for the subsampled output gradient directions
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;   ///< input gradient directions

    BValueMap m_B_ValueMap;
    double m_B_Value;

    double m_TargetB_Value;

    std::vector<double> m_WeightsVector;

    std::vector<vnl_matrix< double > > m_ShellInterpolationMatrixVector;
    std::vector<IndicesVector> m_bZeroIndicesSplitVectors;

    IndicesVector m_allDirectionsIndicies;

    unsigned int m_allDirectionsSize;

   };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiShellAdcAverageReconstructionImageFilter.cpp"
#endif


#endif
