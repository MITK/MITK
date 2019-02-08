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
#include "QmitkDataNodeOpenInAction.h"

// mitk core
#include <mitkImage.h>
#include <mitkRenderingManager.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMessageBox>

// qt
#include <QMenu>

QmitkDataNodeOpenInAction::QmitkDataNodeOpenInAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Open in"));
  InitializeAction();
}

QmitkDataNodeOpenInAction::QmitkDataNodeOpenInAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Open in"));
  InitializeAction();
}

QmitkDataNodeOpenInAction::~QmitkDataNodeOpenInAction()
{
  // nothing here
}

void QmitkDataNodeOpenInAction::SetControlledRenderer(RendererVector controlledRenderer)
{
  if (m_ControlledRenderer != controlledRenderer)
  {
    // set the new set of controlled renderer
    m_ControlledRenderer = controlledRenderer;
  }
}

void QmitkDataNodeOpenInAction::InitializeAction()
{
  setCheckable(true);

  setMenu(new QMenu);
  connect(menu(), &QMenu::aboutToShow, this, &QmitkDataNodeOpenInAction::OnMenuAboutToShow);

  SetControlledRenderer();
}

void QmitkDataNodeOpenInAction::OnMenuAboutToShow()
{
  menu()->clear();
  QAction* action;

  for (const auto& renderer : m_ControlledRenderer)
  {
    action = menu()->addAction(QString::fromStdString(renderer->GetName()));
    connect(action, &QAction::triggered, this, &QmitkDataNodeOpenInAction::OnActionTriggered);
  }
}

void QmitkDataNodeOpenInAction::OnActionTriggered(bool /*checked*/)
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if (nullptr == senderAction)
  {
    return;
  }

  std::string selectedRenderer = senderAction->text().toStdString();
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetByName(selectedRenderer);
  if (nullptr == renderer)
  {
    return;
  }

  auto image = dynamic_cast<mitk::Image*>(dataNode->GetData());
  if (nullptr == image)
  {
    return;
  }

  mitk::RenderingManager::GetInstance()->InitializeView(renderer->GetRenderWindow(), image->GetTimeGeometry());
}

void QmitkDataNodeOpenInAction::SetControlledRenderer()
{
  const mitk::RenderingManager::RenderWindowVector allRegisteredRenderWindows =
    mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  mitk::BaseRenderer *baseRenderer = nullptr;
  m_ControlledRenderer.clear();
  for (const auto &renderWindow : allRegisteredRenderWindows)
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow);
    if (nullptr != baseRenderer)
    {
      m_ControlledRenderer.push_back(baseRenderer);
    }
  }
}
