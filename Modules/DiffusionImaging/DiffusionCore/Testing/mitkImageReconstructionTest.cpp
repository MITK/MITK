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

#include <mitkImageCast.h>
#include <mitkTensorImage.h>
#include <mitkQBallImage.h>
#include <mitkIOUtil.h>
#include <itkDiffusionTensor3D.h>
#include <mitkTestingMacros.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkDiffusionQballReconstructionImageFilter.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>

using namespace std;

int mitkImageReconstructionTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkImageReconstructionTest");

    MITK_TEST_CONDITION_REQUIRED(argc>1,"check for input data")

    try
    {
        mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[1])->GetData());

        itk::VectorImage<short,3>::Pointer itkVectorImagePointer = itk::VectorImage<short,3>::New();
        mitk::CastToItkImage(dwi, itkVectorImagePointer);

        float b_value = mitk::DiffusionPropertyHelper::GetReferenceBValue( dwi );
        mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradients = mitk::DiffusionPropertyHelper::GetGradientContainer(dwi);
        {
            MITK_INFO << "Tensor reconstruction " << argv[2];
            mitk::TensorImage::Pointer tensorImage = dynamic_cast<mitk::TensorImage*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());
            typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, float > TensorReconstructionImageFilterType;
            TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
            filter->SetGradientImage( gradients, itkVectorImagePointer );
            filter->SetBValue( b_value );
            filter->Update();
            mitk::TensorImage::Pointer testImage = mitk::TensorImage::New();
            testImage->InitializeByItk( filter->GetOutput() );
            testImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(*testImage, *tensorImage, 0.0001, true), "tensor reconstruction test.");
        }

        {
            MITK_INFO << "Numerical Q-ball reconstruction " << argv[3];
            mitk::QBallImage::Pointer qballImage = dynamic_cast<mitk::QBallImage*>(mitk::IOUtil::LoadDataNode(argv[3])->GetData());
            typedef itk::DiffusionQballReconstructionImageFilter<short, short, float, QBALL_ODFSIZE> QballReconstructionImageFilterType;
            QballReconstructionImageFilterType::Pointer filter = QballReconstructionImageFilterType::New();
            filter->SetGradientImage( gradients, itkVectorImagePointer );
            filter->SetBValue( b_value );
            filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
            filter->Update();
            mitk::QBallImage::Pointer testImage = mitk::QBallImage::New();
            testImage->InitializeByItk( filter->GetOutput() );
            testImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(*testImage, *qballImage, 0.0001, true), "Numerical Q-ball reconstruction test.");
        }

        {
            MITK_INFO << "Standard Q-ball reconstruction " << argv[4];
            mitk::QBallImage::Pointer qballImage = dynamic_cast<mitk::QBallImage*>(mitk::IOUtil::LoadDataNode(argv[4])->GetData());
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( gradients, itkVectorImagePointer );
            filter->SetBValue( b_value );
            filter->SetLambda(0.006);
            filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            filter->Update();
            mitk::QBallImage::Pointer testImage = mitk::QBallImage::New();
            testImage->InitializeByItk( filter->GetOutput() );
            testImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(*testImage, *qballImage, 0.0001, true), "Standard Q-ball reconstruction test.");
        }

        {
            MITK_INFO << "CSA Q-ball reconstruction " << argv[5];
            mitk::QBallImage::Pointer qballImage = dynamic_cast<mitk::QBallImage*>(mitk::IOUtil::LoadDataNode(argv[5])->GetData());
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( gradients, itkVectorImagePointer );
            filter->SetBValue( b_value );
            filter->SetLambda(0.006);
            filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            mitk::QBallImage::Pointer testImage = mitk::QBallImage::New();
            testImage->InitializeByItk( filter->GetOutput() );
            testImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(*testImage, *qballImage, 0.0001, true), "CSA Q-ball reconstruction test.");
        }

        {
            MITK_INFO << "ADC profile reconstruction " << argv[6];
            mitk::QBallImage::Pointer qballImage = dynamic_cast<mitk::QBallImage*>(mitk::IOUtil::LoadDataNode(argv[6])->GetData());
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( gradients, itkVectorImagePointer );
            filter->SetBValue( b_value );
            filter->SetLambda(0.006);
            filter->SetNormalizationMethod(FilterType::QBAR_ADC_ONLY);
            filter->Update();
            mitk::QBallImage::Pointer testImage = mitk::QBallImage::New();
            testImage->InitializeByItk( filter->GetOutput() );
            testImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(*testImage, *qballImage, 0.0001, true), "ADC profile reconstruction test.");
        }

        {
            MITK_INFO << "Raw signal modeling " << argv[7];
            mitk::QBallImage::Pointer qballImage = dynamic_cast<mitk::QBallImage*>(mitk::IOUtil::LoadDataNode(argv[7])->GetData());
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( gradients, itkVectorImagePointer );
            filter->SetBValue( b_value );
            filter->SetLambda(0.006);
            filter->SetNormalizationMethod(FilterType::QBAR_RAW_SIGNAL);
            filter->Update();
            mitk::QBallImage::Pointer testImage = mitk::QBallImage::New();
            testImage->InitializeByItk( filter->GetOutput() );
            testImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(*testImage, *qballImage, 0.0001, true), "Raw signal modeling test.");
        }
    }
    catch (itk::ExceptionObject e)
    {
        MITK_INFO << e;
        return EXIT_FAILURE;
    }
    catch (std::exception e)
    {
        MITK_INFO << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        MITK_INFO << "ERROR!?!";
        return EXIT_FAILURE;
    }

    MITK_TEST_END();
}
