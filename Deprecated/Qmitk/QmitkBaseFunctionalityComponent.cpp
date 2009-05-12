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
#include "QmitkBaseFunctionalityComponent.h"
#include <itkCommand.h>

#include <QmitkDataTreeComboBox.h>
#include <mitkDataTreeFilter.h>

/***************       CONSTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::QmitkBaseFunctionalityComponent(QObject *parent, const char *name, mitk::DataTreeIteratorBase* dataIt )
: QmitkBaseComponent(parent, name),
m_ParentMitkImage(NULL),
m_MitkImageIterator(NULL)
{
  SetDataTreeIterator(dataIt);
}

/***************        DESTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::~QmitkBaseFunctionalityComponent()
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
