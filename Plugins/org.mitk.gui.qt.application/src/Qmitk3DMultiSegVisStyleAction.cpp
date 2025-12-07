/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "Qmitk3DMultiSegVisStyleAction.h"

// mitk core
#include <mitkImage.h>
#include <mitkLookupTableProperty.h>
#include <mitkLookupTable.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingModeProperty.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMenu>

Qmitk3DMultiSegVisStyleAction::Qmitk3DMultiSegVisStyleAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("3D visualization"));
  InitializeAction();
}

Qmitk3DMultiSegVisStyleAction::Qmitk3DMultiSegVisStyleAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("3D visualization"));
  InitializeAction();
}

void Qmitk3DMultiSegVisStyleAction::InitializeAction()
{
  setCheckable(true);

  setMenu(new QMenu);
  connect(menu(), &QMenu::aboutToShow, this, &Qmitk3DMultiSegVisStyleAction::OnMenuAboutShow);
}

void Qmitk3DMultiSegVisStyleAction::OnMenuAboutShow()
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  bool hide3Dvisualize = false;
  dataNode->GetBoolProperty("org.mitk.multilabel.3D.hide", hide3Dvisualize, baseRenderer);

  menu()->clear();

  auto visAction = menu()->addAction("off");
  visAction->setCheckable(true);
  visAction->setChecked(hide3Dvisualize);
  visAction->setData(true);
  connect(visAction, &QAction::triggered, this, &Qmitk3DMultiSegVisStyleAction::OnActionTriggered);

  visAction = menu()->addAction("on");
  visAction->setCheckable(true);
  visAction->setChecked(!hide3Dvisualize);
  visAction->setData(false);
  connect(visAction, &QAction::triggered, this, &Qmitk3DMultiSegVisStyleAction::OnActionTriggered);
}

void Qmitk3DMultiSegVisStyleAction::OnActionTriggered(bool /*checked*/)
{
  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  auto selectedNodes = GetSelectedNodes();

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if (nullptr == senderAction)
    return;

  auto hide3Dvisualize = senderAction->data().toBool();

  for (auto& dataNode : selectedNodes)
  {
    if (dataNode.IsNull())
    {
      continue;
    }

    dataNode->SetBoolProperty("org.mitk.multilabel.3D.hide", hide3Dvisualize, baseRenderer);
  }

  if (nullptr == baseRenderer)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
  }

}
