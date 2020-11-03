/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKNODEDETAILSDIALOG_H
#define QMITKNODEDETAILSDIALOG_H

#include <mitkDataNode.h>

#include <QDialog>

#include <MitkQtWidgetsExports.h>

class QLineEdit;
class QTextBrowser;

class MITKQTWIDGETS_EXPORT QmitkNodeDetailsDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkNodeDetailsDialog(const QList<mitk::DataNode::ConstPointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  QmitkNodeDetailsDialog(const QList<mitk::DataNode::Pointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

public Q_SLOTS:

  void OnSelectionChanged(const mitk::DataNode*);
  void OnSearchButtonClicked(bool checked = false);
  void OnCancelButtonClicked(bool checked = false);
  void KeyWordTextChanged(const QString& text);

protected:

  bool eventFilter(QObject* obj, QEvent* event) override;

protected:

  QLineEdit* m_KeyWord;
  QPushButton* m_SearchButton;
  QTextBrowser* m_TextBrowser;

private:
  void InitWidgets(const QList<mitk::DataNode::ConstPointer>& nodes);

};

#endif // QMITKNODEDETAILSDIALOG_H
