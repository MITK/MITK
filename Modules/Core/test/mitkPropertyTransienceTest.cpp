/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkIPropertyTransience.h>
#include <mitkPropertyTransience.h>

#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkSlicedData.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkPropertyTransienceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyTransienceTestSuite);

  MITK_TEST(IsTransient_AnyType);
  MITK_TEST(IsTransient_SpecificType);
  MITK_TEST(IsTransient_Subclass);
  MITK_TEST(IsTransient_NullData);
  MITK_TEST(IsTransient_UnregisteredName);
  MITK_TEST(SelectedRegisteredViaService);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_Image;
  mitk::PointSet::Pointer m_PointSet;
  mitk::IPropertyTransience *m_Service;

public:
  void setUp() override
  {
    m_Image = mitk::Image::New();
    m_PointSet = mitk::PointSet::New();
    m_Service = mitk::CreateTestInstancePropertyTransience();
  }

  void tearDown() override
  {
    m_Image = nullptr;
    m_PointSet = nullptr;
    delete m_Service;
  }

  void IsTransient_AnyType()
  {
    m_Service->AddTransient<mitk::BaseData>("selected");

    CPPUNIT_ASSERT_MESSAGE("Any-type rule matches Image", m_Service->IsTransient(m_Image, "selected"));
    CPPUNIT_ASSERT_MESSAGE("Any-type rule matches PointSet", m_Service->IsTransient(m_PointSet, "selected"));
  }

  void IsTransient_SpecificType()
  {
    m_Service->AddTransient<mitk::Image>("image.only");

    CPPUNIT_ASSERT_MESSAGE("Specific-type rule matches its type", m_Service->IsTransient(m_Image, "image.only"));
    CPPUNIT_ASSERT_MESSAGE("Specific-type rule does not match other type",
                           !m_Service->IsTransient(m_PointSet, "image.only"));
  }

  void IsTransient_Subclass()
  {
    // Image is-a SlicedData, so a rule registered for the base type must match the subclass.
    m_Service->AddTransient<mitk::SlicedData>("sliced");

    CPPUNIT_ASSERT_MESSAGE("Base-type rule matches subclass instance", m_Service->IsTransient(m_Image, "sliced"));
    CPPUNIT_ASSERT_MESSAGE("Base-type rule does not match unrelated type",
                           !m_Service->IsTransient(m_PointSet, "sliced"));
  }

  void IsTransient_NullData()
  {
    m_Service->AddTransient<mitk::BaseData>("selected");
    m_Service->AddTransient<mitk::Image>("image.only");

    CPPUNIT_ASSERT_MESSAGE("Any-type rule matches a data-less node (null data)",
                           m_Service->IsTransient(nullptr, "selected"));
    CPPUNIT_ASSERT_MESSAGE("Specific-type rule does not match null data",
                           !m_Service->IsTransient(nullptr, "image.only"));
  }

  void IsTransient_UnregisteredName()
  {
    m_Service->AddTransient<mitk::BaseData>("selected");

    CPPUNIT_ASSERT_MESSAGE("Unregistered property is not transient", !m_Service->IsTransient(m_Image, "visible"));
  }

  void SelectedRegisteredViaService()
  {
    mitk::CoreServicePointer<mitk::IPropertyTransience> transience(mitk::CoreServices::GetPropertyTransience());

    CPPUNIT_ASSERT_MESSAGE("'selected' is registered transient for any data type",
                           transience->IsTransient(m_Image, "selected"));
    CPPUNIT_ASSERT_MESSAGE("'selected' is registered transient for any data type",
                           transience->IsTransient(m_PointSet, "selected"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyTransience)
