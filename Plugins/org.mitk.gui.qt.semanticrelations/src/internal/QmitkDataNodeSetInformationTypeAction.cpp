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
#include "QmitkDataNodeSetInformationTypeAction.h"

// semantic relations module
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsInference.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QInputDialog>

QmitkDataNodeSetInformationTypeAction::QmitkDataNodeSetInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(workbenchPartSite)
{
  setText(tr("Set information type"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetInformationTypeAction::QmitkDataNodeSetInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Set information type"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetInformationTypeAction::~QmitkDataNodeSetInformationTypeAction()
{
  // nothing here
}

void QmitkDataNodeSetInformationTypeAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeSetInformationTypeAction::OnActionTriggered);
}

void QmitkDataNodeSetInformationTypeAction::OnActionTriggered(bool /*checked*/)
{
  if (nullptr == m_SemanticRelationsIntegration)
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  QInputDialog* inputDialog = new QInputDialog(m_Parent);
  inputDialog->setWindowTitle(tr("Set information type of selected node"));
  inputDialog->setLabelText(tr("Information type:"));
  inputDialog->setTextValue(QString::fromStdString(mitk::SemanticRelationsInference::GetInformationTypeOfImage(dataNode)));
  inputDialog->setMinimumSize(250, 100);

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  try
  {
    m_SemanticRelationsIntegration->SetInformationType(dataNode, inputDialog->textValue().toStdString());
  }
  catch (const mitk::SemanticRelationException&)
  {
    return;
  }
}
