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
#include <usModuleContext.h>
#include <usServiceProperties.h>

#include <mitkCommon.h>

const std::string QmitkServiceListWidget::VIEW_ID = "org.mitk.views.QmitkServiceListWidget";

QmitkServiceListWidget::QmitkServiceListWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_AutomaticallySelectFirstEntry(false),
    m_Controls(NULL)
{
  CreateQtPartControl(this);
}

QmitkServiceListWidget::~QmitkServiceListWidget()
{
  m_Context->RemoveServiceListener(this,  &QmitkServiceListWidget::OnServiceEvent);
}

void QmitkServiceListWidget::SetAutomaticallySelectFirstEntry(bool automaticallySelectFirstEntry)
{
  m_AutomaticallySelectFirstEntry = automaticallySelectFirstEntry;
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
  m_Context = us::GetModuleContext();
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
    m_Filter = "(" + us::ServiceConstants::OBJECTCLASS() + "=" + m_Interface + ")";
  else
    m_Filter = filter;
  m_NamingProperty = namingProperty;
  m_Context->RemoveServiceListener(this,  &QmitkServiceListWidget::OnServiceEvent);
  m_Context->AddServiceListener(this, &QmitkServiceListWidget::OnServiceEvent, m_Filter);
    // Empty ListWidget
  this->m_ListContent.clear();
  m_Controls->m_ServiceList->clear();

  // get Services
  std::vector<us::ServiceReferenceU> services = this->GetAllRegisteredServices();
  // Transfer them to the List
  for(std::vector<us::ServiceReferenceU>::iterator it = services.begin(); it != services.end(); ++it)
    AddServiceToList(*it);
}

///////////// Methods & Slots Handling Direct Interaction /////////////////

bool QmitkServiceListWidget::GetIsServiceSelected(){
  return (this->m_Controls->m_ServiceList->currentItem() != 0);
}

void QmitkServiceListWidget::OnServiceSelectionChanged(){
  us::ServiceReferenceU ref = this->GetServiceForListItem(this->m_Controls->m_ServiceList->currentItem());
  if (! ref){
    emit (ServiceSelectionChanged(us::ServiceReferenceU()));
    return;
  }

  emit (ServiceSelectionChanged(ref));
}

us::ServiceReferenceU QmitkServiceListWidget::GetSelectedServiceReference(){
  return this->GetServiceForListItem(this->m_Controls->m_ServiceList->currentItem());
}

std::vector<us::ServiceReferenceU>  QmitkServiceListWidget::GetAllServiceReferences()
{
  std::vector<us::ServiceReferenceU> result;
  for (int i = 0; i < m_ListContent.size(); i++){
    result.push_back(m_ListContent[i].service);
  }
  return result;
}

///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkServiceListWidget::OnServiceEvent(const us::ServiceEvent event){
  //MITK_INFO << "ServiceEvent" << event.GetType();
  switch (event.GetType())
  {
    case us::ServiceEvent::MODIFIED:
      emit(ServiceModified(event.GetServiceReference()));

      // Change service; add a new entry if service wasn't on list before
      if ( ! this->ChangeServiceOnList(event.GetServiceReference()) )
      {
        this->AddServiceToList(event.GetServiceReference());
      }
      break;
    case us::ServiceEvent::REGISTERED:
      emit(ServiceRegistered(event.GetServiceReference()));
      AddServiceToList(event.GetServiceReference());
      break;
    case us::ServiceEvent::UNREGISTERING:
      emit(ServiceUnregistering(event.GetServiceReference()));
      RemoveServiceFromList(event.GetServiceReference());
      break;
    case us::ServiceEvent::MODIFIED_ENDMATCH:
      emit(ServiceModifiedEndMatch(event.GetServiceReference()));
      RemoveServiceFromList(event.GetServiceReference());
      break;
  }
}

/////////////////////// HOUSEHOLDING CODE /////////////////////////////////

QListWidgetItem* QmitkServiceListWidget::AddServiceToList(const us::ServiceReferenceU& serviceRef){
  QListWidgetItem *newItem = new QListWidgetItem;

  newItem->setText(this->CreateCaptionForService(serviceRef));

  // Add new item to QListWidget
  m_Controls->m_ServiceList->addItem(newItem);
  m_Controls->m_ServiceList->sortItems();

  // Construct link and add to internal List for reference
  QmitkServiceListWidget::ServiceListLink link;
  link.service = serviceRef;
  link.item = newItem;
  m_ListContent.push_back(link);

  // Select first entry and emit corresponding signal if the list was
  // empty before and this feature is enabled
  if ( m_AutomaticallySelectFirstEntry && m_Controls->m_ServiceList->selectedItems().isEmpty() )
  {
    m_Controls->m_ServiceList->setCurrentIndex(m_Controls->m_ServiceList->indexAt(QPoint(0, 0)));
    this->OnServiceSelectionChanged();
  }

  return newItem;
}

bool QmitkServiceListWidget::RemoveServiceFromList(const us::ServiceReferenceU& serviceRef){
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

bool QmitkServiceListWidget::ChangeServiceOnList(const us::ServiceReferenceU& serviceRef)
{
  for(std::vector<QmitkServiceListWidget::ServiceListLink>::iterator it = m_ListContent.begin(); it != m_ListContent.end(); ++it){
    if ( serviceRef == it->service )
    {
      it->item->setText(this->CreateCaptionForService(serviceRef));
      return true;
    }
  }
  return false;
}

us::ServiceReferenceU QmitkServiceListWidget::GetServiceForListItem(QListWidgetItem* item)
{
  for(std::vector<QmitkServiceListWidget::ServiceListLink>::iterator it = m_ListContent.begin(); it != m_ListContent.end(); ++it)
    if (item == it->item) return it->service;
  // Return invalid ServiceReference (will evaluate to false in bool expressions)
  return us::ServiceReferenceU();
}

std::vector<us::ServiceReferenceU> QmitkServiceListWidget::GetAllRegisteredServices(){
  //Get Service References
  return m_Context->GetServiceReferences(m_Interface, m_Filter);
}

QString QmitkServiceListWidget::CreateCaptionForService(const us::ServiceReferenceU& serviceRef)
{
  std::string caption;
  //TODO allow more complex formatting
  if (m_NamingProperty.empty())
    caption = m_Interface;
  else
  {
    us::Any prop = serviceRef.GetProperty(m_NamingProperty);
    if (prop.Empty())
    {
      MITK_WARN << "QmitkServiceListWidget tried to resolve property '" + m_NamingProperty + "' but failed. Resorting to interface name for display.";
      caption = m_Interface;
    }
    else
      caption = prop.ToString();
  }

  return QString::fromStdString(caption);
}
