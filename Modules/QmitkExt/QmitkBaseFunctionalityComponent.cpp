/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "QmitkBaseFunctionalityComponent.h"
#include <itkCommand.h>

#include <QmitkDataStorageComboBox.h>
#include <mitkDataNode.h>

/***************       CONSTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::QmitkBaseFunctionalityComponent(QObject *parent, const char *name)
: QmitkBaseComponent(parent, name),
m_ParentMitkImage(NULL)
{
}

/***************        DESTRUCTOR      ***************/
QmitkBaseFunctionalityComponent::~QmitkBaseFunctionalityComponent()
{
}

void QmitkBaseFunctionalityComponent::ImageSelected(const mitk::DataNode*  /*item*/)
{

}

void QmitkBaseFunctionalityComponent::DataStorageChanged(mitk::DataStorage::Pointer  /*ds*/)
{

}

mitk::DataStorage::Pointer QmitkBaseFunctionalityComponent::GetDefaultDataStorage()
{
  return m_DataStorage;
}

