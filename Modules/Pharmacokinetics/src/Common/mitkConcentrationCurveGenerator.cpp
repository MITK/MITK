/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkConcentrationCurveGenerator.h"
#include "mitkConvertToConcentrationTurboFlashFunctor.h"
#include "mitkConvertT2ConcentrationFunctor.h"
#include "mitkConvertToConcentrationViaT1Functor.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkModelBase.h"
#include "mitkExtractTimeGrid.h"
#include "mitkArbitraryTimeGeometry.h"
#include "itkNaryAddImageFilter.h"
#include "mitkImageAccessByItk.h"
#include "itkImageIOBase.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkTernaryFunctorImageFilter.h"
#include <itkExtractImageFilter.h>
#include "itkMeanProjectionImageFilter.h"

mitk::ConcentrationCurveGenerator::ConcentrationCurveGenerator() : m_isT2weightedImage(false), m_isTurboFlashSequence(false),
    m_AbsoluteSignalEnhancement(false), m_RelativeSignalEnhancement(0.0), m_UsingT1Map(false), m_Factor(0.0), m_RecoveryTime(0.0), m_RelaxationTime(0.0),
    m_Relaxivity(0.0), m_FlipAngle(0.0), m_T2Factor(0.0), m_T2EchoTime(0.0)
{
}

mitk::ConcentrationCurveGenerator::~ConcentrationCurveGenerator()
{

}

mitk::Image::Pointer mitk::ConcentrationCurveGenerator::GetConvertedImage()
{
    if(this->m_DynamicImage.IsNull())
    {
        itkExceptionMacro( << "Dynamic Image not set!");
    }
    else {
    Convert();
    }
    return m_ConvertedImage;

}

void mitk::ConcentrationCurveGenerator::Convert()
{

    mitk::Image::Pointer tempImage = mitk::Image::New();
    mitk::PixelType pixeltype = mitk::MakeScalarPixelType<double>();

    tempImage->Initialize(pixeltype,*this->m_DynamicImage->GetTimeGeometry());

    mitk::TimeGeometry::Pointer timeGeometry = (this->m_DynamicImage->GetTimeGeometry())->Clone();
    tempImage->SetTimeGeometry(timeGeometry);

    PrepareBaselineImage();
    mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(this->m_DynamicImage);

    for(unsigned int i = 0; i< this->m_DynamicImage->GetTimeSteps(); ++i)
    {
        imageTimeSelector->SetTimeNr(i);
        imageTimeSelector->UpdateLargestPossibleRegion();

        mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();
        mitk::Image::Pointer outputImage = mitk::Image::New();

        outputImage = ConvertSignalToConcentrationCurve(mitkInputImage,this->m_BaselineImage);

        mitk::ImageReadAccessor accessor(outputImage);
        tempImage->SetVolume(accessor.GetData(), i);
    }

    this->m_ConvertedImage = tempImage;

}

void mitk::ConcentrationCurveGenerator::PrepareBaselineImage()
{

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(this->m_DynamicImage);
  imageTimeSelector->SetTimeNr(0);
  imageTimeSelector->UpdateLargestPossibleRegion();
  mitk::Image::Pointer baselineImage;
  baselineImage = imageTimeSelector->GetOutput();
  
  if (m_BaselineStartTimeStep == m_BaselineEndTimeStep)
  {
    this->m_BaselineImage = imageTimeSelector->GetOutput();
  }
  else
  {
    try
    {
      AccessFixedDimensionByItk(this->m_DynamicImage, mitk::ConcentrationCurveGenerator::CalculateAverageBaselineImage, 4);
    }
    catch (itk::ExceptionObject & err)
    {
      std::cerr << "ExceptionObject in ConcentrationCurveGenerator::CalculateAverageBaselineImage caught!" << std::endl;
      std::cerr << err << std::endl;
    }
  }
}

template<class TPixel>
void mitk::ConcentrationCurveGenerator::CalculateAverageBaselineImage(const itk::Image<TPixel, 4> *itkBaselineImage)
{
  if (itkBaselineImage == NULL)
  {
    mitkThrow() << "Error in ConcentrationCurveGenerator::CalculateAverageBaselineImage. Input image is NULL.";
  }
  if (m_BaselineStartTimeStep > m_BaselineEndTimeStep)
  {
    mitkThrow() << "Error in ConcentrationCurveGenerator::CalculateAverageBaselineImage. End time point is before start time point.";
  }
  
  typedef itk::Image<TPixel, 4> TPixel4DImageType;
  typedef itk::Image<double, 3> Double3DImageType;
  typedef itk::Image<double, 4> Double4DImageType;
  typedef itk::ExtractImageFilter<TPixel4DImageType, TPixel4DImageType> ExtractImageFilterType;
  typedef itk::ExtractImageFilter<Double4DImageType, Double3DImageType> Extract3DImageFilterType;
  typedef itk::MeanProjectionImageFilter<TPixel4DImageType,Double4DImageType> MeanProjectionImageFilterType;

  typename MeanProjectionImageFilterType::Pointer MeanProjectionImageFilter = MeanProjectionImageFilterType::New();
  typename Extract3DImageFilterType::Pointer Extract3DImageFilter = Extract3DImageFilterType::New();
  typename TPixel4DImageType::RegionType region_input = itkBaselineImage->GetLargestPossibleRegion();

  if (m_BaselineEndTimeStep > region_input.GetSize()[3])
  {
    mitkThrow() << "Error in ConcentrationCurveGenerator::CalculateAverageBaselineImage. End time point is larger than total number of time points.";
  }

  typename ExtractImageFilterType::Pointer ExtractFilter = ExtractImageFilterType::New();
  typename TPixel4DImageType::Pointer baselineTimeFrameImage = TPixel4DImageType::New();
  typename TPixel4DImageType::RegionType extractionRegion;
  typename TPixel4DImageType::SizeType size_input_aux = region_input.GetSize();
  size_input_aux[3] = m_BaselineEndTimeStep - m_BaselineStartTimeStep + 1;
  typename TPixel4DImageType::IndexType start_input_aux = region_input.GetIndex();
  start_input_aux[3] = m_BaselineStartTimeStep;
  extractionRegion.SetSize(size_input_aux);
  extractionRegion.SetIndex(start_input_aux);
  ExtractFilter->SetExtractionRegion(extractionRegion);
  ExtractFilter->SetInput(itkBaselineImage);
  ExtractFilter->SetDirectionCollapseToSubmatrix();
  try
  {
    ExtractFilter->Update();
  }
  catch (itk::ExceptionObject & err)
  {
    std::cerr << "ExceptionObject caught!" << std::endl;
    std::cerr << err << std::endl;
  }
  baselineTimeFrameImage = ExtractFilter->GetOutput();
  MeanProjectionImageFilter->SetProjectionDimension(3);
  MeanProjectionImageFilter->SetInput(baselineTimeFrameImage);
  try
  {
    MeanProjectionImageFilter->Update();
  }
  catch (itk::ExceptionObject & err)
  {
    std::cerr << "ExceptionObject caught!" << std::endl;
    std::cerr << err << std::endl;
  }
  Extract3DImageFilter->SetInput(MeanProjectionImageFilter->GetOutput());
  size_input_aux[3] = 0;
  start_input_aux[3] = 0;
  extractionRegion.SetSize(size_input_aux);
  extractionRegion.SetIndex(start_input_aux);
  Extract3DImageFilter->SetExtractionRegion(extractionRegion);
  Extract3DImageFilter->SetDirectionCollapseToSubmatrix();
  try
  {
    Extract3DImageFilter->Update();
  }
  catch (itk::ExceptionObject & err)
  {
    std::cerr << "ExceptionObject caught!" << std::endl;
    std::cerr << err << std::endl;
  }

  Image::Pointer mitkBaselineImage = Image::New();
  CastToMitkImage(Extract3DImageFilter->GetOutput(), mitkBaselineImage);
  this->m_BaselineImage = mitkBaselineImage;
}


mitk::Image::Pointer mitk::ConcentrationCurveGenerator::ConvertSignalToConcentrationCurve(const mitk::Image* inputImage,const mitk::Image* baselineImage)
{
  mitk::PixelType m_PixelType = inputImage->GetPixelType();
  AccessTwoImagesFixedDimensionByItk(inputImage, baselineImage, mitk::ConcentrationCurveGenerator::convertToConcentration, 3);
  return m_ConvertSignalToConcentrationCurve_OutputImage;

}


template<class TPixel_input, class TPixel_baseline>
mitk::Image::Pointer mitk::ConcentrationCurveGenerator::convertToConcentration(const itk::Image<TPixel_input, 3> *itkInputImage, const itk::Image<TPixel_baseline, 3> *itkBaselineImage)
{
    typedef itk::Image<TPixel_input, 3> InputImageType;
    typedef itk::Image<TPixel_baseline, 3> BaselineImageType;


    if (this->m_isT2weightedImage)
    {
        typedef mitk::ConvertT2ConcentrationFunctor <TPixel_input, TPixel_baseline, double> ConversionFunctorT2Type;
        typedef itk::BinaryFunctorImageFilter<InputImageType, BaselineImageType, ConvertedImageType, ConversionFunctorT2Type> FilterT2Type;


        ConversionFunctorT2Type ConversionT2Functor;
        ConversionT2Functor.initialize(this->m_T2Factor, this->m_T2EchoTime);

        typename FilterT2Type::Pointer ConversionT2Filter = FilterT2Type::New();

        ConversionT2Filter->SetFunctor(ConversionT2Functor);
        ConversionT2Filter->SetInput1(itkInputImage);
        ConversionT2Filter->SetInput2(itkBaselineImage);

        ConversionT2Filter->Update();

        m_ConvertSignalToConcentrationCurve_OutputImage = mitk::ImportItkImage(ConversionT2Filter->GetOutput())->Clone();
      }

    else
    {
        if(this->m_isTurboFlashSequence)
        {
            typedef mitk::ConvertToConcentrationTurboFlashFunctor <TPixel_input, TPixel_baseline, double> ConversionFunctorTurboFlashType;
            typedef itk::BinaryFunctorImageFilter<InputImageType, BaselineImageType, ConvertedImageType, ConversionFunctorTurboFlashType> FilterTurboFlashType;

            ConversionFunctorTurboFlashType ConversionTurboFlashFunctor;
            ConversionTurboFlashFunctor.initialize(this->m_RelaxationTime, this->m_Relaxivity, this->m_RecoveryTime);

            typename FilterTurboFlashType::Pointer ConversionTurboFlashFilter = FilterTurboFlashType::New();

            ConversionTurboFlashFilter->SetFunctor(ConversionTurboFlashFunctor);
            ConversionTurboFlashFilter->SetInput1(itkInputImage);
            ConversionTurboFlashFilter->SetInput2(itkBaselineImage);

            ConversionTurboFlashFilter->Update();
            m_ConvertSignalToConcentrationCurve_OutputImage = mitk::ImportItkImage(ConversionTurboFlashFilter->GetOutput())->Clone();


        }
        else if(this->m_UsingT1Map)
        {
            typename ConvertedImageType::Pointer itkT10Image = ConvertedImageType::New();
            mitk::CastToItkImage(m_T10Image, itkT10Image);

            typedef mitk::ConvertToConcentrationViaT1CalcFunctor <TPixel_input, TPixel_baseline, double, double> ConvertToConcentrationViaT1CalcFunctorType;
            typedef itk::TernaryFunctorImageFilter<InputImageType, BaselineImageType, ConvertedImageType, ConvertedImageType, ConvertToConcentrationViaT1CalcFunctorType> FilterT1MapType;

            ConvertToConcentrationViaT1CalcFunctorType ConversionT1MapFunctor;
            ConversionT1MapFunctor.initialize(this->m_Relaxivity, this->m_RecoveryTime, this->m_FlipAngle);

            typename FilterT1MapType::Pointer ConversionT1MapFilter = FilterT1MapType::New();

            ConversionT1MapFilter->SetFunctor(ConversionT1MapFunctor);
            ConversionT1MapFilter->SetInput1(itkInputImage);
            ConversionT1MapFilter->SetInput2(itkBaselineImage);
            ConversionT1MapFilter->SetInput3(itkT10Image);

            ConversionT1MapFilter->Update();
            m_ConvertSignalToConcentrationCurve_OutputImage = mitk::ImportItkImage(ConversionT1MapFilter->GetOutput())->Clone();


        }

        else if(this->m_AbsoluteSignalEnhancement)
        {
            typedef mitk::ConvertToConcentrationAbsoluteFunctor <TPixel_input, TPixel_baseline, double> ConversionFunctorAbsoluteType;
            typedef itk::BinaryFunctorImageFilter<InputImageType, BaselineImageType, ConvertedImageType, ConversionFunctorAbsoluteType> FilterAbsoluteType;

            ConversionFunctorAbsoluteType ConversionAbsoluteFunctor;
            ConversionAbsoluteFunctor.initialize(this->m_Factor);

            typename FilterAbsoluteType::Pointer ConversionAbsoluteFilter = FilterAbsoluteType::New();

            ConversionAbsoluteFilter->SetFunctor(ConversionAbsoluteFunctor);
            ConversionAbsoluteFilter->SetInput1(itkInputImage);
            ConversionAbsoluteFilter->SetInput2(itkBaselineImage);

            ConversionAbsoluteFilter->Update();

            m_ConvertSignalToConcentrationCurve_OutputImage = mitk::ImportItkImage(ConversionAbsoluteFilter->GetOutput())->Clone();
        }

        else if(this->m_RelativeSignalEnhancement)
        {
            typedef mitk::ConvertToConcentrationRelativeFunctor <TPixel_input, TPixel_baseline, double> ConversionFunctorRelativeType;
            typedef itk::BinaryFunctorImageFilter<InputImageType, BaselineImageType, ConvertedImageType, ConversionFunctorRelativeType> FilterRelativeType;

            ConversionFunctorRelativeType ConversionRelativeFunctor;
            ConversionRelativeFunctor.initialize(this->m_Factor);

            typename FilterRelativeType::Pointer ConversionRelativeFilter = FilterRelativeType::New();

            ConversionRelativeFilter->SetFunctor(ConversionRelativeFunctor);
            ConversionRelativeFilter->SetInput1(itkInputImage);
            ConversionRelativeFilter->SetInput2(itkBaselineImage);

            ConversionRelativeFilter->Update();

            m_ConvertSignalToConcentrationCurve_OutputImage = mitk::ImportItkImage(ConversionRelativeFilter->GetOutput())->Clone();
        }

    }
    return m_ConvertSignalToConcentrationCurve_OutputImage;

}


