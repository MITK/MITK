/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// data storage viewer test plugin
#include "QmitkDataStorageViewerTestView.h"

// berry
#include <berryIWorkbenchWindow.h>

// qt
#include <QHBoxLayout>

const std::string QmitkDataStorageViewerTestView::VIEW_ID = "org.mitk.views.datastorageviewertest";

void QmitkDataStorageViewerTestView::SetFocus()
{
  // nothing here
}

void QmitkDataStorageViewerTestView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_Controls.dataStorageDefaultView->SetDataStorage(GetDataStorage());
  m_DataStorageSelectionConnector = std::make_unique<QmitkDataStorageSelectionConnector>(m_Controls.dataStorageDefaultView);
  m_DataStorageSelectionConnector->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());

  m_Controls.dataStorageDefaultView2->SetDataStorage(GetDataStorage());
  m_DataStorageSelectionConnector2 = std::make_unique<QmitkDataStorageSelectionConnector>(m_Controls.dataStorageDefaultView2);
  m_DataStorageSelectionConnector2->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
  
  connect(m_Controls.selectionProviderCheckBox, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider1(bool)));
  connect(m_Controls.selectionProviderCheckBox2, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider2(bool)));
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider1(bool checked)
{
  if (checked)
  {
    m_DataStorageSelectionConnector->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
  }
  else
  {
    m_DataStorageSelectionConnector->RemoveAsSelectionProvider();
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider2(bool checked)
{
  if (checked)
  {
    m_DataStorageSelectionConnector2->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
  }
  else
  {
    m_DataStorageSelectionConnector2->RemoveAsSelectionProvider();
  }
}
