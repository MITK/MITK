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


/***************       CONSTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::QmitkBaseFunctionalityComponent(const char *name, mitk::DataTreeIteratorBase* dataIt ):
m_FuncName(name)
{
  SetDataTreeIterator(dataIt);
  
}

/***************        DESTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::~QmitkBaseFunctionalityComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkBaseFunctionalityComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkBaseFunctionalityComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
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



