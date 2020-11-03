/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkDataNodeAddToSemanticRelationsAction.h"
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"

// semantic relations module
#include <mitkControlPointManager.h>
#include <mitkDICOMHelper.h>
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationsDataStorageAccess.h>
#include <mitkSemanticRelationsInference.h>
#include <mitkSemanticRelationsIntegration.h>
#include <mitkSemanticRelationException.h>
#include <mitkRelationStorage.h>
#include <mitkUIDGeneratorBoost.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// mitk core
#include <mitkTemporoSpatialStringProperty.h>

// qt
#include <QMessageBox>

// namespace that contains the concrete action
namespace AddToSemanticRelationsAction
{
  void Run(mitk::DataStorage* dataStorage, const mitk::DataNode* dataNode)
  {
    if (nullptr == dataStorage
     || nullptr == dataNode)
    {
      return;
    }

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      try
      {
        AddImage(dataStorage, dataNode);
      }
      catch (mitk::SemanticRelationException& e)
      {
        mitkReThrow(e);
      }
    }
    else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      try
      {
        AddSegmentation(dataStorage, dataNode);
      }
      catch (mitk::SemanticRelationException& e)
      {
        mitkReThrow(e);
      }
    }
  }

  void AddImage(mitk::DataStorage* dataStorage, const mitk::DataNode* image)
  {
    mitk::SemanticTypes::InformationType informationType;
    mitk::SemanticTypes::ExaminationPeriod examinationPeriod;
    mitk::SemanticRelationsDataStorageAccess::DataNodeVector allSpecificImages;
    try
    {
      mitk::SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(image);
      informationType = GetDICOMModalityFromDataNode(image);
      // see if the examination period - information type cell is already taken
      examinationPeriod = FindFittingExaminationPeriod(image);
      auto semanticRelationsDataStorageAccess = mitk::SemanticRelationsDataStorageAccess(dataStorage);
      try
      {
        allSpecificImages = semanticRelationsDataStorageAccess.GetAllSpecificImages(caseID, informationType, examinationPeriod);
      }
      catch (const mitk::SemanticRelationException&)
      {
        // just continue since an exception means that there is no specific image
      }
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }

    if (!allSpecificImages.empty())
    {
      // examination period - information type cell is already taken
      // ask if cell should be overwritten
      QMessageBox::StandardButton answerButton =
        QMessageBox::question(nullptr,
          "Specific image already exists.",
          QString::fromStdString("Force overwriting existing image " + informationType + " at " + examinationPeriod.name + "?"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

      if (answerButton == QMessageBox::Yes)
      {
        try
        {
          // remove already existent images at specific cell
          for (const auto& specificImage : allSpecificImages)
          {
            RemoveFromSemanticRelationsAction::Run(dataStorage, specificImage);
          }
        }
        catch (mitk::SemanticRelationException& e)
        {
          mitkReThrow(e);
          return;
        }
      }
      else
      {
        // else case is: no overwriting
        return;
      }
    }

    // specific image does not exist or has been removed; adding the image should work
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    try
    {
      semanticRelationsIntegration.AddImage(image);
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }

  void AddSegmentation(mitk::DataStorage* dataStorage, const mitk::DataNode* segmentation)
  {
    if (nullptr == segmentation)
    {
      return;
    }

    mitk::BaseData* baseData = segmentation->GetData();
    if (nullptr == baseData)
    {
      return;
    }

    // continue with valid segmentation data
    // get parent node of the current segmentation node with the node predicate
    mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = dataStorage->GetSources(segmentation, mitk::NodePredicates::GetImagePredicate(), false);
    if (parentNodes->empty())
    {
      // segmentation without corresponding image will not be added
      QMessageBox msgBox(QMessageBox::Warning,
        "Could not add the selected segmentation.",
        "The program wasn't able to correctly add the selected segmentation.\n"
        "Reason: No parent image found");
      msgBox.exec();
      return;
    }

    // check for already existing, identifying base properties
    auto caseIDPropertyName = mitk::GetCaseIDDICOMProperty();
    auto nodeIDPropertyName = mitk::GetNodeIDDICOMProperty();
    mitk::BaseProperty* caseIDProperty = baseData->GetProperty(caseIDPropertyName.c_str());
    mitk::BaseProperty* nodeIDProperty = baseData->GetProperty(nodeIDPropertyName.c_str());
    if (nullptr == caseIDProperty || nullptr == nodeIDProperty)
    {
      MITK_INFO << "No DICOM tags for case and node identification found. Transferring DICOM tags from the parent node to the selected segmentation node.";

      mitk::SemanticTypes::CaseID caseID;
      mitk::SemanticTypes::ID nodeID;
      try
      {
        caseID = mitk::GetCaseIDFromDataNode(parentNodes->front());
        nodeID = mitk::GetIDFromDataNode(parentNodes->front());
      }
      catch (mitk::SemanticRelationException& e)
      {
        mitkReThrow(e);
        return;
      }

      // transfer DICOM tags to the segmentation node
      baseData->SetProperty(caseIDPropertyName, mitk::TemporoSpatialStringProperty::New(caseID));
      // add UID to distinguish between different segmentations of the same parent node
      baseData->SetProperty(nodeIDPropertyName, mitk::TemporoSpatialStringProperty::New(nodeID + mitk::UIDGeneratorBoost::GenerateUID()));
    }

    // add the parent node if not already existent
    if (!mitk::SemanticRelationsInference::InstanceExists(parentNodes->front()))
    {
      AddImage(dataStorage, parentNodes->front());
    }

    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    try
    {
      // add the segmentation with its parent image to the semantic relations storage
      semanticRelationsIntegration.AddSegmentation(segmentation, parentNodes->front());
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }
}

QmitkDataNodeAddToSemanticRelationsAction::QmitkDataNodeAddToSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Add to semantic relations"));
  InitializeAction();
}

QmitkDataNodeAddToSemanticRelationsAction::QmitkDataNodeAddToSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Add to semantic relations"));
  InitializeAction();
}

void QmitkDataNodeAddToSemanticRelationsAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeAddToSemanticRelationsAction::OnActionTriggered);
}

void QmitkDataNodeAddToSemanticRelationsAction::OnActionTriggered(bool /*checked*/)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  AddToSemanticRelationsAction::Run(m_DataStorage.Lock(), dataNode);
}
