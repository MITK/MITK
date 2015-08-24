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
#include <itkMLBSTrackingFilter.h>
#include <mitkTrackingForestHandler.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>

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
    mitk::TrackingForestHandler<> tfh;
    mitk::Image::Pointer dwi;
    ItkUcharImgType::Pointer seed;

public:

    void setUp() override
    {
        ref = NULL;

        std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/ReferenceTracts.fib"));
        mitk::BaseData::Pointer baseData = fibInfile.at(0);
        ref = dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());

        dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/DiffusionImage.dwi")).GetPointer());


        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/seed.nrrd")).GetPointer());
        seed = ItkUcharImgType::New();
        mitk::CastToItkImage(img, seed);

        tfh.LoadForest(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/forest.rf"));
        tfh.AddRawData(dwi);
    }

    void tearDown() override
    {
        ref = NULL;
    }

    void Track1()
    {
        typedef itk::MLBSTrackingFilter<> TrackerType;
        TrackerType::Pointer tracker = TrackerType::New();
        tracker->SetInput(0, mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi));
        tracker->SetDemoMode(false);
        tracker->SetSeedImage(seed);
        tracker->SetSeedsPerVoxel(1);
        tracker->SetStepSize(-1);
        tracker->SetMinTractLength(20);
        tracker->SetMaxTractLength(400);
        tracker->SetForestHandler(tfh);
        tracker->SetNumberOfSamples(30);
        tracker->SetAposterioriCurvCheck(false);
        tracker->SetRemoveWmEndFibers(false);
        tracker->SetAvoidStop(true);
        tracker->SetSamplingDistance(0.5);
        tracker->SetRandomSampling(false);
        tracker->Update();
        vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
        mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

//        mitk::IOUtil::Save(outFib, mitk::IOUtil::GetTempPath()+"RefFib.fib");

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(outFib));
    }

};

MITK_TEST_SUITE_REGISTRATION(mitkMachineLearningTracking)
