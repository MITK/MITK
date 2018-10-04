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

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkTemporoSpatialStringProperty.h>
#include "mitkCESTImageDetectionHelper.h"
#include "mitkCustomTagParser.h"

class mitkCESTDICOMReaderServiceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCESTDICOMReaderServiceTestSuite);

  // Test the dicom property parsing
  MITK_TEST(LoadCESTDICOMData_Success);
  MITK_TEST(LoadNormalizedCESTDICOMData_Success);
  MITK_TEST(LoadT1DICOMData_Success);

  CPPUNIT_TEST_SUITE_END();

private:


public:
  void setUp() override
  {

  }

  void tearDown() override
  {
  }

  void LoadCESTDICOMData_Success()
  {
    mitk::IFileReader::Options options;
    options["Force type"] = std::string( "Automatic" );
    options["Revision mapping"] = std::string( "Strict" );
    options["Normalize data"] = std::string("No");

    mitk::Image::Pointer cestImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("CEST/B1=0.6MUT/MEI_NER_PHANTOM.MR.E0202_MEISSNER.0587.0001.2017.10.25.22.11.10.373351.41828677.IMA"), options);
    CPPUNIT_ASSERT_MESSAGE("Make certain offsets have been correctly loaded for CEST image." ,cestImage->GetProperty(mitk::CEST_PROPERTY_NAME_OFFSETS().c_str())->GetValueAsString() == "-300 2 -2 1.92982 -1.92982 1.85965 -1.85965 1.78947 -1.78947 1.7193 -1.7193 1.64912 -1.64912 1.57895 -1.57895 1.50877 -1.50877 1.4386 -1.4386 1.36842 -1.36842 1.29825 -1.29825 1.22807 -1.22807 1.15789 -1.15789 1.08772 -1.08772 1.01754 -1.01754 0.947368 -0.947368 0.877193 -0.877193 0.807018 -0.807018 0.736842 -0.736842 0.666667 -0.666667 0.596491 -0.596491 0.526316 -0.526316 0.45614 -0.45614 0.385965 -0.385965 0.315789 -0.315789 0.245614 -0.245614 0.175439 -0.175439 0.105263 -0.105263 0.0350877 -0.0350877");
    std::string temp;
    CPPUNIT_ASSERT_MESSAGE("Make certain image is not loaded as T1.", !mitk::IsCESTT1Image(cestImage));
  }

  void LoadNormalizedCESTDICOMData_Success()
  {
    mitk::IFileReader::Options options;
    options["Force type"] = std::string("Automatic");
    options["Revision mapping"] = std::string("Strict");
    options["Normalize data"] = std::string("Automatic");

    mitk::Image::Pointer cestImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("CEST/B1=0.6MUT/MEI_NER_PHANTOM.MR.E0202_MEISSNER.0587.0001.2017.10.25.22.11.10.373351.41828677.IMA"), options);
    auto tempREsult = cestImage->GetProperty(mitk::CEST_PROPERTY_NAME_OFFSETS().c_str())->GetValueAsString();
    CPPUNIT_ASSERT_MESSAGE("Make certain offsets have been correctly loaded for CEST image.", cestImage->GetProperty(mitk::CEST_PROPERTY_NAME_OFFSETS().c_str())->GetValueAsString() == "2 -2 1.92982 -1.92982 1.85965 -1.85965 1.78947 -1.78947 1.7193 -1.7193 1.64912 -1.64912 1.57895 -1.57895 1.50877 -1.50877 1.4386 -1.4386 1.36842 -1.36842 1.29825 -1.29825 1.22807 -1.22807 1.15789 -1.15789 1.08772 -1.08772 1.01754 -1.01754 0.947368 -0.947368 0.877193 -0.877193 0.807018 -0.807018 0.736842 -0.736842 0.666667 -0.666667 0.596491 -0.596491 0.526316 -0.526316 0.45614 -0.45614 0.385965 -0.385965 0.315789 -0.315789 0.245614 -0.245614 0.175439 -0.175439 0.105263 -0.105263 0.0350877 -0.0350877 ");
    std::string temp;
    CPPUNIT_ASSERT_MESSAGE("Make certain image is not loaded as T1.", !mitk::IsCESTT1Image(cestImage));
  }

  void LoadT1DICOMData_Success()
  {
    mitk::IFileReader::Options options;
    options["Force type"] = std::string("Automatic");
    options["Revision mapping"] = std::string("Strict");

    mitk::Image::Pointer cestImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("CEST/T1MAP/MEI_NER_PHANTOM.MR.E0202_MEISSNER.0279.0001.2017.10.25.20.21.27.996834.41803047.IMA"), options);
    std::string temp;
    CPPUNIT_ASSERT_MESSAGE("Make certain image is loaded as T1.", mitk::IsCESTT1Image(cestImage));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkCESTDICOMReaderService)
