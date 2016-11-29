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
#include "mitkTestFixture.h"

#include "mitkRTPlanReader.h"
#include "mitkImage.h"
#include "mitkGenericProperty.h"
#include "mitkBaseProperty.h"

class mitkRTPlanReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTPlanReaderTestSuite);
  MITK_TEST(TestProperties);
  CPPUNIT_TEST_SUITE_END();

private:
    mitk::Image::ConstPointer m_image;

public:

  void setUp() override
  {
      auto rtPlanReader = mitk::RTPlanReader();
      rtPlanReader.SetInput(GetTestDataFilePath("RT/Plan/rtplan.dcm"));
      auto readerOutput = rtPlanReader.Read();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reader output should have one entry.", static_cast<unsigned int>(1), static_cast<unsigned int>(readerOutput.size()));

      m_image = dynamic_cast<mitk::Image*>(readerOutput.at(0).GetPointer());
      CPPUNIT_ASSERT_EQUAL(m_image.IsNotNull(), true);
  }

  void TestProperties()
  {
      CheckStringProperty("DICOM.300A.0010.[*].300A.0013.0", "1.2.246.352.72.11.320687012.17740.20090508173031");
      CheckStringProperty("DICOM.300A.0010.[*].300A.0013.1", "1.2.246.352.72.11.320687012.17741.20090508173031");

      CheckStringProperty("DICOM.300A.0010.[*].300A.0016.0", "Breast");
      CheckStringProperty("DICOM.300A.0010.[*].300A.0016.1", "CALC POINT");
      
      CheckDoubleProperty("DICOM.300A.0010.[*].300A.0026.0", 14.0);
      CheckDoubleProperty("DICOM.300A.0010.[*].300A.0026.1", 11.3113869239676);
      
      CheckIntProperty("DICOM.300A.0070.[*].300A.0078.0", 7);

      CheckIntProperty("DICOM.300A.0070.[*].300A.0080.0", 4);

      CheckStringProperty("DICOM.300A.00B0.[*].300A.00C6.0", "PHOTON");

      CheckStringProperty("DICOM.300C.0060.[*].0008.1155.0", "1.2.246.352.71.4.320687012.3190.20090511122144");
  }

  void CheckStringProperty(std::string propertyName, std::string expectedPropertyValue)
  {
      auto actualProperty = m_image->GetProperty(propertyName.c_str());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Property not found: " + propertyName, actualProperty.IsNotNull(), true);
      auto actualGenericStringProperty = dynamic_cast<mitk::GenericProperty<std::string>*>(actualProperty.GetPointer());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Property has not type string: " + propertyName, actualGenericStringProperty != nullptr, true);
      std::string actualStringProperty = actualGenericStringProperty->GetValue();
      CPPUNIT_ASSERT_EQUAL_MESSAGE(propertyName + " is not as expected", actualStringProperty, expectedPropertyValue);
  }

  void CheckDoubleProperty(std::string propertyName, double expectedPropertyValue)
  {
      auto actualProperty = m_image->GetProperty(propertyName.c_str());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Property not found: " + propertyName, actualProperty.IsNotNull(), true);
      auto actualGenericDoubleProperty = dynamic_cast<mitk::GenericProperty<double>*>(actualProperty.GetPointer());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Property has not type string: " + propertyName, actualGenericDoubleProperty != nullptr, true);
      double actualDoubleProperty = actualGenericDoubleProperty->GetValue();
      CPPUNIT_ASSERT_EQUAL_MESSAGE(propertyName + " is not as expected", actualDoubleProperty, expectedPropertyValue);
  }

  void CheckIntProperty(std::string propertyName, int expectedPropertyValue)
  {
      auto actualProperty = m_image->GetProperty(propertyName.c_str());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Property not found: " + propertyName, actualProperty.IsNotNull(), true);
      auto actualGenericIntProperty = dynamic_cast<mitk::GenericProperty<int>*>(actualProperty.GetPointer());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Property has not type string: " + propertyName, actualGenericIntProperty != nullptr, true);
      int actualIntProperty = actualGenericIntProperty->GetValue();
      CPPUNIT_ASSERT_EQUAL_MESSAGE(propertyName + " is not as expected", actualIntProperty, expectedPropertyValue);
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkRTPlanReader)
