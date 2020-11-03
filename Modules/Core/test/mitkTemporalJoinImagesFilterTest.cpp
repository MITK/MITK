/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImagePixelReadAccessor.h"

#include "mitkTemporalJoinImagesFilter.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>
#include "mitkTestDynamicImageGenerator.h"
#include "mitkStringProperty.h"
#include "mitkTemporoSpatialStringProperty.h"

class mitkTemporalJoinImagesFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTemporalJoinImagesFilterTestSuite);

  MITK_TEST(InvalidUsage);
  MITK_TEST(FuseDefault);
  MITK_TEST(FuseUseDefinedTimeBounds);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer t0;
  mitk::Image::Pointer t1;
  mitk::Image::Pointer t2;

  mitk::Image::Pointer invalidPixel;
  mitk::Image::Pointer invalidGeometry;

public:
  void setUp() override
  {
    t0 = mitk::GenerateTestFrame(1);
    t0->SetProperty("all", mitk::StringProperty::New("all_0"));
    t0->SetProperty("ts", mitk::TemporoSpatialStringProperty::New("ts_0"));

    t1 = mitk::GenerateTestFrame(20);
    t1->SetProperty("all", mitk::StringProperty::New("all_1"));
    t1->SetProperty("ts", mitk::TemporoSpatialStringProperty::New("ts_1"));
    t1->SetProperty("special", mitk::StringProperty::New("special_1"));

    t2 = mitk::GenerateTestFrame(300);
    t2->SetProperty("all", mitk::StringProperty::New("all_2"));
    t2->SetProperty("ts", mitk::TemporoSpatialStringProperty::New("ts_2"));

    invalidPixel = mitk::Image::New();
    invalidPixel->Initialize(mitk::MakePixelType<int, int, 1>(), *(t0->GetTimeGeometry()));

    invalidGeometry = mitk::Image::New();
    invalidGeometry->Initialize(t0);
    invalidGeometry->SetGeometry(mitk::Geometry3D::New());
  }

  void tearDown() override {}

  void InvalidUsage()
  {
    auto filter = mitk::TemporalJoinImagesFilter::New();

    filter->SetInput(0, t0);
    filter->SetInput(1, invalidPixel);

    CPPUNIT_ASSERT_THROW(filter->Update(), mitk::Exception);

    filter = mitk::TemporalJoinImagesFilter::New();

    filter->SetInput(0, t0);
    filter->SetInput(1, invalidGeometry);

    CPPUNIT_ASSERT_THROW(filter->Update(), mitk::Exception);

    filter = mitk::TemporalJoinImagesFilter::New();

    filter->SetInput(0, t0);
    filter->SetInput(1, t1);
    filter->SetMaxTimeBounds({ 3 });

    CPPUNIT_ASSERT_THROW(filter->Update(), mitk::Exception);
  }

  void FuseDefault()
  {
    auto filter = mitk::TemporalJoinImagesFilter::New();

    filter->SetInput(0, t0);
    filter->SetInput(1, t1);
    filter->SetInput(2, t2);

    filter->Update();
    auto output = filter->GetOutput();

    CPPUNIT_ASSERT(output->GetTimeSteps() == 3);
    auto allProp = output->GetProperty("all");
    CPPUNIT_ASSERT(allProp->GetValueAsString() == "all_0");
    auto specialProp = output->GetProperty("special");
    CPPUNIT_ASSERT(specialProp->GetValueAsString() == "special_1");
    auto tsProp = dynamic_cast<const mitk::TemporoSpatialStringProperty*>(output->GetProperty("ts").GetPointer());
    CPPUNIT_ASSERT(tsProp->GetValueByTimeStep(0) == "ts_0");
    CPPUNIT_ASSERT(tsProp->GetValueByTimeStep(1) == "ts_1");
    CPPUNIT_ASSERT(tsProp->GetValueByTimeStep(2) == "ts_2");

    CPPUNIT_ASSERT(mitk::Equal(*(t0->GetGeometry()), *(output->GetGeometry()), mitk::eps, true));
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMinimumTimePoint(0) == 0.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMaximumTimePoint(0) == 1.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMinimumTimePoint(1) == 1.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMaximumTimePoint(1) == 2.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMinimumTimePoint(2) == 2.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMaximumTimePoint(2) == 3.);

    mitk::ImagePixelReadAccessor<mitk::ScalarType, 4> accessor(output);
    itk::Index<4> testIndex0;
    testIndex0.Fill(2);
    testIndex0[3] = 0;
    itk::Index<4> testIndex1;
    testIndex1.Fill(2);
    testIndex1[3] = 1;
    itk::Index<4> testIndex2;
    testIndex2.Fill(2);
    testIndex2[3] = 2;

    CPPUNIT_ASSERT_EQUAL(28., accessor.GetPixelByIndex(testIndex0));
    CPPUNIT_ASSERT_EQUAL(180., accessor.GetPixelByIndex(testIndex1));
    CPPUNIT_ASSERT_EQUAL(2420., accessor.GetPixelByIndex(testIndex2));
  }

  void FuseUseDefinedTimeBounds()
  {
    auto filter = mitk::TemporalJoinImagesFilter::New();

    filter->SetInput(0, t0);
    filter->SetInput(1, t1);
    filter->SetInput(2, t2);

    filter->SetFirstMinTimeBound(10);
    filter->SetMaxTimeBounds({ 20,30,40 });

    filter->Update();
    auto output = filter->GetOutput();

    CPPUNIT_ASSERT(output->GetTimeSteps() == 3);
    auto allProp = output->GetProperty("all");
    CPPUNIT_ASSERT(allProp->GetValueAsString() == "all_0");
    auto specialProp = output->GetProperty("special");
    CPPUNIT_ASSERT(specialProp->GetValueAsString() == "special_1");
    auto tsProp = dynamic_cast<const mitk::TemporoSpatialStringProperty*>(output->GetProperty("ts").GetPointer());
    CPPUNIT_ASSERT(tsProp->GetValueByTimeStep(0) == "ts_0");
    CPPUNIT_ASSERT(tsProp->GetValueByTimeStep(1) == "ts_1");
    CPPUNIT_ASSERT(tsProp->GetValueByTimeStep(2) == "ts_2");

    CPPUNIT_ASSERT(mitk::Equal(*(t0->GetGeometry()), *(output->GetGeometry()), mitk::eps, true));
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMinimumTimePoint(0) == 10.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMaximumTimePoint(0) == 20.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMinimumTimePoint(1) == 20.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMaximumTimePoint(1) == 30.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMinimumTimePoint(2) == 30.);
    CPPUNIT_ASSERT(output->GetTimeGeometry()->GetMaximumTimePoint(2) == 40.);

    mitk::ImagePixelReadAccessor<mitk::ScalarType, 4> accessor(output);
    itk::Index<4> testIndex0;
    testIndex0.Fill(2);
    testIndex0[3] = 0;
    itk::Index<4> testIndex1;
    testIndex1.Fill(2);
    testIndex1[3] = 1;
    itk::Index<4> testIndex2;
    testIndex2.Fill(2);
    testIndex2[3] = 2;

    CPPUNIT_ASSERT_EQUAL(28., accessor.GetPixelByIndex(testIndex0));
    CPPUNIT_ASSERT_EQUAL(180., accessor.GetPixelByIndex(testIndex1));
    CPPUNIT_ASSERT_EQUAL(2420., accessor.GetPixelByIndex(testIndex2));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTemporalJoinImagesFilter)
