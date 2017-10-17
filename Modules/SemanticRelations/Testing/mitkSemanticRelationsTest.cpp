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

#include "mitkSemanticRelations.h"
#include "mitkSemanticRelationException.h"

// mitk core
#include <mitkIOUtil.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

// mitk persistence
#include <mitkPersistenceService.h>

// mitk multilabel
#include <mitkLabelSetImage.h>

class mitkSemanticRelationsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSemanticRelationsTestSuite);
  MITK_TEST(CombinedTest);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::DataStorage::Pointer m_DataStorage;
  std::vector<mitk::BaseData::Pointer> m_BaseData;
  mitk::PreferenceListReaderOptionsFunctor m_Functor;

public:

  void setUp() override
  {
    mitk::PersistenceService::LoadModule();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    mitk::PreferenceListReaderOptionsFunctor::ListType preference = { "MITK DICOM Reader v2 (classic config)" };
    mitk::PreferenceListReaderOptionsFunctor::ListType emptyList = {};

    m_Functor = mitk::PreferenceListReaderOptionsFunctor(preference, emptyList);
  }

  void tearDown() override
  {
  }

  void CombinedTest()
  {
    TestAddNewData();
    ExtendControlPoint();
    AddAndRemoveInformationType();
    SegmentationAndLesion();
    InvalidData();
    CombinedQuery();
    RemoveAndUnlink();
    Exceptions();
    Overwrite();
  }

  void TestAddNewData()
  {
    MITK_INFO << "TestAddNewData";
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT Pelvis/Abd/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);
    m_DataStorage->Add(dataNode);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    mitk::SemanticRelations semanticRelations(m_DataStorage);
    SemanticRelationsTestHelper::AddImageInstance(dataNode, semanticRelations);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("CaseID not correctly stored", allCaseIDs.front() == "TCGA-VP-A87D");

    SemanticTypes::ControlPoint controlPointOfData = semanticRelations.GetControlPointOfData(dataNode);
    SemanticTypes::Date startDate; startDate.year = 1992; startDate.month = 12; startDate.day = 24;
    SemanticTypes::Date endDate; endDate.year = 1992; endDate.month = 12; endDate.day = 24;
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly stored", controlPointOfData.startPoint == startDate && controlPointOfData.endPoint == endDate);
  }

  void ExtendControlPoint()
  {
    MITK_INFO << "ExtendControlPoint";
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT Pelvis/Abd Pel/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);
    m_DataStorage->Add(dataNode);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    mitk::SemanticRelations semanticRelations(m_DataStorage);
    SemanticRelationsTestHelper::AddImageInstance(dataNode, semanticRelations);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Only one CaseID should be stored", allCaseIDs.size() == 1);

    std::vector<SemanticTypes::ControlPoint> allControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("Only one control point should be stored", allControlPoints.size() == 1);

    SemanticTypes::ControlPoint controlPointOfData = semanticRelations.GetControlPointOfData(dataNode);
    SemanticTypes::Date startDate; startDate.year = 1992; startDate.month = 12; startDate.day = 10;
    SemanticTypes::Date endDate; endDate.year = 1992; endDate.month = 12; endDate.day = 24;
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly extended", controlPointOfData.startPoint == startDate && controlPointOfData.endPoint == endDate);

    std::vector<mitk::DataNode::Pointer> allData = semanticRelations.GetAllDataOfControlPoint(allCaseIDs.front(), controlPointOfData);
    CPPUNIT_ASSERT_MESSAGE("Two images should reference the control point", allData.size() == 2);
  }

  void AddAndRemoveInformationType()
  {
    MITK_INFO << "AddAndRemoveInformationType";
    // load previously stored image
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT Pelvis/Abd Pel/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    mitk::SemanticRelations semanticRelations(m_DataStorage);

    semanticRelations.AddInformationTypeToImage(dataNode, "CT");

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    std::vector<SemanticTypes::InformationType> allInformationTypes = semanticRelations.GetAllInformationTypesOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("One information type should be stored", allInformationTypes.size() == 1);

    SemanticTypes::InformationType informationType = semanticRelations.GetInformationTypeOfImage(dataNode);
    CPPUNIT_ASSERT_MESSAGE("Information type should be equal to 'CT'", informationType == "CT");

    semanticRelations.RemoveInformationTypeFromImage(dataNode, "CT");

    allInformationTypes = semanticRelations.GetAllInformationTypesOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("No information type should be stored", allInformationTypes.size() == 0);
  }

  void SegmentationAndLesion()
  {
    MITK_INFO << "SegmentationAndLesion";
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/TUMOR PET RES/PET/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    mitk::LabelSetImage::Pointer segmentationImage = mitk::LabelSetImage::New();
    
    // workaround: manually set the PatientName tag: use the tag from the real DICOM image
    mitk::PropertyList::Pointer segmentationPropertyList = mitk::PropertyList::New();
    segmentationPropertyList->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str(),
                                          imageData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str()));

    // workaround: manually set the SeriesInstanceUID tag: use the tag from the real DICOM image and modify it to have different identifiers
    std::string seriesInstanceUID = imageData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str())->GetValueAsString() + "_S";
    segmentationPropertyList->SetStringProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(), seriesInstanceUID.c_str());

    segmentationImage->SetPropertyList(segmentationPropertyList);
    mitk::DataNode::Pointer segmentationNode = mitk::DataNode::New();
    segmentationNode->SetData(segmentationImage);
    m_DataStorage->Add(segmentationNode, dataNode);

    mitk::SemanticRelations semanticRelations(m_DataStorage);
    SemanticRelationsTestHelper::AddSegmentationInstance(segmentationNode, dataNode, semanticRelations);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Only one CaseID should be stored", allCaseIDs.size() == 1);

    std::vector<SemanticTypes::Lesion> allLesions = semanticRelations.GetAllLesionsInImage(dataNode);
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesions.size() == 1);

    SemanticTypes::Lesion representedLesion = semanticRelations.GetRepresentedLesion(segmentationNode);
    CPPUNIT_ASSERT_MESSAGE("Represented lesion should be the only stored lesion", allLesions.front().UID == representedLesion.UID);
  }

  void InvalidData()
  {
    MITK_INFO << "InvalidData";
    mitk::SemanticRelations semanticRelations(m_DataStorage);
    std::vector<SemanticTypes::ControlPoint> allControlPoints = semanticRelations.GetAllControlPointsOfCase("noID");
    CPPUNIT_ASSERT_MESSAGE("No control point should be stored, given a non-existing CaseID", allControlPoints.size() == 0);

    // load new image but do not create any semantic relation
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT Abdomen/non-contrast/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);

    SemanticTypes::ControlPoint controlPoint = semanticRelations.GetControlPointOfData(dataNode);
    CPPUNIT_ASSERT_MESSAGE("No control point should be stored for an image that is not stored", controlPoint.UID.empty());

    // TODO: use invalid DICOM data (no tag - leads to empty case ID or empty node ID)
  }

  void CombinedQuery()
  {
    MITK_INFO << "CombinedQuery";
    // load new image with a different time point
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT abd and Pelvis/CAP/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);
    m_DataStorage->Add(dataNode);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    mitk::SemanticRelations semanticRelations(m_DataStorage);

    SemanticRelationsTestHelper::AddImageInstance(dataNode, semanticRelations);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Only one CaseID should be stored", allCaseIDs.size() == 1);

    std::vector<SemanticTypes::ControlPoint> allControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("Two different control points should be stored", allControlPoints.size() == 2);

    semanticRelations.AddInformationTypeToImage(dataNode, "CT");
    std::vector<SemanticTypes::InformationType> allInformationTypes = semanticRelations.GetAllInformationTypesOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("One information type should be stored", allInformationTypes.size() == 1);

    std::vector<SemanticTypes::ControlPoint> filteredControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front(), allInformationTypes.front());
    allInformationTypes = semanticRelations.GetAllInformationTypesOfCase(allCaseIDs.front(), allControlPoints.back());
    CPPUNIT_ASSERT_MESSAGE("Only one control points should match", filteredControlPoints.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Only one information type should match", allInformationTypes.size() == 1);

    filteredControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front(), "MR");
    allInformationTypes = semanticRelations.GetAllInformationTypesOfCase(allCaseIDs.front(), allControlPoints.front());
    CPPUNIT_ASSERT_MESSAGE("No control point should match", filteredControlPoints.size() == 0);
    CPPUNIT_ASSERT_MESSAGE("No information type should match", allInformationTypes.size() == 0);

    std::vector<SemanticTypes::Lesion> allLesions = semanticRelations.GetAllLesionsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesions.size() == 1);

    std::vector<SemanticTypes::Lesion> filteredLesions = semanticRelations.GetAllLesionsOfCase(allCaseIDs.front(), allControlPoints.front());
    CPPUNIT_ASSERT_MESSAGE("No lesion should match", filteredLesions.size() == 0);
  }

  void RemoveAndUnlink()
  {
    MITK_INFO << "RemoveAndUnlink";
    /************************************************************************/
    /* unlink and remove control point                                      */
    /************************************************************************/
    // load already stored image with its own single control point
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT abd and Pelvis/CAP/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    mitk::SemanticRelations semanticRelations(m_DataStorage);

    SemanticTypes::ControlPoint singleControlPoint = semanticRelations.GetControlPointOfData(dataNode);
    semanticRelations.UnlinkDataFromControlPoint(dataNode, singleControlPoint);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Only one CaseID should be stored", allCaseIDs.size() == 1);

    std::vector<SemanticTypes::ControlPoint> allControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("Only one control point should be stored since one has been removed", allControlPoints.size() == 1);

    singleControlPoint = semanticRelations.GetControlPointOfData(dataNode);
    CPPUNIT_ASSERT_MESSAGE("Control point property not correctly reset", singleControlPoint.startPoint == SemanticTypes::ControlPoint().startPoint && singleControlPoint.endPoint == SemanticTypes::ControlPoint().endPoint);

    // load already stored image which extended a previously existing control point
    testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT Pelvis/Abd Pel/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    imageData = m_BaseData.front();
    dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);

    CPPUNIT_ASSERT_MESSAGE("Not a valid data node", dataNode.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Image data is empty", dataNode->GetData() != nullptr);

    SemanticTypes::ControlPoint extendedControlpoint = semanticRelations.GetControlPointOfData(dataNode);
    semanticRelations.UnlinkDataFromControlPoint(dataNode, extendedControlpoint);

    allControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("Still one control point should be stored since none has been removed", allControlPoints.size() == 1);

    extendedControlpoint = allControlPoints.front();
    SemanticTypes::Date startDate; startDate.year = 1992; startDate.month = 12; startDate.day = 24;
    SemanticTypes::Date endDate; endDate.year = 1992; endDate.month = 12; endDate.day = 24;
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly computed", extendedControlpoint.startPoint == startDate && extendedControlpoint.endPoint == endDate);
  }

  void Exceptions()
  {
    MITK_INFO << "Exceptions";
    /************************************************************************/
    /* remove lesion                                                        */
    /************************************************************************/
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/TUMOR PET RES/PET/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();

    mitk::SemanticRelations semanticRelations(m_DataStorage);

    mitk::LabelSetImage::Pointer segmentationImage = mitk::LabelSetImage::New();

    // workaround: manually set the PatientName tag: use the tag from the real DICOM image
    mitk::PropertyList::Pointer segmentationPropertyList = mitk::PropertyList::New();
    segmentationPropertyList->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str(),
      imageData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str()));

    // workaround: manually set the SeriesInstanceUID tag: use the tag from the real DICOM image and modify it to have different identifiers
    std::string seriesInstanceUID = imageData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str())->GetValueAsString() + "_S";
    segmentationPropertyList->SetStringProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(), seriesInstanceUID.c_str());

    segmentationImage->SetPropertyList(segmentationPropertyList);
    mitk::DataNode::Pointer segmentationNode = mitk::DataNode::New();
    segmentationNode->SetData(segmentationImage);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Only one CaseID should be stored", allCaseIDs.size() == 1);

    std::vector<SemanticTypes::Lesion> allLesions = semanticRelations.GetAllLesionsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesions.size() == 1);

    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: Remove lesion instance", semanticRelations.RemoveLesion(allCaseIDs.front(), allLesions.front()), mitk::SemanticRelationException);
    // test adding a lesion instance that already exists
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: Add lesion instance", semanticRelations.AddLesion(allCaseIDs.front(), allLesions.front()), mitk::SemanticRelationException);

    // use the test helper to remove the segmentation instance from the storage so that the referenced lesion instance can be removed (should not throw an exception)
    SemanticRelationsTestHelper::RemoveSegmentationInstance(segmentationNode, semanticRelations);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Semantic relation exception thrown: Remove lesion instance", semanticRelations.RemoveLesion(allCaseIDs.front(), allLesions.front()));
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Semantic relation exception not thrown: Add lesion instance", semanticRelations.AddLesion(allCaseIDs.front(), allLesions.front()));

    // test invalid lesion, control point UID, information type
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: Get segmentations of lesion", semanticRelations.GetAllSegmentationsOfLesion(allCaseIDs.front(), SemanticTypes::Lesion()), mitk::SemanticRelationException);
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: Get data of control point", semanticRelations.GetAllDataOfControlPoint(allCaseIDs.front(), SemanticTypes::ControlPoint()), mitk::SemanticRelationException);
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: Get data of information type", semanticRelations.GetAllDataOfInformationType(allCaseIDs.front(), SemanticTypes::InformationType()), mitk::SemanticRelationException);

    // test overwriting a lesion instance that does not exist
    CPPUNIT_ASSERT_THROW_MESSAGE("Semantic relation exception not thrown: Add lesion instance", semanticRelations.OverwriteLesion(allCaseIDs.front(), SemanticTypes::Lesion()), mitk::SemanticRelationException);
  }

  void Overwrite()
  {
    MITK_INFO << "Overwrite";
    mitk::SemanticRelations semanticRelations(m_DataStorage);

    std::vector<SemanticTypes::CaseID> allCaseIDs = semanticRelations.GetAllCaseIDs();
    CPPUNIT_ASSERT_MESSAGE("Only one CaseID should be stored", allCaseIDs.size() == 1);

    std::vector<SemanticTypes::Lesion> allLesions = semanticRelations.GetAllLesionsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("One lesion should be stored", allLesions.size() == 1);

    SemanticTypes::Lesion overwritingLesion = allLesions.front();
    SemanticTypes::LesionClass overwritingLesionClass = overwritingLesion.lesionClass;

    CPPUNIT_ASSERT_MESSAGE("Only lesion should not have a lesion class", overwritingLesionClass.classType == "");

    overwritingLesionClass.classType = "unspecified lesion class";
    overwritingLesion.lesionClass = overwritingLesionClass;
    semanticRelations.OverwriteLesion(allCaseIDs.front(), overwritingLesion);

    CPPUNIT_ASSERT_MESSAGE("Only lesion should have an unspecified lesion class", overwritingLesionClass.classType == "unspecified lesion class");

    // load an already stored image which extends a previously existing control point (was unlinked before)
    std::string testDataFilePath = GetTestDataFilePath("SemanticRelation/TCGA-VP-A87D/CT Pelvis/Abd Pel/000000.dcm");
    m_BaseData = mitk::IOUtil::Load(testDataFilePath, &m_Functor);
    mitk::BaseData::Pointer imageData = m_BaseData.front();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(imageData);

    std::vector<SemanticTypes::ControlPoint> allControlPoints = semanticRelations.GetAllControlPointsOfCase(allCaseIDs.front());
    CPPUNIT_ASSERT_MESSAGE("Only one control point should be stored", allControlPoints.size() == 1);

    SemanticTypes::ControlPoint extendedControlPoint = ControlPointManager::ExtendClosestControlPoint(dataNode, allControlPoints);
    CPPUNIT_ASSERT_MESSAGE("Extended control point should not be empty", extendedControlPoint.UID != "");

    semanticRelations.OverwriteControlPointAndLinkData(dataNode, extendedControlPoint);

    SemanticTypes::ControlPoint controlPointOfData = semanticRelations.GetControlPointOfData(dataNode);
    SemanticTypes::Date startDate; startDate.year = 1992; startDate.month = 12; startDate.day = 10;
    SemanticTypes::Date endDate; endDate.year = 1992; endDate.month = 12; endDate.day = 24;
    CPPUNIT_ASSERT_MESSAGE("Control point not correctly extended", controlPointOfData.startPoint == startDate && controlPointOfData.endPoint == endDate);

    std::vector<mitk::DataNode::Pointer> allData = semanticRelations.GetAllDataOfControlPoint(allCaseIDs.front(), controlPointOfData);
    CPPUNIT_ASSERT_MESSAGE("Two images should reference the control point", allData.size() == 2);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSemanticRelations)
