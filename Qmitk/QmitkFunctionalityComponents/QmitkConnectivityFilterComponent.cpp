/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "QmitkConnectivityFilterComponent.h"
#include "QmitkConnectivityFilterComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include <QmitkPointListWidget.h>

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkConnectivityFilterComponent::QmitkConnectivityFilterComponent(QObject * parent, const char * parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector)
: QmitkInteractionFunctionalityComponent(parent, parentName, mitkStdMultiWidget, it),
m_ConnectivityFilterComponentGUI(NULL)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("PointSet");
}

/***************        DESTRUCTOR      ***************/
QmitkConnectivityFilterComponent::~QmitkConnectivityFilterComponent()
{

}

///*************** GET DATA TREE ITERATOR ***************/
//mitk::DataTreeIteratorBase* QmitkPixelGreyValueManipulatorComponent::GetDataTreeIterator()
//{
//  return m_DataTreeIterator.GetPointer();
//}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkConnectivityFilterComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkConnectivityFilterComponent::GetTreeNodeSelector()
{
  return m_ConnectivityFilterComponentGUI->GetTreeNodeSelector();
}

/***************   GET POINT SET NODE   ***************/
 mitk::DataTreeNode::Pointer QmitkConnectivityFilterComponent::GetPointSetNode()
 {
  return  m_ConnectivityNode;
 }

 /************ Update DATATREECOMBOBOX(ES) *************/
void QmitkConnectivityFilterComponent::UpdateDataTreeComboBoxes()
{
  if(GetTreeNodeSelector() != NULL)
  {
   GetTreeNodeSelector()->Update();
  }
}

/***************       CONNECTIONS      ***************/
void QmitkConnectivityFilterComponent::CreateConnections()
{
  if ( m_ConnectivityFilterComponentGUI )
  {
    connect( (QObject*)(m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect((QObject*)(m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowTreeNodeSelector()));
    connect((QObject*)(m_ConnectivityFilterComponentGUI->GetShowComponent()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowComponentContent()));

  }
}

/*************** SHOW COMPONENT CONTENT ***************/
void QmitkConnectivityFilterComponent::ShowComponentContent()
{
 m_ConnectivityFilterComponentGUI->GetComponentContent()->setShown(m_ConnectivityFilterComponentGUI->GetShowComponent()->isChecked());
}

/*************** SHOW TREE NODE SELECTOR **************/
void QmitkConnectivityFilterComponent::ShowTreeNodeSelector()
{
m_ConnectivityFilterComponentGUI->GetTreeNodeSelectorContentGroupBox()->setShown(m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
}

/***************     IMAGE SELECTED     ***************/
void QmitkConnectivityFilterComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_ConnectivityFilterComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter())
    {
      if(imageIt)
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }
  if(currentItem)
  {
    currentItem->SetSelected(true);
  }

  TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkConnectivityFilterComponent::CreateControlWidget(QWidget* parent)
{
  m_ConnectivityFilterComponentGUI = new QmitkConnectivityFilterComponentGUI(parent);
  m_GUI = m_ConnectivityFilterComponentGUI;

  m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  //m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithProperty<mitk::Image>("Points"));
  m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithProperty<mitk::PointSet>("SeedPoints"));

  return m_ConnectivityFilterComponentGUI;
}

/***************        ACTIVATED       ***************/
void QmitkConnectivityFilterComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
}

/***************       DEACTIVATED      ***************/
void QmitkConnectivityFilterComponent::Deactivated()
{

}


