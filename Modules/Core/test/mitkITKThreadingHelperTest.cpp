/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>
#include <cstdlib>
#include <thread>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkITKThreadingHelper.h>

#include <itkImage.h>
#include <itkRandomImageSource.h>
#include <itksys/SystemTools.hxx>

#ifdef ITK_USE_TBB
#include <itkTBBMultiThreader.h>
#endif

class mitkITKThreadingHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkITKThreadingHelperTestSuite);
  MITK_TEST(NullFilterTest);
  MITK_TEST(ApplyTest);
  MITK_TEST(EnvironmentOverrideTest);
  CPPUNIT_TEST_SUITE_END();

  using FilterType = itk::RandomImageSource<itk::Image<float, 2>>;

public:
  void NullFilterTest()
  {
    CPPUNIT_ASSERT_THROW(mitk::UseAllLogicalProcessors(nullptr), mitk::Exception);
  }

  void ApplyTest()
  {
#ifdef ITK_USE_TBB
    if (nullptr != std::getenv("ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"))
      return; // user-configured environment: the helper is specified to be a no-op

    auto filter = FilterType::New();
    mitk::UseAllLogicalProcessors(filter);

    auto* threader = dynamic_cast<itk::TBBMultiThreader*>(filter->GetMultiThreader());
    CPPUNIT_ASSERT_MESSAGE("Testing if a TBB multi threader was set", threader != nullptr);

    const auto expected = std::min<itk::ThreadIdType>(
      std::max<itk::ThreadIdType>(itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads(),
                                  std::thread::hardware_concurrency()),
      itk::MultiThreaderBase::GetGlobalMaximumNumberOfThreads());
    CPPUNIT_ASSERT_EQUAL(expected, threader->GetMaximumNumberOfThreads());
#endif
  }

  void EnvironmentOverrideTest()
  {
    itksys::SystemTools::PutEnv("ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=2");

    auto filter = FilterType::New();
    auto* threaderBefore = filter->GetMultiThreader();
    mitk::UseAllLogicalProcessors(filter);

    CPPUNIT_ASSERT_MESSAGE("Testing if the threader stays untouched when the user configured ITK "
                           "threading explicitly",
      filter->GetMultiThreader() == threaderBefore);

    itksys::SystemTools::UnPutEnv("ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkITKThreadingHelper)
