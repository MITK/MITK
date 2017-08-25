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

// semantic relation module
#include "mitkRelationStorage.h"

// multi label module
#include <mitkLabelSetImage.h>

// mitk core
#include <mitkIPersistenceService.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
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
    return std::vector<SemanticTypes::Lesion>();
  }

  std::vector<std::string> vectorValue = vectorProperty->GetValue();
  std::vector<SemanticTypes::Lesion> allLesionsOfCase;
  for (const auto& lesionID : vectorValue)
  {
    // the lesion class ID is stored under the lesion ID
    mitk::StringProperty* lesionClassIDProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionID));
    if (nullptr == lesionClassIDProperty)
    {
      MITK_INFO << "Incorrect lesion storage. Lesion " << lesionID << " can not be created and retrieved";
      continue;
    }

    // the lesion class type is stored under the lesion class ID
    std::string lesionClassID = lesionClassIDProperty->GetValue();
    mitk::StringProperty* lesionClassProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionClassID));

    if (nullptr != lesionClassProperty)
    {
      SemanticTypes::LesionClass generatedLesionClass;
      generatedLesionClass.UID = lesionClassID;
      generatedLesionClass.classType = lesionClassProperty->GetValue();

      SemanticTypes::Lesion generatedLesion;
      generatedLesion.UID = lesionID;
      generatedLesion.lesionClass = generatedLesionClass;

      allLesionsOfCase.push_back(generatedLesion);
    }
    else
    {
      MITK_INFO << "Incorrect lesion class storage. Lesion " << lesionID << " can not be created and retrieved";
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
    SemanticTypes::Lesion representedLesion;

    std::string lesionID = segmentationVectorValue[1];
    // the lesion class ID is stored under the lesion ID
    std::string lesionClassID;
    propertyList->GetStringProperty(lesionID.c_str(), lesionClassID);
    representedLesion.UID = lesionID;

    // the lesion class type is stored under the lesion class ID
    std::string lesionClass;
    propertyList->GetStringProperty(lesionClassID.c_str(), lesionClass);

    SemanticTypes::LesionClass representedLesionClass;
    representedLesionClass.UID = lesionClassID;
    representedLesionClass.classType = lesionClass;

    representedLesion.lesionClass = representedLesionClass;

    return representedLesion;
  }
}

std::vector<mitk::DataNode::Pointer> mitk::RelationStorage::GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID)
{
  if (m_DataStorage.IsNull())
  {
    MITK_INFO << "No valid data storage found in the mitkPersistenceService-class. Segmentations of the current case can not be retrieved.";
    return std::vector<mitk::DataNode::Pointer>();
  }

  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return std::vector<mitk::DataNode::Pointer>();
  }
  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  mitk::VectorProperty<std::string>* segmentationsProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == segmentationsProperty)
  {
    return std::vector<mitk::DataNode::Pointer>();
  }

  std::vector<std::string> allSegmentationIDsOfCase = segmentationsProperty->GetValue();
  // prepare node predicate to find segmentation nodes
  NodePredicateAnd::Pointer segmentationPredicate = NodePredicateAnd::New();
  segmentationPredicate->AddPredicate(TNodePredicateDataType<LabelSetImage>::New());
  segmentationPredicate->AddPredicate(NodePredicateNot::New(NodePredicateProperty::New("helper object")));

  std::vector<DataNode::Pointer> allSegmentationsOfCase;
  // get all segmentation nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage->GetSubset(segmentationPredicate);
  for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
  {
    DataNode* segmentationNode = it->Value();
    // find the corresponding segmentation node for the given segmentation ID
    std::string nodeCaseID = DICOMHelper::GetCaseIDFromDataNode(segmentationNode);
    std::string nodeSegmentationID = DICOMHelper::GetIDFromDataNode(segmentationNode);
    if (nodeCaseID == caseID && (std::find(allSegmentationIDsOfCase.begin(), allSegmentationIDsOfCase.end(), nodeSegmentationID) != allSegmentationIDsOfCase.end()))
    {
      // found current image node in the storage, add it to the return vector
      allSegmentationsOfCase.push_back(segmentationNode);
    }
  }

  return allSegmentationsOfCase;
}

mitk::SemanticTypes::ControlPoint mitk::RelationStorage::GetControlPointOfData(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID)
{
  mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_INFO << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ControlPoint();
  }
  // retrieve a vector property that contains the information type and the referenced ID of a data node (0. information type 1. control point ID)
  mitk::VectorProperty<std::string>* dataNodeVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(dataNodeID));
  if (nullptr == dataNodeVectorProperty)
  {
    MITK_INFO << "Could not find the data node " << dataNodeID << " in the storage.";
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
    // retrieve a vector property that contains the referenced ID of the dates of a control point (0. startPoint ID 1. endPoint ID)
    mitk::VectorProperty<std::string>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(controlPointID));
    if (nullptr == controlPointVectorProperty)
    {
      MITK_INFO << "Could not find the control point " << controlPointID << " in the storage.";
      return SemanticTypes::ControlPoint();
    }

    std::vector<std::string> controlPointVectorValue = controlPointVectorProperty->GetValue();
    // a control point has to have exactly two values (the ID of two dates)
    if (controlPointVectorValue.size() != 2)
    {
      MITK_INFO << "Incorrect control point storage. Not two (2) IDs of the start point and the end point are stored.";
      return SemanticTypes::ControlPoint();
    }
    else
    {
      // retrieve the start date
      std::string startPointID = controlPointVectorValue[0];
      mitk::VectorProperty<int>* startPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(startPointID));
      if (nullptr == startPointVectorProperty)
      {
        MITK_INFO << "Could not find the start date " << startPointID << " of the control point " << controlPointID << " in the storage.";
        return SemanticTypes::ControlPoint();
      }

      std::vector<int> startPointValue = startPointVectorProperty->GetValue();
      SemanticTypes::Date startPoint;
      // a date has to have exactly three integer values
      if (startPointValue.size() != 3)
      {
        MITK_INFO << "Incorrect start point storage. Not three (3) values of the date are stored.";
        return SemanticTypes::ControlPoint();
      }
      else
      {
        startPoint.UID = startPointID;
        startPoint.year = startPointValue[0];
        startPoint.month = startPointValue[1];
        startPoint.day = startPointValue[2];
      }

      // retrieve the end date
      std::string endPointID = controlPointVectorValue[1];
      mitk::VectorProperty<int>* endPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(endPointID));
      if (nullptr == endPointVectorProperty)
      {
        MITK_INFO << "Could not find the end date " << endPointID << " of the control point " << controlPointID << " in the storage.";
        return SemanticTypes::ControlPoint();
      }

      std::vector<int> endPointValue = endPointVectorProperty->GetValue();
      SemanticTypes::Date endPoint;
      // a date has to have exactly three integer values
      if (endPointValue.size() != 3)
      {
        MITK_INFO << "Incorrect end point storage. Not three (3) values of the date are stored.";
        return SemanticTypes::ControlPoint();
      }
      else
      {
        endPoint.UID = endPointID;
        endPoint.year = endPointValue[0];
        endPoint.month = endPointValue[1];
        endPoint.day = endPointValue[2];
      }

      // set the values of the control point
      controlPoint.UID = controlPointID;
      controlPoint.startPoint = startPoint;
      controlPoint.endPoint = endPoint;
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
    return std::vector<SemanticTypes::ControlPoint>();
  }

  std::vector<std::string> vectorValue = vectorProperty->GetValue();
  std::vector<SemanticTypes::ControlPoint> allControlPoints;
  for (const auto& controlPointID : vectorValue)
  {
    // retrieve a vector property that contains the referenced ID of the dates of a control point (0. startPoint ID 1. endPoint ID)
    mitk::VectorProperty<std::string>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(controlPointID));
    if (nullptr == controlPointVectorProperty)
    {
      MITK_INFO << "Could not find the control point " << controlPointID << " in the storage.";
      continue;
    }

    std::vector<std::string> controlPointVectorValue = controlPointVectorProperty->GetValue();
    // a control point has to have exactly two values (the ID of two dates)
    if (controlPointVectorValue.size() != 2)
    {
      MITK_INFO << "Incorrect control point storage. Not two (2) IDs of the start point and the end point are stored.";
      continue;
    }
    else
    {
      // retrieve the start date
      std::string startPointID = controlPointVectorValue[0];
      mitk::VectorProperty<int>* startPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(startPointID));
      if (nullptr == startPointVectorProperty)
      {
        MITK_INFO << "Could not find the start date " << startPointID << " of the control point " << controlPointID << " in the storage.";
        continue;
      }

      std::vector<int> startPointValue = startPointVectorProperty->GetValue();
      SemanticTypes::Date startPoint;
      // a date has to have exactly three integer values
      if (startPointValue.size() != 3)
      {
        MITK_INFO << "Incorrect start point storage. Not three (3) values of the date are stored.";
        continue;
      }
      else
      {
        startPoint.UID = startPointID;
        startPoint.year = startPointValue[0];
        startPoint.month = startPointValue[1];
        startPoint.day = startPointValue[2];
      }

      // retrieve the end date
      std::string endPointID = controlPointVectorValue[1];
      mitk::VectorProperty<int>* endPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(endPointID));
      if (nullptr == endPointVectorProperty)
      {
        MITK_INFO << "Could not find the end date " << endPointID << " of the control point " << controlPointID << " in the storage.";
        continue;
      }

      std::vector<int> endPointValue = endPointVectorProperty->GetValue();
      SemanticTypes::Date endPoint;
      // a date has to have exactly three integer values
      if (endPointValue.size() != 3)
      {
        MITK_INFO << "Incorrect end point storage. Not three (3) values of the date are stored.";
        continue;
      }
      else
      {
        endPoint.UID = endPointID;
        endPoint.year = endPointValue[0];
        endPoint.month = endPointValue[1];
        endPoint.day = endPointValue[2];
      }

      SemanticTypes::ControlPoint generatedControlPoint;
      generatedControlPoint.UID = controlPointID;
      generatedControlPoint.startPoint = startPoint;
      generatedControlPoint.endPoint = endPoint;

      allControlPoints.push_back(generatedControlPoint);
    }
  }
  return allControlPoints;
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

  // prepare node predicate to find image nodes
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  /*
  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
  */

  // prepare node predicate to filter segmentation nodes
  NodePredicateAnd::Pointer segmentationPredicate = NodePredicateAnd::New();
  segmentationPredicate->AddPredicate(TNodePredicateDataType<LabelSetImage>::New());
  segmentationPredicate->AddPredicate(NodePredicateNot::New(NodePredicateProperty::New("helper object")));

  NodePredicateOr::Pointer validImages = NodePredicateOr::New();
  validImages->AddPredicate(isImage);
  /*
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isQbi);
  */
  NodePredicateAnd::Pointer imagePredicate = NodePredicateAnd::New();
  imagePredicate->AddPredicate(validImages);
  imagePredicate->AddPredicate(NodePredicateNot::New(segmentationPredicate));
  imagePredicate->AddPredicate(NodePredicateNot::New(NodePredicateAnd::New(isBinary, isImage)));
  imagePredicate->AddPredicate(NodePredicateNot::New(NodePredicateProperty::New("helper object")));

  std::vector<DataNode::Pointer> allImagesOfCase;
  // get all image nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer imageNodes = m_DataStorage->GetSubset(imagePredicate);
  for (auto it = imageNodes->Begin(); it != imageNodes->End(); ++it)
  {
    DataNode* imageNode = it->Value();
    // find the corresponding image node for the given segmentation ID
    std::string nodeCaseID = DICOMHelper::GetCaseIDFromDataNode(imageNode);
    std::string nodeImageID = DICOMHelper::GetIDFromDataNode(imageNode);
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
  if (allSegmentationsIDs.size() == 0)
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

    // add the lesion with the lesion UID as the key and the lesion class UID as value
    std::string lesionID = lesion.UID;
    std::string lesionClassID = lesion.lesionClass.UID;
    propertyList->SetStringProperty(lesionID.c_str(), lesionClassID.c_str());

    // add the lesion class with the lesion class UID as key and the class type as value
    std::string lesionClassType = lesion.lesionClass.classType;
    propertyList->SetStringProperty(lesionClassID.c_str(), lesionClassType.c_str());
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
    // overwrite the lesion with the new, given lesion class UID
    std::string lesionID = *existingLesion;
    std::string lesionClassID = lesion.lesionClass.UID;
    propertyList->SetStringProperty(lesionID.c_str(), lesionClassID.c_str());

    // overwrite the lesion class with the lesion class UID as key and the class type as value
    std::string lesionClassType = lesion.lesionClass.classType;
    propertyList->SetStringProperty(lesionClassID.c_str(), lesionClassType.c_str());
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
  if (lesionVectorValue.size() == 0)
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
  // the lesion class ID is stored under the lesion ID
  mitk::StringProperty* lesionClassIDProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesion.UID));
  if (nullptr == lesionClassIDProperty)
  {
    MITK_INFO << "Lesion " << lesion.UID << " not found (already removed?). Cannot remove the lesion.";
    return;
  }

  // the lesion class type is stored under the lesion class ID
  std::string lesionClassID = lesionClassIDProperty->GetValue();
  mitk::StringProperty* lesionClassProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionClassID));
  if (nullptr == lesionClassProperty)
  {
    MITK_INFO << "No lesion class found for " << lesion.UID << ". Just removing the lesion class ID.";
  }
  else
  {
    // check if the lesion class ID is referenced by any other lesion
    const auto existingLesionClass = std::find_if(lesionVectorValue.begin(), lesionVectorValue.end(),
      [&propertyList, &lesionClassID](const std::string& lesionID)
    {
      mitk::StringProperty* lesionClassIDProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionID));
      if (nullptr == lesionClassIDProperty)
      {
        return false;
      }
      else
      {
        return lesionClassIDProperty->GetValue() == lesionClassID;
      }
    });

    if (existingLesionClass == lesionVectorValue.end())
    {
      // lesion class ID not referenced; remove lesion class
      propertyList->DeleteProperty(lesionClassID);
    }
  }
  propertyList->DeleteProperty(lesion.UID);
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

    // set the year, month and day of the start point
    std::vector<int> startPointValue;
    startPointValue.push_back(controlPoint.startPoint.year);
    startPointValue.push_back(controlPoint.startPoint.month);
    startPointValue.push_back(controlPoint.startPoint.day);

    // store the start point
    mitk::VectorProperty<int>::Pointer startPointVectorProperty = mitk::VectorProperty<int>::New();
    startPointVectorProperty->SetValue(startPointValue);
    propertyList->SetProperty(controlPoint.startPoint.UID, startPointVectorProperty);
    
    // set the year, month and day of the end point
    std::vector<int> endPointValue;
    endPointValue.push_back(controlPoint.endPoint.year);
    endPointValue.push_back(controlPoint.endPoint.month);
    endPointValue.push_back(controlPoint.endPoint.day);

    // store the end point
    mitk::VectorProperty<int>::Pointer endPointVectorProperty = mitk::VectorProperty<int>::New();
    endPointVectorProperty->SetValue(endPointValue);
    propertyList->SetProperty(controlPoint.endPoint.UID, endPointVectorProperty);
    
    // store the start point UID and the end point UID
    std::vector<std::string> controlPointDateReferences;
    controlPointDateReferences.push_back(controlPoint.startPoint.UID);
    controlPointDateReferences.push_back(controlPoint.endPoint.UID);
    // store the control point references (the start point UID and the end point UID)
    mitk::VectorProperty<std::string>::Pointer newControlPointVectorProperty = mitk::VectorProperty<std::string>::New();
    newControlPointVectorProperty->SetValue(controlPointDateReferences);
    propertyList->SetProperty(controlPoint.UID, newControlPointVectorProperty);
  }
}

void mitk::RelationStorage::OverwriteControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
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
    // retrieve the start date
    mitk::VectorProperty<int>* startPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(controlPoint.startPoint.UID));
    if (nullptr == startPointVectorProperty)
    {
      MITK_INFO << "Could not find the start date " << controlPoint.startPoint.UID << " of the control point " << controlPoint.UID << " in the storage.";
      return;
    }
    std::vector<int> startPointValue = startPointVectorProperty->GetValue();
    // a date has to have exactly three integer values
    if (startPointValue.size() != 3)
    {
      MITK_INFO << "Incorrect start point storage. Not three (3) values of the date are stored.";
      return;
    }
    else
    {
      // set the year, month and day of the start point
      startPointValue[0] = controlPoint.startPoint.year;
      startPointValue[1] = controlPoint.startPoint.month;
      startPointValue[2] = controlPoint.startPoint.day;
      // overwrite the start point
      startPointVectorProperty->SetValue(startPointValue);
    }

    // retrieve the end date
    mitk::VectorProperty<int>* endPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(controlPoint.endPoint.UID));
    if (nullptr == endPointVectorProperty)
    {
      MITK_INFO << "Could not find the end date " << controlPoint.endPoint.UID << " of the control point " << controlPoint.UID << " in the storage.";
      return;
    }
    std::vector<int> endPointValue = endPointVectorProperty->GetValue();
    // a date has to have exactly three integer values
    if (endPointValue.size() != 3)
    {
      MITK_INFO << "Incorrect end point storage. Not three (3) values of the date are stored.";
      return;
    }
    else
    {
      // set the year, month and day of the end point
      endPointValue[0] = controlPoint.endPoint.year;
      endPointValue[1] = controlPoint.endPoint.month;
      endPointValue[2] = controlPoint.endPoint.day;
      // overwrite the end point
      endPointVectorProperty->SetValue(endPointValue);
    }
  }
  else
  {
    MITK_INFO << "Could not find control point " << controlPoint.UID << " in the storage. Cannot overwrite the control point.";
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
  // the start and end point is stored under the control point ID
  // retrieve a vector property that contains the referenced ID of the dates of a control point (0. startPoint ID 1. endPoint ID)
  mitk::VectorProperty<std::string>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(controlPoint.UID));
  if (nullptr == controlPointVectorProperty)
  {
    MITK_INFO << "Control point " << controlPoint.UID << " not found (already removed?). Cannot remove the control point.";
    return;
  }

  std::vector<std::string> controlPointVectorValue = controlPointVectorProperty->GetValue();
  // a control point has to have exactly two values (the ID of two dates)
  if (controlPointVectorValue.size() != 2)
  {
    MITK_INFO << "Incorrect control point storage. Not two (2) IDs of the start point and the end point are stored.";
  }
  else
  {
    // retrieve the start date
    std::string startPointID = controlPointVectorValue[0];
    // delete the start date property
    propertyList->DeleteProperty(startPointID);
    // retrieve the end date
    std::string endPointID = controlPointVectorValue[1];
    // delete the end date property
    propertyList->DeleteProperty(endPointID);
  }
  propertyList->DeleteProperty(controlPoint.UID);
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
  if (informationTypeVectorValue.size() == 0)
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
