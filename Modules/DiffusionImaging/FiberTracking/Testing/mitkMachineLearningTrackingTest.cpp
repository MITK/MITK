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

#include "mitkTestingMacros.h"
#include <mitkFiberBundle.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>
#include <itkStreamlineTrackingFilter.h>
#include <mitkTrackingHandlerRandomForest.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <omp.h>

#include "mitkTestFixture.h"

class mitkMachineLearningTrackingTestSuite : public mitk::TestFixture
{

    CPPUNIT_TEST_SUITE(mitkMachineLearningTrackingTestSuite);
    MITK_TEST(Track1);
    CPPUNIT_TEST_SUITE_END();

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;

private:

    /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
    mitk::FiberBundle::Pointer ref;
    mitk::TrackingHandlerRandomForest<6, 100>* tfh;
    mitk::Image::Pointer dwi;
    ItkUcharImgType::Pointer seed;

public:

    void setUp() override
    {
        ref = NULL;
        tfh = new mitk::TrackingHandlerRandomForest<6,100>();

        std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/ReferenceTracts.fib"));
        mitk::BaseData::Pointer baseData = fibInfile.at(0);
        ref = dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());

        dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/DiffusionImage.dwi")).GetPointer());


        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/seed.nrrd")).GetPointer());
        seed = ItkUcharImgType::New();
        mitk::CastToItkImage(img, seed);

        tfh->LoadForest(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/forest.rf"));
        tfh->AddDwi(dwi);
    }

    void tearDown() override
    {
        delete tfh;
        ref = NULL;
    }

    void Track1()
    {
        omp_set_num_threads(1);
        typedef itk::StreamlineTrackingFilter TrackerType;
        TrackerType::Pointer tracker = TrackerType::New();
        tracker->SetDemoMode(false);
        tracker->SetSeedImage(seed);
        tracker->SetSeedsPerVoxel(1);
        tracker->SetStepSize(-1);
        tracker->SetAngularThreshold(45);
        tracker->SetMinTractLength(20);
        tracker->SetMaxTractLength(400);
        tracker->SetTrackingHandler(tfh);
        tracker->SetAposterioriCurvCheck(false);
        tracker->SetAvoidStop(true);
        tracker->SetSamplingDistance(0.5);
        tracker->SetRandomSampling(false);
        tracker->Update();
        vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
        mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

        //MITK_INFO << mitk::IOUtil::GetTempPath() << "ReferenceTracts.fib";
        //mitk::IOUtil::Save(outFib, mitk::IOUtil::GetTempPath()+"ReferenceTracts.fib");

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(outFib));
    }

};

MITK_TEST_SUITE_REGISTRATION(mitkMachineLearningTracking)
