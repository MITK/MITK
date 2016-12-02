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

#include "QmitkLayerManagerAddNodeWidget.h"

#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>

const QString QmitkLayerManagerAddNodeWidget::VIEW_ID = "org.mitk.Widgets.QmitkLayerManagerAddNode";

QmitkLayerManagerAddNodeWidget::QmitkLayerManagerAddNodeWidget(QWidget* parent /*= nullptr*/)
  : QWidget(parent, Qt::Window)
  , m_DataStorage(nullptr)
{
  Init();
}

QmitkLayerManagerAddNodeWidget::~QmitkLayerManagerAddNodeWidget()
{
  // nothing here
}

void QmitkLayerManagerAddNodeWidget::Init()
{
  m_Controls.setupUi(this);
  m_DataNodeListModel = new QStringListModel(this);
  m_Controls.listViewDataNodes->setModel(m_DataNodeListModel);
  m_Controls.listViewDataNodes->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(m_Controls.pushButtonAddDataNode, SIGNAL(clicked()), this, SLOT(AddDataNodeToRenderer()));
  connect(m_Controls.pushButtonAddDataNode, SIGNAL(clicked()), this, SLOT(hide()));

  m_Controls.pushButtonAddDataNode->setEnabled(false);
}

void QmitkLayerManagerAddNodeWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkLayerManagerAddNodeWidget::ListDataNodes()
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

    m_DataNodeListModel->setStringList(stringList);

    if (stringList.isEmpty())
    {
      m_Controls.pushButtonAddDataNode->setEnabled(false);
    }
    else
    {
      m_Controls.pushButtonAddDataNode->setEnabled(true);
    }
  }
}

void QmitkLayerManagerAddNodeWidget::AddDataNodeToRenderer()
{  
  QModelIndex selectedIndex = m_Controls.listViewDataNodes->currentIndex();
  if (selectedIndex.isValid())
  {
    int listRow = selectedIndex.row();
    QString dataNodeName = m_DataNodeListModel->stringList().at(listRow);

    mitk::DataNode* dataNode = m_DataStorage->GetNamedNode(dataNodeName.toStdString());

    emit NodeToAddSelected(dataNode);
  }
}
