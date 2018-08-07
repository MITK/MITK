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
#include <usGetModuleContext.h>
#include <usServiceReference.h>



QmitkNavigationDataSourceSelectionWidget::QmitkNavigationDataSourceSelectionWidget(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
  m_Controls = nullptr;
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
    m_Controls->m_NavigationDataSourceWidget->Initialize<mitk::NavigationDataSource>(mitk::NavigationDataSource::US_PROPKEY_DEVICENAME,empty);

  }
}

void QmitkNavigationDataSourceSelectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_NavigationDataSourceWidget), SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(NavigationDataSourceSelected(us::ServiceReferenceU)) );
    connect((QObject*)(m_Controls->m_ToolView), SIGNAL(currentRowChanged(int)), this, SLOT(NavigationToolSelected(int)));

  }
}

void QmitkNavigationDataSourceSelectionWidget::NavigationToolSelected(int selection)
{
  emit NavigationToolSelected(this->m_CurrentStorage->GetTool(selection));
}

void QmitkNavigationDataSourceSelectionWidget::NavigationDataSourceSelected(us::ServiceReferenceU s)
  {
    if (!s) //no device selected
      {
        //reset everything
        m_CurrentSource = nullptr;
        m_CurrentStorage = nullptr;
        emit NavigationDataSourceSelected(m_CurrentSource);
        return;
      }

    // Get Source
    us::ModuleContext* context = us::GetModuleContext();
    m_CurrentSource = context->GetService<mitk::NavigationDataSource>(s);

    // clear tool list before filling it
    m_Controls->m_ToolView->clear();
    //Fill tool list
    MITK_INFO<<"no outputs: "<<m_CurrentSource->GetNumberOfOutputs();
    for(std::size_t i = 0; i < m_CurrentSource->GetNumberOfOutputs(); i++)
    {
      new QListWidgetItem(tr(m_CurrentSource->GetOutput(i)->GetName()), m_Controls->m_ToolView);
    }

    // Get corresponding tool storage
    m_CurrentStorage = m_CurrentSource->GetToolMetaDataCollection();

    if (m_CurrentStorage.IsNull())
      {
      MITK_WARN << "Found an invalid storage object!";
      return;
      }
    if (m_CurrentStorage->GetToolCount() != m_CurrentSource->GetNumberOfOutputs()) //there is something wrong with the storage
      {
      MITK_WARN << "Found a tool storage, but it has not the same number of tools like the NavigationDataSource. This storage won't be used because it isn't the right one.";
      m_CurrentStorage = nullptr;
      }

    emit NavigationDataSourceSelected(m_CurrentSource);
  }

mitk::NavigationDataSource::Pointer QmitkNavigationDataSourceSelectionWidget::GetSelectedNavigationDataSource()
  {
  return this->m_CurrentSource;
  }


int QmitkNavigationDataSourceSelectionWidget::GetSelectedToolID()
  {
    return this->m_Controls->m_ToolView->currentIndex().row();
  }


mitk::NavigationTool::Pointer QmitkNavigationDataSourceSelectionWidget::GetSelectedNavigationTool()
  {
    if (this->m_CurrentStorage.IsNull()) return nullptr;
    if ((m_Controls->m_ToolView->currentIndex().row() < 0) || (static_cast<unsigned int>(m_Controls->m_ToolView->currentIndex().row()) >= m_CurrentStorage->GetToolCount())) return nullptr;
    return this->m_CurrentStorage->GetTool(m_Controls->m_ToolView->currentIndex().row());
  }


mitk::NavigationToolStorage::Pointer QmitkNavigationDataSourceSelectionWidget::GetNavigationToolStorageOfSource()
  {
    return this->m_CurrentStorage;
  }
