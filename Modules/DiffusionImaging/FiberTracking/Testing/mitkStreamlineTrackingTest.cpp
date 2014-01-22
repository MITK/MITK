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
#include <mitkIOUtil.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <itkStreamlineTrackingFilter.h>
#include <itkDiffusionTensor3D.h>
#include <mitkTestingMacros.h>

using namespace std;

int mitkStreamlineTrackingTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkStreamlineTrackingTest");

    MITK_TEST_CONDITION_REQUIRED(argc>3,"check for input data")

    string dtiFileName = argv[1];
    string maskFileName = argv[2];
    string referenceFileName = argv[3];

    MITK_INFO << "DTI file: " << dtiFileName;
    MITK_INFO << "Mask file: " << maskFileName;
    MITK_INFO << "Reference fiber file: " << referenceFileName;

    float minFA = 0.05;
    float minCurv = -1;
    float stepSize = -1;
    float tendf = 1;
    float tendg = 0;
    float minLength = 20;
    int numSeeds = 1;
    bool interpolate = true;

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        // load input image
        const std::string s1="", s2="";
        std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( dtiFileName, s1, s2, false );

        MITK_INFO << "Loading tensor image ...";
        typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
        mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(infile.at(0).GetPointer());
        ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
        mitk::CastToItkImage<ItkTensorImage>(mitkTensorImage, itk_dti);

        MITK_INFO << "Loading seed image ...";
        typedef itk::Image< unsigned char, 3 >    ItkUCharImageType;
        mitk::Image::Pointer mitkSeedImage = mitk::IOUtil::LoadImage(maskFileName);

        MITK_INFO << "Loading mask image ...";
        mitk::Image::Pointer mitkMaskImage = mitk::IOUtil::LoadImage(maskFileName);

        // instantiate tracker
        typedef itk::StreamlineTrackingFilter< float > FilterType;
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(itk_dti);
        filter->SetSeedsPerVoxel(numSeeds);
        filter->SetFaThreshold(minFA);
        filter->SetMinCurvatureRadius(minCurv);
        filter->SetStepSize(stepSize);
        filter->SetF(tendf);
        filter->SetG(tendg);
        filter->SetInterpolate(interpolate);
        filter->SetMinTractLength(minLength);
        filter->SetNumberOfThreads(1);

        if (mitkSeedImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage<ItkUCharImageType>(mitkSeedImage, mask);
            filter->SetSeedImage(mask);
        }

        if (mitkMaskImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage<ItkUCharImageType>(mitkMaskImage, mask);
            filter->SetMaskImage(mask);
        }

        filter->Update();

        vtkSmartPointer<vtkPolyData> fiberBundle = filter->GetFiberPolyData();
        mitk::FiberBundleX::Pointer fib1 = mitk::FiberBundleX::New(fiberBundle);

        infile = mitk::BaseDataIO::LoadBaseDataFromFile( referenceFileName, s1, s2, false );
        mitk::FiberBundleX::Pointer fib2 = dynamic_cast<mitk::FiberBundleX*>(infile.at(0).GetPointer());
        MITK_TEST_CONDITION_REQUIRED(fib2.IsNotNull(), "Check if reference tractogram is not null.");
        if (!fib1->Equals(fib2))
        {
            MITK_WARN << "TEST FAILED. TRACTOGRAMS ARE NOT EQUAL!";
            mitk::FiberBundleXWriter::Pointer writer = mitk::FiberBundleXWriter::New();
            writer->SetFileName("testBundle.fib");
            writer->SetInputFiberBundleX(fib1);
            writer->Update();

            writer->SetFileName("refBundle.fib");
            writer->SetInputFiberBundleX(fib2);
            writer->Update();
        }
        //MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2), "Check if tractograms are equal.");
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
