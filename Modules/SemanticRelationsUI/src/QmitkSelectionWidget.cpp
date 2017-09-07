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

// semantic relations UI module
#include "QmitkSelectionWidget.h"

QmitkSelectionWidget::QmitkSelectionWidget(mitk::DataStorage* dataStorage, QWidget* parent /*=nullptr*/)
  : QWidget(parent)
  , m_DataStorage(dataStorage)
{
  m_SemanticRelations = std::make_shared<mitk::SemanticRelations>(m_DataStorage);
}

QmitkSelectionWidget::~QmitkSelectionWidget()
{
  // nothing here
}

void QmitkSelectionWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
    m_SemanticRelations = std::make_shared<mitk::SemanticRelations>(m_DataStorage);
  }
}
