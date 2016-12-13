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

#ifndef QMITKLAYERMANAGERADDLAYERWIDGET_H
#define QMITKLAYERMANAGERADDLAYERWIDGET_H

#include "ui_QmitkLayerManagerAddLayerControls.h"

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
class QmitkLayerManagerAddLayerWidget : public QWidget
{
  Q_OBJECT

public:
  static const QString VIEW_ID;

  QmitkLayerManagerAddLayerWidget(QWidget* parent = nullptr);
  ~QmitkLayerManagerAddLayerWidget();

  /**
  * @brief set the data storage of the tree model
  *
  * @param[in] dataStorage    the mitk::DataStorage that should be used
  */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  void ListLayer();

public Q_SLOTS:
  void AddLayerToRenderer();

Q_SIGNALS:
  void LayerToAddSelected(mitk::DataNode* dataNode);

private:
  void Init();

  Ui::QmitkLayerManagerAddLayerControls m_Controls;

  QStringListModel* m_LayerListModel;
  mitk::DataStorage::Pointer m_DataStorage;
};

#endif // QMITKLAYERMANAGERADDLAYERWIDGET_H
