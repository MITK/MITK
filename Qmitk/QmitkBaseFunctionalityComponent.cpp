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
#include "QmitkBaseFunctionalityComponent.h"
#include <itkCommand.h>

#include <QmitkDataTreeComboBox.h>
#include <mitkDataTreeFilter.h>
//#include "mitkDataTreeFilterFunctions.h"

/***************       CONSTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::QmitkBaseFunctionalityComponent(QObject *parent, const char *name, mitk::DataTreeIteratorBase* dataIt )
: QObject(parent, name),
m_FuncName(name),
m_ParentMitkImage(NULL),
m_MitkImageIterator(NULL),
m_Parent(parent)
{
  SetDataTreeIterator(dataIt);
  
}

/***************        DESTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::~QmitkBaseFunctionalityComponent()
{

}

/***************       CONNECTIONS      ***************/
void QmitkBaseFunctionalityComponent::CreateConnections()
{

}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkBaseFunctionalityComponent::CreateControlWidget(QWidget* /*parent*/)
{
  return (QWidget*) NULL;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkBaseFunctionalityComponent::TreeChanged()
{
}

void QmitkBaseFunctionalityComponent::ImageSelected(const mitk::DataTreeFilter::Item * /*imageIt*/)
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkBaseFunctionalityComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

void QmitkBaseFunctionalityComponent::SetMitkImageIterator(mitk::DataTreeIteratorClone mitkImageIterator)
{
 m_MitkImageIterator = mitkImageIterator;
}

void QmitkBaseFunctionalityComponent::SetTreeIterator(mitk::DataTreeIteratorClone dataIt)
{
  m_DataTreeIterator = dataIt;
}
/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkBaseFunctionalityComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

/*************** GET DATA TREE ITERATOR ***************/
QWidget*  QmitkBaseFunctionalityComponent::GetGUIControls()
{
    return (QWidget*) NULL;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkBaseFunctionalityComponent::SetFunctionalityName(QString name)
{
  m_FuncName = name;
}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkBaseFunctionalityComponent::GetFunctionalityName()
{
  return m_FuncName;
}



/***************       GET PARENT       ***************/
  QObject* QmitkBaseFunctionalityComponent::GetParent()
  {
   return m_Parent;
  }

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkBaseFunctionalityComponent::GetContentContainer()
{
 return (QGroupBox*) NULL;
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkBaseFunctionalityComponent::GetMainCheckBoxContainer()
{
 return (QGroupBox*) NULL;
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
void QmitkBaseFunctionalityComponent::SetContentContainerVisibility(bool)
{

}

/***************        ACTIVATED       ***************/
void QmitkBaseFunctionalityComponent::Activated()
{
  m_Activated = true;
}

/***************       DEACTIVATED      ***************/
void QmitkBaseFunctionalityComponent::Deactivated()
{
  m_Activated = false;
}

/***************      IS ACTIVATED      ***************/
bool QmitkBaseFunctionalityComponent::IsActivated()
{
  return m_Activated;
}

/***************      IS AVAILABLE      ***************/
bool QmitkBaseFunctionalityComponent::IsAvailable()
{
  return m_Available;
}

/***************     SET AVAILABLE      ***************/
void QmitkBaseFunctionalityComponent::SetAvailability(bool available)
{
  this->m_Available = available;
}

/*************** ADD COMPONENT LISTENER ***************/
void QmitkBaseFunctionalityComponent::AddComponentListener(QmitkBaseFunctionalityComponent* component)
{  
  if(component!=NULL)
  {
    m_AddedChildList.push_back(component);
  }
}
 
/************* REMOVE COMPONENT LISTENER **************/
void QmitkBaseFunctionalityComponent::RemoveComponentListener(QmitkBaseFunctionalityComponent* component)
{  
  if(component!=NULL)
  {
    std::vector<QmitkBaseFunctionalityComponent*>::iterator it = m_AddedChildList.begin();   
    while (it != m_AddedChildList.end()) 
    {
      if(*it == component) 
      {
        m_AddedChildList.erase(it);
        break;
      }
      ++it; 
    }
  }
}
