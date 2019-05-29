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

#include <QmitkDataNodeSurfaceRepresentationAction.h>

// mitk core
#include <mitkEnumerationProperty.h>
#include <mitkRenderingManager.h>

// qt
#include <QMenu>

QmitkDataNodeSurfaceRepresentationAction::QmitkDataNodeSurfaceRepresentationAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Surface Representation"));
  InitializeAction();
}

QmitkDataNodeSurfaceRepresentationAction::QmitkDataNodeSurfaceRepresentationAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Surface Representation"));
  InitializeAction();
}

void QmitkDataNodeSurfaceRepresentationAction::InitializeAction()
{
  setCheckable(true);

  setMenu(new QMenu);
  connect(menu(), &QMenu::aboutToShow, this, &QmitkDataNodeSurfaceRepresentationAction::OnMenuAboutShow);
}

void QmitkDataNodeSurfaceRepresentationAction::OnMenuAboutShow()
{
  mitk::DataNode::Pointer dataNode = GetSelectedNode();
  if (nullptr == dataNode)
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();
  mitk::EnumerationProperty* representationProp = dynamic_cast<mitk::EnumerationProperty*>(dataNode->GetProperty("material.representation", baseRenderer));
  if (nullptr == representationProp)
  {
    return;
  }

  menu()->clear();
  QAction* tmp;

  for (mitk::EnumerationProperty::EnumConstIterator it = representationProp->Begin(); it != representationProp->End(); ++it)
  {
    tmp = menu()->addAction(QString::fromStdString(it->second));
    tmp->setCheckable(true);

    if (it->second == representationProp->GetValueAsString())
    {
      tmp->setChecked(true);
    }

    connect(tmp, &QAction::triggered, this, &QmitkDataNodeSurfaceRepresentationAction::OnActionTriggered);
  }
}

void QmitkDataNodeSurfaceRepresentationAction::OnActionTriggered(bool /*checked*/)
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();
  mitk::EnumerationProperty* representationProp = dynamic_cast<mitk::EnumerationProperty*>(dataNode->GetProperty("material.representation", baseRenderer));
  if (nullptr == representationProp)
  {
    return;
  }

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if (nullptr == senderAction)
  {
    return;
  }

  std::string activatedItem = senderAction->text().toStdString();
  if (activatedItem != representationProp->GetValueAsString())
  {
    if (representationProp->IsValidEnumerationValue(activatedItem))
    {
      representationProp->SetValue(activatedItem);
      representationProp->InvokeEvent(itk::ModifiedEvent());
      representationProp->Modified();

      if (nullptr == baseRenderer)
      {
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
      else
      {
        mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
      }
    }
  }
}
