/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageGenerator.h>
#include <mitkOtsuSegmentationFilter.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkCommand.h>

namespace
{
  /** \brief Counts the progress events its subject reports. */
  class ProgressCounter : public itk::Command
  {
  public:
    using Self = ProgressCounter;
    using Pointer = itk::SmartPointer<Self>;

    itkFactorylessNewMacro(Self);
    itkOverrideGetNameOfClassMacro(ProgressCounter);

    unsigned int Count = 0;

    /**
     * Both overloads count, so that the test cannot pass or fail depending on
     * which of them the observed object happens to invoke.
     */
    void Execute(itk::Object *, const itk::EventObject &) override { ++Count; }

    void Execute(const itk::Object *, const itk::EventObject &) override { ++Count; }

  protected:
    ProgressCounter() = default;
  };
}

/**
 * mitk::OtsuSegmentationFilter does its work in an ITK pipeline of its own.
 * Whoever started the operation observes the MITK filter, not that pipeline, so
 * the filter has to pass on what the pipeline reports or no progress is heard
 * at all and no notification is ever raised.
 */
class mitkOtsuSegmentationFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOtsuSegmentationFilterTestSuite);
  MITK_TEST(Update_ReportsProgress_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void Update_ReportsProgress_Success()
  {
    // Random intensities, so that Otsu has a histogram worth thresholding.
    auto input = mitk::ImageGenerator::GenerateRandomImage<unsigned short>(32, 32, 32, 1, 1.0, 1.0, 1.0, 4096, 0);
    CPPUNIT_ASSERT_MESSAGE("Test image could be generated", input.IsNotNull());

    auto filter = mitk::OtsuSegmentationFilter::New();
    filter->SetNumberOfThresholds(2);
    filter->SetInput(input);

    auto counter = ProgressCounter::New();
    filter->AddObserver(itk::ProgressEvent(), counter);

    filter->Update();

    // Not a null check: ImageSource installs output 0 in its constructor, so
    // the pointer is never null and only its content says the filter ran.
    auto output = filter->GetOutput();
    CPPUNIT_ASSERT_MESSAGE("Output was produced", output->IsInitialized());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Output covers the input",
                                 input->GetDimension(), output->GetDimension());
    CPPUNIT_ASSERT_MESSAGE("The filter has to report the progress of the pipeline inside it",
                           counter->Count > 0);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkOtsuSegmentationFilter)
