/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITK_NODE_SELECTION_LIST_ITEM_WIDGET_H
#define QMITK_NODE_SELECTION_LIST_ITEM_WIDGET_H

#include <MitkQtWidgetsExports.h>

#include <ui_QmitkNodeSelectionListItemWidget.h>

#include <QWidget>

#include <mitkDataNode.h>

class MITKQTWIDGETS_EXPORT QmitkNodeSelectionListItemWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkNodeSelectionListItemWidget(QWidget* parent = nullptr);
  ~QmitkNodeSelectionListItemWidget() override;

  const mitk::DataNode* GetSelectedNode() const;

public Q_SLOTS :
  virtual void SetSelectedNode(const mitk::DataNode* node);
  virtual void SetClearAllowed(bool allowed);

signals:
  void ClearSelection(const mitk::DataNode* node);

protected Q_SLOTS:
  void OnClearSelection();

protected:
  bool eventFilter(QObject *obj, QEvent *ev) override;

  Ui_QmitkNodeSelectionListItemWidget m_Controls;
};


#endif // QMITK_NODE_SELECTION_LIST_ITEM_WIDGET_H
