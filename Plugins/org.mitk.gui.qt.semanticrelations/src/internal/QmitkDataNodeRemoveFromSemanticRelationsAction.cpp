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
  void Run(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* dataNode)
  {
    if (nullptr == dataNode)
    {
      return;
    }

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      RemoveImage(semanticRelationsIntegration, dataNode);
    }
    else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      RemoveSegmentation(semanticRelationsIntegration, dataNode);
    }
  }

  void RemoveImage(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* image)
  {
    if (nullptr == image)
    {
      return;
    }

    try
    {
      // remove the image from the semantic relations storage
      semanticRelationsIntegration->RemoveImage(image);
    }
    catch (const mitk::SemanticRelationException& e)
    {
      std::stringstream exceptionMessage; exceptionMessage << e;
      QMessageBox msgBox;
      msgBox.setWindowTitle("Could not remove the selected image.");
      msgBox.setText("The program wasn't able to correctly remove the selected image.\n"
        "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }

  void RemoveSegmentation(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* segmentation)
  {
    if (nullptr == segmentation)
    {
      return;
    }

    try
    {
      // remove the segmentation from the semantic relations storage
      semanticRelationsIntegration->RemoveSegmentation(segmentation);
    }
    catch (const mitk::SemanticRelationException& e)
    {
      std::stringstream exceptionMessage; exceptionMessage << e;
      QMessageBox msgBox;
      msgBox.setWindowTitle("Could not remove the selected segmentation.");
      msgBox.setText("The program wasn't able to correctly remove the selected segmentation.\n"
        "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
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
  auto dataNode = GetSelectedNode();
  RemoveFromSemanticRelationsAction::Run(m_SemanticRelationsIntegration.get(), dataNode);
}
