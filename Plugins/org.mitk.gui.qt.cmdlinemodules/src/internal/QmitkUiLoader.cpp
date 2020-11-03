/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUiLoader.h"
#include "QmitkDataStorageComboBoxWithSelectNone.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateOr.h"
#include "mitkImage.h"

//-----------------------------------------------------------------------------
QmitkUiLoader::QmitkUiLoader(const mitk::DataStorage* dataStorage, QObject *parent)
  : ctkCmdLineModuleQtUiLoader(parent)
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
  QStringList availableWidgets = ctkCmdLineModuleQtUiLoader::availableWidgets();
  availableWidgets << "QmitkDataStorageComboBoxWithSelectNone";
  return availableWidgets;
}


//-----------------------------------------------------------------------------
QWidget* QmitkUiLoader::createWidget(const QString& className, QWidget* parent, const QString& name)
{
  QWidget* widget = nullptr;
  if (className == "QmitkDataStorageComboBoxWithSelectNone")
  {
    auto   comboBox = new QmitkDataStorageComboBoxWithSelectNone(parent);
    comboBox->setObjectName(name);
    comboBox->SetAutoSelectNewItems(false);
    comboBox->SetPredicate(mitk::TNodePredicateDataType< mitk::Image >::New());
    comboBox->SetDataStorage(const_cast<mitk::DataStorage*>(m_DataStorage));
    comboBox->setCurrentIndex(0);
    widget = comboBox;
  }
  else
  {
    widget = ctkCmdLineModuleQtUiLoader::createWidget(className, parent, name);
  }
  return widget;
}
