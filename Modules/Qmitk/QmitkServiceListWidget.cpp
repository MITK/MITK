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

//#define _USE_MATH_DEFINES
#include <QmitkServiceListWidget.h>

// STL HEaders
#include <list>

//QT headers
#include <QColor>

//microservices
#include <usGetModuleContext.h>
#include "mitkModuleContext.h"
#include <usServiceProperties.h>


const std::string QmitkServiceListWidget::VIEW_ID = "org.mitk.views.QmitkServiceListWidget";

QmitkServiceListWidget::QmitkServiceListWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkServiceListWidget::~QmitkServiceListWidget()
{

}


//////////////////// INITIALIZATION /////////////////////

void QmitkServiceListWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkServiceListWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkServiceListWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_ServiceList, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *)), this, SLOT(OnDeviceSelectionChanged()) );
  }
}

void QmitkServiceListWidget::Initialize(mitk::ModuleContext* context, std::string interfaceName, std::string filter)
{
  m_Context = context;
  m_Filter = filter;
  m_Interface = interfaceName;
  m_Context->AddServiceListener(this, &QmitkServiceListWidget::OnServiceEvent, m_Filter);
}


///////////////////////// Getter & Setter /////////////////////////////////

template <class T>
T QmitkServiceListWidget::GetSelectedService()
{
  return this->GetDeviceForListItem(this->m_Controls->m_ServiceList->currentItem());
}

///////////// Methods & Slots Handling Direct Interaction /////////////////


void QmitkServiceListWidget::OnServiceSelectionChanged(){
  mitk::ServiceReference* ref = this->GetServiceForListItem(this->m_Controls->m_ServiceList->currentItem());
  if (ref == 0) return;

  emit (ServiceSelected(ref));
}


///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkServiceListWidget::OnServiceEvent(const mitk::ServiceEvent event){
  // Empty ListWidget
  this->m_ListContent.clear();
  m_Controls->m_ServiceList->clear();

  // get Services
  std::list<mitk::ServiceReference> services = this->GetAllRegisteredServices();
  // Transfer them to the List
  for(std::list<mitk::ServiceReference>::iterator it = services.begin(); it != services.end(); ++it)
  {
    QListWidgetItem *newItem = ConstructItemFromService(& *it);
    //Add new item to QListWidget
    m_Controls->m_ServiceList->addItem(newItem);
    // Construct link and add to internal List for reference
    QmitkServiceListWidget::ServiceListLink link;
    link.service = &*it;
    link.item = newItem;
    m_ListContent.push_back(link);
  }
}


/////////////////////// HOUSEHOLDING CODE /////////////////////////////////

QListWidgetItem* QmitkServiceListWidget::ConstructItemFromService(mitk::ServiceReference* serviceRef){
  QListWidgetItem *result = new QListWidgetItem;

  //TODO allow formatting
  std::string text = "ThisIsAService";

  result->setText(text.c_str());

  return result;
}


mitk::ServiceReference* QmitkServiceListWidget::GetServiceForListItem(QListWidgetItem* item)
{
  for(std::vector<QmitkServiceListWidget::ServiceListLink>::iterator it = m_ListContent.begin(); it != m_ListContent.end(); ++it)
  {
    if (item == it->item) return it->service;
  }
  return 0;
}


std::list<mitk::ServiceReference> QmitkServiceListWidget::GetAllRegisteredServices(){
  //Get Service References
  return m_Context->GetServiceReferences(m_Interface, m_Filter);
}
