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

// mitk semantic relations
#include "mitkSemanticRelationException.h"
#include "mitkSemanticRelationsInference.h"
#include "mitkSemanticRelationsIntegration.h"
#include "mitkSemanticRelationsDataStorageAccess.h"
#include "mitkRelationStorage.h"
#include "mitkControlPointManager.h"
#include "mitkDICOMHelper.h"
#include "mitkLesionManager.h"
#include "mitkSemanticRelationsTestHelper.h"
#include "mitkUIDGeneratorBoost.h"

// mitk core
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkPropertyNameHelper.h>

// mitk persistence
#include <mitkPersistenceService.h>

class mitkSemanticRelationsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSemanticRelationsTestSuite);
  MITK_TEST(IntegrationTest);
  MITK_TEST(InferenceTest);
  MITK_TEST(DataStorageAccessTest);
  MITK_TEST(RemoveAndUnlinkTest);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::DataStorage::Pointer m_DataStorage;

public:

  void setUp() override
  {
    mitk::PersistenceService::LoadModule();
    m_DataStorage = mitk::StandaloneDataStorage::New();
  }

  void tearDown() override
  {
    // clear semantic relations storage
    mitk::SemanticRelationsTestHelper::ClearRelationStorage();
  }

  //////////////////////////////////////////////////////////////////////////
  // SPECIFIC TEST GROUPS
  //////////////////////////////////////////////////////////////////////////
  void IntegrationTest()
  {
    MITK_INFO << "=== IntegrationTest start ===";
    AddNewData();
    ExaminationPeriod();
    SegmentationAndLesion();
    InvalidData();
    MITK_INFO << "=== IntegrationTest end ===";
  }

  void InferenceTest()
  {
    MITK_INFO << "=== InferenceTest start ===";
    CombinedQueries();
    InstanceExistences();
    MITK_INFO << "=== InferenceTest end ===";
  }

  void DataStorageAccessTest()
  {
    MITK_INFO << "=== DataStorageAccessTest start ===";
    DataStorageAccess();
    MITK_INFO << "=== DataStorageAccessTest end ===";
  }

  void RemoveAndUnlinkTest()
  {
    MITK_INFO << "=== RemoveAndUnlinkTest start ===";
    CPRemoveAndUnlink();
    LesionRemoveAndUnlink();
    RemoveImagesAndSegmentation();
    MITK_INFO << "=== RemoveAndUnlinkTest end ===";
  }

  //////////////////////////////////////////////////////////////////////////
  // SPECIFIC TESTS
  //////////////////////////////////////////////////////////////////////////
  // IntegrationTest
  void AddNewData()
  {
    MITK_INFO << "=== AddNewData";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;

    auto image = mitk::SemanticRelationsTestHelper::GetPatientOneCTImage();
    m_DataStorage->Add(image);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", image.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", image->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(image);

    // start test
    auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("CaseID not correctly stored", allCaseIDs.front() == "Patient1");
    auto caseID = allCaseIDs.front();

    // test control point
    auto allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One control point should be stored", allControlPoints.size() == 1);

    auto controlPointOfImage = mitk::SemanticRelationsInference::GetControlPointOfImage(image);
    mitk::SemanticTypes::ControlPoint controlPointToCompare;
    controlPointToCompare.date = boost::gregorian::date(2019, 01, 01);
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly stored", controlPointOfImage.date == controlPointToCompare.date);

    // test information type
    auto allInformationTypes = mitk::RelationStorage::GetAllInformationTypesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One information type should be stored", allInformationTypes.size() == 1);

    auto informationType = mitk::SemanticRelationsInference::GetInformationTypeOfImage(image);
    CPPUNIT_ASSERT_MESSAGE("Information type not correctly stored", informationType == "CT");

    semanticRelationsIntegration.SetInformationType(image, "MR");
    informationType = mitk::SemanticRelationsInference::GetInformationTypeOfImage(image);
    CPPUNIT_ASSERT_MESSAGE("Information type not correctly stored", informationType == "MR");
  }

  void ExaminationPeriod()
  {
    MITK_INFO << "=== ExaminationPeriod";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;

    auto MRImage = mitk::SemanticRelationsTestHelper::GetPatientOneMRImage();
    m_DataStorage->Add(MRImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", MRImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", MRImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(MRImage);

    // start test
    auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("One CaseID should be stored", allCaseIDs.size() == 1);
    auto caseID = allCaseIDs.front();

    auto allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One control point should be stored", allControlPoints.size() == 1);

    auto controlPointOfImage = mitk::SemanticRelationsInference::GetControlPointOfImage(MRImage);
    mitk::SemanticTypes::ControlPoint controlPoint;
    controlPoint.date = boost::gregorian::date(2019, 01, 01);
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly stored", controlPointOfImage.date == controlPoint.date);

    auto examinationPeriod = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be stored", examinationPeriod.size() == 1);

    auto allImageIDs = mitk::RelationStorage::GetAllImageIDsOfControlPoint(caseID, controlPointOfImage);
    CPPUNIT_ASSERT_MESSAGE("Two images should reference the same control point", allImageIDs.size() == 2);

    // add additional control point to the same examination period
    // load data
    auto CTImage = mitk::SemanticRelationsTestHelper::GetPatientOneOtherCTImage();
    m_DataStorage->Add(CTImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", CTImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", CTImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(CTImage);

    // start test
    allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two control points should be stored", allControlPoints.size() == 2);

    examinationPeriod = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be stored", examinationPeriod.size() == 1);

    // modify control point to create new examination period
    // current control point is 2019, 01, 31
    // new control point should exceed threshold (30 days) and create new examination period
    // new control point is 2019, 02, 01
    controlPoint.UID = mitk::UIDGeneratorBoost::GenerateUID();
    controlPoint.date = boost::gregorian::date(2019, 02, 01);
    semanticRelationsIntegration.UnlinkImageFromControlPoint(CTImage);
    semanticRelationsIntegration.SetControlPointOfImage(CTImage, controlPoint);

    allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two control points should be stored", allControlPoints.size() == 2);

    examinationPeriod = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two examination periods should be stored", examinationPeriod.size() == 2);
  }

  void SegmentationAndLesion()
  {
    MITK_INFO << "=== SegmentationAndLesion";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;

    // Patient2
    auto image = mitk::SemanticRelationsTestHelper::GetPatientTwoPETImage();
    m_DataStorage->Add(image);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", image.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", image->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(image);

    auto segmentation = mitk::SemanticRelationsTestHelper::GetPatientTwoSegmentation();
    m_DataStorage->Add(segmentation, image);
    CPPUNIT_ASSERT_MESSAGE("Not a valid segmentation data node", segmentation.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Segmentation data is empty", segmentation->GetData() != nullptr);
    semanticRelationsIntegration.AddSegmentation(segmentation, image);

    // start test
    auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Two CaseIDs should be stored", allCaseIDs.size() == 2);

    // test lesion (add and link)
    auto lesion = mitk::GenerateNewLesion();
    semanticRelationsIntegration.AddLesionAndLinkSegmentation(segmentation, lesion);

    auto representedLesion = mitk::SemanticRelationsInference::GetLesionOfSegmentation(segmentation);
    CPPUNIT_ASSERT_MESSAGE("Represented lesion should be the only stored lesion", lesion.UID == representedLesion.UID);
  }

  void InvalidData()
  {
    MITK_INFO << "=== InvalidData";

    // TEST INVALID CASE ID
    std::string caseID = "invalidID";
    auto allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No lesion should be stored, given a non-existing CaseID", allLesions.size() == 0);

    auto allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No control point should be stored, given a non-existing CaseID", allControlPoints.size() == 0);

    auto allExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No examination periods should be stored, given a non-existing CaseID", allExaminationPeriods.size() == 0);

    auto allInformationTypes = mitk::RelationStorage::GetAllInformationTypesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No information type should be stored, given a non-existing CaseID", allInformationTypes.size() == 0);

    auto allImageIDsOfCase = mitk::RelationStorage::GetAllImageIDsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No image ID should be stored, given a non-existing CaseID", allControlPoints.size() == 0);

    // TEST INVALID DATE
    mitk::DataNode::Pointer invalidDateImage = mitk::SemanticRelationsTestHelper::GetInvalidDate();
    m_DataStorage->Add(invalidDateImage);

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", invalidDateImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", invalidDateImage->GetData() != nullptr);

    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: AddImage",
      semanticRelationsIntegration.AddImage(invalidDateImage),
      mitk::SemanticRelationException);

    auto controlPointOfImage = mitk::SemanticRelationsInference::GetControlPointOfImage(invalidDateImage);
    CPPUNIT_ASSERT_MESSAGE("No control point should be stored for an invalid DICOM date", controlPointOfImage.UID.empty());

    // set invalid date format for current image
    // 0x0008, 0x0022 (AcquisitionDate), should be 20180101
    invalidDateImage->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0022).c_str(),
      mitk::StringProperty::New("201811"));
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: GenerateControlPoint",
      mitk::GenerateControlPoint(invalidDateImage),
      mitk::SemanticRelationException);

    // set valid date format for current image and add image again (only control point should be updated)
    // 0x0008, 0x0022 (AcquisitionDate), should be 20180101
    invalidDateImage->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0022).c_str(),
      mitk::StringProperty::New("20180101"));
    semanticRelationsIntegration.AddImage(invalidDateImage);

    controlPointOfImage = mitk::SemanticRelationsInference::GetControlPointOfImage(invalidDateImage);
    mitk::SemanticTypes::ControlPoint controlPointToCompare;
    controlPointToCompare.date = boost::gregorian::date(2018, 01, 01);
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly stored", controlPointOfImage.date == controlPointToCompare.date);

    // TEST INVALID MODALITY
    mitk::DataNode::Pointer invalidModalityImage = mitk::SemanticRelationsTestHelper::GetInvalidModality();
    m_DataStorage->Add(invalidModalityImage);

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", invalidModalityImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", invalidModalityImage->GetData() != nullptr);

    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: AddImage",
      semanticRelationsIntegration.AddImage(invalidModalityImage),
      mitk::SemanticRelationException);

    auto informationTypeOfImage = mitk::SemanticRelationsInference::GetInformationTypeOfImage(invalidModalityImage);
    CPPUNIT_ASSERT_MESSAGE("No information type should be stored for an invalid DICOM date", informationTypeOfImage.empty());

    // set valid modality for current image and add image again
    // 0x0008, 0x0060 (Modality), should be CT
    invalidModalityImage->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(),
      mitk::StringProperty::New("CT"));
    semanticRelationsIntegration.AddImage(invalidModalityImage);

    auto informationType = mitk::SemanticRelationsInference::GetInformationTypeOfImage(invalidModalityImage);
    CPPUNIT_ASSERT_MESSAGE("Information type not correctly stored", informationType == "CT");

    // TEST INVALID ID
    mitk::DataNode::Pointer invalidIDImage = mitk::SemanticRelationsTestHelper::GetInvalidID();
    m_DataStorage->Add(invalidIDImage);

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", invalidModalityImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", invalidModalityImage->GetData() != nullptr);

    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: AddImage",
      semanticRelationsIntegration.AddImage(invalidIDImage),
      mitk::SemanticRelationException);

    // set valid ID for current image and add image again
    // 0x0020, 0x000e (SeriesInstanceUID)
    auto generatedUID = mitk::UIDGeneratorBoost::GenerateUID();
    invalidIDImage->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(),
      mitk::StringProperty::New(generatedUID));
    semanticRelationsIntegration.AddImage(invalidIDImage);

    auto IDFromNode = mitk::GetIDFromDataNode(invalidIDImage);
    CPPUNIT_ASSERT_MESSAGE("ID not correctly stored", IDFromNode == generatedUID);

    // TEST INVALID CASE ID
    mitk::DataNode::Pointer invalidCaseIDImage = mitk::SemanticRelationsTestHelper::GetInvalidCaseID();
    m_DataStorage->Add(invalidCaseIDImage);

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", invalidModalityImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", invalidModalityImage->GetData() != nullptr);

    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: AddImage",
      semanticRelationsIntegration.AddImage(invalidCaseIDImage),
      mitk::SemanticRelationException);

    // set valid case ID for current image and add image again
    // 0x0010, 0x0010 (PatientName)
    invalidCaseIDImage->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str(),
      mitk::StringProperty::New("Patient7"));
    semanticRelationsIntegration.AddImage(invalidCaseIDImage);

    auto caseIDFromNode = mitk::GetCaseIDFromDataNode(invalidCaseIDImage);
    CPPUNIT_ASSERT_MESSAGE("Case ID not correctly stored", caseIDFromNode == "Patient7");
  }

  // InferenceTest
  void CombinedQueries()
  {
    MITK_INFO << "=== CombinedQueries";

    // add image with segmentation and lesion
    auto image = mitk::SemanticRelationsTestHelper::GetPatientTwoPETImage();
    m_DataStorage->Add(image);

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", image.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", image->GetData() != nullptr);

    auto segmentation = mitk::SemanticRelationsTestHelper::GetPatientTwoSegmentation();
    m_DataStorage->Add(segmentation, image);

    CPPUNIT_ASSERT_MESSAGE("Not a valid segmentation data node", segmentation.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Segmentation data is empty", segmentation->GetData() != nullptr);

    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    semanticRelationsIntegration.AddImage(image);
    semanticRelationsIntegration.AddSegmentation(segmentation, image);

    auto caseID = mitk::GetCaseIDFromDataNode(image);

    auto lesion = mitk::GenerateNewLesion();
    semanticRelationsIntegration.AddLesionAndLinkSegmentation(segmentation, lesion);

    auto allLesionsOfImage = mitk::SemanticRelationsInference::GetAllLesionsOfImage(image);
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesionsOfImage.size() == 1);

    auto controlPoint = mitk::SemanticRelationsInference::GetControlPointOfImage(image);
    auto allLesionsOfControlPoint = mitk::SemanticRelationsInference::GetAllLesionsOfControlPoint(caseID, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("Lesions should be the same", (allLesionsOfImage.size() == allLesionsOfControlPoint.size())
      && (allLesionsOfImage.front().UID == allLesionsOfControlPoint.front().UID));

    auto allImageIDsOfLesion = mitk::SemanticRelationsInference::GetAllImageIDsOfLesion(caseID, allLesionsOfControlPoint.front());
    CPPUNIT_ASSERT_MESSAGE("Image IDs should be the same", (allImageIDsOfLesion.size() == 1)
      && (allImageIDsOfLesion.front() == mitk::GetIDFromDataNode(image)));

    auto allControlPointsOfLesion = mitk::SemanticRelationsInference::GetAllControlPointsOfLesion(caseID, allLesionsOfImage.front());
    CPPUNIT_ASSERT_MESSAGE("Control points should be the same", (allControlPointsOfLesion.size() == 1)
      && (allControlPointsOfLesion.front().date == controlPoint.date));

    auto informationType = mitk::SemanticRelationsInference::GetInformationTypeOfImage(image);
    auto allControlPointsOfInformationType = mitk::SemanticRelationsInference::GetAllControlPointsOfInformationType(caseID, informationType);
    CPPUNIT_ASSERT_MESSAGE("Control points should be the same", (allControlPointsOfLesion.size() == 1)
      && (allControlPointsOfLesion.front().date == controlPoint.date));

    auto allInformationTypesOfControlPoint = mitk::SemanticRelationsInference::GetAllInformationTypesOfControlPoint(caseID, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("Information types should be the same", (allInformationTypesOfControlPoint.size() == 1)
      && (allInformationTypesOfControlPoint.front() == informationType));
  }

  void InstanceExistences()
  {
    MITK_INFO << "=== InstanceExistences";

    mitk::SemanticRelationsDataStorageAccess semanticRelationsDataStorageAccess(m_DataStorage);
    auto allSegmentationsOfCase = semanticRelationsDataStorageAccess.GetAllSegmentationsOfCase("Patient2");
    CPPUNIT_ASSERT_MESSAGE("One segmentation should be stored", allSegmentationsOfCase.size() == 1);

    auto segmentation = allSegmentationsOfCase.front();
    CPPUNIT_ASSERT_MESSAGE("Not a valid segmentation data node", segmentation.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Segmentation data is empty", segmentation->GetData() != nullptr);

    bool valid = mitk::SemanticRelationsInference::IsRepresentingALesion(segmentation);
    CPPUNIT_ASSERT_MESSAGE("Segmentation node should represent a lesion", valid);

    auto caseID = "Patient2";
    valid = mitk::SemanticRelationsInference::IsRepresentingALesion(caseID, mitk::GetIDFromDataNode(segmentation));
    CPPUNIT_ASSERT_MESSAGE("Segmentation (via ID) should represent a lesion", valid);

    auto allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesions.size() == 1);

    auto lesion = allLesions.front();
    valid = mitk::SemanticRelationsInference::IsLesionPresent(lesion, segmentation);
    CPPUNIT_ASSERT_MESSAGE("Lesion should be present on segmentation", valid);

    auto allImagesOfCase = semanticRelationsDataStorageAccess.GetAllImagesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One image should be stored", allImagesOfCase.size() == 1);

    auto image = allImagesOfCase.front();
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", image.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", image->GetData() != nullptr);

    valid = mitk::SemanticRelationsInference::IsLesionPresent(lesion, image);
    CPPUNIT_ASSERT_MESSAGE("Lesion should be present on image", valid);

    valid = mitk::SemanticRelationsInference::IsLesionPresentOnImage(caseID, lesion, mitk::GetIDFromDataNode(image));
    CPPUNIT_ASSERT_MESSAGE("Lesion should be present on segmentation", valid);

    valid = mitk::SemanticRelationsInference::IsLesionPresentOnSegmentation(caseID, lesion, mitk::GetIDFromDataNode(segmentation));
    CPPUNIT_ASSERT_MESSAGE("Lesion should be present on image", valid);

    auto controlPoint = mitk::SemanticRelationsInference::GetControlPointOfImage(image);
    valid = mitk::SemanticRelationsInference::IsLesionPresentAtControlPoint(caseID, lesion, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("Lesion should be present at control point", valid);

    controlPoint.date = boost::gregorian::date(2019, 01, 01);
    valid = mitk::SemanticRelationsInference::IsLesionPresentAtControlPoint(caseID, lesion, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("No lesion should be present at control point", !valid);

    valid = mitk::SemanticRelationsInference::InstanceExists(image);
    CPPUNIT_ASSERT_MESSAGE("Image should exist", valid);

    valid = mitk::SemanticRelationsInference::InstanceExists(segmentation);
    CPPUNIT_ASSERT_MESSAGE("Segmentation should exist", valid);

    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, lesion);
    CPPUNIT_ASSERT_MESSAGE("Lesion should exist", valid);

    auto emptyLesion = mitk::GenerateNewLesion("ExampleLesionClass");
    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, emptyLesion);
    CPPUNIT_ASSERT_MESSAGE("Lesion should not exist", !valid);

    auto newControlPoint = mitk::SemanticTypes::ControlPoint();
    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, newControlPoint);
    CPPUNIT_ASSERT_MESSAGE("Control point should not exist for this case", !valid);

    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("Control point should exist for this case", valid);

    auto allExaminationPeriod = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be stored", allExaminationPeriod.size() == 1);
    auto examinationPeriod = allExaminationPeriod.front();

    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, examinationPeriod);
    CPPUNIT_ASSERT_MESSAGE("Examination period should exist for this case", valid);

    auto informationType = mitk::SemanticRelationsInference::GetInformationTypeOfImage(image);
    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, informationType);
    CPPUNIT_ASSERT_MESSAGE("Control point should exist for this case", valid);

    valid = mitk::SemanticRelationsInference::InstanceExists(caseID, "CT");
    CPPUNIT_ASSERT_MESSAGE("Control point should not exist for this case", !valid);

    valid = mitk::SemanticRelationsInference::SpecificImageExists(caseID, lesion, informationType);
    CPPUNIT_ASSERT_MESSAGE("Specific image should exist for this case", valid);

    valid = mitk::SemanticRelationsInference::SpecificImageExists(caseID, lesion, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("Specific image should exist for this case", valid);

    valid = mitk::SemanticRelationsInference::SpecificImageExists(caseID, informationType, controlPoint);
    CPPUNIT_ASSERT_MESSAGE("Specific image should exist for this case", valid);
  }

  // DataStorageAccessTest
  void DataStorageAccess()
  {
    MITK_INFO << "=== DataStorageAccess";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;

    // Patient1
    auto CTImage = mitk::SemanticRelationsTestHelper::GetPatientOneCTImage();
    m_DataStorage->Add(CTImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", CTImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", CTImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(CTImage);

    auto MRImage = mitk::SemanticRelationsTestHelper::GetPatientOneMRImage();
    m_DataStorage->Add(MRImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", MRImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", MRImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(MRImage);

    auto otherCTImage = mitk::SemanticRelationsTestHelper::GetPatientOneOtherCTImage();
    m_DataStorage->Add(otherCTImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", otherCTImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", otherCTImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(otherCTImage);

    // Patient2
    auto PETImage = mitk::SemanticRelationsTestHelper::GetPatientTwoPETImage();
    m_DataStorage->Add(PETImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", PETImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", PETImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(PETImage);

    auto PETSegmentation = mitk::SemanticRelationsTestHelper::GetPatientTwoSegmentation();
    m_DataStorage->Add(PETSegmentation, PETImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid segmentation data node", PETSegmentation.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Segmentation data is empty", PETSegmentation->GetData() != nullptr);
    semanticRelationsIntegration.AddSegmentation(PETSegmentation, PETImage);

    auto lesion = mitk::GenerateNewLesion();
    semanticRelationsIntegration.AddLesionAndLinkSegmentation(PETSegmentation, lesion);

    // start test
    mitk::SemanticRelationsDataStorageAccess semanticRelationsDataStorageAccess(m_DataStorage);

    auto caseID = "Patient1";
    auto allSegmentationsOfCase = semanticRelationsDataStorageAccess.GetAllSegmentationsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No segmentation should be stored", allSegmentationsOfCase.size() == 0);

    auto allImagesOfCase = semanticRelationsDataStorageAccess.GetAllImagesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Three images should be stored", allImagesOfCase.size() == 3);

    auto allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No lesion should be stored", allLesions.size() == 0);

    auto emptyLesion = mitk::GenerateNewLesion("ExampleLesionClass");
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: GetAllSegmentationsOfLesion",
      semanticRelationsDataStorageAccess.GetAllSegmentationsOfLesion(caseID, emptyLesion),
      mitk::SemanticRelationException);

    caseID = "Patient2";
    allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesions.size() == 1);

    lesion = allLesions.front();
    auto allSegmentationsOfLesion = semanticRelationsDataStorageAccess.GetAllSegmentationsOfLesion(caseID, lesion);
    CPPUNIT_ASSERT_MESSAGE("One segmentation should be stored", allSegmentationsOfLesion.size() == 1);

    auto allImagesOfLesion = semanticRelationsDataStorageAccess.GetAllImagesOfLesion(caseID, lesion);
    CPPUNIT_ASSERT_MESSAGE("One image should be stored", allImagesOfLesion.size() == 1);

    auto allControlPointsOfLesion = mitk::SemanticRelationsInference::GetAllControlPointsOfLesion(caseID, lesion);
    CPPUNIT_ASSERT_MESSAGE("One control point should be stored", allControlPointsOfLesion.size() == 1);
    auto controlPoint = allControlPointsOfLesion.front();

    auto allSpecificImages = semanticRelationsDataStorageAccess.GetAllSpecificImages(caseID, controlPoint, "PT");
    CPPUNIT_ASSERT_MESSAGE("One image should be stored", allSpecificImages.size() == 1);

    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: GetAllSpecificImages",
      semanticRelationsDataStorageAccess.GetAllSpecificImages(caseID, controlPoint, "CT"),
      mitk::SemanticRelationException);

    auto allSpecificSegmentations = semanticRelationsDataStorageAccess.GetAllSpecificSegmentations(caseID, controlPoint, "PT");
    CPPUNIT_ASSERT_MESSAGE("One segmentation should be stored", allSpecificSegmentations.size() == 1);

    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: GetAllSpecificSegmentations",
      semanticRelationsDataStorageAccess.GetAllSpecificSegmentations(caseID, controlPoint, "MR"),
      mitk::SemanticRelationException);
  }

  // RemoveAndUnlinkTest
  void CPRemoveAndUnlink()
  {
    MITK_INFO << "=== RemoveAndUnlink";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;

    auto CTImage = mitk::SemanticRelationsTestHelper::GetPatientThreeCTImage();
    m_DataStorage->Add(CTImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", CTImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", CTImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(CTImage);

    auto MRImage = mitk::SemanticRelationsTestHelper::GetPatientThreeMRImage();
    m_DataStorage->Add(MRImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", MRImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", MRImage->GetData() != nullptr);
    semanticRelationsIntegration.AddImage(MRImage);

    // start test
    auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("One CaseID should be stored", allCaseIDs.size() == 1);

    auto caseID = allCaseIDs.front();
    CPPUNIT_ASSERT_MESSAGE("CaseID should be Patient3", caseID == "Patient3");

    auto allExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be sored", allExaminationPeriods.size() == 1);

    auto allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two control points should be stored", allControlPoints.size() == 2);

    // control points
    auto CTControlPoint = mitk::SemanticRelationsInference::GetControlPointOfImage(CTImage);
    semanticRelationsIntegration.UnlinkImageFromControlPoint(CTImage);

    allExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be stored", allExaminationPeriods.size() == 1);

    allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One control point should be stored since one has been removed", allControlPoints.size() == 1);

    CTControlPoint = mitk::SemanticRelationsInference::GetControlPointOfImage(CTImage);
    CPPUNIT_ASSERT_MESSAGE("Control point should be removed", CTControlPoint.UID.empty());

    CTControlPoint = mitk::GenerateControlPoint(CTImage);
    semanticRelationsIntegration.SetControlPointOfImage(CTImage, CTControlPoint);

    allExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be sored", allExaminationPeriods.size() == 1);

    allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two control points should be stored", allControlPoints.size() == 2);

    mitk::SemanticTypes::ControlPoint newCTControlPoint;
    newCTControlPoint.UID = mitk::UIDGeneratorBoost::GenerateUID();
    newCTControlPoint.date = boost::gregorian::date(2019, 03, 01);
    semanticRelationsIntegration.SetControlPointOfImage(CTImage, newCTControlPoint);

    allExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One examination period should be sored", allExaminationPeriods.size() == 1);

    allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two control points should be stored", allControlPoints.size() == 2);

    newCTControlPoint.UID = mitk::UIDGeneratorBoost::GenerateUID();
    newCTControlPoint.date = boost::gregorian::date(2019, 01, 01);
    semanticRelationsIntegration.SetControlPointOfImage(CTImage, newCTControlPoint);

    allExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two examination periods should be stored", allExaminationPeriods.size() == 2);

    allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two control points should be stored", allControlPoints.size() == 2);
  }

  void LesionRemoveAndUnlink()
  {
    MITK_INFO << "=== LesionRemoveAndUnlink";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    mitk::SemanticRelationsDataStorageAccess semanticRelationsDataStorageAccess(m_DataStorage);

    auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("One CaseID should be stored", allCaseIDs.size() == 1);
    auto caseID = allCaseIDs.front();

    auto allImages = semanticRelationsDataStorageAccess.GetAllImagesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two images should be stored", allImages.size() == 2);

    auto allCTImageIDs = mitk::RelationStorage::GetAllImageIDsOfInformationType(caseID, "CT");
    CPPUNIT_ASSERT_MESSAGE("One image should be stored", allCTImageIDs.size() == 1);

    auto allMRImageIDs = mitk::RelationStorage::GetAllImageIDsOfInformationType(caseID, "MR");
    CPPUNIT_ASSERT_MESSAGE("One image should be stored", allMRImageIDs.size() == 1);

    mitk::DataNode::Pointer CTImage;
    mitk::DataNode::Pointer MRImage;
    for (const auto& image : allImages)
    {
      auto imageID = mitk::GetIDFromDataNode(image);
      if (imageID == allCTImageIDs.front())
      {
        CTImage = image;
      }
      else if (imageID == allMRImageIDs.front())
      {
        MRImage = image;
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", CTImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", CTImage->GetData() != nullptr);

    CPPUNIT_ASSERT_MESSAGE("Not a valid image data node", MRImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", MRImage->GetData() != nullptr);

    auto CTSegmentation = mitk::SemanticRelationsTestHelper::GetPatientThreeCTSegmentation();
    m_DataStorage->Add(CTSegmentation, CTImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid segmentation data node", CTSegmentation.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Segmentation data is empty", CTSegmentation->GetData() != nullptr);
    semanticRelationsIntegration.AddSegmentation(CTSegmentation, CTImage);

    auto MRSegmentation = mitk::SemanticRelationsTestHelper::GetPatientThreeMRSegmentation();
    m_DataStorage->Add(MRSegmentation, MRImage);
    CPPUNIT_ASSERT_MESSAGE("Not a valid segmentation data node", MRSegmentation.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Segmentation data is empty", MRSegmentation->GetData() != nullptr);
    semanticRelationsIntegration.AddSegmentation(MRSegmentation, MRImage);

    auto newLesion = mitk::GenerateNewLesion();
    semanticRelationsIntegration.AddLesionAndLinkSegmentation(CTSegmentation, newLesion);
    newLesion = mitk::GenerateNewLesion();
    semanticRelationsIntegration.AddLesionAndLinkSegmentation(MRSegmentation, newLesion);

    // start test
    auto allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two lesions should be stored", allLesions.size() == 2);

    auto MRLesion = mitk::SemanticRelationsInference::GetLesionOfSegmentation(MRSegmentation);
    semanticRelationsIntegration.UnlinkSegmentationFromLesion(MRSegmentation);

    allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two lesions should be stored", allLesions.size() == 2);

    auto emptyLesion = mitk::SemanticRelationsInference::GetLesionOfSegmentation(MRSegmentation);
    CPPUNIT_ASSERT_MESSAGE("Lesion should be removed", emptyLesion.UID.empty());

    semanticRelationsIntegration.LinkSegmentationToLesion(MRSegmentation, MRLesion);
    allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two lesions should be stored", allLesions.size() == 2);

    auto CTLesion = mitk::SemanticRelationsInference::GetLesionOfSegmentation(CTSegmentation);
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: RemoveLesion",
      semanticRelationsIntegration.RemoveLesion(caseID, CTLesion),
      mitk::SemanticRelationException);

    semanticRelationsIntegration.UnlinkSegmentationFromLesion(CTSegmentation);
    semanticRelationsIntegration.RemoveLesion(caseID, CTLesion);

    allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One lesions should be stored since one has been removed", allLesions.size() == 1);

    CTLesion = mitk::SemanticRelationsInference::GetLesionOfSegmentation(CTSegmentation);
    CPPUNIT_ASSERT_MESSAGE("Lesion should be removed", CTLesion.UID.empty());
  }

  void RemoveImagesAndSegmentation()
  {
    MITK_INFO << "=== RemoveImagesAndSegmentation";

    // load data
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    mitk::SemanticRelationsDataStorageAccess semanticRelationsDataStorageAccess(m_DataStorage);

    auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("One CaseID should be stored", allCaseIDs.size() == 1);
    auto caseID = allCaseIDs.front();

    auto allImages = semanticRelationsDataStorageAccess.GetAllImagesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two images should be stored", allImages.size() == 2);

    auto allSegmentations = semanticRelationsDataStorageAccess.GetAllSegmentationsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("Two segmentations should be stored", allSegmentations.size() == 2);

    for (const auto& image : allImages)
    {
      semanticRelationsIntegration.RemoveImage(image);
    }

    for (const auto& segmentation : allSegmentations)
    {
      semanticRelationsIntegration.RemoveSegmentation(segmentation);
    }

    allImages = semanticRelationsDataStorageAccess.GetAllImagesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No images should be stored", allImages.size() == 0);

    allSegmentations = semanticRelationsDataStorageAccess.GetAllSegmentationsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No segmentations should be stored", allSegmentations.size() == 0);

    auto allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No control point should be stored", allControlPoints.size() == 0);

    auto allInformationTypes = mitk::RelationStorage::GetAllInformationTypesOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("No information type should be stored", allInformationTypes.size() == 0);

    auto allLesions = mitk::RelationStorage::GetAllLesionsOfCase(caseID);
    CPPUNIT_ASSERT_MESSAGE("One lesions should be stored", allLesions.size() == 1);
  }
  
};

MITK_TEST_SUITE_REGISTRATION(mitkSemanticRelations)
