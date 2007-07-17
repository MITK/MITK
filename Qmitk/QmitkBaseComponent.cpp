/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-12 17:53:29 +0200 (Do, 12 Jul 2007) $
Version:   $Revision: 11256 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkBaseComponent.h"
#include <itkCommand.h>
#include <qgroupbox.h>

/***************       CONSTRUCTOR      ***************/
QmitkBaseComponent::QmitkBaseComponent(QObject *parent, const char *name)
: QObject(parent, name),
m_FuncName(name),
m_Parent(parent)
{
}

/***************        DESTRUCTOR      ***************/
QmitkBaseComponent::~QmitkBaseComponent()
{
}

/***************       CONNECTIONS      ***************/
void QmitkBaseComponent::CreateConnections()
{
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkBaseComponent::CreateControlWidget(QWidget* /*parent*/)
{
  return (QWidget*) NULL;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkBaseComponent::TreeChanged()
{
}


/*************** GET DATA TREE ITERATOR ***************/
QWidget*  QmitkBaseComponent::GetGUIControls()
{
  return (QWidget*) NULL;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkBaseComponent::SetFunctionalityName(QString name)
{
  m_FuncName = name;
}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkBaseComponent::GetFunctionalityName()
{
  return m_FuncName;
}

/***************       GET PARENT       ***************/
QObject* QmitkBaseComponent::GetParent()
{
  return m_Parent;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox* QmitkBaseComponent::GetContentContainer()
{
  return (QGroupBox*) NULL;
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox* QmitkBaseComponent::GetMainCheckBoxContainer()
{
  return (QGroupBox*) NULL;
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
void QmitkBaseComponent::SetContentContainerVisibility(bool)
{
}

/***************        ACTIVATED       ***************/
void QmitkBaseComponent::Activated()
{
  m_Activated = true;
}

/***************       DEACTIVATED      ***************/
void QmitkBaseComponent::Deactivated()
{
  m_Activated = false;
}

/***************      IS ACTIVATED      ***************/
bool QmitkBaseComponent::IsActivated()
{
  return m_Activated;
}

/***************      IS AVAILABLE      ***************/
bool QmitkBaseComponent::IsAvailable()
{
  return m_Available;
}

/***************     SET AVAILABLE      ***************/
void QmitkBaseComponent::SetAvailability(bool available)
{
  this->m_Available = available;
}

/*************** ADD COMPONENT LISTENER ***************/
void QmitkBaseComponent::AddComponentListener(QmitkBaseComponent* component)
{  
  if(component!=NULL)
  {
    m_AddedChildList.push_back(component);
  }
}

/************* REMOVE COMPONENT LISTENER **************/
void QmitkBaseComponent::RemoveComponentListener(QmitkBaseComponent* component)
{  
  if(component!=NULL)
  {
    std::vector<QmitkBaseComponent*>::iterator it = m_AddedChildList.begin();   
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
