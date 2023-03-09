/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBoundingObjectWidget_h
#define QmitkBoundingObjectWidget_h

// includes
#include "MitkQtWidgetsExtExports.h"
#include <QComboBox>
#include <QPushButton>
#include <QTreeWidget>
#include <mitkBoundingObject.h>
#include <mitkBoundingObjectGroup.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <qwidget.h>

class MITKQTWIDGETSEXT_EXPORT QmitkBoundingObjectWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkBoundingObjectWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkBoundingObjectWidget() override;

  void SetDataStorage(mitk::DataStorage *dataStorage);
  mitk::DataStorage *GetDataStorage();
  mitk::BoundingObject::Pointer GetSelectedBoundingObject();
  mitk::DataNode::Pointer GetSelectedBoundingObjectNode();

  mitk::DataNode::Pointer GetAllBoundingObjects();

  void setEnabled(bool flag);

  void OnBoundingObjectModified(const itk::EventObject &e);

  void RemoveAllItems();

signals:
  // signal when bo has changed
  void BoundingObjectsChanged();

protected slots:
  void CreateBoundingObject(int type);
  void OnDelButtonClicked();
  void SelectionChanged();
  void OnItemDoubleClicked(QTreeWidgetItem *item, int col);
  void OnItemDataChanged(QTreeWidgetItem *item, int col);

protected:
  void AddItem(mitk::DataNode *node);
  void RemoveItem();

  mitk::DataStorage *m_DataStorage;
  QTreeWidget *m_TreeWidget;
  QComboBox *m_addComboBox;
  QPushButton *m_DelButton;
  QPushButton *m_SaveButton;
  QPushButton *m_LoadButton;
  QTreeWidgetItem *m_lastSelectedItem;
  unsigned long m_lastAffineObserver;

  typedef std::map<QTreeWidgetItem *, mitk::DataNode *> ItemNodeMapType;
  ItemNodeMapType m_ItemNodeMap;

  unsigned int m_BoundingObjectCounter;

  enum BoundingObjectType
  {
    CUBOID,
    CONE,
    ELLIPSOID,
    CYLINDER,
  };
};
#endif
