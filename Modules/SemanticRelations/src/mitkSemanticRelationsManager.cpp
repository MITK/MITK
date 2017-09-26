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

#include "mitkSemanticRelationsmanager.h"

// semantic relation module
#include "mitkSemanticRelationException.h"
#include "mitkUIDGeneratorBoost.h"

// c++
#include <algorithm>

mitk::SemanticRelationsManager::SemanticRelationsManager(mitk::DataStorage::Pointer dataStorage)
{
  m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(dataStorage);
}

mitk::SemanticRelationsManager::~SemanticRelationsManager()
{
  // nothing here
}

void mitk::SemanticRelationsManager::AddObserver(ISemanticRelationsObserver* observer)
{
  std::vector<mitk::ISemanticRelationsObserver*>::iterator existingObserver = std::find(m_ObserverVector.begin(), m_ObserverVector.end(), observer);
  if (existingObserver != m_ObserverVector.end())
  {
    // no need to add the already existing observer
    return;
  }

  m_ObserverVector.push_back(observer);
}

void mitk::SemanticRelationsManager::RemoveObserver(ISemanticRelationsObserver* observer)
{
  m_ObserverVector.erase(std::remove(m_ObserverVector.begin(), m_ObserverVector.end(), observer), m_ObserverVector.end());
}

void mitk::SemanticRelationsManager::NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) const
{
  // if the case ID of updates instance is equal to the currently active caseID
  if (caseID == m_CaseID)
  {
    for (auto& observer : m_ObserverVector)
    {
      observer->Update(m_CaseID);
    }
  }
}

void mitk::SemanticRelationsManager::SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  // notify observer so that they update their view to the data of the new patient (case)
  NotifyObserver(m_CaseID);
}

/************************************************************************/
/* functions to add/remove image and segmentation instances				      */
/************************************************************************/

void mitk::SemanticRelationsManager::AddImageInstance(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = DICOMHelper::GetCaseIDFromDataNode(dataNode);
  SemanticTypes::ID nodeID = DICOMHelper::GetIDFromDataNode(dataNode);

  mitk::RelationStorage& relationStorage = m_SemanticRelations->GetRelationStorage();
  relationStorage.AddCase(caseID);
  relationStorage.AddImage(caseID, nodeID);

  m_SemanticRelations->AddInformationTypeToImage(dataNode, "unspecified");
  // find the correct control point for this image
  std::vector<SemanticTypes::ControlPoint> allControlPoints = relationStorage.GetAllControlPointsOfCase(caseID);
  if (allControlPoints.empty())
  {
    // no stored control point
    // create a new control point for the image data
    SemanticTypes::ControlPoint newControlPoint = ControlPointManager::GenerateControlPoint(dataNode);
    try
    {
      m_SemanticRelations->AddControlPointAndLinkData(dataNode, newControlPoint);
      NotifyObserver(caseID);
    }
    catch (const mitk::SemanticRelationException&)
    {
      MITK_INFO << "The control point cannot be added and the data can not be linked to this control point.";
    }
    return;
  }

  // some control points already exist - find a control point where the date of the data node fits in
  SemanticTypes::ControlPoint fittingControlPoint = ControlPointManager::FindFittingControlPoint(dataNode, allControlPoints);
  if (fittingControlPoint.UID.empty())
  {
    // did not find a fitting control point, although some control points already exist
    // need to see if a close control point can be extended
    SemanticTypes::ControlPoint extendedControlPoint = ControlPointManager::ExtendClosestControlPoint(dataNode, allControlPoints);
    if (extendedControlPoint.UID.empty())
    {
      // closest control point can not be extended
      // create a new control point for the image data
      SemanticTypes::ControlPoint newControlPoint = ControlPointManager::GenerateControlPoint(dataNode);
      try
      {
        m_SemanticRelations->AddControlPointAndLinkData(dataNode, newControlPoint);
        NotifyObserver(caseID);
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The control point cannot be added and the data can not be linked to this control point.";
      }
    }
    else
    {
      try
      {
        // found a control point that was extended
        m_SemanticRelations->OverwriteControlPointAndLinkData(dataNode, extendedControlPoint);
        NotifyObserver(caseID);
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The extended control point can not be overwritten and the data can not be linked to this control point.";
      }
    }
  }
  else
  {
    // found a fitting control point
    m_SemanticRelations->LinkDataToControlPoint(dataNode, fittingControlPoint);
    NotifyObserver(caseID);
  }
}

void mitk::SemanticRelationsManager::RemoveImageInstance(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = DICOMHelper::GetCaseIDFromDataNode(dataNode);
  SemanticTypes::ID nodeID = DICOMHelper::GetIDFromDataNode(dataNode);

  m_SemanticRelations->RemoveInformationTypeFromImage(dataNode);
  m_SemanticRelations->UnlinkDataFromControlPoint(dataNode);
  mitk::RelationStorage& relationStorage = m_SemanticRelations->GetRelationStorage();
  relationStorage.RemoveImage(caseID, nodeID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelationsManager::AddSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode)
{
  if (nullptr == segmentationNode)
  {
    MITK_INFO << "Not a valid segmentation data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = DICOMHelper::GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationNodeID = DICOMHelper::GetIDFromDataNode(segmentationNode);
  SemanticTypes::ID parentNodeID = DICOMHelper::GetIDFromDataNode(parentNode);

  mitk::RelationStorage& relationStorage = m_SemanticRelations->GetRelationStorage();
  relationStorage.AddCase(caseID);
  relationStorage.AddSegmentation(caseID, segmentationNodeID, parentNodeID);

  SemanticTypes::Lesion lesion;
  lesion.UID = UIDGeneratorBoost::GenerateUID();
  lesion.lesionClass = SemanticTypes::LesionClass();
  lesion.lesionClass.UID = UIDGeneratorBoost::GenerateUID();
  m_SemanticRelations->AddLesionAndLinkData(segmentationNode, lesion);
  NotifyObserver(caseID);
}

void mitk::SemanticRelationsManager::RemoveSegmentationInstance(const mitk::DataNode* segmentationNode)
{
  if (nullptr == segmentationNode)
  {
    MITK_INFO << "Not a valid segmentation data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = DICOMHelper::GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationNodeID = DICOMHelper::GetIDFromDataNode(segmentationNode);

  m_SemanticRelations->UnlinkSegmentationFromLesion(segmentationNode);
  mitk::RelationStorage& relationStorage = m_SemanticRelations->GetRelationStorage();
  relationStorage.RemoveSegmentation(caseID, segmentationNodeID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelationsManager::GenerateNewLesion(const mitk::SemanticTypes::CaseID caseID)
{
  SemanticTypes::Lesion newLesion;
  newLesion.UID = UIDGeneratorBoost::GenerateUID();
  newLesion.lesionClass = SemanticTypes::LesionClass();
  newLesion.lesionClass.UID = UIDGeneratorBoost::GenerateUID();

  m_SemanticRelations->AddLesion(caseID, newLesion);
  NotifyObserver(caseID);
}
