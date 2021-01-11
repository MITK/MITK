/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkRTConstants.h>
#include <mitkProperties.h>
#include "mitkTemporoSpatialStringProperty.h"
#include <mitkIOUtil.h>

class mitkRTDoseReaderServiceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTDoseReaderServiceTestSuite);
  MITK_TEST(TestDoseImage);
  MITK_TEST(TestProperties);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::ConstPointer m_doseImage;
  mitk::Image::ConstPointer m_referenceImage;

public:

  void setUp() override
  {
    m_referenceImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("RT/Dose/RT_Dose.nrrd"));
    m_doseImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("RT/Dose/RD.dcm"));
  }

  void TestDoseImage()
  {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("image should not be null", m_doseImage.IsNotNull(), true);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reference image and image should be equal", true, mitk::Equal(*m_doseImage, *m_referenceImage, mitk::eps, true));
  }

  void TestProperties() {
    CheckStringProperty("DICOM.0008.0060", "RTDOSE"); //Modality
    auto prescibedDoseProperty = m_doseImage->GetProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str());
    auto prescribedDoseGenericProperty = dynamic_cast<mitk::DoubleProperty*>(prescibedDoseProperty.GetPointer());
    double actualPrescribedDose = prescribedDoseGenericProperty->GetValue();
    double expectedPrescribedDose = 65535 * 0.0010494648*0.8;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("prescribed dose property is not as expected", expectedPrescribedDose, actualPrescribedDose, 1e-5);
    CheckStringProperty("DICOM.0020.000D", "1.2.826.0.1.3680043.8.176.2013826104517910.408.3433258507"); //StudyInstanceUID
    CheckStringProperty("DICOM.3004.000E", "0.0010494648"); //DoseGridScaling
    CheckStringProperty("DICOM.300C.0002.[0].0008.1155", "1.2.826.0.1.3680043.8.176.2013826104526987.672.1228523524"); //ReferencedRTPlanSequence.ReferencedSOPInstanceUID
  }

  void CheckStringProperty(const std::string& propertyName, const std::string& expectedPropertyValue)
  {
    auto actualProperty = m_doseImage->GetProperty(propertyName.c_str());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Property not found: " + propertyName, actualProperty.IsNotNull(), true);
    auto actualTemporoSpatialStringProperty = dynamic_cast<mitk::TemporoSpatialStringProperty*>(actualProperty.GetPointer());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Property has not type string: " + propertyName, actualTemporoSpatialStringProperty != nullptr, true);
    std::string actualStringProperty = actualTemporoSpatialStringProperty->GetValue();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(propertyName + " is not as expected", actualStringProperty, expectedPropertyValue);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRTDoseReaderService)
