/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMITKIGTNavigationToolManagerView.h"

// MITK
#include <usGetModuleContext.h>

// Qt
#include <QMessageBox>

const std::string QmitkMITKIGTNavigationToolManagerView::VIEW_ID = "org.mitk.views.mitkigtnavigationtoolmanager";

QmitkMITKIGTNavigationToolManagerView::QmitkMITKIGTNavigationToolManagerView()
  : QmitkAbstractView()
  , m_Controls(nullptr)
{
}

QmitkMITKIGTNavigationToolManagerView::~QmitkMITKIGTNavigationToolManagerView()
{
  for (auto storage : m_AllStoragesHandledByThisWidget)
    storage->UnRegisterMicroservice();
}

void QmitkMITKIGTNavigationToolManagerView::CreateQtPartControl(QWidget *parent)
{
  // build up qt view, unless already done
  if (!m_Controls)
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkMITKIGTNavigationToolManagerViewControls;
    m_Controls->setupUi(parent);
    connect((QObject*)(m_Controls->m_toolManagerWidget), SIGNAL(NewStorageAdded(mitk::NavigationToolStorage::Pointer, std::string)), this, SLOT(NewStorageByWidget(mitk::NavigationToolStorage::Pointer)));
    connect((QObject*)(m_Controls->m_ToolStorageListWidget), SIGNAL(NavigationToolStorageSelected(mitk::NavigationToolStorage::Pointer)), this, SLOT(ToolStorageSelected(mitk::NavigationToolStorage::Pointer)));
  }
  m_Controls->m_toolManagerWidget->Initialize(this->GetDataStorage());
}

void QmitkMITKIGTNavigationToolManagerView::SetFocus()
{
  m_Controls->m_ToolStorageListWidget->setFocus();
}

void QmitkMITKIGTNavigationToolManagerView::NewStorageByWidget(mitk::NavigationToolStorage::Pointer storage)
{
  storage->RegisterAsMicroservice();
  m_AllStoragesHandledByThisWidget.push_back(storage);
}

void QmitkMITKIGTNavigationToolManagerView::ToolStorageSelected(mitk::NavigationToolStorage::Pointer storage)
{
  if (storage.IsNull()) //no storage selected
  {
    //reset everything
    return;
  }

  this->m_Controls->m_toolManagerWidget->LoadStorage(storage);
}
