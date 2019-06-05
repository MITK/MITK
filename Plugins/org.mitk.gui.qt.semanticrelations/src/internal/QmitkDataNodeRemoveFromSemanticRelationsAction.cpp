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
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"

// semantic relations module
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMessageBox>

// namespace that contains the concrete action
namespace RemoveFromSemanticRelationsAction
{
  void Run(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, mitk::DataStorage* dataStorage, const mitk::DataNode* dataNode)
  {
    if (nullptr == semanticRelationsIntegration
     || nullptr == dataStorage
     || nullptr == dataNode)
    {
      return;
    }

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      try
      {
        RemoveImage(semanticRelationsIntegration, dataStorage, dataNode);
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
        RemoveSegmentation(semanticRelationsIntegration, dataNode);
      }
      catch (mitk::SemanticRelationException& e)
      {
        mitkReThrow(e);
      }
    }
  }

  void RemoveImage(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, mitk::DataStorage* dataStorage, const mitk::DataNode* image)
  {
    try
    {
      // remove each corresponding segmentation from the semantic relations storage
      mitk::DataStorage::SetOfObjects::ConstPointer childNodes = dataStorage->GetDerivations(image, mitk::NodePredicates::GetSegmentationPredicate(), false);
      for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
      {
        RemoveSegmentation(semanticRelationsIntegration, it->Value());
      }
      // remove the image from the semantic relations storage
      semanticRelationsIntegration->RemoveImage(image);
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }

  void RemoveSegmentation(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* segmentation)
  {
    try
    {
      // remove the segmentation from the semantic relations storage
      semanticRelationsIntegration->RemoveSegmentation(segmentation);
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }
}

QmitkDataNodeRemoveFromSemanticRelationsAction::QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(workbenchPartSite)
{
  setText(tr("Remove from semantic relations"));
  InitializeAction();
}

QmitkDataNodeRemoveFromSemanticRelationsAction::QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Remove from semantic relations"));
  InitializeAction();
}

QmitkDataNodeRemoveFromSemanticRelationsAction::~QmitkDataNodeRemoveFromSemanticRelationsAction()
{
  // nothing here
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeRemoveFromSemanticRelationsAction::OnActionTriggered);
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::OnActionTriggered(bool /*checked*/)
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
  RemoveFromSemanticRelationsAction::Run(m_SemanticRelationsIntegration.get(), m_DataStorage.Lock(),dataNode);
}
