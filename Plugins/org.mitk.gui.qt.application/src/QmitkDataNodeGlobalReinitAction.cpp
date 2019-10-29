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

#include <QmitkDataNodeGlobalReinitAction.h>

// mitk core
#include <mitkRenderingManager.h>

// mitk gui common plugin
#include <mitkWorkbenchUtil.h>

const QString QmitkDataNodeGlobalReinitAction::ACTION_ID = "org.mitk.gui.qt.application.globalreinitaction";

// namespace that contains the concrete action
namespace GlobalReinitAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite, mitk::DataStorage::Pointer dataStorage)
  {
    auto renderWindow = mitk::WorkbenchUtil::GetRenderWindowPart(workbenchPartSite->GetPage(), mitk::WorkbenchUtil::NONE);
    if (nullptr == renderWindow)
    {
      renderWindow = mitk::WorkbenchUtil::OpenRenderWindowPart(workbenchPartSite->GetPage(), false);
      if (nullptr == renderWindow)
      {
        // no render window available
        return;
      }
    }

    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
  }
}

QmitkDataNodeGlobalReinitAction::QmitkDataNodeGlobalReinitAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Global Reinit"));
  InitializeAction();
}

QmitkDataNodeGlobalReinitAction::QmitkDataNodeGlobalReinitAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Global Reinit"));
  InitializeAction();
}

void QmitkDataNodeGlobalReinitAction::InitializeAction()
{
  connect(this, &QmitkDataNodeGlobalReinitAction::triggered, this, &QmitkDataNodeGlobalReinitAction::OnActionTriggered);
}

void QmitkDataNodeGlobalReinitAction::OnActionTriggered(bool /*checked*/)
{
  if (m_WorkbenchPartSite.Expired())
  {
    return;
  }

  if (m_DataStorage.IsExpired())
  {
    return;
  }

  GlobalReinitAction::Run(m_WorkbenchPartSite.Lock(), m_DataStorage.Lock());
}
