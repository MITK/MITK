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

#include "mitkDICOMDCMTKTagScanner.h"
#include "mitkDICOMFileReaderTestHelper.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkStringProperty.h"

class mitkDICOMDCMTKTagScannerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMDCMTKTagScannerTestSuite);

  MITK_TEST(DeepScanning);
  MITK_TEST(MultiFileScanning);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::DICOMDCMTKTagScanner::Pointer scanner;

  mitk::StringList doseFiles;
  mitk::StringList ctFiles;

public:

  void setUp() override
  {
    doseFiles.push_back(GetTestDataFilePath("RT/Dose/RD.dcm"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/100"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/101"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/102"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/104"));

    scanner = mitk::DICOMDCMTKTagScanner::New();
  }

  void tearDown() override
  {
  }

  void DeepScanning()
  {
    mitk::DICOMTagPath planUIDPath;
    planUIDPath.AddAnySelection(0x300C, 0x0002).AddElement(0x0008, 0x1155);
    mitk::DICOMTagPath planUIDPathRef;
    planUIDPathRef.AddSelection(0x300C, 0x0002, 0).AddElement(0x0008,0x1155);

    mitk::DICOMTagPath patientName(0x0010, 0x0010);

    scanner->SetInputFiles(doseFiles);
    scanner->AddTagPath(planUIDPath);
    scanner->AddTagPath(patientName);

    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", frames.size() == 1);

    mitk::DICOMDatasetAccess::FindingsListType findings = frames.front()->GetTagValueAsString(planUIDPath);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of first plan finding", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of first plan finding", findings.front().path == planUIDPathRef);
    CPPUNIT_ASSERT_MESSAGE("Testing value of first plan finding", findings.front().value == "1.2.826.0.1.3680043.8.176.2013826104526987.672.1228523524");

    findings = frames.front()->GetTagValueAsString(patientName);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of first plan finding", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of first plan finding", findings.front().path == patientName);
    CPPUNIT_ASSERT_MESSAGE("Testing value of first plan finding", findings.front().value == "L_H");
  }

  void MultiFileScanning()
  {
    mitk::DICOMTagPath instanceUID(0x0008, 0x0018);

    scanner->SetInputFiles(ctFiles);
    scanner->AddTagPath(instanceUID);

    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", frames.size() == 4);

    mitk::DICOMDatasetAccess::FindingsListType findings = frames[0]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 0", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 0", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 0", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940051");

    findings = frames[1]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 1", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 1", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 1", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940052");

    findings = frames[2]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 2", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 2", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 2", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940053");

    findings = frames[3]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 3", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 3", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 3", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940055");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMDCMTKTagScanner)
