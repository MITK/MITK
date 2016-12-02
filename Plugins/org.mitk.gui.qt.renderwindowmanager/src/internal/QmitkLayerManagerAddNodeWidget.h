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

#ifndef QMITKLAYERMANAGERADDNODEWIDGET_H
#define QMITKLAYERMANAGERADDNODEWIDGET_H

#include "ui_QmitkLayerManagerAddNodeControls.h"

// mitk
#include <mitkDataStorage.h>

// Qt includes
#include <QItemSelection>
#include <QStringListModel.h>
#include <QWidget.h>

/**
* @brief
*
*/
class QmitkLayerManagerAddNodeWidget : public QWidget
{
  Q_OBJECT

public:
  static const QString VIEW_ID;

  QmitkLayerManagerAddNodeWidget(QWidget* parent = nullptr);
  ~QmitkLayerManagerAddNodeWidget();

  /**
  * @brief set the data storage of the tree model
  *
  * @param[in] dataStorage    the mitk::DataStorage that should be used
  */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  void ListDataNodes();

public Q_SLOTS:
  void AddDataNodeToRenderer();

Q_SIGNALS:
  void NodeToAddSelected(mitk::DataNode* dataNode);

private:
  void Init();

  Ui::QmitkLayerManagerAddNodeControls m_Controls;

  QStringListModel* m_DataNodeListModel;
  mitk::DataStorage::Pointer m_DataStorage;

  // TODO: map for nodename (to display) and sopinstanceuid for identifying a data node
};

#endif // QMITKLAYERMANAGERADDNODEWIDGET_H
