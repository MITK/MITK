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
#include "itkImageIOBase.h"

#include "itkBinaryFunctorImageFilter.h"
#include "itkTernaryFunctorImageFilter.h"

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

        this->m_BaselineImage = imageTimeSelector->GetOutput();

}

mitk::Image::Pointer mitk::ConcentrationCurveGenerator::ConvertSignalToConcentrationCurve(const mitk::Image* inputImage, const mitk::Image* baselineImage)
{
    mitk::PixelType m_PixelType = inputImage->GetPixelType();
    mitk::Image::Pointer outputImage;

    if(inputImage->GetPixelType().GetComponentType() != baselineImage->GetPixelType().GetComponentType())
    {
        mitkThrow() << "Input Image and Baseline Image have different Pixel Types. Data not supported";
    }


    if(m_PixelType.GetComponentType() == itk::ImageIOBase::USHORT)
    {
        outputImage = convertToConcentration<unsigned short>(inputImage, baselineImage);
    }
    else if(m_PixelType.GetComponentType() == itk::ImageIOBase::UINT)
    {
        outputImage = convertToConcentration<unsigned int>(inputImage, baselineImage);
    }
    else if(m_PixelType.GetComponentType() == itk::ImageIOBase::INT)
    {
        outputImage = convertToConcentration<int>(inputImage, baselineImage);
    }
    else if(m_PixelType.GetComponentType() == itk::ImageIOBase::SHORT)
    {
        outputImage = convertToConcentration<short>(inputImage, baselineImage);
    }
    else if(m_PixelType.GetComponentType() == itk::ImageIOBase::DOUBLE)
    {
        outputImage = convertToConcentration<double>(inputImage, baselineImage);
    }
    else if(m_PixelType.GetComponentType() == itk::ImageIOBase::FLOAT)
    {
        outputImage = convertToConcentration<double>(inputImage, baselineImage);
    }
    else
    {
        mitkThrow() << "PixelType is "<<m_PixelType.GetComponentTypeAsString()<< ". Data Pixel Type not supported";
    }


    return outputImage;
}

template<class Tpixel>
mitk::Image::Pointer mitk::ConcentrationCurveGenerator::convertToConcentration(const mitk::Image* inputImage, const mitk::Image* baselineImage)
{
    typedef itk::Image<Tpixel, 3> InputImageType;

    typename InputImageType::Pointer itkInputImage = InputImageType::New();
    typename InputImageType::Pointer itkBaselineImage = InputImageType::New();

    mitk::CastToItkImage(inputImage, itkInputImage );
    mitk::CastToItkImage(baselineImage, itkBaselineImage );



    mitk::Image::Pointer outputImage;
    if(this->m_isT2weightedImage)
    {
        typedef mitk::ConvertT2ConcentrationFunctor <Tpixel, Tpixel, double> ConversionFunctorT2Type;
        typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorT2Type> FilterT2Type;

        ConversionFunctorT2Type ConversionT2Functor;
        ConversionT2Functor.initialize(this->m_T2Factor, this->m_T2EchoTime);

        typename FilterT2Type::Pointer ConversionT2Filter = FilterT2Type::New();

        ConversionT2Filter->SetFunctor(ConversionT2Functor);
        ConversionT2Filter->SetInput1(itkInputImage);
        ConversionT2Filter->SetInput2(itkBaselineImage);

        ConversionT2Filter->Update();

        outputImage = mitk::ImportItkImage(ConversionT2Filter->GetOutput())->Clone();
    }

    else
    {
        if(this->m_isTurboFlashSequence)
        {
            typedef mitk::ConvertToConcentrationTurboFlashFunctor <Tpixel, Tpixel, double> ConversionFunctorTurboFlashType;
            typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorTurboFlashType> FilterTurboFlashType;

            ConversionFunctorTurboFlashType ConversionTurboFlashFunctor;
            ConversionTurboFlashFunctor.initialize(this->m_RelaxationTime, this->m_Relaxivity, this->m_RecoveryTime);

            typename FilterTurboFlashType::Pointer ConversionTurboFlashFilter = FilterTurboFlashType::New();

            ConversionTurboFlashFilter->SetFunctor(ConversionTurboFlashFunctor);
            ConversionTurboFlashFilter->SetInput1(itkInputImage);
            ConversionTurboFlashFilter->SetInput2(itkBaselineImage);

            ConversionTurboFlashFilter->Update();
            outputImage = mitk::ImportItkImage(ConversionTurboFlashFilter->GetOutput())->Clone();


        }
        else if(this->m_UsingT1Map)
        {
            typename InputImageType::Pointer itkT10Image = InputImageType::New();
            mitk::CastToItkImage(m_T10Image, itkT10Image);

            typedef mitk::ConvertToConcentrationViaT1CalcFunctor <Tpixel, Tpixel, Tpixel, double> ConvertToConcentrationViaT1CalcFunctorType;
            typedef itk::TernaryFunctorImageFilter<InputImageType, InputImageType, InputImageType,ConvertedImageType, ConvertToConcentrationViaT1CalcFunctorType> FilterT1MapType;

            ConvertToConcentrationViaT1CalcFunctorType ConversionT1MapFunctor;
            ConversionT1MapFunctor.initialize(this->m_Relaxivity, this->m_RecoveryTime, this->m_FlipAngle);

            typename FilterT1MapType::Pointer ConversionT1MapFilter = FilterT1MapType::New();

            ConversionT1MapFilter->SetFunctor(ConversionT1MapFunctor);
            ConversionT1MapFilter->SetInput1(itkInputImage);
            ConversionT1MapFilter->SetInput2(itkBaselineImage);
            ConversionT1MapFilter->SetInput3(itkT10Image);

            ConversionT1MapFilter->Update();
            outputImage = mitk::ImportItkImage(ConversionT1MapFilter->GetOutput())->Clone();


        }

        else if(this->m_AbsoluteSignalEnhancement)
        {
            typedef mitk::ConvertToConcentrationAbsoluteFunctor <Tpixel, Tpixel, double> ConversionFunctorAbsoluteType;
            typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorAbsoluteType> FilterAbsoluteType;

            ConversionFunctorAbsoluteType ConversionAbsoluteFunctor;
            ConversionAbsoluteFunctor.initialize(this->m_Factor);

            typename FilterAbsoluteType::Pointer ConversionAbsoluteFilter = FilterAbsoluteType::New();

            ConversionAbsoluteFilter->SetFunctor(ConversionAbsoluteFunctor);
            ConversionAbsoluteFilter->SetInput1(itkInputImage);
            ConversionAbsoluteFilter->SetInput2(itkBaselineImage);

            ConversionAbsoluteFilter->Update();

            outputImage = mitk::ImportItkImage(ConversionAbsoluteFilter->GetOutput())->Clone();
        }

        else if(this->m_RelativeSignalEnhancement)
        {
            typedef mitk::ConvertToConcentrationRelativeFunctor <Tpixel, Tpixel, double> ConversionFunctorRelativeType;
            typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorRelativeType> FilterRelativeType;

            ConversionFunctorRelativeType ConversionRelativeFunctor;
            ConversionRelativeFunctor.initialize(this->m_Factor);

            typename FilterRelativeType::Pointer ConversionRelativeFilter = FilterRelativeType::New();

            ConversionRelativeFilter->SetFunctor(ConversionRelativeFunctor);
            ConversionRelativeFilter->SetInput1(itkInputImage);
            ConversionRelativeFilter->SetInput2(itkBaselineImage);

            ConversionRelativeFilter->Update();

            outputImage = mitk::ImportItkImage(ConversionRelativeFilter->GetOutput())->Clone();
        }
    }



    return outputImage;

}


