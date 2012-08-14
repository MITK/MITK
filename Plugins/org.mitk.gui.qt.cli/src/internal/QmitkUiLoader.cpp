/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
 
#include "QmitkUiLoader.h"
#include "QmitkDataStorageComboBox.h"
#include "mitkNodePredicateDataType.h"

//-----------------------------------------------------------------------------
QmitkUiLoader::QmitkUiLoader(const mitk::DataStorage* dataStorage, QObject *parent)
  : QUiLoader(parent)
, m_DataStorage(dataStorage)
{

}


//-----------------------------------------------------------------------------
QmitkUiLoader::~QmitkUiLoader()
{

}


//-----------------------------------------------------------------------------
QStringList QmitkUiLoader::availableWidgets () const
{
  QStringList availableWidgets = QUiLoader::availableWidgets();
  availableWidgets << "QmitkDataStorageComboBox";
  return availableWidgets;
}


//-----------------------------------------------------------------------------
QWidget* QmitkUiLoader::createWidget(const QString& className, QWidget* parent, const QString& name)
{
  QWidget* widget = NULL;
  if (className == "QmitkDataStorageComboBox")
  {
    QmitkDataStorageComboBox* comboBox = new QmitkDataStorageComboBox(parent);
    comboBox->setObjectName(name);
    comboBox->SetDataStorage(const_cast<mitk::DataStorage*>(m_DataStorage));
    comboBox->SetPredicate(mitk::NodePredicateDataType::New("Image"));
    comboBox->SetAutoSelectNewItems(false);
    comboBox->addItem("please select");
    widget = comboBox;
  }
  else
  {
    widget = QUiLoader::createWidget(className, parent, name);
  }
  return widget;
}
