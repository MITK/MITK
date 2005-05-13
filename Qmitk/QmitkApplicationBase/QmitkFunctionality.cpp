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


#include "QmitkFunctionality.h"

#include <qlcdnumber.h>
#include <qslider.h>
#include <itkCommand.h>


QmitkFunctionality::QmitkFunctionality(QObject *parent, const char *name, mitk::DataTreeIteratorBase* dataIt) : 
QObject(parent, name), m_Available(false), m_Activated(false), m_DataTreeIterator(NULL), m_TreeChangedWhileInActive(false), m_ObserverTag(0)
{
  SetDataTree(dataIt);
}

QmitkFunctionality::~QmitkFunctionality()
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
}

QString QmitkFunctionality::GetFunctionalityName()
{
  return name();
}

void QmitkFunctionality::Activated()
{
  m_Activated = true;
  if(m_TreeChangedWhileInActive)
  {
    TreeChanged();
    m_TreeChangedWhileInActive = false;
  }
}

void QmitkFunctionality::Deactivated()
{
  m_Activated = false;
}

bool QmitkFunctionality::IsActivated()
{
  return m_Activated;
}

bool QmitkFunctionality::IsAvailable()
{
  return m_Available;
}

void QmitkFunctionality::SetAvailability(bool available)
{
  this->m_Available=available;
  emit AvailabilityChanged(this);
  emit AvailabilityChanged();
}

void QmitkFunctionality::SetDataTree(mitk::DataTreeIteratorBase* it)
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
  m_DataTreeIterator = it;
  if(m_DataTreeIterator.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkFunctionality>::Pointer command = itk::ReceptorMemberCommand<QmitkFunctionality>::New();
    command->SetCallbackFunction(this, &QmitkFunctionality::TreeChanged);
    m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  }
}

mitk::DataTreeIteratorBase* QmitkFunctionality::GetDataTree()
{
  return m_DataTreeIterator.GetPointer();
}

void QmitkFunctionality::TreeChanged(const itk::EventObject & treeChangedEvent)
{
  if(IsActivated())
  {
    m_TreeChangedWhileInActive = false;
    TreeChanged();
  }
  else
    m_TreeChangedWhileInActive = true;
}

void QmitkFunctionality::TreeChanged()
{
}
