/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

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
  setDataTree(dataIt);
}

QmitkFunctionality::~QmitkFunctionality()
{

}

QString QmitkFunctionality::getFunctionalityName()
{
	return name();
}

void QmitkFunctionality::activated()
{
  m_Activated = true;
  if(m_TreeChangedWhileInActive)
  {
    treeChanged();
    m_TreeChangedWhileInActive = false;
  }
}

void QmitkFunctionality::deactivated()
{
  m_Activated = false;
}

bool QmitkFunctionality::isActivated()
{
	return m_Activated;
}

bool QmitkFunctionality::isAvailable()
{
  return m_Available;
}

void QmitkFunctionality::setAvailability(bool available)
{
	this->m_Available=available;
	emit availabilityChanged(this);
	emit availabilityChanged();
}

void QmitkFunctionality::setDataTree(mitk::DataTreeIteratorBase* it)
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
	m_DataTreeIterator = it;
  if(m_DataTreeIterator.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkFunctionality>::Pointer command = itk::ReceptorMemberCommand<QmitkFunctionality>::New();
    command->SetCallbackFunction(this, &QmitkFunctionality::treeChanged);
    m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  }
}

mitk::DataTreeIteratorBase* QmitkFunctionality::getDataTree()
{
	return m_DataTreeIterator.GetPointer();
}

void QmitkFunctionality::treeChanged(const itk::EventObject & treeChangedEvent)
{
  if(isActivated())
  {
    m_TreeChangedWhileInActive = false;
    treeChanged();
  }
  else
    m_TreeChangedWhileInActive = true;
}

void QmitkFunctionality::treeChanged()
{
}
