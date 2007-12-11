/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkDataTreeFilterDemo.h"
#include "QmitkDataTreeFilterDemoControls.h"
#include "QmitkPropertyViewFactory.h"
#include "QmitkDataTreeComboBox.h"
#include "QmitkDataTreeListView.h"
#include "QmitkStdMultiWidget.h"

#include "icon.xpm"

#include "mitkDataTreeFilterFunctions.h"
#include "mitkSurface.h"

  QmitkDataTreeFilterDemo::QmitkDataTreeFilterDemo(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), 
  m_MultiWidget(mitkStdMultiWidget), 
  m_Controls(NULL), 
  m_FilterInitialized(false)
{
  SetAvailability(true);
}


QmitkDataTreeFilterDemo::~QmitkDataTreeFilterDemo()
{
  if(m_DataTreeFilter.IsNotNull())
    m_DataTreeFilter->RemoveObserver( m_ListboxModifiedConnection );
}


QWidget* QmitkDataTreeFilterDemo::CreateMainWidget(QWidget* /*parent*/)
{
  return NULL;
}


QWidget* QmitkDataTreeFilterDemo::CreateControlWidget(QWidget* parent)
{
  if (!m_Controls)
  {
    m_Controls = new QmitkDataTreeFilterDemoControls(parent);
  }
  return m_Controls;
}


void QmitkDataTreeFilterDemo::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->TreeComboBox, SIGNAL(activated(const mitk::DataTreeFilter::Item*)), 
                                 this, SLOT(onComboBoxItemSelected(const mitk::DataTreeFilter::Item*)) );
    
    connect( m_Controls->TreeListBox, SIGNAL(clicked(const mitk::DataTreeFilter::Item*, bool)), 
                                this, SLOT(onListboxItemClicked(const mitk::DataTreeFilter::Item*, bool)) );

    connect( m_Controls->TreeListBox, SIGNAL(newItem(const mitk::DataTreeFilter::Item*)), 
                                this, SLOT(onListboxItemAdded(const mitk::DataTreeFilter::Item*)) );

    connect( m_Controls->cmbFilterFunction, SIGNAL(activated(int)), 
                                      this, SLOT(onCmbFilterFunctionActivated(int)) );
    
    connect( (QObject*)(m_Controls->chkHierarchy), SIGNAL(toggled(bool)), 
                                 this, SLOT(onChkHierarchyToggled(bool)) );
    }
}


QAction* QmitkDataTreeFilterDemo::CreateAction(QActionGroup* parent)
{
  QAction* action;
    action = new QAction( tr( "Demo for mitk::DataTreeFilter" ), QPixmap((const char**)icon_xpm), tr( "DataTreeFilterDemo" ), 0, parent, "DataTreeFilterDemo" );
  return action;
}


void QmitkDataTreeFilterDemo::TreeChanged()
{
  m_Controls->TreeComboBox->Update(); // recreate contents. To update it all of the time, even when the functionality is deactivated, 
                                     //  call m_Controls->TreeComboBox->SetAutoUpdate(true) at some point
}


void QmitkDataTreeFilterDemo::Activated()
{
  QmitkFunctionality::Activated();
  
  if (!m_FilterInitialized)
  {
   // init the combobox (to show only images, which is also the default)
    m_Controls->TreeComboBox->SetDataTree( GetDataTreeIterator() );
    m_Controls->TreeComboBox->GetFilter()->SetFilter( mitk::IsBaseDataType<mitk::Image>() );  // this line could be skipped because this filter is the default at the moment
    
    // uncomment to get more output from the DataTreeFilter
    //m_Controls->TreeComboBox->GetFilter()->SetDebugOn();
  
    // define the list of segmentations
    m_DataTreeFilter = mitk::DataTreeFilter::New( GetDataTreeIterator()->GetTree() );
    m_DataTreeFilter->SetSelectionMode(mitk::DataTreeFilter::SINGLE_SELECT);
    m_DataTreeFilter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
    m_DataTreeFilter->SetFilter( mitk::IsGoodDataTreeNode() ); // show everything with data

    // define what is displayed about the segmentations
    mitk::DataTreeFilter::PropertyList visible_props;
    visible_props.push_back("visible");
    visible_props.push_back("color");
    visible_props.push_back("name");
    m_DataTreeFilter->SetVisibleProperties(visible_props);
    
    mitk::DataTreeFilter::PropertyList editable_props;
    editable_props.push_back("visible");
    editable_props.push_back("name");
    editable_props.push_back("color");
    m_DataTreeFilter->SetEditableProperties(editable_props);

    mitk::DataTreeFilter::PropertyList property_labels;
    property_labels.push_back(" ");
    property_labels.push_back(" ");
    property_labels.push_back("Name");
    m_DataTreeFilter->SetPropertiesLabels(property_labels);
    
    m_Controls->TreeListBox->SetViewType("name", QmitkPropertyViewFactory::etON_DEMAND_EDIT);
    m_Controls->TreeListBox->SetFilter( m_DataTreeFilter );
    
    // uncomment to get more output from the DataTreeFilter
    //m_DataTreeFilter->SetDebugOn();

    ConnectListboxNotification();

    m_FilterInitialized = true;
  }
}


void QmitkDataTreeFilterDemo::Deactivated()
{
  QmitkFunctionality::Deactivated();
}


void QmitkDataTreeFilterDemo::ConnectListboxNotification()
{
  itk::ReceptorMemberCommand<QmitkDataTreeFilterDemo>::Pointer command = itk::ReceptorMemberCommand<QmitkDataTreeFilterDemo>::New();
  command->SetCallbackFunction(this, &QmitkDataTreeFilterDemo::ListboxModifiedHandler);
  m_ListboxModifiedConnection = m_DataTreeFilter->AddObserver(itk::ModifiedEvent(), command);
}


void QmitkDataTreeFilterDemo::onComboBoxItemSelected(const mitk::DataTreeFilter::Item* item)
{
    std::cout << "(Combobox) Item " << item << " selected." << std::endl;
  if (item)
  {
    const mitk::DataTreeNode* node = item->GetNode();

    if (node && node->GetData())
    {
      // reinit multi-widget for the selected image
      m_MultiWidget->InitializeStandardViews( node->GetData()->GetTimeSlicedGeometry() ); 
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}


void QmitkDataTreeFilterDemo::MoveCrossHairToItem(const mitk::DataTreeFilter::Item* item)
{
  if (!item) return;

  // get tree node
  const mitk::DataTreeNode* node( item->GetNode() );
  if (!node) return;
  
  // determine center of node
  mitk::Point2D p2d;
  if ( node->GetData() &&
       node->GetData()->GetGeometry() )
  {
    mitk::Point3D center( node->GetData()->GetGeometry()->GetCenter() );
  
    // tell the multiwidget to move there
    m_MultiWidget->MoveCrossToPosition( center );
  }
}


void QmitkDataTreeFilterDemo::onListboxItemClicked(const mitk::DataTreeFilter::Item* item, bool selected)
{
  if (selected)
    std::cout << "(Listbox) Item " << item << " selected." << std::endl;
  else
    std::cout << "(Listbox) Item " << item << " deselected." << std::endl;
  
  if (!selected) return;
 
  MoveCrossHairToItem(item);
}


void QmitkDataTreeFilterDemo::onListboxItemAdded(const mitk::DataTreeFilter::Item* item)
{
  if ( !IsActivated() ) return; // don't do anything if functionality isn't activated

  MoveCrossHairToItem(item);
}


void QmitkDataTreeFilterDemo::ListboxModifiedHandler( const itk::EventObject& /*e*/ )
{
  // std::cout << "(Listbox) Something changed." << std::endl;
 
  //const mitk::DataTreeFilter::ItemList* items = m_DataTreeFilter->GetItems(); //in case you want to iterate over them
}


void QmitkDataTreeFilterDemo::onCmbFilterFunctionActivated(int i)
{
  switch (i)
  {
    case 0:
      m_DataTreeFilter->SetFilter( mitk::IsGoodDataTreeNode() );
      break;
    case 1:
      m_DataTreeFilter->SetFilter( mitk::IsBaseDataType<mitk::Image>() );
      break;
    case 2:
      m_DataTreeFilter->SetFilter( mitk::IsBaseDataType<mitk::Surface>() );
      break;
    case 3:
      m_DataTreeFilter->SetFilter( mitk::IsDataTreeNode() ); // probably not what the label promises
      break;
   }
}


void QmitkDataTreeFilterDemo::onChkHierarchyToggled(bool on)
{
  if (on)
    m_DataTreeFilter->SetHierarchyHandling( mitk::DataTreeFilter::PRESERVE_HIERARCHY );
  else
    m_DataTreeFilter->SetHierarchyHandling( mitk::DataTreeFilter::FLATTEN_HIERARCHY );
}
