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
#include <mitkTractographyForest.h>
#include <mitkStreamlineTractographyParameters.h>

#include "mitkTestFixture.h"

class mitkMachineLearningTrackingTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkMachineLearningTrackingTestSuite);
  MITK_TEST(Track1);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::Image<float, 3> ItkFloatImgType;

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::FiberBundle::Pointer ref;
  mitk::TrackingHandlerRandomForest<6, 100>* tfh;
  mitk::Image::Pointer dwi;
  ItkFloatImgType::Pointer seed;

public:

  void setUp() override
  {
    ref = nullptr;
    tfh = new mitk::TrackingHandlerRandomForest<6,100>();

    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/ReferenceTracts.fib"));
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    ref = dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());

    dwi = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/DiffusionImage.dwi"));


    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/seed.nrrd"));
    seed = ItkFloatImgType::New();
    mitk::CastToItkImage(img, seed);

    mitk::TractographyForest::Pointer forest = mitk::IOUtil::Load<mitk::TractographyForest>(GetTestDataFilePath("DiffusionImaging/MachineLearningTracking/forest.rf"));

    tfh->SetForest(forest);
    tfh->AddDwi(dwi);
  }

  void tearDown() override
  {
    delete tfh;
    ref = nullptr;
  }

  void Track1()
  {
    std::shared_ptr<mitk::StreamlineTractographyParameters> params = std::make_shared<mitk::StreamlineTractographyParameters>();
    params->m_RandomSampling = false;
    params->SetSamplingDistanceVox(0.5);
    params->m_AvoidStop = true;
    params->SetAngularThresholdDeg(45);
    params->SetStepSizeVox(-1);
    params->m_SeedsPerVoxel = 1;
    params->m_InterpolateRoiImages = false;
    omp_set_num_threads(1);
    typedef itk::StreamlineTrackingFilter TrackerType;
    TrackerType::Pointer tracker = TrackerType::New();
    tracker->SetDemoMode(false);
    tracker->SetSeedImage(seed);
    tracker->SetTrackingHandler(tfh);
    tracker->SetParameters(params);
    tracker->Update();
    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    //MITK_INFO << mitk::IOUtil::GetTempPath() << "ReferenceTracts.fib";
    if (!ref->Equals(outFib))
      mitk::IOUtil::Save(outFib, mitk::IOUtil::GetTempPath()+"ML_Track1.fib");

    CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(outFib));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkMachineLearningTracking)
