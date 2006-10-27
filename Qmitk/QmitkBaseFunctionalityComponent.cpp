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
m_Parent(parent),
m_FuncName(name)
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
QWidget* QmitkBaseFunctionalityComponent::CreateControlWidget(QWidget* parent)
{
  return (QWidget*) NULL;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkBaseFunctionalityComponent::TreeChanged()
{
}

void QmitkBaseFunctionalityComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
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



  QObject* QmitkBaseFunctionalityComponent::GetParent()
  {
   return m_Parent;
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



