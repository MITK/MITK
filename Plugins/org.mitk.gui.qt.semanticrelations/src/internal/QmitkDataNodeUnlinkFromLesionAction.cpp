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
#include "QmitkDataNodeUnlinkFromLesionAction.h"

// semantic relations module
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMessageBox>

// namespace that contains the concrete action
namespace UnlinkFromLesionAction
{
  void Run(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* dataNode)
  {
    if (nullptr == dataNode)
    {
      return;
    }

    if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      try
      {
        semanticRelationsIntegration->UnlinkSegmentationFromLesion(dataNode);
      }
      catch (const mitk::SemanticRelationException& e)
      {
        std::stringstream exceptionMessage; exceptionMessage << e;
        QMessageBox msgBox(QMessageBox::Warning,
          "Could not unlink the selected segmentation.",
          "The program wasn't able to correctly unlink the selected segmentation.\n"
          "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
        msgBox.exec();
      }
    }
    else
    {
      QMessageBox msgBox(QMessageBox::Warning,
        "Could not unlink the selected data node.",
        "Please chose a valid segmentation to unlink from its represented lesion!");
      msgBox.exec();
    }
  }
}

QmitkDataNodeUnlinkFromLesionAction::QmitkDataNodeUnlinkFromLesionAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(workbenchPartSite)
{
  setText(tr("Unlink from lesion"));
  InitializeAction();
}

QmitkDataNodeUnlinkFromLesionAction::QmitkDataNodeUnlinkFromLesionAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Unlink from lesion"));
  InitializeAction();
}

QmitkDataNodeUnlinkFromLesionAction::~QmitkDataNodeUnlinkFromLesionAction()
{
  // nothing here
}

void QmitkDataNodeUnlinkFromLesionAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeUnlinkFromLesionAction::OnActionTriggered);
}

void QmitkDataNodeUnlinkFromLesionAction::OnActionTriggered(bool /*checked*/)
{
  if (nullptr == m_SemanticRelationsIntegration)
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  UnlinkFromLesionAction::Run(m_SemanticRelationsIntegration.get(), dataNode);
}
