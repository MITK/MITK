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

// semantic relations module
#include "mitkRelationStorage.h"
#include "mitkNodePredicates.h"

// multi label module
#include <mitkLabelSetImage.h>

// mitk core
#include <mitkIPersistenceService.h>
#include <mitkVectorProperty.h>

// c++
#include <algorithm>
#include <iostream>

void mitk::RelationStorage::SetDataStorage(DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }
}

std::vector<mitk::SemanticTypes::Lesion> mitk::RelationStorage::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::Lesion>();
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  mitk::VectorProperty<std::string>* vectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == vectorProperty)
  {
    MITK_INFO << "Could not find any lesion in the storage.";
    return std::vector<SemanticTypes::Lesion>();
  }

  std::vector<std::string> vectorValue = vectorProperty->GetValue();
  std::vector<SemanticTypes::Lesion> allLesionsOfCase;
  for (const auto& lesionID : vectorValue)
  {
    SemanticTypes::Lesion generatedLesion = GenerateLesion(caseID, lesionID);
    if (!generatedLesion.UID.empty())
    {
      allLesionsOfCase.push_back(generatedLesion);
    }
  }

  return allLesionsOfCase;
}

mitk::SemanticTypes::Lesion mitk::RelationStorage::GetRepresentedLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::Lesion();
  }

  // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
  mitk::VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
  if (nullptr == segmentationVectorProperty)
  {
    MITK_INFO << "Could not find the segmentation node " << segmentationID << " in the storage.";
    return SemanticTypes::Lesion();
  }

  std::vector<std::string> segmentationVectorValue = segmentationVectorProperty->GetValue();
  // the lesion ID of a segmentation is the second value in the vector
  if (segmentationVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect segmentation storage. Not two (2) IDs stored.";
    return SemanticTypes::Lesion();
  }
  else
  {
    std::string lesionID = segmentationVectorValue[1];
    return GenerateLesion(caseID, lesionID);
  }
}

std::vector<mitk::DataNode::Pointer> mitk::RelationStorage::GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID)
{
  if (m_DataStorage.IsNull())
  {
    MITK_INFO << "No valid data storage found in the mitkPersistenceService-class. Segmentations of the current case can not be retrieved.";
    return std::vector<mitk::DataNode::Pointer>();
  }

  std::vector<std::string> allSegmentationIDsOfCase = GetAllSegmentationIDsOfCase(caseID);
  std::vector<DataNode::Pointer> allSegmentationsOfCase;
  // get all segmentation nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage->GetSubset(NodePredicates::GetSegmentationPredicate());
  for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
  {
    DataNode* segmentationNode = it->Value();
    try
    {
      // find the corresponding segmentation node for the given segmentation ID
      std::string nodeCaseID = GetCaseIDFromDataNode(segmentationNode);
      std::string nodeSegmentationID = GetIDFromDataNode(segmentationNode);
      if (nodeCaseID == caseID && (std::find(allSegmentationIDsOfCase.begin(), allSegmentationIDsOfCase.end(), nodeSegmentationID) != allSegmentationIDsOfCase.end()))
      {
        // found current image node in the storage, add it to the return vector
        allSegmentationsOfCase.push_back(segmentationNode);
      }
    }
    catch (const std::exception&)
    {
      // found a segmentation node that is not stored in the semantic relations
      // this segmentation node does not have any DICOM information --> exception thrown
      // continue with the next segmentation to compare IDs
      continue;
    }
  }

  return allSegmentationsOfCase;
}

std::vector<std::string> mitk::RelationStorage::GetAllSegmentationIDsOfCase(const SemanticTypes::CaseID& caseID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::InformationType>();
  }
  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  mitk::VectorProperty<std::string>* allSegmentationsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == allSegmentationsVectorProperty)
  {
    MITK_INFO << "Could not find any segmentation in the storage.";
    return std::vector<SemanticTypes::InformationType>();
  }

  return allSegmentationsVectorProperty->GetValue();
}

mitk::SemanticTypes::ControlPoint mitk::RelationStorage::GetControlPointOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ControlPoint();
  }
  // retrieve a vector property that contains the information type and the referenced ID of a control point (0. information type 1. control point ID)
  mitk::VectorProperty<std::string>* dataNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == dataNodeVectorProperty)
  {
    MITK_INFO << "Could not find the data node " << imageID << " in the storage.";
    return SemanticTypes::ControlPoint();
  }

  std::vector<std::string> dataNodeVectorValue = dataNodeVectorProperty->GetValue();
  SemanticTypes::ControlPoint controlPoint;
  // an image node has to have exactly two values (the information type and the ID of the control point)
  if (dataNodeVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
    return SemanticTypes::ControlPoint();
  }
  else
  {
    // the second value of the data node vector is the ID of the referenced control point
    std::string controlPointID = dataNodeVectorValue[1];
    // retrieve a vector property that contains the integer values of the date of a control point (0. year 1. month 2. day)
    mitk::VectorProperty<int>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(controlPointID));
    if (nullptr == controlPointVectorProperty)
    {
      MITK_INFO << "Could not find the control point " << controlPointID << " in the storage.";
      return SemanticTypes::ControlPoint();
    }

    std::vector<int> controlPointVectorValue = controlPointVectorProperty->GetValue();
    // a control point has to have exactly three integer values (year, month and day)
    if (controlPointVectorValue.size() != 3)
    {
      MITK_INFO << "Incorrect control point storage. Not three (3) values of the date are stored.";
      return SemanticTypes::ControlPoint();
    }
    else
    {
      // set the values of the control point
      controlPoint.UID = controlPointID;
      controlPoint.date = boost::gregorian::date(controlPointVectorValue[0],
                        controlPointVectorValue[1],
                        controlPointVectorValue[2]);
    }
  }

  return controlPoint;
}

std::vector<mitk::SemanticTypes::ControlPoint> mitk::RelationStorage::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::ControlPoint>();
  }

  // retrieve a vector property that contains the valid control point-IDs for the current case
  mitk::VectorProperty<std::string>* vectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  if (nullptr == vectorProperty)
  {
    MITK_INFO << "Could not find any control points in the storage.";
    return std::vector<SemanticTypes::ControlPoint>();
  }

  std::vector<std::string> vectorValue = vectorProperty->GetValue();
  std::vector<SemanticTypes::ControlPoint> allControlPointsOfCase;
  for (const auto& controlPointUID : vectorValue)
  {
    SemanticTypes::ControlPoint generatedControlPoint = GenerateControlpoint(caseID, controlPointUID);
    if (!generatedControlPoint.UID.empty())
    {
      allControlPointsOfCase.push_back(generatedControlPoint);
    }
  }

  return allControlPointsOfCase;
}

std::vector<mitk::SemanticTypes::ExaminationPeriod> mitk::RelationStorage::GetAllExaminationPeriodsOfCase(const SemanticTypes::CaseID& caseID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::ExaminationPeriod>();
  }

  // retrieve a vector property that contains the valid examination period UIDs for the current case
  mitk::VectorProperty<std::string>::Pointer vectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("examinationperiods"));
  if (nullptr == vectorProperty)
  {
    MITK_INFO << "Could not find any examination periods in the storage.";
    return std::vector<SemanticTypes::ExaminationPeriod>();
  }

  std::vector<std::string> vectorValue = vectorProperty->GetValue();
  std::vector<SemanticTypes::ExaminationPeriod> allExaminationPeriods;
  for (const auto& examinationPeriodID : vectorValue)
  {
    // retrieve a vector property that contains the represented control point-IDs
    mitk::VectorProperty<std::string>::Pointer examinationPeriodVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(examinationPeriodID));
    if (nullptr == examinationPeriodVectorProperty)
    {
      MITK_INFO << "Could not find the examination period " << examinationPeriodID << " in the storage.";
      continue;
    }

    std::vector<std::string> examinationPeriodVectorValue = examinationPeriodVectorProperty->GetValue();
    // an examination period has an arbitrary number of vector values (name and control point UIDs) (at least one for the name)
    if (examinationPeriodVectorValue.empty())
    {
      MITK_INFO << "Incorrect examination period storage. At least one (1) value for the examination period name has to be stored.";
      continue;
    }
    else
    {
      // set the values of the name and the control points
      SemanticTypes::ExaminationPeriod generatedExaminationPeriod;
      generatedExaminationPeriod.UID = examinationPeriodID;
      generatedExaminationPeriod.name = examinationPeriodVectorValue[0];
      for (int i = 1; i < examinationPeriodVectorValue.size(); ++i)
      {
        generatedExaminationPeriod.controlPointUIDs.push_back(examinationPeriodVectorValue[i]);
      }

      allExaminationPeriods.push_back(generatedExaminationPeriod);
    }
  }
  return allExaminationPeriods;
}

mitk::SemanticTypes::InformationType mitk::RelationStorage::GetInformationTypeOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::InformationType();
  }
  // retrieve a vector property that contains the information type and the referenced ID of an image data node (0. information type 1. control point ID)
  mitk::VectorProperty<std::string>* dataNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == dataNodeVectorProperty)
  {
    MITK_INFO << "Could not find the image " << imageID << " in the storage.";
    return SemanticTypes::InformationType();
  }

  std::vector<std::string> dataNodeVectorValue = dataNodeVectorProperty->GetValue();
  // an image node has to have exactly two values (the information type and the ID of the control point)
  if (dataNodeVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
    return SemanticTypes::InformationType();
  }
  else
  {
    // the first value of the data node vector is the information type
    return dataNodeVectorValue[0];
  }
}

std::vector<mitk::SemanticTypes::InformationType> mitk::RelationStorage::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::InformationType>();
  }
  // retrieve a vector property that contains the valid information types of the current case
  mitk::VectorProperty<std::string>* informationTypeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("informationtypes"));
  if (nullptr == informationTypeVectorProperty)
  {
    MITK_INFO << "Could not find any information types in the storage.";
    return std::vector<SemanticTypes::InformationType>();
  }

  return informationTypeVectorProperty->GetValue();
}

std::vector<mitk::DataNode::Pointer> mitk::RelationStorage::GetAllImagesOfCase(const SemanticTypes::CaseID& caseID)
{
  if (m_DataStorage.IsNull())
  {
    MITK_INFO << "No valid data storage found in the mitkPersistenceService-class. Images of the current case can not be retrieved.";
    return std::vector<mitk::DataNode::Pointer>();
  }

  std::vector<std::string> allImageIDsOfCase = GetAllImageIDsOfCase(caseID);
  std::vector<DataNode::Pointer> allImagesOfCase;
  // get all image nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer imageNodes = m_DataStorage->GetSubset(NodePredicates::GetImagePredicate());
  for (auto it = imageNodes->Begin(); it != imageNodes->End(); ++it)
  {
    DataNode* imageNode = it->Value();
    // find the corresponding image node for the given segmentation ID
    std::string nodeCaseID = GetCaseIDFromDataNode(imageNode);
    std::string nodeImageID = GetIDFromDataNode(imageNode);
    if (nodeCaseID == caseID && (std::find(allImageIDsOfCase.begin(), allImageIDsOfCase.end(), nodeImageID) != allImageIDsOfCase.end()))
    {
      // found current image node in the storage, add it to the return vector
      allImagesOfCase.push_back(imageNode);
    }
  }

  return allImagesOfCase;
}

std::vector<std::string> mitk::RelationStorage::GetAllImageIDsOfCase(const SemanticTypes::CaseID& caseID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::InformationType>();
  }
  // retrieve a vector property that contains the valid image-IDs of the current case
  mitk::VectorProperty<std::string>* allImagesVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  if (nullptr == allImagesVectorProperty)
  {
    MITK_INFO << "Could not find any image in the storage.";
    return std::vector<SemanticTypes::InformationType>();
  }

  return allImagesVectorProperty->GetValue();
}

std::vector<mitk::SemanticTypes::CaseID> mitk::RelationStorage::GetAllCaseIDs()
{
  PERSISTENCE_GET_SERVICE_MACRO
  if (nullptr == persistenceService)
  {
    MITK_INFO << "Persistence service could not be loaded";
    return std::vector<SemanticTypes::CaseID>();
  }
  // the property list is valid for a certain scenario and contains all the case IDs of the radiological user's MITK session
  std::string listIdentifier = "caseIDs";
  mitk::PropertyList::Pointer propertyList = persistenceService->GetPropertyList(listIdentifier);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << listIdentifier << " for the current MITK workbench / session.";
    return std::vector<SemanticTypes::CaseID>();
  }
  // retrieve a vector property that contains all case IDs
  mitk::VectorProperty<std::string>* caseIDsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(listIdentifier));
  if (nullptr == caseIDsVectorProperty)
  {
    MITK_INFO << "Could not find the property " << listIdentifier << " for the " << listIdentifier << " property list.";
    return std::vector<SemanticTypes::CaseID>();
  }

  return caseIDsVectorProperty->GetValue();
}

void mitk::RelationStorage::AddCase(const SemanticTypes::CaseID& caseID)
{
  PERSISTENCE_GET_SERVICE_MACRO
  if (nullptr == persistenceService)
  {
    MITK_INFO << "Persistence service could not be loaded";
    return;
  }
  // the property list is valid for a certain scenario and contains all the case IDs of the radiological user's MITK session
  std::string listIdentifier = "caseIDs";
  mitk::PropertyList::Pointer propertyList = persistenceService->GetPropertyList(listIdentifier);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << listIdentifier << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains all case IDs
  mitk::VectorProperty<std::string>::Pointer caseIDsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(listIdentifier));
  std::vector<std::string> caseIDsVectorValue;
  if (nullptr == caseIDsVectorProperty)
  {
    caseIDsVectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    caseIDsVectorValue = caseIDsVectorProperty->GetValue();
  }

  auto existingCase = std::find(caseIDsVectorValue.begin(), caseIDsVectorValue.end(), caseID);
  if (existingCase != caseIDsVectorValue.end())
  {
    return;
  }
  else
  {
    // add case to the "caseIDs" property list
    caseIDsVectorValue.push_back(caseID);
    caseIDsVectorProperty->SetValue(caseIDsVectorValue);
    propertyList->SetProperty(listIdentifier, caseIDsVectorProperty);
  }
}

void mitk::RelationStorage::AddImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageNodeID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid image-IDs for the current case
  mitk::VectorProperty<std::string>::Pointer allImagesVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  std::vector<std::string> allImagesIDs;
  if (nullptr == allImagesVectorProperty)
  {
    allImagesVectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    allImagesIDs = allImagesVectorProperty->GetValue();
  }

  auto existingImage = std::find(allImagesIDs.begin(), allImagesIDs.end(), imageNodeID);
  if (existingImage != allImagesIDs.end())
  {
    return;
  }
  else
  {
    // add image to the "images" property list
    allImagesIDs.push_back(imageNodeID);
    allImagesVectorProperty->SetValue(allImagesIDs);
    propertyList->SetProperty("images", allImagesVectorProperty);

    // add the image itself
    mitk::VectorProperty<std::string>::Pointer imageNodeVectorProperty = mitk::VectorProperty<std::string>::New();
    // an image node has to have exactly two values (the information type and the ID of the control point)
    std::vector<std::string> imageNodeVectorValue(2);
    imageNodeVectorProperty->SetValue(imageNodeVectorValue);
    propertyList->SetProperty(imageNodeID, imageNodeVectorProperty);
  }
}

void mitk::RelationStorage::RemoveImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageNodeID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid image-IDs for the current case
  mitk::VectorProperty<std::string>::Pointer allImagesVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  if (nullptr == allImagesVectorProperty)
  {
    MITK_INFO << "Could not find any images in the storage.";
    return;
  }

  // remove the image reference from the list of all images of the current case
  std::vector<std::string> allImagesIDs = allImagesVectorProperty->GetValue();
  allImagesIDs.erase(std::remove(allImagesIDs.begin(), allImagesIDs.end(), imageNodeID), allImagesIDs.end());
  if (allImagesIDs.empty())
  {
    // no more images stored -> remove the images property list
    propertyList->DeleteProperty("images");
  }
  else
  {
    // or store the modified vector value
    allImagesVectorProperty->SetValue(allImagesIDs);
  }

  // remove the image instance itself
  propertyList->DeleteProperty(imageNodeID);
}

void mitk::RelationStorage::AddSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationNodeID, const SemanticTypes::ID& parentNodeID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  mitk::VectorProperty<std::string>::Pointer allSegmentationsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  std::vector<std::string> allSegmentationsIDs;
  if (nullptr == allSegmentationsVectorProperty)
  {
    allSegmentationsVectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    allSegmentationsIDs = allSegmentationsVectorProperty->GetValue();
  }

  auto existingImage = std::find(allSegmentationsIDs.begin(), allSegmentationsIDs.end(), segmentationNodeID);
  if (existingImage != allSegmentationsIDs.end())
  {
    return;
  }
  else
  {
    // add segmentation to the "segmentations" property list
    allSegmentationsIDs.push_back(segmentationNodeID);
    allSegmentationsVectorProperty->SetValue(allSegmentationsIDs);
    propertyList->SetProperty("segmentations", allSegmentationsVectorProperty);

    // add the segmentation itself
    mitk::VectorProperty<std::string>::Pointer segmentationNodeVectorProperty = mitk::VectorProperty<std::string>::New();
    // a segmentation node has to have exactly two values (the ID of the referenced image and the ID of the referenced lesion)
    std::vector<std::string> segmentationNodeVectorValue(2);
    segmentationNodeVectorValue[0] = parentNodeID;
    segmentationNodeVectorProperty->SetValue(segmentationNodeVectorValue);
    propertyList->SetProperty(segmentationNodeID, segmentationNodeVectorProperty);
  }
}

void mitk::RelationStorage::RemoveSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationNodeID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  mitk::VectorProperty<std::string>::Pointer allSegmentationsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == allSegmentationsVectorProperty)
  {
    MITK_INFO << "Could not find any segmentation in the storage.";
    return;
  }

  // remove the lesion reference from the list of all lesions of the current case
  std::vector<std::string> allSegmentationsIDs = allSegmentationsVectorProperty->GetValue();
  allSegmentationsIDs.erase(std::remove(allSegmentationsIDs.begin(), allSegmentationsIDs.end(), segmentationNodeID), allSegmentationsIDs.end());
  if (allSegmentationsIDs.empty())
  {
    // no more segmentations stored -> remove the segmentations property list
    propertyList->DeleteProperty("segmentations");
  }
  else
  {
    // or store the modified vector value
    allSegmentationsVectorProperty->SetValue(allSegmentationsIDs);
  }

  // remove the lesion instance itself
  propertyList->DeleteProperty(segmentationNodeID);
}

void mitk::RelationStorage::AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  mitk::VectorProperty<std::string>::Pointer lesionsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  std::vector<std::string> lesionsVectorValue;
  if (nullptr == lesionsVectorProperty)
  {
    lesionsVectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    lesionsVectorValue = lesionsVectorProperty->GetValue();
  }

  const auto& existingIndex = std::find(lesionsVectorValue.begin(), lesionsVectorValue.end(), lesion.UID);
  if (existingIndex != lesionsVectorValue.end())
  {
    return;
  }
  else
  {
    // add the new lesion id from the given lesion to the vector of all current lesion IDs
    lesionsVectorValue.push_back(lesion.UID);
    // overwrite the current vector property with the new, extended string vector
    lesionsVectorProperty->SetValue(lesionsVectorValue);
    propertyList->SetProperty("lesions", lesionsVectorProperty);

    // add the lesion with the lesion UID as the key and the lesion information as value
    std::vector<std::string> lesionData;
    lesionData.push_back(lesion.name);
    lesionData.push_back(lesion.lesionClass.UID);
    mitk::VectorProperty<std::string>::Pointer newLesionVectorProperty = mitk::VectorProperty<std::string>::New();
    newLesionVectorProperty->SetValue(lesionData);
    propertyList->SetProperty(lesion.UID, newLesionVectorProperty);

    // add the lesion class with the lesion class UID as key and the class type as value
    std::string lesionClassType = lesion.lesionClass.classType;
    propertyList->SetStringProperty(lesion.lesionClass.UID.c_str(), lesionClassType.c_str());
  }
}

void mitk::RelationStorage::OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  mitk::VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    MITK_INFO << "Could not find any lesion in the storage.";
    return;
  }

  std::vector<std::string> lesionVectorValue = lesionVectorProperty->GetValue();
  const auto existingLesion = std::find(lesionVectorValue.begin(), lesionVectorValue.end(), lesion.UID);
  if (existingLesion != lesionVectorValue.end())
  {
    // overwrite the referenced lesion class UID with the new, given lesion class data
    std::vector<std::string> lesionData;
    lesionData.push_back(lesion.name);
    lesionData.push_back(lesion.lesionClass.UID);
    mitk::VectorProperty<std::string>::Pointer newLesionVectorProperty = mitk::VectorProperty<std::string>::New();
    newLesionVectorProperty->SetValue(lesionData);
    propertyList->SetProperty(lesion.UID, newLesionVectorProperty);

    // overwrite the lesion class with the lesion class UID as key and the new, given class type as value
    std::string lesionClassType = lesion.lesionClass.classType;
    propertyList->SetStringProperty(lesion.lesionClass.UID.c_str(), lesionClassType.c_str());
  }
  else
  {
    MITK_INFO << "Could not find lesion " << lesion.UID << " in the storage. Cannot overwrite the lesion.";    
  }
}

void mitk::RelationStorage::LinkSegmentationToLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::Lesion& lesion)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  mitk::VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    MITK_INFO << "Could not find any lesion property in the storage.";
    return;
  }

  std::vector<std::string> lesionVectorValue = lesionVectorProperty->GetValue();
  const auto existingLesion = std::find(lesionVectorValue.begin(), lesionVectorValue.end(), lesion.UID);
  if (existingLesion != lesionVectorValue.end())
  {
    // set / overwrite the lesion reference of the given segmentation
    // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
    mitk::VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
    if (nullptr == segmentationVectorProperty)
    {
      MITK_INFO << "Could not find the segmentation node " << segmentationID << " in the storage. Cannot link segmentation to lesion.";
      return;
    }

    std::vector<std::string> segmentationVectorValue = segmentationVectorProperty->GetValue();
    if (segmentationVectorValue.size() != 2)
    {
      MITK_INFO << "Incorrect segmentation storage. Not two (2) IDs stored.";
      return;
    }
    else
    {
      // the lesion ID of a segmentation is the second value in the vector
      segmentationVectorValue[1] = lesion.UID;
      segmentationVectorProperty->SetValue(segmentationVectorValue);
    }
  }
  else
  {
    MITK_INFO << "Could not find lesion " << lesion.UID << " in the storage. Cannot link segmentation to lesion.";
  }
}

void mitk::RelationStorage::UnlinkSegmentationFromLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
  mitk::VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
  if (nullptr == segmentationVectorProperty)
  {
    MITK_INFO << "Could not find the segmentation node " << segmentationID << " in the storage. Cannot unlink lesion from segmentation.";
    return;
  }

  std::vector<std::string> segmentationVectorValue = segmentationVectorProperty->GetValue();
  // a segmentation has to have exactly two values (the ID of the linked image and the ID of the lesion)
  if (segmentationVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
    return;
  }
  else
  {
    // the second value of the data node vector is the ID of the referenced lesion
    // set the lesion reference to an empty string for removal
    segmentationVectorValue[1] = "";
    segmentationVectorProperty->SetValue(segmentationVectorValue);
  }
}

void mitk::RelationStorage::RemoveLesion(const mitk::SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesions of the current case
  mitk::VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    MITK_INFO << "Could not find any lesion property in the storage.";
    return;
  }

  // remove the lesion reference from the list of all lesions of the current case
  std::vector<std::string> lesionVectorValue = lesionVectorProperty->GetValue();
  lesionVectorValue.erase(std::remove(lesionVectorValue.begin(), lesionVectorValue.end(), lesion.UID), lesionVectorValue.end());
  if (lesionVectorValue.empty())
  {
    // no more lesions stored -> remove the lesions property list
    propertyList->DeleteProperty("lesions");
  }
  else
  {
    // or store the modified vector value
    lesionVectorProperty->SetValue(lesionVectorValue);
  }

  // remove the lesion instance itself
  // the lesion data is stored under the lesion ID
  mitk::VectorProperty<std::string>* lesionDataProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(lesion.UID));
  if (nullptr == lesionDataProperty)
  {
    MITK_INFO << "Lesion " << lesion.UID << " not found (already removed?). Cannot remove the lesion.";
    return;
  }

  std::vector<std::string> lesionData = lesionDataProperty->GetValue();
  // a lesion date has to have exactly two values (the name of the lesion and the UID of the lesion class)
  if (lesionData.size() != 2)
  {
    MITK_INFO << "Incorrect lesion data storage. Not two (2) strings of the lesion UID and the lesion name are stored.";
  }
  else
  {
    std::string lesionClassID = lesionData[1];
    RemoveLesionClass(caseID, lesionClassID);
  }
  propertyList->DeleteProperty(lesion.UID);
}

void mitk::RelationStorage::RemoveLesionClass(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& lesionClassID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the lesion class
  mitk::StringProperty* lesionClassProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionClassID));
  if (nullptr == lesionClassProperty)
  {
    MITK_INFO << "Lesion class " << lesionClassID << " not found (already removed?). Cannot remove the lesion class.";
    return;
  }

  // retrieve a vector property that contains the valid lesions of the current case
  mitk::VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    return;
  }

  // check if the lesion class ID is referenced by any other lesion
  std::vector<std::string> lesionVectorValue = lesionVectorProperty->GetValue();
  const auto existingLesionClass = std::find_if(lesionVectorValue.begin(), lesionVectorValue.end(),
    [&propertyList, &lesionClassID](const std::string& lesionID)
  {
    mitk::VectorProperty<std::string>* lesionDataProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(lesionID));
    if (nullptr == lesionDataProperty)
    {
      return false;
    }
    else
    {
      std::vector<std::string> lesionData = lesionDataProperty->GetValue();
      // a lesion date has to have exactly two values (the name of the lesion and the UID of the lesion class)
      if (lesionData.size() != 2)
      {
        return false;
      }
      else
      {
        return lesionData[1] == lesionClassID;
      }
    }
  });

  if (existingLesionClass == lesionVectorValue.end())
  {
    // lesion class ID not referenced; remove lesion class
    propertyList->DeleteProperty(lesionClassID);
  }
}

void mitk::RelationStorage::AddControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid controlPoint UIDs for the current case
  mitk::VectorProperty<std::string>::Pointer controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  std::vector<std::string> controlPointVectorValue;
  if (nullptr == controlPointVectorProperty)
  {
    controlPointVectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    controlPointVectorValue = controlPointVectorProperty->GetValue();
  }

  const auto existingControlPoint = std::find(controlPointVectorValue.begin(), controlPointVectorValue.end(), controlPoint.UID);
  if (existingControlPoint != controlPointVectorValue.end())
  {
    return;
  }
  else
  {
    // add the new control point UID from the given control point to the vector of all current control point UIDs
    controlPointVectorValue.push_back(controlPoint.UID);
    // overwrite the current vector property with the new, extended string vector
    controlPointVectorProperty->SetValue(controlPointVectorValue);
    propertyList->SetProperty("controlpoints", controlPointVectorProperty);

    // store the control point values (the three integer values of a date)
    std::vector<int> controlPointDate;
    controlPointDate.push_back(controlPoint.date.year());
    controlPointDate.push_back(controlPoint.date.month());
    controlPointDate.push_back(controlPoint.date.day());

    mitk::VectorProperty<int>::Pointer newControlPointVectorProperty = mitk::VectorProperty<int>::New();
    newControlPointVectorProperty->SetValue(controlPointDate);
    propertyList->SetProperty(controlPoint.UID, newControlPointVectorProperty);
  }
}

void mitk::RelationStorage::LinkDataToControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID, const SemanticTypes::ControlPoint& controlPoint)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid controlPoint UIDs for the current case
  mitk::VectorProperty<std::string>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  if (nullptr == controlPointVectorProperty)
  {
    MITK_INFO << "Could not find any control point property in the storage.";
    return;
  }

  std::vector<std::string> controlPointVectorValue = controlPointVectorProperty->GetValue();
  const auto existingControlPoint = std::find(controlPointVectorValue.begin(), controlPointVectorValue.end(), controlPoint.UID);
  if (existingControlPoint != controlPointVectorValue.end())
  {
    // set / overwrite the control point reference of the given data
    // retrieve a vector property that contains the referenced ID of a data node (0. information type 1. control point ID)
    mitk::VectorProperty<std::string>* dataNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(dataNodeID));
    if (nullptr == dataNodeVectorProperty)
    {
      MITK_INFO << "Could not find the data node " << dataNodeID << " in the storage. Cannot link data to control point.";
      return;
    }

    std::vector<std::string> dataNodeVectorValue = dataNodeVectorProperty->GetValue();
    // an image node has to have exactly two values (the information type and the ID of the control point)
    if (dataNodeVectorValue.size() != 2)
    {
      MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
      return;
    }
    else
    {
      // the second value of the data node vector is the ID of the referenced control point
      dataNodeVectorValue[1] = controlPoint.UID;
      dataNodeVectorProperty->SetValue(dataNodeVectorValue);
    }
  }
  else
  {
    MITK_INFO << "Could not find control point " << controlPoint.UID << " in the storage. Cannot link data to control point.";
  }
}

void mitk::RelationStorage::UnlinkDataFromControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the referenced ID of a date (0. information type 1. control point ID)
  mitk::VectorProperty<std::string>* dataNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(dataNodeID));
  if (nullptr == dataNodeVectorProperty)
  {
    MITK_INFO << "Could not find the date " << dataNodeID << " in the storage. Cannot unlink control point from date.";
    return;
  }

  std::vector<std::string> dataNodeVectorValue = dataNodeVectorProperty->GetValue();
  // a data node has to have exactly two values (the information type and the ID of the control point)
  if (dataNodeVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
    return;
  }
  else
  {
    // the second value of the data node vector is the ID of the referenced control point
    // set the control point reference to an empty string for removal
    dataNodeVectorValue[1] = "";
    dataNodeVectorProperty->SetValue(dataNodeVectorValue);
  }
}

void mitk::RelationStorage::RemoveControlPointFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid controlPoint UIDs for the current case
  mitk::VectorProperty<std::string>* allControlPointsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  if (nullptr == allControlPointsVectorProperty)
  {
    MITK_INFO << "Could not find any control point property in the storage.";
    return;
  }

  // remove the control point reference from the list of all control points of the current case
  std::vector<std::string> currentControlPointVectorValue = allControlPointsVectorProperty->GetValue();
  currentControlPointVectorValue.erase(std::remove(currentControlPointVectorValue.begin(), currentControlPointVectorValue.end(), controlPoint.UID), currentControlPointVectorValue.end());
  allControlPointsVectorProperty->SetValue(currentControlPointVectorValue);

  // remove the control point instance itself
  propertyList->DeleteProperty(controlPoint.UID);
}

void mitk::RelationStorage::AddExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid examination period UIDs for the current case
  mitk::VectorProperty<std::string>::Pointer vectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("examinationperiods"));
  std::vector<std::string> examinationPeriodsVectorValue;
  if (nullptr == vectorProperty)
  {
    vectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    examinationPeriodsVectorValue = vectorProperty->GetValue();
  }

  const auto& existingIndex = std::find(examinationPeriodsVectorValue.begin(), examinationPeriodsVectorValue.end(), examinationPeriod.UID);
  if (existingIndex != examinationPeriodsVectorValue.end())
  {
    return;
  }
  else
  {
    // add the new examination period id from the given examination period to the vector of all current examination period UIDs
    examinationPeriodsVectorValue.push_back(examinationPeriod.UID);
    // overwrite the current vector property with the new, extended string vector
    vectorProperty->SetValue(examinationPeriodsVectorValue);
    propertyList->SetProperty("examinationperiods", vectorProperty);

    // add the examination period with the UID as the key and the name as as the vector value
    std::vector<std::string> examinationPeriodData;
    examinationPeriodData.push_back(examinationPeriod.name);
    mitk::VectorProperty<std::string>::Pointer newExaminationPeriodVectorProperty = mitk::VectorProperty<std::string>::New();
    newExaminationPeriodVectorProperty->SetValue(examinationPeriodData);
    propertyList->SetProperty(examinationPeriod.UID, newExaminationPeriodVectorProperty);
  }
}

void mitk::RelationStorage::AddControlPointToExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod examinationPeriod)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the represented control point UIDs of the given examination period
  mitk::VectorProperty<std::string>* controlPointUIDsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(examinationPeriod.UID));
  if (nullptr == controlPointUIDsVectorProperty)
  {
    MITK_INFO << "Could not find the examination period " << examinationPeriod.UID << " in the storage. Cannot add the control point to the examination period.";
    return;
  }

  std::vector<std::string> controlPointUIDsVectorValue = controlPointUIDsVectorProperty->GetValue();
  // store the control point UID
  controlPointUIDsVectorValue.push_back(controlPoint.UID);
  // sort the vector according to the date of the control points referenced by the UIDs
  auto lambda = [&caseID, this](const SemanticTypes::ID& leftControlPointUID, const SemanticTypes::ID& rightControlPointUID)
  {
    const auto& leftControlPoint = GenerateControlpoint(caseID, leftControlPointUID);
    const auto& rightControlPoint = GenerateControlpoint(caseID, rightControlPointUID);

    return leftControlPoint.date <= rightControlPoint.date;
  };

  std::sort(controlPointUIDsVectorValue.begin(), controlPointUIDsVectorValue.end(), lambda);
  // store the modified and sorted control point UID vector of this examination period
  controlPointUIDsVectorProperty->SetValue(controlPointUIDsVectorValue);
}

void mitk::RelationStorage::RemoveControlPointFromExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod examinationPeriod)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the represented control point UIDs of the given examination period
  mitk::VectorProperty<std::string>* controlPointUIDsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(examinationPeriod.UID));
  if (nullptr == controlPointUIDsVectorProperty)
  {
    MITK_INFO << "Could not find examination period " << examinationPeriod.UID << " in the storage. Cannot add the control point to the examination period.";
    return;
  }

  std::vector<std::string> controlPointUIDsVectorValue = controlPointUIDsVectorProperty->GetValue();
  // an examination period has an arbitrary number of vector values (name and control point UIDs) (at least one for the name)
  if (controlPointUIDsVectorValue.size() < 2)
  {
    MITK_INFO << "Incorrect examination period storage. At least one (1) control point ID has to be stored.";
    return;
  }
  else
  {
    controlPointUIDsVectorValue.erase(std::remove(controlPointUIDsVectorValue.begin(), controlPointUIDsVectorValue.end(), controlPoint.UID), controlPointUIDsVectorValue.end());
    if (controlPointUIDsVectorValue.size() < 2)
    {
      RemoveExaminationPeriodFromCase(caseID, examinationPeriod);
    }
    else
    {
      // store the modified vector value
      controlPointUIDsVectorProperty->SetValue(controlPointUIDsVectorValue);
    }
  }
}

void mitk::RelationStorage::RemoveExaminationPeriodFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod examinationPeriod)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid examination period UIDs for the current case
  mitk::VectorProperty<std::string>::Pointer vectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("examinationperiods"));
  if (nullptr == vectorProperty)
  {
    MITK_INFO << "Could not find any examination periods in the storage.";
    return;
  }

  std::vector<std::string> examinationPeriodVectorValue = vectorProperty->GetValue();
  examinationPeriodVectorValue.erase(std::remove(examinationPeriodVectorValue.begin(), examinationPeriodVectorValue.end(), examinationPeriod.UID), examinationPeriodVectorValue.end());
  if (examinationPeriodVectorValue.empty())
  {
    // no more examination periods stored -> remove the examination period property list
    propertyList->DeleteProperty("examinationperiods");
  }
  else
  {
    // or store the modified vector value
    vectorProperty->SetValue(examinationPeriodVectorValue);
  }
}

void mitk::RelationStorage::AddInformationTypeToImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::InformationType informationType)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid information types of the current case
  mitk::VectorProperty<std::string>::Pointer informationTypeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("informationtypes"));
  std::vector<std::string> informationTypeVectorValue;
  if (nullptr == informationTypeVectorProperty)
  {
    informationTypeVectorProperty = mitk::VectorProperty<std::string>::New();
  }
  else
  {
    informationTypeVectorValue = informationTypeVectorProperty->GetValue();
  }
 
  const auto existingInformationType = std::find(informationTypeVectorValue.begin(), informationTypeVectorValue.end(), informationType);
  if (existingInformationType == informationTypeVectorValue.end())
  {
    // at first: add the information type to the storage
    informationTypeVectorValue.push_back(informationType);
    informationTypeVectorProperty->SetValue(informationTypeVectorValue);
    propertyList->SetProperty("informationtypes", informationTypeVectorProperty);
  }

  // set / overwrite the information type of the given data
  // retrieve a vector property that contains the referenced ID of an image (0. information type 1. control point ID)
  mitk::VectorProperty<std::string>* imageNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageNodeVectorProperty)
  {
    MITK_INFO << "Could not find the image " << imageID << " in the storage. Cannot add information type to image.";
    return;
  }

  std::vector<std::string> imageNodeVectorValue = imageNodeVectorProperty->GetValue();
  // an image node has to have exactly two values (the information type and the ID of the control point)
  if (imageNodeVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
    return;
  }
  else
  {
    // the first value of the data node vector is the information type
    imageNodeVectorValue[0] = informationType;
    imageNodeVectorProperty->SetValue(imageNodeVectorValue);
  }
}

void mitk::RelationStorage::RemoveInformationTypeFromImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the referenced ID of an image (0. information type 1. control point ID)
  mitk::VectorProperty<std::string>* imageNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageNodeVectorProperty)
  {
    MITK_INFO << "Could not find the image " << imageID << " in the storage. Cannot remove information type from image.";
    return;
  }

  std::vector<std::string> imageNodeVectorValue = imageNodeVectorProperty->GetValue();
  // an image node has to have exactly two values (the information type and the ID of the control point)
  if (imageNodeVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect data storage. Not two (2) values stored.";
    return;
  }
  else
  {
    // the first value of the data node vector is the information type
    // set the information type to an empty string for removal
    imageNodeVectorValue[0] = "";
    imageNodeVectorProperty->SetValue(imageNodeVectorValue);
  }
}

void mitk::RelationStorage::RemoveInformationTypeFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType informationType)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid information types of the current case
  mitk::VectorProperty<std::string>* informationTypeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("informationtypes"));
  if (nullptr == informationTypeVectorProperty)
  {
    MITK_INFO << "Could not find any information type property in the storage.";
    return;
  }

  std::vector<std::string> informationTypeVectorValue = informationTypeVectorProperty->GetValue();
  informationTypeVectorValue.erase(std::remove(informationTypeVectorValue.begin(), informationTypeVectorValue.end(), informationType), informationTypeVectorValue.end());
  if (informationTypeVectorValue.empty())
  {
    // no more information types stored -> remove the information types property list
    propertyList->DeleteProperty("informationtypes");
  }
  else
  {
    // or store the modified vector value
    informationTypeVectorProperty->SetValue(informationTypeVectorValue);
  }
}

mitk::PropertyList::Pointer mitk::RelationStorage::GetStorageData(const SemanticTypes::CaseID& caseID)
{
  // access the storage
  PERSISTENCE_GET_SERVICE_MACRO
  if (nullptr == persistenceService)
  {
    MITK_INFO << "Persistence service could not be loaded";
    return nullptr;
  }

  // the property list is valid for a whole case and contains all the properties for the current case
  // the persistence service may create a new property list with the given ID, if no property list is found
  return persistenceService->GetPropertyList(const_cast<SemanticTypes::CaseID&>(caseID));
}

mitk::SemanticTypes::Lesion mitk::RelationStorage::GenerateLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& lesionID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::Lesion();
  }

  mitk::VectorProperty<std::string>* lesionDataProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(lesionID));
  if (nullptr == lesionDataProperty)
  {
    MITK_INFO << "Lesion " << lesionID << " not found. Lesion can not be retrieved.";
    return SemanticTypes::Lesion();
  }

  std::vector<std::string> lesionData = lesionDataProperty->GetValue();
  // a lesion date has to have exactly two values (the name of the lesion and the UID of the lesion class)
  if (lesionData.size() != 2)
  {
    MITK_INFO << "Incorrect lesion data storage. Not two (2) strings of the lesion name and the lesion UID are stored.";
    return SemanticTypes::Lesion();
  }

  // the lesion class ID is stored as the second property
  std::string lesionClassID = lesionData[1];
  mitk::StringProperty* lesionClassProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionClassID));
  if (nullptr != lesionClassProperty)
  {
    SemanticTypes::LesionClass generatedLesionClass;
    generatedLesionClass.UID = lesionClassID;
    generatedLesionClass.classType = lesionClassProperty->GetValue();

    SemanticTypes::Lesion generatedLesion;
    generatedLesion.UID = lesionID;
    generatedLesion.name = lesionData[0];
    generatedLesion.lesionClass = generatedLesionClass;

    return generatedLesion;
  }
  else
  {
    MITK_INFO << "Incorrect lesion class storage. Lesion " << lesionID << " can not be retrieved.";
    return SemanticTypes::Lesion();
  }
}

mitk::SemanticTypes::ControlPoint mitk::RelationStorage::GenerateControlpoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& controlPointUID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ControlPoint();
  }

  // retrieve a vector property that contains the integer values of the date of a control point (0. year 1. month 2. day)
  mitk::VectorProperty<int>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(controlPointUID));
  if (nullptr == controlPointVectorProperty)
  {
    MITK_INFO << "Could not find the control point " << controlPointUID << " in the storage.";
    return SemanticTypes::ControlPoint();
  }

  std::vector<int> controlPointVectorValue = controlPointVectorProperty->GetValue();
  // a control point has to have exactly three integer values (year, month and day)
  if (controlPointVectorValue.size() != 3)
  {
    MITK_INFO << "Incorrect control point storage. Not three (3) values of the date are stored.";
    return SemanticTypes::ControlPoint();
  }

  // set the values of the control point
  SemanticTypes::ControlPoint generatedControlPoint;
  generatedControlPoint.UID = controlPointUID;
  generatedControlPoint.date = boost::gregorian::date(controlPointVectorValue[0],
                                                      controlPointVectorValue[1],
                                                      controlPointVectorValue[2]);

  return generatedControlPoint;
}
