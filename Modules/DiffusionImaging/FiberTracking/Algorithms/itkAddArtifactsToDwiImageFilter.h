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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkAddArtifactsToDwiImageFilter_h_
#define __itkAddArtifactsToDwiImageFilter_h_

#include "FiberTrackingExports.h"
#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkKspaceImageFilter.h>
#include <itkDftImageFilter.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkGibbsRingingArtifact.h>

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
    typedef mitk::DiffusionNoiseModel<TPixelType>                       NoiseModelType;
    typedef mitk::GibbsRingingArtifact<double>                          GibbsRingingType;
    typedef itk::Image< double, 2 >                                     SliceType;
    typedef typename itk::KspaceImageFilter< double >::OutputImageType  ComplexSliceType;
    typedef itk::Image<double, 3>                                       ItkDoubleImgType;

    void SetRingingModel(GibbsRingingType* ringingModel){ m_RingingModel = ringingModel; }
    void SetNoiseModel(NoiseModelType* noiseModel){ m_NoiseModel = noiseModel; }
    itkSetMacro( FrequencyMap, ItkDoubleImgType::Pointer )
    itkSetMacro( kOffset, double )
    itkSetMacro( tLine, double )

  protected:
    AddArtifactsToDwiImageFilter();
    ~AddArtifactsToDwiImageFilter() {}

    typename ComplexSliceType::Pointer RearrangeSlice(typename ComplexSliceType::Pointer slice);

    void GenerateData();

    GibbsRingingType*                   m_RingingModel;
    NoiseModelType*                     m_NoiseModel;
    ItkDoubleImgType::Pointer           m_FrequencyMap;
    double                              m_kOffset;
    double                              m_tLine;
    double                              m_SignalScale;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAddArtifactsToDwiImageFilter.cpp"
#endif

#endif //__itkAddArtifactsToDwiImageFilter_h_

