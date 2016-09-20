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

#include <vector>

#include <mitkRTConstants.h>
#include <mitkGenericProperty.h>
#include <mitkIOUtil.h>
#include "mitkRTDoseReader.h"

class mitkRTDoseReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTDoseReaderTestSuite);
  MITK_TEST(TestDoseImage);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
  }

  void TestDoseImage()
  {
      mitk::Image::Pointer refImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("RT/Dose/RT_Dose.nrrd"));

      auto doseReader = mitk::RTDoseReader();
      doseReader.SetInput(GetTestDataFilePath("RT/Dose/RD.dcm"));
      std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput = doseReader.Read();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reader output should have one entry.", static_cast<unsigned int>(1), static_cast<unsigned int>(readerOutput.size()));

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(readerOutput.at(0).GetPointer());

      CPPUNIT_ASSERT_EQUAL_MESSAGE("read image should not be null", image.IsNotNull(), true);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reference image and image should be equal", true, mitk::Equal(*image, *refImage, mitk::eps, true));

      auto prescibedDoseProperty = image->GetProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str());
      auto prescribedDoseGenericProperty = dynamic_cast<mitk::GenericProperty<double>*>(prescibedDoseProperty.GetPointer());
      double actualPrescribedDose = prescribedDoseGenericProperty->GetValue();
      double expectedPrescribedDose = 65535 * 0.0010494648*0.8;
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("prescribed dose property is not as expected", expectedPrescribedDose, actualPrescribedDose, 1e-5);

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRTDoseReader)
