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

#include "mitkSemanticRelationsTestHelper.h"

// semantic relation module
#include "mitkSemanticRelationException.h"
#include "mitkUIDGenerator.h"

// c++
#include <algorithm>

/************************************************************************/
/* functions to add/remove image and segmentation instances				      */
/************************************************************************/

void SemanticRelationsTestHelper::AddImageInstance(const mitk::DataNode* dataNode, mitk::SemanticRelations& semanticRelationsInstance)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = NodeIdentifier::GetCaseIDFromData(dataNode);
  SemanticTypes::ID nodeID = NodeIdentifier::GetIDFromData(dataNode);

  std::shared_ptr<mitk::RelationStorage> relationStorage = semanticRelationsInstance.GetRelationStorage();
  relationStorage->AddCase(caseID);
  relationStorage->AddImage(caseID, nodeID);

  // find the correct control point for this image
  std::vector<SemanticTypes::ControlPoint> allControlPoints = relationStorage->GetAllControlPointsOfCase(caseID);
  if (allControlPoints.empty())
  {
    // no stored control point
    // create a new control point for the image data
    SemanticTypes::ControlPoint newControlPoint = ControlPointManager::GenerateControlPoint(dataNode);
    try
    {
      semanticRelationsInstance.AddControlPointAndLinkData(dataNode, newControlPoint);
      return;
    }
    catch (const mitk::SemanticRelationException&)
    {
      MITK_INFO << "The control point can not be added and the data can not be linked to this control point.";
      return;
    }
  }

  // some control points already exist - find a control point where the date of the data node fits in
  SemanticTypes::ControlPoint fittingControlPoint = ControlPointManager::FindControlPoint(dataNode, allControlPoints);
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
        semanticRelationsInstance.AddControlPointAndLinkData(dataNode, newControlPoint);
        return;
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The control point can not be added and the data can not be linked to this control point.";
        return;
      }
    }
    else
    {
      try
      {
        // found a control point that was extended
        semanticRelationsInstance.OverwriteControlPointAndLinkData(dataNode, extendedControlPoint);
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The fitting control point can not be overwritten and the data can not be linked to this control point.";
        return;
      }
    }
  }
  else
  {
    // found a fitting control point
    semanticRelationsInstance.LinkDataToControlPoint(dataNode, fittingControlPoint);
  }
}

void SemanticRelationsTestHelper::AddSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode, mitk::SemanticRelations& semanticRelationsInstance)
{
  if (nullptr == segmentationNode)
  {
    MITK_INFO << "Not a valid segmentation data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = NodeIdentifier::GetCaseIDFromData(segmentationNode);
  SemanticTypes::ID segmentationNodeID = NodeIdentifier::GetIDFromData(segmentationNode);
  SemanticTypes::ID parentNodeID = NodeIdentifier::GetIDFromData(parentNode);

  std::shared_ptr<mitk::RelationStorage> relationStorage = semanticRelationsInstance.GetRelationStorage();
  relationStorage->AddCase(caseID);
  relationStorage->AddSegmentation(caseID, segmentationNodeID, parentNodeID);

  SemanticTypes::Lesion lesion; lesion.UID = UIDGeneratorBoost::GenerateUID();
  lesion.lesionClass = SemanticTypes::LesionClass();
  semanticRelationsInstance.AddLesionAndLinkData(segmentationNode, lesion);
}

void SemanticRelationsTestHelper::RemoveSegmentationInstance(const mitk::DataNode* segmentationNode, mitk::SemanticRelations& semanticRelationsInstance)
{
  if (nullptr == segmentationNode)
  {
    MITK_INFO << "Not a valid segmentation data node.";
    return;
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = NodeIdentifier::GetCaseIDFromData(segmentationNode);
  SemanticTypes::ID segmentationNodeID = NodeIdentifier::GetIDFromData(segmentationNode);

  std::shared_ptr<mitk::RelationStorage> relationStorage = semanticRelationsInstance.GetRelationStorage();
  relationStorage->RemoveSegmentation(caseID, segmentationNodeID);
}
