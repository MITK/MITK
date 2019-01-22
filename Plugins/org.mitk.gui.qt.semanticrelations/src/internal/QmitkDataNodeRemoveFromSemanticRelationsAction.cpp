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
#include <mitkUIDGeneratorBoost.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMessageBox>

// namespace that contains the concrete action
namespace RemoveFromSemanticRelationsAction
{
  void Run(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* dataNode)
  {
    if (nullptr == dataNode)
    {
      return;
    }

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      RemoveImage(semanticRelations, dataNode);
    }
    else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      RemoveSegmentation(semanticRelations, dataNode);
    }
  }

  void RemoveImage(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* image)
  {
    if (nullptr == image)
    {
      return;
    }

    try
    {
      // remove the image from the semantic relations storage
      semanticRelations->RemoveImage(image);
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

  void RemoveSegmentation(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* segmentation)
  {
    if (nullptr == segmentation)
    {
      return;
    }

    try
    {
      semanticRelations->RemoveSegmentation(segmentation);
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

QmitkDataNodeRemoveFromSemanticRelationsAction::~QmitkDataNodeRemoveFromSemanticRelationsAction()
{
  // nothing here
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
    m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(m_DataStorage.Lock());
  }
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeRemoveFromSemanticRelationsAction::OnActionTriggered);
}

void QmitkDataNodeRemoveFromSemanticRelationsAction::OnActionTriggered(bool checked)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  RemoveFromSemanticRelationsAction::Run(m_SemanticRelations.get(), dataNode);
}
