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

#include <QmitkDataNodeHideAllAction.h>

// mitk core
#include <mitkRenderingManager.h>

namespace HideAllAction
{
	void Run(mitk::DataStorage::Pointer dataStorage)
	{
		auto nodeset = dataStorage->GetAll();
		for (auto& node : *nodeset)
		{
			if (node.IsNotNull())
			{
				node->SetVisibility(false);
			}
		}

		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

QmitkDataNodeHideAllAction::QmitkDataNodeHideAllAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Hide all nodes"));
  InitializeAction();
}

QmitkDataNodeHideAllAction::QmitkDataNodeHideAllAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Hide all nodes"));
  InitializeAction();
}

QmitkDataNodeHideAllAction::~QmitkDataNodeHideAllAction()
{
  // nothing here
}

void QmitkDataNodeHideAllAction::InitializeAction()
{
  connect(this, &QmitkDataNodeHideAllAction::triggered, this, &QmitkDataNodeHideAllAction::OnActionTriggered);
}

void QmitkDataNodeHideAllAction::OnActionTriggered(bool /*checked*/)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

	HideAllAction::Run(m_DataStorage.Lock());
}
