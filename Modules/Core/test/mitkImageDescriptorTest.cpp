/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkImageDescriptor.h>
#include <mitkPixelType.h>

#include <stdexcept>

class mitkImageDescriptorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageDescriptorTestSuite);

  MITK_TEST(GetChannelTypeByName_KnownName_ReturnsPixelType);
  MITK_TEST(GetChannelTypeByName_UnknownName_Throws);
  MITK_TEST(GetChannelTypeById_OutOfRange_Throws);
  MITK_TEST(GetChannelDescriptor_OutOfRange_Throws);
  MITK_TEST(GetChannelName_OutOfRange_ReturnsPlaceholder);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::ImageDescriptor::Pointer m_Descriptor;

public:
  void setUp() override
  {
    m_Descriptor = mitk::ImageDescriptor::New();
    const unsigned int dims[2] = {4, 4};
    m_Descriptor->Initialize(dims, 2);
    m_Descriptor->AddNewChannel(mitk::MakePixelType<int, int, 1>(), "intensity");
    m_Descriptor->AddNewChannel(mitk::MakePixelType<float, float, 1>(), "probability");
  }

  void tearDown() override { m_Descriptor = nullptr; }

  void GetChannelTypeByName_KnownName_ReturnsPixelType()
  {
    const auto intType = mitk::MakePixelType<int, int, 1>();
    const auto floatType = mitk::MakePixelType<float, float, 1>();
    CPPUNIT_ASSERT(m_Descriptor->GetChannelTypeByName("intensity") == intType);
    CPPUNIT_ASSERT(m_Descriptor->GetChannelTypeByName("probability") == floatType);
  }

  void GetChannelTypeByName_UnknownName_Throws()
  {
    CPPUNIT_ASSERT_THROW(m_Descriptor->GetChannelTypeByName("does-not-exist"), std::invalid_argument);
  }

  void GetChannelTypeById_OutOfRange_Throws()
  {
    CPPUNIT_ASSERT_THROW(m_Descriptor->GetChannelTypeById(2), std::invalid_argument);
  }

  void GetChannelDescriptor_OutOfRange_Throws()
  {
    CPPUNIT_ASSERT_THROW(m_Descriptor->GetChannelDescriptor(2), std::invalid_argument);
  }

  void GetChannelName_OutOfRange_ReturnsPlaceholder()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("Out-of-range-access"), m_Descriptor->GetChannelName(2));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageDescriptor)
