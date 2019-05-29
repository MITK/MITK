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

#include "mitkRelationStorage.h"

// semantic relations module
#include "mitkDICOMHelper.h"

// multi label module
#include <mitkLabelSetImage.h>

// mitk core
#include <mitkIPersistenceService.h>
#include <mitkVectorProperty.h>

// c++
#include <algorithm>
#include <iostream>

namespace
{
  std::vector<mitk::SemanticTypes::CaseID> GetCaseIDs()
  {
    PERSISTENCE_GET_SERVICE_MACRO
    if (nullptr == persistenceService)
    {
      MITK_DEBUG << "Persistence service could not be loaded";
      return std::vector<mitk::SemanticTypes::CaseID>();
    }
    // the property list is valid for a certain scenario and contains all the case IDs of the radiological user's MITK session
    std::string listIdentifier = "caseIDs";
    mitk::PropertyList::Pointer propertyList = persistenceService->GetPropertyList(listIdentifier);
    if (nullptr == propertyList)
    {
      MITK_DEBUG << "Could not find the property list " << listIdentifier << " for the current MITK workbench / session.";
      return std::vector<mitk::SemanticTypes::CaseID>();
    }
    // retrieve a vector property that contains all case IDs
    mitk::VectorProperty<std::string>* caseIDsVectorProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(listIdentifier));
    if (nullptr == caseIDsVectorProperty)
    {
      MITK_DEBUG << "Could not find the property " << listIdentifier << " for the " << listIdentifier << " property list.";
      return std::vector<mitk::SemanticTypes::CaseID>();
    }

    return caseIDsVectorProperty->GetValue();
  }

  bool CaseIDExists(const mitk::SemanticTypes::CaseID& caseID)
  {
    auto allCaseIDs = GetCaseIDs();
    auto existingCase = std::find(allCaseIDs.begin(), allCaseIDs.end(), caseID);
    if (existingCase == allCaseIDs.end())
    {
      return false;
    }

    return true;
  }

  mitk::PropertyList::Pointer GetStorageData(const mitk::SemanticTypes::CaseID& caseID)
  {
    // access the storage
    PERSISTENCE_GET_SERVICE_MACRO
    if (nullptr == persistenceService)
    {
      MITK_DEBUG << "Persistence service could not be loaded";
      return nullptr;
    }

    // The persistence service may create a new property list with the given ID, if no property list is found.
    // Since we don't want to return a new property list but rather inform the user that the given case
    // is not a valid, stored case, we will return nullptr in that case.
    if (CaseIDExists(caseID))
    {
      // the property list is valid for a whole case and contains all the properties for the current case
      return persistenceService->GetPropertyList(const_cast<mitk::SemanticTypes::CaseID&>(caseID));
    }

    return nullptr;
  }

  mitk::SemanticTypes::Lesion GenerateLesion(const mitk::SemanticTypes::CaseID& caseID, const mitk::SemanticTypes::ID& lesionID)
  {
    mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
    if (nullptr == propertyList)
    {
      MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
      return mitk::SemanticTypes::Lesion();
    }

    mitk::VectorProperty<std::string>* lesionDataProperty = dynamic_cast<mitk::VectorProperty<std::string>*>(propertyList->GetProperty(lesionID));
    if (nullptr == lesionDataProperty)
    {
      MITK_DEBUG << "Lesion " << lesionID << " not found. Lesion can not be retrieved.";
      return mitk::SemanticTypes::Lesion();
    }

    std::vector<std::string> lesionData = lesionDataProperty->GetValue();
    // a lesion date has to have exactly two values (the name of the lesion and the UID of the lesion class)
    if (lesionData.size() != 2)
    {
      MITK_DEBUG << "Incorrect lesion data storage. Not two (2) strings of the lesion name and the lesion UID are stored.";
      return mitk::SemanticTypes::Lesion();
    }

    // the lesion class ID is stored as the second property
    std::string lesionClassID = lesionData[1];
    mitk::StringProperty* lesionClassProperty = dynamic_cast<mitk::StringProperty*>(propertyList->GetProperty(lesionClassID));
    if (nullptr != lesionClassProperty)
    {
      mitk::SemanticTypes::LesionClass generatedLesionClass;
      generatedLesionClass.UID = lesionClassID;
      generatedLesionClass.classType = lesionClassProperty->GetValue();

      mitk::SemanticTypes::Lesion generatedLesion;
      generatedLesion.UID = lesionID;
      generatedLesion.name = lesionData[0];
      generatedLesion.lesionClass = generatedLesionClass;

      return generatedLesion;
    }

    MITK_DEBUG << "Incorrect lesion class storage. Lesion " << lesionID << " can not be retrieved.";
    return mitk::SemanticTypes::Lesion();
  }

  mitk::SemanticTypes::ControlPoint GenerateControlpoint(const mitk::SemanticTypes::CaseID& caseID, const mitk::SemanticTypes::ID& controlPointUID)
  {
    mitk::PropertyList::Pointer propertyList = GetStorageData(caseID);
    if (nullptr == propertyList)
    {
      MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
      return mitk::SemanticTypes::ControlPoint();
    }

    // retrieve a vector property that contains the integer values of the date of a control point (0. year 1. month 2. day)
    mitk::VectorProperty<int>* controlPointVectorProperty = dynamic_cast<mitk::VectorProperty<int>*>(propertyList->GetProperty(controlPointUID));
    if (nullptr == controlPointVectorProperty)
    {
      MITK_DEBUG << "Could not find the control point " << controlPointUID << " in the storage.";
      return mitk::SemanticTypes::ControlPoint();
    }

    std::vector<int> controlPointVectorPropertyValue = controlPointVectorProperty->GetValue();
    // a control point has to have exactly three integer values (year, month and day)
    if (controlPointVectorPropertyValue.size() != 3)
    {
      MITK_DEBUG << "Incorrect control point storage. Not three (3) values of the date are stored.";
      return mitk::SemanticTypes::ControlPoint();
    }

    // set the values of the control point
    mitk::SemanticTypes::ControlPoint generatedControlPoint;
    generatedControlPoint.UID = controlPointUID;
    generatedControlPoint.date = boost::gregorian::date(controlPointVectorPropertyValue[0],
      controlPointVectorPropertyValue[1],
      controlPointVectorPropertyValue[2]);

    return generatedControlPoint;
  }
}

mitk::SemanticTypes::LesionVector mitk::RelationStorage::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::LesionVector();
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  VectorProperty<std::string>* lesionsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionsVectorProperty)
  {
    MITK_DEBUG << "Could not find any lesion in the storage.";
    return SemanticTypes::LesionVector();
  }

  std::vector<std::string> lesionsVectorPropertyValue = lesionsVectorProperty->GetValue();
  SemanticTypes::LesionVector allLesionsOfCase;
  for (const auto& lesionID : lesionsVectorPropertyValue)
  {
    SemanticTypes::Lesion generatedLesion = GenerateLesion(caseID, lesionID);
    if (!generatedLesion.UID.empty())
    {
      allLesionsOfCase.push_back(generatedLesion);
    }
  }

  return allLesionsOfCase;
}

mitk::SemanticTypes::Lesion mitk::RelationStorage::GetLesionOfSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::Lesion();
  }

  // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
  VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
  if (nullptr == segmentationVectorProperty)
  {
    MITK_DEBUG << "Could not find the segmentation " << segmentationID << " in the storage.";
    return SemanticTypes::Lesion();
  }

  std::vector<std::string> segmentationVectorPropertyValue = segmentationVectorProperty->GetValue();
  // the lesion ID of a segmentation is the second value in the vector
  if (segmentationVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect segmentation storage. Not two (2) IDs stored.";
    return SemanticTypes::Lesion();
  }

  std::string lesionID = segmentationVectorPropertyValue[1];
  if (lesionID.empty())
  {
    // segmentation does not refer to any lesion; return empty lesion
    return SemanticTypes::Lesion();
  }

  return GenerateLesion(caseID, lesionID);
}

mitk::SemanticTypes::ControlPointVector mitk::RelationStorage::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ControlPointVector();
  }

  // retrieve a vector property that contains the valid control point-IDs for the current case
  VectorProperty<std::string>* controlPointsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  if (nullptr == controlPointsVectorProperty)
  {
    MITK_DEBUG << "Could not find any control points in the storage.";
    return SemanticTypes::ControlPointVector();
  }

  std::vector<std::string> controlPointsVectorPropertyValue = controlPointsVectorProperty->GetValue();
  SemanticTypes::ControlPointVector allControlPointsOfCase;
  for (const auto& controlPointUID : controlPointsVectorPropertyValue)
  {
    SemanticTypes::ControlPoint generatedControlPoint = GenerateControlpoint(caseID, controlPointUID);
    if (!generatedControlPoint.UID.empty())
    {
      allControlPointsOfCase.push_back(generatedControlPoint);
    }
  }

  return allControlPointsOfCase;
}

mitk::SemanticTypes::ControlPoint mitk::RelationStorage::GetControlPointOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ControlPoint();
  }
  // retrieve a vector property that contains the information type and the referenced ID of a control point (0. information type 1. control point ID)
  VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageVectorProperty)
  {
    MITK_DEBUG << "Could not find the image " << imageID << " in the storage.";
    return SemanticTypes::ControlPoint();
  }

  std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
  SemanticTypes::ControlPoint controlPoint;
  // an image has to have exactly two values (the information type and the ID of the control point)
  if (imageVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
    return SemanticTypes::ControlPoint();
  }

  // the second value of the image vector is the ID of the referenced control point
  std::string controlPointID = imageVectorPropertyValue[1];
  // retrieve a vector property that contains the integer values of the date of a control point (0. year 1. month 2. day)
  VectorProperty<int>* controlPointVectorProperty = dynamic_cast<VectorProperty<int>*>(propertyList->GetProperty(controlPointID));
  if (nullptr == controlPointVectorProperty)
  {
    MITK_DEBUG << "Could not find the control point " << controlPointID << " in the storage.";
    return SemanticTypes::ControlPoint();
  }

  std::vector<int> controlPointVectorPropertyValue = controlPointVectorProperty->GetValue();
  // a control point has to have exactly three integer values (year, month and day)
  if (controlPointVectorPropertyValue.size() != 3)
  {
    MITK_DEBUG << "Incorrect control point storage. Not three (3) values of the date are stored.";
    return SemanticTypes::ControlPoint();
  }

  // set the values of the control point
  controlPoint.UID = controlPointID;
  controlPoint.date = boost::gregorian::date(controlPointVectorPropertyValue[0],
                                             controlPointVectorPropertyValue[1],
                                             controlPointVectorPropertyValue[2]);

  return controlPoint;
}

mitk::SemanticTypes::ExaminationPeriodVector mitk::RelationStorage::GetAllExaminationPeriodsOfCase(const SemanticTypes::CaseID& caseID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ExaminationPeriodVector();
  }

  // retrieve a vector property that contains the valid examination period UIDs for the current case
  VectorProperty<std::string>::Pointer examinationPeriodsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("examinationperiods"));
  if (nullptr == examinationPeriodsVectorProperty)
  {
    MITK_DEBUG << "Could not find any examination periods in the storage.";
    return SemanticTypes::ExaminationPeriodVector();
  }

  std::vector<std::string> examinationPeriodsVectorPropertyValue = examinationPeriodsVectorProperty->GetValue();
  SemanticTypes::ExaminationPeriodVector allExaminationPeriods;
  for (const auto& examinationPeriodID : examinationPeriodsVectorPropertyValue)
  {
    // retrieve a vector property that contains the represented control point-IDs
    VectorProperty<std::string>::Pointer examinationPeriodVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(examinationPeriodID));
    if (nullptr == examinationPeriodVectorProperty)
    {
      MITK_DEBUG << "Could not find the examination period " << examinationPeriodID << " in the storage.";
      continue;
    }

    std::vector<std::string> examinationPeriodVectorPropertyValue = examinationPeriodVectorProperty->GetValue();
    // an examination period has an arbitrary number of vector values (name and control point UIDs) (at least one for the name)
    if (examinationPeriodVectorPropertyValue.empty())
    {
      MITK_DEBUG << "Incorrect examination period storage. At least one (1) value for the examination period name has to be stored.";
      continue;
    }
    else
    {
      // set the values of the name and the control points
      SemanticTypes::ExaminationPeriod generatedExaminationPeriod;
      generatedExaminationPeriod.UID = examinationPeriodID;
      generatedExaminationPeriod.name = examinationPeriodVectorPropertyValue[0];
      for (size_t i = 1; i < examinationPeriodVectorPropertyValue.size(); ++i)
      {
        generatedExaminationPeriod.controlPointUIDs.push_back(examinationPeriodVectorPropertyValue[i]);
      }

      allExaminationPeriods.push_back(generatedExaminationPeriod);
    }
  }
  return allExaminationPeriods;
}

mitk::SemanticTypes::InformationTypeVector mitk::RelationStorage::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::InformationTypeVector();
  }
  // retrieve a vector property that contains the valid information types of the current case
  VectorProperty<std::string>* informationTypesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("informationtypes"));
  if (nullptr == informationTypesVectorProperty)
  {
    MITK_DEBUG << "Could not find any information types in the storage.";
    return SemanticTypes::InformationTypeVector();
  }

  return informationTypesVectorProperty->GetValue();
}

mitk::SemanticTypes::InformationType mitk::RelationStorage::GetInformationTypeOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::InformationType();
  }
  // retrieve a vector property that contains the information type and the referenced ID of an image (0. information type 1. control point ID)
  VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageVectorProperty)
  {
    MITK_DEBUG << "Could not find the image " << imageID << " in the storage.";
    return SemanticTypes::InformationType();
  }

  std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
  // an image has to have exactly two values (the information type and the ID of the control point)
  if (imageVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
    return SemanticTypes::InformationType();
  }

  // the first value of the image vector is the information type
  return imageVectorPropertyValue[0];
}

mitk::SemanticTypes::IDVector mitk::RelationStorage::GetAllImageIDsOfCase(const SemanticTypes::CaseID& caseID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::IDVector();
  }
  // retrieve a vector property that contains the valid image-IDs of the current case
  VectorProperty<std::string>* imagesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  if (nullptr == imagesVectorProperty)
  {
    MITK_DEBUG << "Could not find any image in the storage.";
    return SemanticTypes::IDVector();
  }

  return imagesVectorProperty->GetValue();
}

mitk::SemanticTypes::IDVector mitk::RelationStorage::GetAllImageIDsOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::IDVector();
  }
  // retrieve a vector property that contains the valid image-IDs of the current case
  VectorProperty<std::string>* imagesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  if (nullptr == imagesVectorProperty)
  {
    MITK_DEBUG << "Could not find any image in the storage.";
    return SemanticTypes::IDVector();
  }

  mitk::SemanticTypes::IDVector allImageIDsOfControlPoint;
  std::vector<std::string> imagesVectorPropertyValue = imagesVectorProperty->GetValue();
  for (const auto& imageID : imagesVectorPropertyValue)
  {
    // retrieve a vector property that contains the referenced ID of an image (0. information type 1. control point ID)
    VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
    if (nullptr == imageVectorProperty)
    {
      continue;
    }

    std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
    // an image has to have exactly two values (the information type and the ID of the control point)
    if (imageVectorPropertyValue.size() != 2)
    {
      continue;
    }

    // the second value of the image vector is the ID of the referenced control point
    if (imageVectorPropertyValue[1] == controlPoint.UID)
    {
      allImageIDsOfControlPoint.push_back(imageID);
    }
  }

  return allImageIDsOfControlPoint;
}

mitk::SemanticTypes::IDVector mitk::RelationStorage::GetAllImageIDsOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::IDVector();
  }
  // retrieve a vector property that contains the valid image-IDs of the current case
  VectorProperty<std::string>* imagesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  if (nullptr == imagesVectorProperty)
  {
    MITK_DEBUG << "Could not find any image in the storage.";
    return SemanticTypes::IDVector();
  }

  mitk::SemanticTypes::IDVector allImageIDsOfInformationType;
  std::vector<std::string> imagesVectorPropertyValue = imagesVectorProperty->GetValue();
  for (const auto& imageID : imagesVectorPropertyValue)
  {
    // retrieve a vector property that contains the referenced ID of an image (0. information type 1. control point ID)
    VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
    if (nullptr == imageVectorProperty)
    {
      continue;
    }

    std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
    // an image has to have exactly two values (the information type and the ID of the control point)
    if (imageVectorPropertyValue.size() != 2)
    {
      continue;
    }

    // the first value of the image vector is the ID of the referenced information type
    if (imageVectorPropertyValue[0] == informationType)
    {
      allImageIDsOfInformationType.push_back(imageID);
    }
  }

  return allImageIDsOfInformationType;
}

mitk::SemanticTypes::IDVector mitk::RelationStorage::GetAllSegmentationIDsOfCase(const SemanticTypes::CaseID& caseID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::IDVector();
  }
  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  VectorProperty<std::string>* segmentationsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == segmentationsVectorProperty)
  {
    MITK_DEBUG << "Could not find any segmentation in the storage.";
    return SemanticTypes::IDVector();
  }

  return segmentationsVectorProperty->GetValue();
}

mitk::SemanticTypes::IDVector mitk::RelationStorage::GetAllSegmentationIDsOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::IDVector();
  }
  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  VectorProperty<std::string>* segmentationsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == segmentationsVectorProperty)
  {
    MITK_DEBUG << "Could not find any segmentation in the storage.";
    return SemanticTypes::IDVector();
  }

  mitk::SemanticTypes::IDVector allSegmentationIDsOfImage;
  std::vector<std::string> segmentationsVectorPropertyValue = segmentationsVectorProperty->GetValue();
  for (const auto& segmentationID : segmentationsVectorPropertyValue)
  {
    // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
    VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
    if (nullptr == segmentationVectorProperty)
    {
      continue;
    }

    std::vector<std::string> segmentationVectorPropertyValue = segmentationVectorProperty->GetValue();
    // a segmentation has to have exactly two values (the ID of the referenced image and the ID of the referenced lesion)
    if (segmentationVectorPropertyValue.size() != 2)
    {
      continue;
    }

    // the first value of the segmentation vector is the ID of the referenced image
    if (segmentationVectorPropertyValue[0] == imageID)
    {
      allSegmentationIDsOfImage.push_back(segmentationID);
    }
  }

  return allSegmentationIDsOfImage;
}

mitk::SemanticTypes::IDVector mitk::RelationStorage::GetAllSegmentationIDsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::IDVector();
  }
  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  VectorProperty<std::string>* segmentationsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == segmentationsVectorProperty)
  {
    MITK_DEBUG << "Could not find any segmentation in the storage.";
    return SemanticTypes::IDVector();
  }

  mitk::SemanticTypes::IDVector allSegmentationIDsOfLesion;
  std::vector<std::string> segmentationsVectorPropertyValue = segmentationsVectorProperty->GetValue();
  for (const auto& segmentationID : segmentationsVectorPropertyValue)
  {
    // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
    VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
    if (nullptr == segmentationVectorProperty)
    {
      continue;
    }

    std::vector<std::string> segmentationVectorPropertyValue = segmentationVectorProperty->GetValue();
    // a segmentation has to have exactly two values (the ID of the referenced image and the ID of the referenced lesion)
    if (segmentationVectorPropertyValue.size() != 2)
    {
      continue;
    }

    // the second value of the segmentation vector is the ID of the referenced lesion
    if (segmentationVectorPropertyValue[1] == lesion.UID)
    {
      allSegmentationIDsOfLesion.push_back(segmentationID);
    }
  }

  return allSegmentationIDsOfLesion;
}

mitk::SemanticTypes::ID mitk::RelationStorage::GetImageIDOfSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return SemanticTypes::ID();
  }

  // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
  VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
  if (nullptr == segmentationVectorProperty)
  {
    MITK_DEBUG << "Could not find the segmentation " << segmentationID << " in the storage.";
    return SemanticTypes::ID();
  }

  std::vector<std::string> segmentationVectorPropertyValue = segmentationVectorProperty->GetValue();
  // the lesion ID of a segmentation is the second value in the vector
  if (segmentationVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect segmentation storage. Not two (2) IDs stored.";
    return SemanticTypes::ID();
  }

  return segmentationVectorPropertyValue[0];
}

std::vector<mitk::SemanticTypes::CaseID> mitk::RelationStorage::GetAllCaseIDs()
{
  return GetCaseIDs();
}

bool mitk::RelationStorage::InstanceExists(const SemanticTypes::CaseID& caseID)
{
  return CaseIDExists(caseID);
}

void mitk::RelationStorage::AddCase(const SemanticTypes::CaseID& caseID)
{
  PERSISTENCE_GET_SERVICE_MACRO
  if (nullptr == persistenceService)
  {
    MITK_DEBUG << "Persistence service could not be loaded";
    return;
  }
  // the property list is valid for a certain scenario and contains all the case IDs of the radiological user's MITK session
  std::string listIdentifier = "caseIDs";
  PropertyList::Pointer propertyList = persistenceService->GetPropertyList(listIdentifier);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << listIdentifier << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains all case IDs
  VectorProperty<std::string>::Pointer caseIDsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(listIdentifier));
  std::vector<std::string> caseIDsVectorPropertyValue;
  if (nullptr == caseIDsVectorProperty)
  {
    caseIDsVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    caseIDsVectorPropertyValue = caseIDsVectorProperty->GetValue();
  }

  auto existingCase = std::find(caseIDsVectorPropertyValue.begin(), caseIDsVectorPropertyValue.end(), caseID);
  if (existingCase != caseIDsVectorPropertyValue.end())
  {
    return;
  }

  // add case to the "caseIDs" property list
  caseIDsVectorPropertyValue.push_back(caseID);
  caseIDsVectorProperty->SetValue(caseIDsVectorPropertyValue);
  propertyList->SetProperty(listIdentifier, caseIDsVectorProperty);
}

void mitk::RelationStorage::AddImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid image-IDs for the current case
  VectorProperty<std::string>::Pointer imagesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  std::vector<std::string> imagesVectorPropertyValue;
  if (nullptr == imagesVectorProperty)
  {
    imagesVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    imagesVectorPropertyValue = imagesVectorProperty->GetValue();
  }

  auto existingImage = std::find(imagesVectorPropertyValue.begin(), imagesVectorPropertyValue.end(), imageID);
  if (existingImage != imagesVectorPropertyValue.end())
  {
    return;
  }

  // add image to the "images" property list
  imagesVectorPropertyValue.push_back(imageID);
  imagesVectorProperty->SetValue(imagesVectorPropertyValue);
  propertyList->SetProperty("images", imagesVectorProperty);

  // add the image itself
  VectorProperty<std::string>::Pointer imageVectorProperty = VectorProperty<std::string>::New();
  // an image has to have exactly two values (the information type and the ID of the control point)
  std::vector<std::string> imageVectorPropertyValue(2);
  imageVectorProperty->SetValue(imageVectorPropertyValue);
  propertyList->SetProperty(imageID, imageVectorProperty);
}

void mitk::RelationStorage::RemoveImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid image-IDs for the current case
  VectorProperty<std::string>::Pointer imagesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("images"));
  if (nullptr == imagesVectorProperty)
  {
    MITK_DEBUG << "Could not find any images in the storage.";
    return;
  }

  // remove the image reference from the list of all images of the current case
  std::vector<std::string> imagesVectorPropertyValue = imagesVectorProperty->GetValue();
  imagesVectorPropertyValue.erase(std::remove(imagesVectorPropertyValue.begin(), imagesVectorPropertyValue.end(), imageID), imagesVectorPropertyValue.end());
  if (imagesVectorPropertyValue.empty())
  {
    // no more images stored -> remove the images property list
    propertyList->DeleteProperty("images");
  }
  else
  {
    // or store the modified vector value
    imagesVectorProperty->SetValue(imagesVectorPropertyValue);
  }

  // remove the image instance itself
  propertyList->DeleteProperty(imageID);
}

void mitk::RelationStorage::AddSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::ID& parentID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  VectorProperty<std::string>::Pointer segmentationsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  std::vector<std::string> segmentationsVectorPropertyValue;
  if (nullptr == segmentationsVectorProperty)
  {
    segmentationsVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    segmentationsVectorPropertyValue = segmentationsVectorProperty->GetValue();
  }

  auto existingSegmentation = std::find(segmentationsVectorPropertyValue.begin(), segmentationsVectorPropertyValue.end(), segmentationID);
  if (existingSegmentation != segmentationsVectorPropertyValue.end())
  {
    return;
  }

  // add segmentation to the "segmentations" property list
  segmentationsVectorPropertyValue.push_back(segmentationID);
  segmentationsVectorProperty->SetValue(segmentationsVectorPropertyValue);
  propertyList->SetProperty("segmentations", segmentationsVectorProperty);

  // add the segmentation itself
  VectorProperty<std::string>::Pointer segmentationVectorProperty = VectorProperty<std::string>::New();
  // a segmentation has to have exactly two values (the ID of the referenced image and the ID of the referenced lesion)
  std::vector<std::string> segmentationVectorPropertyValue(2);
  segmentationVectorPropertyValue[0] = parentID;
  segmentationVectorProperty->SetValue(segmentationVectorPropertyValue);
  propertyList->SetProperty(segmentationID, segmentationVectorProperty);
}

void mitk::RelationStorage::RemoveSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the valid segmentation-IDs for the current case
  VectorProperty<std::string>::Pointer segmentationsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("segmentations"));
  if (nullptr == segmentationsVectorProperty)
  {
    MITK_DEBUG << "Could not find any segmentation in the storage.";
    return;
  }

  // remove the lesion reference from the list of all lesions of the current case
  std::vector<std::string> segmentationsVectorPropertyValue = segmentationsVectorProperty->GetValue();
  segmentationsVectorPropertyValue.erase(std::remove(segmentationsVectorPropertyValue.begin(), segmentationsVectorPropertyValue.end(), segmentationID), segmentationsVectorPropertyValue.end());
  if (segmentationsVectorPropertyValue.empty())
  {
    // no more segmentations stored -> remove the segmentations property list
    propertyList->DeleteProperty("segmentations");
  }
  else
  {
    // or store the modified vector value
    segmentationsVectorProperty->SetValue(segmentationsVectorPropertyValue);
  }

  // remove the lesion instance itself
  propertyList->DeleteProperty(segmentationID);
}

void mitk::RelationStorage::AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  VectorProperty<std::string>::Pointer lesionsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  std::vector<std::string> lesionsVectorPropertyValue;
  if (nullptr == lesionsVectorProperty)
  {
    lesionsVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    lesionsVectorPropertyValue = lesionsVectorProperty->GetValue();
  }

  const auto& existingIndex = std::find(lesionsVectorPropertyValue.begin(), lesionsVectorPropertyValue.end(), lesion.UID);
  if (existingIndex != lesionsVectorPropertyValue.end())
  {
    return;
  }

  // add the new lesion id from the given lesion to the vector of all current lesion IDs
  lesionsVectorPropertyValue.push_back(lesion.UID);
  // overwrite the current vector property with the new, extended string vector
  lesionsVectorProperty->SetValue(lesionsVectorPropertyValue);
  propertyList->SetProperty("lesions", lesionsVectorProperty);

  // add the lesion with the lesion UID as the key and the lesion information as value
  std::vector<std::string> lesionData;
  lesionData.push_back(lesion.name);
  lesionData.push_back(lesion.lesionClass.UID);
  VectorProperty<std::string>::Pointer newLesionVectorProperty = VectorProperty<std::string>::New();
  newLesionVectorProperty->SetValue(lesionData);
  propertyList->SetProperty(lesion.UID, newLesionVectorProperty);

  // add the lesion class with the lesion class UID as key and the class type as value
  std::string lesionClassType = lesion.lesionClass.classType;
  propertyList->SetStringProperty(lesion.lesionClass.UID.c_str(), lesionClassType.c_str());
}

void mitk::RelationStorage::OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    MITK_DEBUG << "Could not find any lesion in the storage.";
    return;
  }

  std::vector<std::string> lesionVectorPropertyValue = lesionVectorProperty->GetValue();
  const auto existingLesion = std::find(lesionVectorPropertyValue.begin(), lesionVectorPropertyValue.end(), lesion.UID);
  if (existingLesion != lesionVectorPropertyValue.end())
  {
    // overwrite the referenced lesion class UID with the new, given lesion class data
    std::vector<std::string> lesionData;
    lesionData.push_back(lesion.name);
    lesionData.push_back(lesion.lesionClass.UID);
    VectorProperty<std::string>::Pointer newLesionVectorProperty = VectorProperty<std::string>::New();
    newLesionVectorProperty->SetValue(lesionData);
    propertyList->SetProperty(lesion.UID, newLesionVectorProperty);

    // overwrite the lesion class with the lesion class UID as key and the new, given class type as value
    std::string lesionClassType = lesion.lesionClass.classType;
    propertyList->SetStringProperty(lesion.lesionClass.UID.c_str(), lesionClassType.c_str());
  }
  else
  {
    MITK_DEBUG << "Could not find lesion " << lesion.UID << " in the storage. Cannot overwrite the lesion.";
  }
}

void mitk::RelationStorage::LinkSegmentationToLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::Lesion& lesion)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesion-IDs for the current case
  VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    MITK_DEBUG << "Could not find any lesion in the storage.";
    return;
  }

  std::vector<std::string> lesionVectorPropertyValue = lesionVectorProperty->GetValue();
  const auto existingLesion = std::find(lesionVectorPropertyValue.begin(), lesionVectorPropertyValue.end(), lesion.UID);
  if (existingLesion != lesionVectorPropertyValue.end())
  {
    // set / overwrite the lesion reference of the given segmentation
    // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
    VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
    if (nullptr == segmentationVectorProperty)
    {
      MITK_DEBUG << "Could not find the segmentation " << segmentationID << " in the storage. Cannot link segmentation to lesion.";
      return;
    }

    std::vector<std::string> segmentationVectorPropertyValue = segmentationVectorProperty->GetValue();
    if (segmentationVectorPropertyValue.size() != 2)
    {
      MITK_DEBUG << "Incorrect segmentation storage. Not two (2) IDs stored.";
      return;
    }

    // the lesion ID of a segmentation is the second value in the vector
    segmentationVectorPropertyValue[1] = lesion.UID;
    segmentationVectorProperty->SetValue(segmentationVectorPropertyValue);
    return;
  }

  MITK_DEBUG << "Could not find lesion " << lesion.UID << " in the storage. Cannot link segmentation to lesion.";
}

void mitk::RelationStorage::UnlinkSegmentationFromLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the referenced ID of a segmentation (0. image ID 1. lesion ID)
  VectorProperty<std::string>* segmentationVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(segmentationID));
  if (nullptr == segmentationVectorProperty)
  {
    MITK_DEBUG << "Could not find the segmentation " << segmentationID << " in the storage. Cannot unlink lesion from segmentation.";
    return;
  }

  std::vector<std::string> segmentationVectorPropertyValue = segmentationVectorProperty->GetValue();
  // a segmentation has to have exactly two values (the ID of the linked image and the ID of the lesion)
  if (segmentationVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
    return;
  }

  // the second value of the segmentation vector is the ID of the referenced lesion
  // set the lesion reference to an empty string for removal
  segmentationVectorPropertyValue[1] = "";
  segmentationVectorProperty->SetValue(segmentationVectorPropertyValue);
}

void mitk::RelationStorage::RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid lesions of the current case
  VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    MITK_DEBUG << "Could not find any lesion in the storage.";
    return;
  }

  // remove the lesion reference from the list of all lesions of the current case
  std::vector<std::string> lesionsVectorPropertyValue = lesionVectorProperty->GetValue();
  lesionsVectorPropertyValue.erase(std::remove(lesionsVectorPropertyValue.begin(), lesionsVectorPropertyValue.end(), lesion.UID), lesionsVectorPropertyValue.end());
  if (lesionsVectorPropertyValue.empty())
  {
    // no more lesions stored -> remove the lesions property list
    propertyList->DeleteProperty("lesions");
  }
  else
  {
    // or store the modified vector value
    lesionVectorProperty->SetValue(lesionsVectorPropertyValue);
  }

  // remove the lesion instance itself
  // the lesion data is stored under the lesion ID
  VectorProperty<std::string>* lesionDataProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(lesion.UID));
  if (nullptr == lesionDataProperty)
  {
    MITK_DEBUG << "Lesion " << lesion.UID << " not found (already removed?). Cannot remove the lesion.";
    return;
  }

  std::vector<std::string> lesionData = lesionDataProperty->GetValue();
  // a lesion date has to have exactly two values (the name of the lesion and the UID of the lesion class)
  if (lesionData.size() != 2)
  {
    MITK_DEBUG << "Incorrect lesion data storage. Not two (2) strings of the lesion UID and the lesion name are stored.";
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
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the lesion class
  StringProperty* lesionClassProperty = dynamic_cast<StringProperty*>(propertyList->GetProperty(lesionClassID));
  if (nullptr == lesionClassProperty)
  {
    MITK_DEBUG << "Lesion class " << lesionClassID << " not found (already removed?). Cannot remove the lesion class.";
    return;
  }

  // retrieve a vector property that contains the valid lesions of the current case
  VectorProperty<std::string>* lesionVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("lesions"));
  if (nullptr == lesionVectorProperty)
  {
    return;
  }

  // check if the lesion class ID is referenced by any other lesion
  std::vector<std::string> lesionsVectorPropertyValue = lesionVectorProperty->GetValue();
  const auto existingLesionClass = std::find_if(lesionsVectorPropertyValue.begin(), lesionsVectorPropertyValue.end(),
    [&propertyList, &lesionClassID](const std::string& lesionID)
  {
    VectorProperty<std::string>* lesionDataProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(lesionID));
    if (nullptr == lesionDataProperty)
    {
      return false;
    }

    std::vector<std::string> lesionData = lesionDataProperty->GetValue();
    // a lesion date has to have exactly two values (the name of the lesion and the UID of the lesion class)
    if (lesionData.size() != 2)
    {
      return false;
    }

    return lesionData[1] == lesionClassID;
  });

  if (existingLesionClass == lesionsVectorPropertyValue.end())
  {
    // lesion class ID not referenced; remove lesion class
    propertyList->DeleteProperty(lesionClassID);
  }
}

void mitk::RelationStorage::AddControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid controlPoint UIDs for the current case
  VectorProperty<std::string>::Pointer controlPointsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  std::vector<std::string> controlPointsVectorPropertyValue;
  if (nullptr == controlPointsVectorProperty)
  {
    controlPointsVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    controlPointsVectorPropertyValue = controlPointsVectorProperty->GetValue();
  }

  const auto existingControlPoint = std::find(controlPointsVectorPropertyValue.begin(), controlPointsVectorPropertyValue.end(), controlPoint.UID);
  if (existingControlPoint != controlPointsVectorPropertyValue.end())
  {
    return;
  }

  // add the new control point UID from the given control point to the vector of all current control point UIDs
  controlPointsVectorPropertyValue.push_back(controlPoint.UID);
  // overwrite the current vector property with the new, extended string vector
  controlPointsVectorProperty->SetValue(controlPointsVectorPropertyValue);
  propertyList->SetProperty("controlpoints", controlPointsVectorProperty);

  // store the control point values (the three integer values of a date)
  std::vector<int> controlPointDate;
  controlPointDate.push_back(controlPoint.date.year());
  controlPointDate.push_back(controlPoint.date.month());
  controlPointDate.push_back(controlPoint.date.day());

  VectorProperty<int>::Pointer newControlPointVectorProperty = VectorProperty<int>::New();
  newControlPointVectorProperty->SetValue(controlPointDate);
  propertyList->SetProperty(controlPoint.UID, newControlPointVectorProperty);
}

void mitk::RelationStorage::LinkImageToControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::ControlPoint& controlPoint)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid controlPoint UIDs for the current case
  VectorProperty<std::string>* controlPointsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  if (nullptr == controlPointsVectorProperty)
  {
    MITK_DEBUG << "Could not find any control point in the storage.";
    return;
  }

  std::vector<std::string> controlPointsVectorPropertyValue = controlPointsVectorProperty->GetValue();
  const auto existingControlPoint = std::find(controlPointsVectorPropertyValue.begin(), controlPointsVectorPropertyValue.end(), controlPoint.UID);
  if (existingControlPoint != controlPointsVectorPropertyValue.end())
  {
    // set / overwrite the control point reference of the given data
    // retrieve a vector property that contains the referenced ID of a image (0. information type 1. control point ID)
    VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
    if (nullptr == imageVectorProperty)
    {
      MITK_DEBUG << "Could not find the image " << imageID << " in the storage. Cannot link data to control point.";
      return;
    }

    std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
    // an image has to have exactly two values (the information type and the ID of the control point)
    if (imageVectorPropertyValue.size() != 2)
    {
      MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
      return;
    }

    // the second value of the image vector is the ID of the referenced control point
    imageVectorPropertyValue[1] = controlPoint.UID;
    imageVectorProperty->SetValue(imageVectorPropertyValue);
    return;
  }

  MITK_DEBUG << "Could not find control point " << controlPoint.UID << " in the storage. Cannot link data to control point.";
}

void mitk::RelationStorage::UnlinkImageFromControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the referenced ID of a date (0. information type 1. control point ID)
  VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageVectorProperty)
  {
    MITK_DEBUG << "Could not find the date " << imageID << " in the storage. Cannot unlink control point from date.";
    return;
  }

  std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
  // an image has to have exactly two values (the information type and the ID of the control point)
  if (imageVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
    return;
  }

  // the second value of the image vector is the ID of the referenced control point
  // set the control point reference to an empty string for removal
  imageVectorPropertyValue[1] = "";
  imageVectorProperty->SetValue(imageVectorPropertyValue);
}

void mitk::RelationStorage::RemoveControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid controlPoint UIDs for the current case
  VectorProperty<std::string>* controlPointsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("controlpoints"));
  if (nullptr == controlPointsVectorProperty)
  {
    MITK_DEBUG << "Could not find any control point in the storage.";
    return;
  }

  // remove the control point reference from the list of all control points of the current case
  std::vector<std::string> controlPointsVectorPropertyValue = controlPointsVectorProperty->GetValue();
  controlPointsVectorPropertyValue.erase(std::remove(controlPointsVectorPropertyValue.begin(), controlPointsVectorPropertyValue.end(), controlPoint.UID), controlPointsVectorPropertyValue.end());
  if (controlPointsVectorPropertyValue.empty())
  {
    // no more control points stored -> remove the control point property list
    propertyList->DeleteProperty("controlpoints");
  }
  else
  {
    // or store the modified vector value
    controlPointsVectorProperty->SetValue(controlPointsVectorPropertyValue);
  }

  // remove the control point instance itself
  propertyList->DeleteProperty(controlPoint.UID);
}

void mitk::RelationStorage::AddExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid examination period UIDs for the current case
  VectorProperty<std::string>::Pointer examinationPeriodsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("examinationperiods"));
  std::vector<std::string> examinationPeriodsVectorPropertyValue;
  if (nullptr == examinationPeriodsVectorProperty)
  {
    examinationPeriodsVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    examinationPeriodsVectorPropertyValue = examinationPeriodsVectorProperty->GetValue();
  }

  const auto& existingIndex = std::find(examinationPeriodsVectorPropertyValue.begin(), examinationPeriodsVectorPropertyValue.end(), examinationPeriod.UID);
  if (existingIndex != examinationPeriodsVectorPropertyValue.end())
  {
    return;
  }

  // add the new examination period id from the given examination period to the vector of all current examination period UIDs
  examinationPeriodsVectorPropertyValue.push_back(examinationPeriod.UID);
  // overwrite the current vector property with the new, extended string vector
  examinationPeriodsVectorProperty->SetValue(examinationPeriodsVectorPropertyValue);
  propertyList->SetProperty("examinationperiods", examinationPeriodsVectorProperty);

  // add the examination period with the UID as the key and the name as as the vector value
  std::vector<std::string> examinationPeriodData;
  examinationPeriodData.push_back(examinationPeriod.name);
  VectorProperty<std::string>::Pointer newExaminationPeriodVectorProperty = VectorProperty<std::string>::New();
  newExaminationPeriodVectorProperty->SetValue(examinationPeriodData);
  propertyList->SetProperty(examinationPeriod.UID, newExaminationPeriodVectorProperty);
}

void mitk::RelationStorage::AddControlPointToExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the represented control point UIDs of the given examination period
  VectorProperty<std::string>* controlPointUIDsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(examinationPeriod.UID));
  if (nullptr == controlPointUIDsVectorProperty)
  {
    MITK_DEBUG << "Could not find the examination period " << examinationPeriod.UID << " in the storage. Cannot add the control point to the examination period.";
    return;
  }

  std::vector<std::string> controlPointUIDsVectorPropertyValue = controlPointUIDsVectorProperty->GetValue();
  // store the control point UID
  controlPointUIDsVectorPropertyValue.push_back(controlPoint.UID);
  // sort the vector according to the date of the control points referenced by the UIDs
  auto lambda = [&caseID](const SemanticTypes::ID& leftControlPointUID, const SemanticTypes::ID& rightControlPointUID)
  {
    const auto& leftControlPoint = GenerateControlpoint(caseID, leftControlPointUID);
    const auto& rightControlPoint = GenerateControlpoint(caseID, rightControlPointUID);

    return leftControlPoint.date <= rightControlPoint.date;
  };

  std::sort(controlPointUIDsVectorPropertyValue.begin(), controlPointUIDsVectorPropertyValue.end(), lambda);
  // store the modified and sorted control point UID vector of this examination period
  controlPointUIDsVectorProperty->SetValue(controlPointUIDsVectorPropertyValue);
}

void mitk::RelationStorage::RemoveControlPointFromExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }

  // retrieve a vector property that contains the represented control point UIDs of the given examination period
  VectorProperty<std::string>* controlPointUIDsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(examinationPeriod.UID));
  if (nullptr == controlPointUIDsVectorProperty)
  {
    MITK_DEBUG << "Could not find examination period " << examinationPeriod.UID << " in the storage. Cannot add the control point to the examination period.";
    return;
  }

  std::vector<std::string> controlPointUIDsVectorPropertyValue = controlPointUIDsVectorProperty->GetValue();
  // an examination period has an arbitrary number of vector values (name and control point UIDs) (at least one for the name)
  if (controlPointUIDsVectorPropertyValue.size() < 2)
  {
    MITK_DEBUG << "Incorrect examination period storage. At least one (1) control point ID has to be stored.";
    return;
  }
  else
  {
    controlPointUIDsVectorPropertyValue.erase(std::remove(controlPointUIDsVectorPropertyValue.begin(), controlPointUIDsVectorPropertyValue.end(), controlPoint.UID), controlPointUIDsVectorPropertyValue.end());
    if (controlPointUIDsVectorPropertyValue.size() < 2)
    {
      RemoveExaminationPeriod(caseID, examinationPeriod);
    }
    else
    {
      // store the modified vector value
      controlPointUIDsVectorProperty->SetValue(controlPointUIDsVectorPropertyValue);
    }
  }
}

void mitk::RelationStorage::RemoveExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid examination period UIDs for the current case
  VectorProperty<std::string>::Pointer examinationPeriodsVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("examinationperiods"));
  if (nullptr == examinationPeriodsVectorProperty)
  {
    MITK_DEBUG << "Could not find any examination periods in the storage.";
    return;
  }

  std::vector<std::string> examinationPeriodsVectorPropertyValue = examinationPeriodsVectorProperty->GetValue();
  examinationPeriodsVectorPropertyValue.erase(std::remove(examinationPeriodsVectorPropertyValue.begin(), examinationPeriodsVectorPropertyValue.end(), examinationPeriod.UID), examinationPeriodsVectorPropertyValue.end());
  if (examinationPeriodsVectorPropertyValue.empty())
  {
    // no more examination periods stored -> remove the examination period property list
    propertyList->DeleteProperty("examinationperiods");
  }
  else
  {
    // or store the modified vector value
    examinationPeriodsVectorProperty->SetValue(examinationPeriodsVectorPropertyValue);
  }

  // remove the examination period instance itself
  propertyList->DeleteProperty(examinationPeriod.UID);
}

void mitk::RelationStorage::AddInformationTypeToImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::InformationType& informationType)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid information types of the current case
  VectorProperty<std::string>::Pointer informationTypesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("informationtypes"));
  std::vector<std::string> informationTypesVectorPropertyValue;
  if (nullptr == informationTypesVectorProperty)
  {
    informationTypesVectorProperty = VectorProperty<std::string>::New();
  }
  else
  {
    informationTypesVectorPropertyValue = informationTypesVectorProperty->GetValue();
  }
 
  const auto existingInformationType = std::find(informationTypesVectorPropertyValue.begin(), informationTypesVectorPropertyValue.end(), informationType);
  if (existingInformationType == informationTypesVectorPropertyValue.end())
  {
    // at first: add the information type to the storage
    informationTypesVectorPropertyValue.push_back(informationType);
    informationTypesVectorProperty->SetValue(informationTypesVectorPropertyValue);
    propertyList->SetProperty("informationtypes", informationTypesVectorProperty);
  }

  // set / overwrite the information type of the given data
  // retrieve a vector property that contains the referenced ID of an image (0. information type 1. control point ID)
  VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageVectorProperty)
  {
    MITK_DEBUG << "Could not find the image " << imageID << " in the storage. Cannot add information type to image.";
    return;
  }

  std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
  // an image has to have exactly two values (the information type and the ID of the control point)
  if (imageVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
    return;
  }

  // the first value of the image vector is the information type
  imageVectorPropertyValue[0] = informationType;
  imageVectorProperty->SetValue(imageVectorPropertyValue);
}

void mitk::RelationStorage::RemoveInformationTypeFromImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the referenced ID of an image (0. information type 1. control point ID)
  VectorProperty<std::string>* imageVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty(imageID));
  if (nullptr == imageVectorProperty)
  {
    MITK_DEBUG << "Could not find the image " << imageID << " in the storage. Cannot remove information type from image.";
    return;
  }

  std::vector<std::string> imageVectorPropertyValue = imageVectorProperty->GetValue();
  // an image has to have exactly two values (the information type and the ID of the control point)
  if (imageVectorPropertyValue.size() != 2)
  {
    MITK_DEBUG << "Incorrect data storage. Not two (2) values stored.";
    return;
  }

  // the first value of the image vector is the information type
  // set the information type to an empty string for removal
  imageVectorPropertyValue[0] = "";
  imageVectorProperty->SetValue(imageVectorPropertyValue);
}

void mitk::RelationStorage::RemoveInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType)
{
  PropertyList::Pointer propertyList = GetStorageData(caseID);
  if (nullptr == propertyList)
  {
    MITK_DEBUG << "Could not find the property list " << caseID << " for the current MITK workbench / session.";
    return;
  }
  // retrieve a vector property that contains the valid information types of the current case
  VectorProperty<std::string>* informationTypesVectorProperty = dynamic_cast<VectorProperty<std::string>*>(propertyList->GetProperty("informationtypes"));
  if (nullptr == informationTypesVectorProperty)
  {
    MITK_DEBUG << "Could not find any information type in the storage.";
    return;
  }

  std::vector<std::string> informationTypesVectorPropertyValue = informationTypesVectorProperty->GetValue();
  informationTypesVectorPropertyValue.erase(std::remove(informationTypesVectorPropertyValue.begin(), informationTypesVectorPropertyValue.end(), informationType), informationTypesVectorPropertyValue.end());
  if (informationTypesVectorPropertyValue.empty())
  {
    // no more information types stored -> remove the information types property list
    propertyList->DeleteProperty("informationtypes");
  }
  else
  {
    // or store the modified vector value
    informationTypesVectorProperty->SetValue(informationTypesVectorPropertyValue);
  }
}
