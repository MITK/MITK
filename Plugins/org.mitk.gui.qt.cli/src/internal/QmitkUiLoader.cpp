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
 
#include "QmitkUiLoader.h"
#include "QmitkDataStorageComboBox.h"

QmitkUiLoader::QmitkUiLoader(QObject *parent)
  : QUiLoader(parent)
{

}


QmitkUiLoader::~QmitkUiLoader()
{

}

QStringList QmitkUiLoader::availableWidgets () const
{
  QStringList availableWidgets = QUiLoader::availableWidgets();
  availableWidgets << "QmitkDataStorageComboBox";
  return availableWidgets;
}

QWidget* QmitkUiLoader::createWidget(const QString& className, QWidget* parent, const QString& name)
{
  QWidget* widget = QUiLoader::createWidget(className, parent, name);
  if (widget == NULL)
  {
    if (className == "QmitkDataStorageComboBox")
    {
      QmitkDataStorageComboBox* comboBox = new QmitkDataStorageComboBox(parent);
      comboBox->setObjectName(name);
      widget = comboBox;
    }
  }
  return widget;
}



