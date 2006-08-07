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


QmitkBaseFunctionalityComponent::QmitkBaseFunctionalityComponent(const char *name, mitk::DataTreeIteratorBase* dataIt )
{
  SetDataTreeIterator(dataIt);
}


QmitkBaseFunctionalityComponent::~QmitkBaseFunctionalityComponent()
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
}

void QmitkBaseFunctionalityComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
  m_DataTreeIterator = it;
  if(m_DataTreeIterator.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkBaseFunctionalityComponent>::Pointer command = itk::ReceptorMemberCommand<QmitkBaseFunctionalityComponent>::New();
    command->SetCallbackFunction(this, &QmitkBaseFunctionalityComponent::TreeChanged);
    m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  }
}
//QWidget* QmitkBaseFunctionalityComponent::CreateContainerWidget(QWidget* parent)
//{
// return parent;
//}

mitk::DataTreeIteratorBase* QmitkBaseFunctionalityComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

QString QmitkBaseFunctionalityComponent::GetFunctionalityName()
{
  return m_Name;
}

void QmitkBaseFunctionalityComponent::SetFunctionalityName(QString name)
{
  m_Name = name;
}

void QmitkBaseFunctionalityComponent::Activated()
{
  m_Activated = true;
  if(m_TreeChangedWhileInActive)
  {
    TreeChanged();
    m_TreeChangedWhileInActive = false;
  }
}

void QmitkBaseFunctionalityComponent::Deactivated()
{
  m_Activated = false;
}

bool QmitkBaseFunctionalityComponent::IsActivated()
{
  return m_Activated;
}

bool QmitkBaseFunctionalityComponent::IsAvailable()
{
  return m_Available;
}

void QmitkBaseFunctionalityComponent::SetAvailability(bool available)
{
  this->m_Available=available;
  //emit AvailabilityChanged(this); TODO
  //emit AvailabilityChanged();
}

void QmitkBaseFunctionalityComponent::TreeChanged(const itk::EventObject & /*treeChangedEvent*/)
{
  if(IsActivated())
  {
    m_TreeChangedWhileInActive = false;
    //TreeChanged();
  }
  else
    m_TreeChangedWhileInActive = true;
}

void QmitkBaseFunctionalityComponent::TreeChanged()
{
  
}

void QmitkBaseFunctionalityComponent::TreeChanged(mitk::DataTreeIteratorBase* it)
{

}

QWidget* QmitkBaseFunctionalityComponent::GetGUI()
{
  return NULL;
}
