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

// semantic relations plugin
#include "QmitkDataNodeAddToSemanticRelationsAction.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMessageBox>

// namespace that contains the concrete action
namespace AddToSemanticRelationsAction
{
  void Run(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataStorage* dataStorage, const mitk::DataNode* dataNode)
  {
    if (nullptr == dataNode)
    {
      return;
    }

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      AddImage(semanticRelationsIntegration, dataNode);
    }
    else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      AddSegmentation(semanticRelationsIntegration, dataStorage, dataNode);
    }
  }

  void AddImage(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* image)
  {
    if (nullptr == image)
    {
      return;
    }

    try
    {
      // add the image to the semantic relations storage
      semanticRelationsIntegration->AddImage(image);
    }
    catch (const mitk::SemanticRelationException& e)
    {
      std::stringstream exceptionMessage; exceptionMessage << e;
      QMessageBox msgBox;
      msgBox.setWindowTitle("Could not add the selected image.");
      msgBox.setText("The program wasn't able to correctly add the selected images.\n"
        "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }

  void AddSegmentation(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataStorage* dataStorage, const mitk::DataNode* segmentation)
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

    // check for already existing, identifying base properties
    mitk::BaseProperty* caseIDProperty = baseData->GetProperty("DICOM.0010.0010");
    mitk::BaseProperty* nodeIDProperty = baseData->GetProperty("DICOM.0020.000E");
    if (nullptr == caseIDProperty || nullptr == nodeIDProperty)
    {
      MITK_INFO << "No DICOM tags for case and node identification found. Transferring DICOM tags from the parent node to the selected segmentation node.";

      mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(parentNodes->front());
      mitk::SemanticTypes::ID nodeID = mitk::GetIDFromDataNode(parentNodes->front());
      // transfer DICOM tags to the segmentation node
      mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(caseID);
      baseData->SetProperty("DICOM.0010.0010", caseIDTag); // DICOM tag is "PatientName"

                                                           // add UID to distinguish between different segmentations of the same parent node
      mitk::StringProperty::Pointer nodeIDTag = mitk::StringProperty::New(nodeID + mitk::UIDGeneratorBoost::GenerateUID());
      baseData->SetProperty("DICOM.0020.000E", nodeIDTag); // DICOM tag is "SeriesInstanceUID"
    }

    try
    {
      // add the segmentation with its parent image to the semantic relations storage
      semanticRelationsIntegration->AddSegmentation(segmentation, parentNodes->front());
    }
    catch (const mitk::SemanticRelationException& e)
    {
      std::stringstream exceptionMessage; exceptionMessage << e;
      QMessageBox msgBox;
      msgBox.setWindowTitle("Could not add the selected segmentation.");
      msgBox.setText("The program wasn't able to correctly add the selected segmentation.\n"
        "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }
}

QmitkDataNodeAddToSemanticRelationsAction::QmitkDataNodeAddToSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(workbenchPartSite)
{
  setText(tr("Add to semantic relations"));
  InitializeAction();
}

QmitkDataNodeAddToSemanticRelationsAction::QmitkDataNodeAddToSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Add to semantic relations"));
  InitializeAction();
}

QmitkDataNodeAddToSemanticRelationsAction::~QmitkDataNodeAddToSemanticRelationsAction()
{
  // nothing here
}

void QmitkDataNodeAddToSemanticRelationsAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeAddToSemanticRelationsAction::OnActionTriggered);
}

void QmitkDataNodeAddToSemanticRelationsAction::OnActionTriggered(bool /*checked*/)
{
  if (nullptr == m_SemanticRelationsIntegration)
  {
    return;
  }

  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  AddToSemanticRelationsAction::Run(m_SemanticRelationsIntegration.get(), m_DataStorage.Lock(), dataNode);
}
