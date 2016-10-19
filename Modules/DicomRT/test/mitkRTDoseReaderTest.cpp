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

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkRTDoseReader.h>
#include <mitkRTConstants.h>
#include <mitkGenericProperty.h>
#include <mitkIOUtil.h>

class mitkRTDoseReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTDoseReaderTestSuite);
  MITK_TEST(TestDoseImage);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::RTDoseReader::Pointer m_rtDoseReader;

public:

  void setUp() override
  {
    m_rtDoseReader = mitk::RTDoseReader::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize RTDoseReader", m_rtDoseReader.IsNotNull());
  }

  void TestDoseImage()
  {
      mitk::Image::Pointer refImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("RT/Dose/RT_Dose.nrrd"));

      mitk::DataNode::Pointer node = m_rtDoseReader->LoadRTDose(GetTestDataFilePath("RT/Dose/RD.dcm").c_str());
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

      MITK_ASSERT_EQUAL(refImage, image, "reference-Image and image should be equal");

      auto prescibedDoseProperty = image->GetProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str());
      auto prescribedDoseGenericProperty = dynamic_cast<mitk::GenericProperty<double>*>(prescibedDoseProperty.GetPointer());
      double actualPrescribedDose = prescribedDoseGenericProperty->GetValue();
      double expectedPrescribedDose = 65535 * 0.0010494648*0.8;
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("prescribed dose property is not as expected", actualPrescribedDose, expectedPrescribedDose, 1e-5);

      float actualReferenceDose;
      auto referenceDoseProperty = node->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), actualReferenceDose);
      float expectedReferenceDose = 40;

      CPPUNIT_ASSERT_EQUAL_MESSAGE("reference dose property is not as expected", static_cast<float>(actualReferenceDose), expectedReferenceDose);

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRTDoseReader)
