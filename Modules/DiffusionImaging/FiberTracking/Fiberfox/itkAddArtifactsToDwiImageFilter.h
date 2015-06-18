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

#include <MitkFiberTrackingExports.h>
#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkKspaceImageFilter.h>
#include <itkDftImageFilter.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkFiberfoxParameters.h>

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
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(AddArtifactsToDwiImageFilter, ImageToImageFilter)

    typedef VectorImage< TPixelType, 3 >                                InputImageType;
    typedef itk::Image< double, 2 >                                     SliceType;
    typedef typename itk::KspaceImageFilter< double >::OutputImageType  ComplexSliceType;
    typedef itk::Image<double, 3>                                       ItkDoubleImgType;
    typedef itk::Matrix<double, 3, 3>                                   MatrixType;

    itkGetMacro( StatusText, std::string )
    itkSetMacro( UseConstantRandSeed, bool )

    void SetParameters( FiberfoxParameters<short> param ){ m_Parameters = param; }
    FiberfoxParameters<short> GetParameters(){ return m_Parameters; }

    virtual void UpdateOutputInformation();

  protected:
    AddArtifactsToDwiImageFilter();
    ~AddArtifactsToDwiImageFilter() {}

    std::string GetTime();

    void GenerateData();

    FiberfoxParameters<short>                                       m_Parameters;
    std::string                                                     m_StatusText;
    time_t                                                          m_StartTime;
    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    bool                                                            m_UseConstantRandSeed;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAddArtifactsToDwiImageFilter.cpp"
#endif

#endif //__itkAddArtifactsToDwiImageFilter_h_

