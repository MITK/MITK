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
#include <mitkFiberBundle.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <itkDiffusionTensor3D.h>
#include <mitkTestingMacros.h>
#include <omp.h>

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
        MITK_INFO << "Loading tensor image ...";
        typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
        mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(mitk::IOUtil::LoadDataNode(dtiFileName)->GetData());
        ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
        mitk::CastToItkImage(mitkTensorImage, itk_dti);

        MITK_INFO << "Loading seed image ...";
        typedef itk::Image< unsigned char, 3 >    ItkUCharImageType;
        mitk::Image::Pointer mitkSeedImage = mitk::IOUtil::LoadImage(maskFileName);

        MITK_INFO << "Loading mask image ...";
        mitk::Image::Pointer mitkMaskImage = mitk::IOUtil::LoadImage(maskFileName);

        omp_set_num_threads(1);

        // tensor tracking handler
        mitk::TrackingHandlerTensor* handler = new mitk::TrackingHandlerTensor();
        handler->SetF(tendf);
        handler->SetG(tendg);
        handler->SetInterpolate(interpolate);
        handler->SetFaThreshold(minFA);
        handler->AddTensorImage(itk_dti);

        // tracker
        typedef itk::StreamlineTrackingFilter TrackerType;
        TrackerType::Pointer tracker = TrackerType::New();
        tracker->SetSeedsPerVoxel(numSeeds);
        tracker->SetNumberOfSamples(0);
        tracker->SetStepSize(stepSize);
        tracker->SetAposterioriCurvCheck(false);
        tracker->SetSeedOnlyGm(false);
        tracker->SetTrackingHandler(handler);
        tracker->SetMinTractLength(minLength);
        tracker->SetRandom(false);

        if (mitkSeedImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage(mitkSeedImage, mask);
            tracker->SetSeedImage(mask);
        }

        if (mitkMaskImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage(mitkMaskImage, mask);
            tracker->SetMaskImage(mask);
        }

        tracker->Update();

        vtkSmartPointer<vtkPolyData> fiberBundle = tracker->GetFiberPolyData();
        mitk::FiberBundle::Pointer fib1 = mitk::FiberBundle::New(fiberBundle);

        mitk::FiberBundle::Pointer fib2 = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::LoadDataNode(referenceFileName)->GetData());
        MITK_TEST_CONDITION_REQUIRED(fib2.IsNotNull(), "Check if reference tractogram is not null.");
        bool ok = fib1->Equals(fib2);
        if (!ok)
        {
            MITK_WARN << "TEST FAILED. TRACTOGRAMS ARE NOT EQUAL!";
            mitk::IOUtil::SaveBaseData(fib1, mitk::IOUtil::GetTempPath()+"testBundle.fib");
            MITK_INFO << "OUTPUT: " << mitk::IOUtil::GetTempPath();
        }
        MITK_TEST_CONDITION_REQUIRED(ok, "Check if tractograms are equal.");

        delete handler;
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
