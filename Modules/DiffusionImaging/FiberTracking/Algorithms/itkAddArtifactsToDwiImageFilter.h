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

#ifndef __itkAddArtifactsToDwiImageFilter_h_
#define __itkAddArtifactsToDwiImageFilter_h_

#include "FiberTrackingExports.h"
#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkKspaceImageFilter.h>
#include <itkDftImageFilter.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>

namespace itk{

/**
* \brief Adds several artifacts to the input DWI.   */

  template< class TPixelType >
  class AddArtifactsToDwiImageFilter :
      public ImageToImageFilter< VectorImage< TPixelType, 3 >, VectorImage< TPixelType, 3 > >
  {

  public:

    typedef AddArtifactsToDwiImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TPixelType, 3 >, VectorImage< TPixelType, 3 > > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(AddArtifactsToDwiImageFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType                         DiffusionImageType;
    typedef mitk::DiffusionNoiseModel<short>                           NoiseModelType;
    typedef itk::Image< double, 2 >                                     SliceType;
    typedef typename itk::KspaceImageFilter< double >::OutputImageType  ComplexSliceType;
    typedef itk::Image<double, 3>                                       ItkDoubleImgType;
    typedef itk::Matrix<double, 3, 3>                                   MatrixType;

    void SetNoiseModel(NoiseModelType* noiseModel){ m_NoiseModel = noiseModel; }
    itkSetMacro( FrequencyMap, ItkDoubleImgType::Pointer )
    itkSetMacro( kOffset, double )
    itkSetMacro( tLine, double )
    itkSetMacro( SimulateEddyCurrents, bool )
    itkSetMacro( EddyGradientStrength, double )
    void SetGradientList(mitk::DiffusionSignalModel<double>::GradientListType list) { m_GradientList=list; }
    itkSetMacro( TE, double )
    itkSetMacro( AddGibbsRinging, bool )
    itkSetMacro( Spikes, int )
    itkSetMacro( SpikeAmplitude, double )
    itkSetMacro( Wrap, double )
    itkGetMacro( StatusText, std::string )
    itkSetMacro( UseConstantRandSeed, bool )

  protected:
    AddArtifactsToDwiImageFilter();
    ~AddArtifactsToDwiImageFilter() {}

    std::string GetTime();

    void GenerateData();

    NoiseModelType*                     m_NoiseModel;
    ItkDoubleImgType::Pointer           m_FrequencyMap;
    double                              m_kOffset;
    double                              m_tLine;
    bool                                m_SimulateEddyCurrents;
    double                              m_EddyGradientStrength;
    mitk::DiffusionSignalModel<double>::GradientListType m_GradientList;
    double                              m_TE;
    bool                                m_AddGibbsRinging;           ///< causes ringing artifacts
    int                                 m_Spikes;
    double                              m_SpikeAmplitude;
    double                              m_Wrap;
    std::string                         m_StatusText;
    time_t                              m_StartTime;
    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    bool                                m_UseConstantRandSeed;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAddArtifactsToDwiImageFilter.cpp"
#endif

#endif //__itkAddArtifactsToDwiImageFilter_h_

