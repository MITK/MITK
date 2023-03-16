/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOpenStdMultiWidgetEditorAction.h"

#include "mitkCoreObjectFactory.h"

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbench.h>
#include <mitkDataStorageEditorInput.h>

#include "internal/QmitkCommonExtPlugin.h"
#include <mitkIDataStorageService.h>

class ctkPluginContext;

QmitkOpenStdMultiWidgetEditorAction::QmitkOpenStdMultiWidgetEditorAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
{
  this->init(window);
}

QmitkOpenStdMultiWidgetEditorAction::QmitkOpenStdMultiWidgetEditorAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
{
  this->setIcon(icon);

  this->init(window);
}

void QmitkOpenStdMultiWidgetEditorAction::init(berry::IWorkbenchWindow::Pointer window)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("Standard Display");
  this->setToolTip("Open the standard multi widget editor");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkOpenStdMultiWidgetEditorAction::Run()
{
  // check if there is an open perspective, if not open the default perspective
  if (m_Window->GetActivePage().IsNull())
  {
    QString defaultPerspId = m_Window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
    m_Window->GetWorkbench()->ShowPerspective(defaultPerspId, m_Window);
  }

  ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
  ctkServiceReference serviceRef = context->getServiceReference<mitk::IDataStorageService>();
  if (serviceRef)
  {
    mitk::IDataStorageService* dsService = context->getService<mitk::IDataStorageService>(serviceRef);
    if (dsService)
    {
      mitk::IDataStorageReference::Pointer dsRef = dsService->GetDataStorage();
      berry::IEditorInput::Pointer editorInput(new mitk::DataStorageEditorInput(dsRef));
      m_Window->GetActivePage()->OpenEditor(editorInput, "org.mitk.editors.stdmultiwidget", true, berry::IWorkbenchPage::MATCH_ID);
    }
  }
}
