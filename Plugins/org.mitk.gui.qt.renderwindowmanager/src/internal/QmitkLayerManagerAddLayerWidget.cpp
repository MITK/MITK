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

#include "QmitkLayerManagerAddLayerWidget.h"

#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>

const QString QmitkLayerManagerAddLayerWidget::VIEW_ID = "org.mitk.Widgets.QmitkLayerManagerAddLayer";

QmitkLayerManagerAddLayerWidget::QmitkLayerManagerAddLayerWidget(QWidget* parent /*= nullptr*/)
  : QWidget(parent, Qt::Window)
  , m_DataStorage(nullptr)
{
  Init();
}

QmitkLayerManagerAddLayerWidget::~QmitkLayerManagerAddLayerWidget()
{
  // nothing here
}

void QmitkLayerManagerAddLayerWidget::Init()
{
  m_Controls.setupUi(this);
  m_LayerListModel = new QStringListModel(this);
  m_Controls.listViewLayers->setModel(m_LayerListModel);
  m_Controls.listViewLayers->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(m_Controls.pushButtonAddLayer, SIGNAL(clicked()), this, SLOT(AddLayerToRenderer()));
  connect(m_Controls.pushButtonAddLayer, SIGNAL(clicked()), this, SLOT(hide()));

  m_Controls.pushButtonAddLayer->setEnabled(false);
}

void QmitkLayerManagerAddLayerWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkLayerManagerAddLayerWidget::ListLayer()
{
  if (m_DataStorage.IsNotNull())
  {
    // get the data nodes from the data storage that are not helper objects
    mitk::NodePredicateProperty::Pointer helperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer notAHelperObject = mitk::NodePredicateNot::New(helperObject);
    mitk::DataStorage::SetOfObjects::ConstPointer filteredDataNodes = m_DataStorage->GetSubset(notAHelperObject);

    QStringList stringList;
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = filteredDataNodes->Begin(); it != filteredDataNodes->End(); ++it)
    {
      stringList << QString::fromStdString(it->Value()->GetName());
    }

    m_LayerListModel->setStringList(stringList);

    if (stringList.isEmpty())
    {
      m_Controls.pushButtonAddLayer->setEnabled(false);
    }
    else
    {
      m_Controls.pushButtonAddLayer->setEnabled(true);
    }
  }
}

void QmitkLayerManagerAddLayerWidget::AddLayerToRenderer()
{  
  QModelIndex selectedIndex = m_Controls.listViewLayers->currentIndex();
  if (selectedIndex.isValid())
  {
    int listRow = selectedIndex.row();
    QString dataNodeName = m_LayerListModel->stringList().at(listRow);
    mitk::DataNode* dataNode = m_DataStorage->GetNamedNode(dataNodeName.toStdString());

    emit LayerToAddSelected(dataNode);
  }
}
