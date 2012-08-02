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

#include "QmitkNavigationDataSourceSelectionWidget.h"

//mitk headers
#include <mitkNavigationDataSource.h>
#include <mitkGetModuleContext.h>
#include <qlistwidget.h>




QmitkNavigationDataSourceSelectionWidget::QmitkNavigationDataSourceSelectionWidget(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();

}


QmitkNavigationDataSourceSelectionWidget::~QmitkNavigationDataSourceSelectionWidget()
{

}

void QmitkNavigationDataSourceSelectionWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationDataSourceSelectionWidgetControls;
    m_Controls->setupUi(parent);

    std::string empty = "";
    m_Controls->m_NaviagationDataSourceWidget->Initialize<mitk::NavigationDataSource>(empty,empty);

  }
}

void QmitkNavigationDataSourceSelectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_NaviagationDataSourceWidget), SIGNAL(ServiceSelectionChanged(mitk::ServiceReference*)), this, SLOT(NavigationDataSourceSelected(mitk::ServiceReference*)) );

  }
}

void QmitkNavigationDataSourceSelectionWidget::NavigationDataSourceSelected(mitk::ServiceReference* s)
  {
    // Get Source
    m_CurrentSource = this->m_Controls->m_NaviagationDataSourceWidget->TranslateReference<mitk::NavigationDataSource>(*s);
    std::string id = s->GetProperty(mitk::NavigationDataSource::US_PROPKEY_ID).ToString();
    mitk::ModuleContext* context = mitk::GetModuleContext();
    //Fill tool list
    for(int i = 0; i < m_CurrentSource->GetNumberOfOutputs(); i++) {new QListWidgetItem(tr(m_CurrentSource->GetOutput(i)->GetName()), m_Controls->m_ToolView);}
    // Create Filter for ToolStorage
    std::string filter = "(&(" + mitk::ServiceConstants::OBJECTCLASS() + "=" + mitk::NavigationDataSource::US_INTERFACE_NAME + ")("+ mitk::NavigationDataSource::US_PROPKEY_ID + " = " + id + "))";
    // Get Storage
    std::list<mitk::ServiceReference> refs = context->GetServiceReferences(mitk::NavigationDataSource::US_INTERFACE_NAME, filter);
    if (refs.size() == 0) return;
    this->m_CurrentStorage = context->GetService<mitk::NavigationToolStorage>(refs.front());
  }