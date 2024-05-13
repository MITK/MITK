/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNodeDetailsDialog_h
#define QmitkNodeDetailsDialog_h

#include <mitkDataNode.h>
#include <MitkQtWidgetsExports.h>

#include <QDialog>

namespace Ui
{
  class QmitkNodeDetailsDialog;
}

class MITKQTWIDGETS_EXPORT QmitkNodeDetailsDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkNodeDetailsDialog(const QList<mitk::DataNode::ConstPointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  QmitkNodeDetailsDialog(const QList<mitk::DataNode::Pointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  ~QmitkNodeDetailsDialog() override;

  void OnSelectionChanged(const mitk::DataNode*);
  void OnSearchButtonClicked();
  void KeywordTextChanged(const QString& text);

private:
  bool eventFilter(QObject* obj, QEvent* event) override;
  void InitWidgets(const QList<mitk::DataNode::ConstPointer>& nodes);

  Ui::QmitkNodeDetailsDialog* m_Ui;
};

#endif
