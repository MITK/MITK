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
#include "QmitkEmptyComponent.h"
#include "QmitkEmptyComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkEmptyComponent::QmitkEmptyComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * /*mitkStdMultiWidget*/, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_EmptyImageNode(NULL),
m_EmptyComponentGUI(NULL),
m_EmptyNodeExisting(false)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("Empty");
  m_Node = it->Get();
}

/***************        DESTRUCTOR      ***************/
QmitkEmptyComponent::~QmitkEmptyComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkEmptyComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
  m_Node = m_DataTreeIterator->Get();
}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkEmptyComponent::SetSelectorVisibility(bool visibility)
{
  if(m_EmptyComponentGUI)
  {
    m_EmptyComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
  }
}

 /***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkEmptyComponent::GetImageContent()
{
  return (QGroupBox*) m_EmptyComponentGUI->GetImageContent();
}


/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkEmptyComponent::GetTreeNodeSelector()
{
  return m_EmptyComponentGUI->GetTreeNodeSelector();
}

/***************       CONNECTIONS      ***************/
void QmitkEmptyComponent::CreateConnections()
{
  if ( m_EmptyComponentGUI )
  {
    connect( (QObject*)(m_EmptyComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect( (QObject*)(m_EmptyComponentGUI->GetEmptyFinderGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowEmptyContent(bool)));     
    connect( (QObject*)(m_EmptyComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool))); 
    connect( (QObject*)(m_EmptyComponentGUI->GetEmptyFinderGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkEmptyComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_EmptyComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_EmptyComponentGUI->GetTreeNodeSelector()->GetFilter())
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
  if(m_EmptyComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);
    }
  }
  m_Node = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());
  DataObjectSelected();
}

/***************  DATA OBJECT SELECTED   **************/
void QmitkEmptyComponent::DataObjectSelected()
{
  if(m_Active)
  {
    if(m_EmptyNodeExisting)
    {
      m_EmptyImageNode->SetData(m_Node->GetData());
    }
    else
    {
      CreateEmptyImageNode();
      m_EmptyImageNode->SetData(m_Node->GetData());
    }
  }
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkEmptyComponent::CreateControlWidget(QWidget* parent)
{
  m_EmptyComponentGUI = new QmitkEmptyComponentGUI(parent);
  m_GUI = m_EmptyComponentGUI;

  m_EmptyComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

 	if(m_ShowSelector)
	{
		m_EmptyComponentGUI->GetImageContent()->setShown(m_EmptyComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_EmptyComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}

  return m_EmptyComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkEmptyComponent::GetContentContainer()
{
 return m_EmptyComponentGUI->GetContainerContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkEmptyComponent::GetMainCheckBoxContainer()
{
 return m_EmptyComponentGUI->GetEmptyFinderGroupBox();
}

///*********** SET CONTENT CONTAINER VISIBLE ************/
//void QmitkEmptyComponent::SetContentContainerVisibility()
//{
//     for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
//    {
//      if(m_AddedChildList[i]->GetContentContainer() != NULL)
//      {
//        m_AddedChildList[i]->GetContentContainer()->setShown(GetMainCheckBoxContainer()->isChecked());
//      }
//    } 
//}

/***************        ACTIVATED       ***************/
void QmitkEmptyComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  } 
  CreateEmptyImageNode();
}

/***************       DEACTIVATED      ***************/
void QmitkEmptyComponent::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 
  if(m_EmptyComponentGUI->GetDeleteImageIfDeactivatedCheckBox()->isChecked())
  {
    DeleteEmptyNode();
  }
}

///*************CREATE EMPTY IMAGE NODE************/
void QmitkEmptyComponent::CreateEmptyImageNode()
{ 
  if(m_Active)
  {
    if(!m_EmptyNodeExisting)
    {
      if (m_Node)
      {
        m_EmptyImageNode = mitk::DataTreeNode::New();
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty("emtpy image" );
        m_EmptyImageNode->SetProperty( "name", nameProp );
        mitk::BoolProperty::Pointer emptyComponentImageProp = new mitk::BoolProperty(true);
        m_EmptyImageNode->SetProperty( "isEmptyComponentImage", emptyComponentImageProp );

        m_EmptyImageNode->SetData(m_Node->GetData());
        m_EmptyImageNode->SetColor(0.0,1.0,0.0);
        m_EmptyImageNode->SetOpacity(.25);
        int layer = 0;
        m_Node->GetIntProperty("layer", layer);
        m_EmptyImageNode->SetIntProperty("layer", layer+1);
        mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
        iteratorClone->GoToBegin();
        while ( !iteratorClone->IsAtEnd() )
        {
          mitk::DataTreeNode::Pointer node = iteratorClone->Get();
          if (  node == m_Node )
          {
            iteratorClone->Add(m_EmptyImageNode);
          }
          ++iteratorClone;
          m_EmptyNodeExisting = true;
        }
      }
    }
  }
}

///************ SHOW EMPTY CONTENT ***********/
void QmitkEmptyComponent::ShowEmptyContent(bool)
{
	m_EmptyComponentGUI->GetContainerContent()->setShown(m_EmptyComponentGUI->GetEmptyFinderGroupBox()->isChecked());
  
  if(m_ShowSelector)
  {
    m_EmptyComponentGUI->GetSelectDataGroupBox()->setShown(m_EmptyComponentGUI->GetEmptyFinderGroupBox()->isChecked());
  }
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkEmptyComponent::ShowImageContent(bool)
{
  m_EmptyComponentGUI->GetImageContent()->setShown(m_EmptyComponentGUI->GetSelectDataGroupBox()->isChecked());

  	if(m_ShowSelector)
	{
		m_EmptyComponentGUI->GetImageContent()->setShown(m_EmptyComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_EmptyComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}
}


///***************  DELETE EMPTYD NODE  **************/
void QmitkEmptyComponent::DeleteEmptyNode()
{
  if(m_EmptyImageNode)
  {
    mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();

      std::string name;
      node->GetName(name);

      if(name == "empty image")
      {
        iteratorClone->Disconnect();
        m_EmptyNodeExisting = false;
      }
      ++iteratorClone;
    }
  }
}
