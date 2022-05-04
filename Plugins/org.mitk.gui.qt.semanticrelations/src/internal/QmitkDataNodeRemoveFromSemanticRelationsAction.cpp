/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"

// semantic relations module
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsIntegration.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMessageBox>

// namespace that contains the concrete action
namespace RemoveFromSemanticRelationsAction
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
        RemoveImage(dataStorage, dataNode);
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
        RemoveSegmentation(dataNode);
      }
      catch (mitk::SemanticRelationException& e)
      {
        mitkReThrow(e);
      }
    }
  }

  void RemoveImage(mitk::DataStorage* dataStorage, const mitk::DataNode* image)
  {
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    try
    {
      // remove each corresponding segmentation from the semantic relations storage
      mitk::DataStorage::SetOfObjects::ConstPointer childNodes = dataStorage->GetDerivations(image, mitk::NodePredicates::GetSegmentationPredicate(), false);
      for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
      {
        RemoveSegmentation(it->Value());
      }
      // remove the image from the semantic relations storage
      semanticRelationsIntegration.RemoveImage(image);
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }

  void RemoveSegmentation(const mitk::DataNode* segmentation)
  {
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    try
    {
      // remove the segmentation from the semantic relations storage
      semanticRelationsIntegration.RemoveSegmentation(segmentation);
    }
    catch (mitk::SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }
}

QmitkDataNodeRemoveFromSemanticRelationsAction::QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Remove from semantic relations"));
  InitializeAction();
}

QmitkDataNodeRemoveFromSemanticRelationsAction::QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Remove from semantic relations"));
  InitializeAction();
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeRemoveFromSemanticRelationsAction::OnActionTriggered);
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::OnActionTriggered(bool /*checked*/)
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  RemoveFromSemanticRelationsAction::Run(dataStorage, dataNode);
}
