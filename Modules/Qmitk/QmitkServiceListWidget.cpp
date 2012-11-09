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

// STL Headers
#include <list>

//microservices
#include <usGetModuleContext.h>
#include <mitkModuleContext.h>
#include <usServiceProperties.h>

#include <mitkCommon.h>


const std::string QmitkServiceListWidget::VIEW_ID = "org.mitk.views.QmitkServiceListWidget";

QmitkServiceListWidget::QmitkServiceListWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkServiceListWidget::~QmitkServiceListWidget()
{
  m_Context->RemoveServiceListener(this,  &QmitkServiceListWidget::OnServiceEvent);
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
  m_Context = mitk::GetModuleContext();
}

void QmitkServiceListWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_ServiceList, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *)), this, SLOT(OnServiceSelectionChanged()) );
  }
}

void QmitkServiceListWidget::InitPrivate(const std::string& namingProperty, const std::string& filter)
{
  if (filter.empty())
    m_Filter = "(" + mitk::ServiceConstants::OBJECTCLASS() + "=" + m_Interface + ")";
  else
    m_Filter = filter;
  m_NamingProperty = namingProperty;
  m_Context->RemoveServiceListener(this,  &QmitkServiceListWidget::OnServiceEvent);
  m_Context->AddServiceListener(this, &QmitkServiceListWidget::OnServiceEvent, m_Filter);
    // Empty ListWidget
  this->m_ListContent.clear();
  m_Controls->m_ServiceList->clear();

  // get Services
  std::list<mitk::ServiceReference> services = this->GetAllRegisteredServices();
  // Transfer them to the List
  for(std::list<mitk::ServiceReference>::iterator it = services.begin(); it != services.end(); ++it)
    AddServiceToList(*it);
}

///////////// Methods & Slots Handling Direct Interaction /////////////////

bool QmitkServiceListWidget::GetIsServiceSelected(){
  return (this->m_Controls->m_ServiceList->currentItem() != 0);
}

void QmitkServiceListWidget::OnServiceSelectionChanged(){
  mitk::ServiceReference ref = this->GetServiceForListItem(this->m_Controls->m_ServiceList->currentItem());
  if (! ref){
    emit (ServiceSelectionChanged(mitk::ServiceReference()));
    return;
  }

  emit (ServiceSelectionChanged(ref));
}

mitk::ServiceReference QmitkServiceListWidget::GetSelectedServiceReference(){
  return this->GetServiceForListItem(this->m_Controls->m_ServiceList->currentItem());
}


///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkServiceListWidget::OnServiceEvent(const mitk::ServiceEvent event){

  //MITK_INFO << "ServiceEvent" << event.GetType();
  switch (event.GetType())
  {
    case mitk::ServiceEvent::MODIFIED:
      emit(ServiceModified(event.GetServiceReference()));
      RemoveServiceFromList(event.GetServiceReference());
      AddServiceToList(event.GetServiceReference());
      break;
    case mitk::ServiceEvent::REGISTERED:
      emit(ServiceRegistered(event.GetServiceReference()));
      AddServiceToList(event.GetServiceReference());
      break;
    case mitk::ServiceEvent::UNREGISTERING:
      emit(ServiceUnregistering(event.GetServiceReference()));
      RemoveServiceFromList(event.GetServiceReference());
      break;
    case mitk::ServiceEvent::MODIFIED_ENDMATCH:
      emit(ServiceModifiedEndMatch(event.GetServiceReference()));
      RemoveServiceFromList(event.GetServiceReference());
      break;
  }
}


/////////////////////// HOUSEHOLDING CODE /////////////////////////////////

QListWidgetItem* QmitkServiceListWidget::AddServiceToList(mitk::ServiceReference serviceRef){
  QListWidgetItem *newItem = new QListWidgetItem;
  std::string caption;
  //TODO allow more complex formatting
  if (m_NamingProperty.empty())
    caption = m_Interface;
  else
  {
    mitk::Any prop = serviceRef.GetProperty(m_NamingProperty);
    if (prop.Empty())
    {
      MITK_WARN << "QmitkServiceListWidget tried to resolve property '" + m_NamingProperty + "' but failed. Resorting to interface name for display.";
      caption = m_Interface;
    }
    else
      caption = prop.ToString();
  }

  newItem->setText(caption.c_str());

  // Add new item to QListWidget
  m_Controls->m_ServiceList->addItem(newItem);
  m_Controls->m_ServiceList->sortItems();
  // Construct link and add to internal List for reference
  QmitkServiceListWidget::ServiceListLink link;
  link.service = serviceRef;
  link.item = newItem;
  m_ListContent.push_back(link);
  return newItem;
}

bool QmitkServiceListWidget::RemoveServiceFromList(mitk::ServiceReference serviceRef){
  for(std::vector<QmitkServiceListWidget::ServiceListLink>::iterator it = m_ListContent.begin(); it != m_ListContent.end(); ++it){
    if ( serviceRef == it->service )
    {
      int row = m_Controls->m_ServiceList->row(it->item);
      QListWidgetItem* oldItem = m_Controls->m_ServiceList->takeItem(row);
      delete oldItem;
      this->m_ListContent.erase(it);
      return true;
    }
  }
  return false;
}


mitk::ServiceReference QmitkServiceListWidget::GetServiceForListItem(QListWidgetItem* item)
{
  for(std::vector<QmitkServiceListWidget::ServiceListLink>::iterator it = m_ListContent.begin(); it != m_ListContent.end(); ++it)
    if (item == it->item) return it->service;
  // Return invalid ServiceReference (will evaluate to false in bool expressions)
  return mitk::ServiceReference();
}


std::list<mitk::ServiceReference> QmitkServiceListWidget::GetAllRegisteredServices(){
  //Get Service References
  return m_Context->GetServiceReferences(m_Interface, m_Filter);
}