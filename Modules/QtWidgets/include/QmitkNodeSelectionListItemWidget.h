/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkNodeSelectionListItemWidget_h
#define QmitkNodeSelectionListItemWidget_h

#include <MitkQtWidgetsExports.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <memory>

namespace Ui { class QmitkNodeSelectionListItemWidget; }

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

  std::unique_ptr<Ui::QmitkNodeSelectionListItemWidget> m_Controls;
};


#endif
