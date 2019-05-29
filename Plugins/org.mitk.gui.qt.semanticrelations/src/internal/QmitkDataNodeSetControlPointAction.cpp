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
#include "QmitkDataNodeSetControlPointAction.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsInference.h>
#include <mitkUIDGeneratorBoost.h>

// semantic relations UI module
#include "QmitkControlPointDialog.h"

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QInputDialog>

QmitkDataNodeSetControlPointAction::QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(workbenchPartSite)
{
  setText(tr("Set control point"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetControlPointAction::QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Set control point"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetControlPointAction::~QmitkDataNodeSetControlPointAction()
{
  // nothing here
}

void QmitkDataNodeSetControlPointAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeSetControlPointAction::OnActionTriggered);
}

void QmitkDataNodeSetControlPointAction::OnActionTriggered(bool /*checked*/)
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

  QmitkControlPointDialog* inputDialog = new QmitkControlPointDialog(m_Parent);
  inputDialog->setWindowTitle("Set control point");
  inputDialog->SetCurrentDate(mitk::SemanticRelationsInference::GetControlPointOfImage(dataNode));

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  const QDate& userSelectedDate = inputDialog->GetCurrentDate();
  mitk::SemanticTypes::ControlPoint controlPoint;
  controlPoint.UID = mitk::UIDGeneratorBoost::GenerateUID();
  controlPoint.date = boost::gregorian::date(userSelectedDate.year(),
                                             userSelectedDate.month(),
                                             userSelectedDate.day());

  try
  {
    m_SemanticRelationsIntegration->UnlinkImageFromControlPoint(dataNode);
    m_SemanticRelationsIntegration->SetControlPointOfImage(dataNode, controlPoint);
  }
  catch (const mitk::SemanticRelationException&)
  {
    return;
  }
}
