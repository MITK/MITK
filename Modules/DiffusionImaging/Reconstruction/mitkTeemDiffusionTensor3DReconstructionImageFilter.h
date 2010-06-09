/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkTeemDiffusionTensor3DReconstructionImageFilter_h__
#define __mitkTeemDiffusionTensor3DReconstructionImageFilter_h__

#include "mitkImage.h"
#include "mitkTensorImage.h"
#include "mitkDiffusionImage.h"
#include "itkDiffusionTensor3D.h"

namespace mitk
{

  enum TeemTensorEstimationMethods{ 
    TeemTensorEstimationMethodsLLS,
    TeemTensorEstimationMethodsNLS,
    TeemTensorEstimationMethodsWLS,
    TeemTensorEstimationMethodsMLE,
  }; 

  template< class DiffusionImagePixelType = short, 
    class TTensorPixelType=float >
  class TeemDiffusionTensor3DReconstructionImageFilter : public itk::Object
  {
  public:

    typedef TTensorPixelType TensorPixelType;

    typedef itk::Vector<TensorPixelType,7> VectorType;
    typedef itk::Image<VectorType,3> VectorImageType;

    typedef itk::DiffusionTensor3D<TensorPixelType> TensorType;
    typedef itk::Image<TensorType,3 > ItkTensorImageType;

    typedef itk::Vector<TensorPixelType,6> ItkTensorVectorType;
    typedef itk::Image<ItkTensorVectorType,3> ItkTensorVectorImageType;

    typedef DiffusionImagePixelType DiffusionPixelType;
    typedef itk::VectorImage< DiffusionPixelType, 3 >   DiffusionImageType;

    mitkClassMacro( TeemDiffusionTensor3DReconstructionImageFilter, 
      itk::Object );
    itkNewMacro(Self);

    itkGetMacro(Input, 
      typename DiffusionImage<DiffusionImagePixelType>::Pointer);
    itkSetMacro(Input, 
      typename DiffusionImage<DiffusionImagePixelType>::Pointer);

    itkGetMacro(EstimateErrorImage, bool);
    itkSetMacro(EstimateErrorImage, bool);

    itkGetMacro(Sigma, float);
    itkSetMacro(Sigma, float);

    itkGetMacro(EstimationMethod, TeemTensorEstimationMethods);
    itkSetMacro(EstimationMethod, TeemTensorEstimationMethods);

    itkGetMacro(NumIterations, int);
    itkSetMacro(NumIterations, int);

    itkGetMacro(ConfidenceThreshold, double);
    itkSetMacro(ConfidenceThreshold, double);

    itkGetMacro(ConfidenceFuzzyness, float);
    itkSetMacro(ConfidenceFuzzyness, float);

    itkGetMacro(MinPlausibleValue, double);
    itkSetMacro(MinPlausibleValue, double);

    itkGetMacro(Output, mitk::TensorImage::Pointer);
    itkGetMacro(OutputItk, mitk::TensorImage::Pointer);

    // do the work
    virtual void Update();

  protected:

    TeemDiffusionTensor3DReconstructionImageFilter();
    virtual ~TeemDiffusionTensor3DReconstructionImageFilter();

    typename DiffusionImage<DiffusionImagePixelType>::Pointer m_Input;
    bool m_EstimateErrorImage;
    float m_Sigma;
    TeemTensorEstimationMethods m_EstimationMethod;
    int m_NumIterations;
    double m_ConfidenceThreshold;
    float m_ConfidenceFuzzyness;
    double m_MinPlausibleValue;
    mitk::TensorImage::Pointer m_Output;
    mitk::TensorImage::Pointer m_OutputItk;
    mitk::Image::Pointer m_ErrorImage;
  };

}

#include "mitkTeemDiffusionTensor3DReconstructionImageFilter.cpp"

#endif


