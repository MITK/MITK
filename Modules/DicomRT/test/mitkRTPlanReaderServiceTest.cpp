/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkImage.h"
#include "mitkTemporoSpatialStringProperty.h"
#include "mitkBaseProperty.h"
#include <mitkIOUtil.h>

class mitkRTPlanReaderServiceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTPlanReaderServiceTestSuite);
  MITK_TEST(TestProperties);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::ConstPointer m_image;

public:

  void setUp() override
  {
    m_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("RT/Plan/rtplan.dcm"));
    CPPUNIT_ASSERT_EQUAL(m_image.IsNotNull(), true);
  }

  void TestProperties()
  {
    CheckStringProperty("DICOM.300A.0010.[0].300A.0013", "1.2.246.352.72.11.320687012.17740.20090508173031");
    CheckStringProperty("DICOM.300A.0010.[1].300A.0013", "1.2.246.352.72.11.320687012.17741.20090508173031");
    CheckStringProperty("DICOM.300A.0010.[0].300A.0016", "Breast");
    CheckStringProperty("DICOM.300A.0010.[1].300A.0016", "CALC POINT");
    CheckStringProperty("DICOM.300A.0010.[0].300A.0026", "14");
    CheckStringProperty("DICOM.300A.0010.[1].300A.0026", "11.3113869239676");

    CheckStringProperty("DICOM.300A.0070.[0].300A.0078", "7");
    CheckStringProperty("DICOM.300A.0070.[0].300A.0080", "4");

    CheckStringProperty("DICOM.300A.00B0.[0].300A.00C6", "PHOTON");
    CheckStringProperty("DICOM.300A.00B0.[1].300A.00C6", "PHOTON");
    CheckStringProperty("DICOM.300A.00B0.[2].300A.00C6", "PHOTON");
    CheckStringProperty("DICOM.300A.00B0.[3].300A.00C6", "PHOTON");

    CheckStringProperty("DICOM.300C.0060.[0].0008.1155", "1.2.246.352.71.4.320687012.3190.20090511122144");
  }

  void CheckStringProperty(const std::string& propertyName, const std::string& expectedPropertyValue)
  {
    auto actualProperty = m_image->GetProperty(propertyName.c_str());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Property not found: " + propertyName, actualProperty.IsNotNull(), true);
    auto actualTemporoSpatialStringProperty = dynamic_cast<mitk::TemporoSpatialStringProperty*>(actualProperty.GetPointer());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Property has not type string: " + propertyName, actualTemporoSpatialStringProperty != nullptr, true);
    std::string actualStringProperty = actualTemporoSpatialStringProperty->GetValue();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(propertyName + " is not as expected", actualStringProperty, expectedPropertyValue);
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkRTPlanReaderService)
