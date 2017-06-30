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

#include "QmitkNavigationToolStorageSelectionWidget.h"

//mitk headers
#include <mitkNavigationDataSource.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>



QmitkNavigationToolStorageSelectionWidget::QmitkNavigationToolStorageSelectionWidget(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
  m_Controls = nullptr;
  CreateQtPartControl(this);
  CreateConnections();

}


QmitkNavigationToolStorageSelectionWidget::~QmitkNavigationToolStorageSelectionWidget()
{

}

void QmitkNavigationToolStorageSelectionWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationToolStorageSelectionWidgetControls;
    m_Controls->setupUi(parent);
  }
}

void QmitkNavigationToolStorageSelectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ServiceListWidget), SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(NavigationToolStorageSelected(us::ServiceReferenceU)) );

  }

  //initialize service list widget
  std::string empty = "";
  m_Controls->m_ServiceListWidget->Initialize<mitk::NavigationToolStorage>(mitk::NavigationToolStorage::US_PROPKEY_STORAGE_NAME,empty);
}

void QmitkNavigationToolStorageSelectionWidget::NavigationToolStorageSelected(us::ServiceReferenceU s)
  {
    if (!s) //nothing selected
      {
        //reset everything
        m_CurrentStorage = nullptr;
        emit NavigationToolStorageSelected(m_CurrentStorage);
        return;
      }

    // Get storage
    us::ModuleContext* context = us::GetModuleContext();
    m_CurrentStorage = context->GetService<mitk::NavigationToolStorage>(s);
    emit NavigationToolStorageSelected(m_CurrentStorage);
  }

mitk::NavigationToolStorage::Pointer QmitkNavigationToolStorageSelectionWidget::GetSelectedNavigationToolStorage()
  {
  return this->m_CurrentStorage;
  }
