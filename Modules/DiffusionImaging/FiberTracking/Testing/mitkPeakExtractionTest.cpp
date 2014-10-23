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

#include <itkImageFileWriter.h>
#include <itkResampleImageFilter.h>
#include <itkFiniteDiffOdfMaximaExtractionFilter.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionImage.h>
#include <mitkQBallImage.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkTensorImage.h>

#include <mitkCoreObjectFactory.h>
#include <itkShCoefficientImageImporter.h>
#include <itkFlipImageFilter.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>

using namespace std;

int mitkPeakExtractionTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkStreamlineTrackingTest");

    MITK_TEST_CONDITION_REQUIRED(argc>3,"check for input data")

    string shCoeffFileName = argv[1];
    string maskFileName = argv[2];
    string referenceFileName = argv[3];

    MITK_INFO << "SH-coefficient file: " << shCoeffFileName;
    MITK_INFO << "Mask file: " << maskFileName;
    MITK_INFO << "Reference fiber file: " << referenceFileName;

    try
    {
        mitk::CoreObjectFactory::GetInstance();

        mitk::Image::Pointer image = mitk::IOUtil::LoadImage(shCoeffFileName);
        mitk::Image::Pointer mitkMaskImage = mitk::IOUtil::LoadImage(maskFileName);

        typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
        typedef itk::FiniteDiffOdfMaximaExtractionFilter< float, 4, 20242 > MaximaExtractionFilterType;
        MaximaExtractionFilterType::Pointer filter = MaximaExtractionFilterType::New();

        MITK_INFO << "Casting mask image ...";
        ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
        mitk::CastToItkImage(mitkMaskImage, itkMask);
        filter->SetMaskImage(itkMask);

        MITK_INFO << "Casting SH image ...";
        typedef mitk::ImageToItk< MaximaExtractionFilterType::CoefficientImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(image);
        caster->Update();
        filter->SetInput(caster->GetOutput());
        filter->SetMaxNumPeaks(2);
        filter->SetPeakThreshold(0.4);
        filter->SetAbsolutePeakThreshold(0.01);
        filter->SetAngularThreshold(25);
        filter->SetNormalizationMethod(MaximaExtractionFilterType::MAX_VEC_NORM);
        filter->SetNumberOfThreads(1);
        MITK_INFO << "Starting extraction ...";
        filter->Update();
        mitk::FiberBundleX::Pointer fib1 = filter->GetOutputFiberBundle();

        MITK_INFO << "Loading reference ...";
        const std::string s1="", s2="";
        std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( referenceFileName, s1, s2, false );
        mitk::FiberBundleX::Pointer fib2 = dynamic_cast<mitk::FiberBundleX*>(infile.at(0).GetPointer());

        // TODO: reduce epsilon. strange issues with differing values between windows and linux.
        MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2), "Check if tractograms are equal.");
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
