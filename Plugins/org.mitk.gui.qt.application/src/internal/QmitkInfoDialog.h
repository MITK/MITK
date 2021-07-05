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

#ifndef QMITKINFODIALOG_H
#define QMITKINFODIALOG_H

#include <mitkDataNode.h>

#include <QDialog>

class QLineEdit;
class QTextBrowser;

class QmitkInfoDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkInfoDialog(const QList<mitk::DataNode::Pointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

public Q_SLOTS:

  void OnSelectionChanged(const mitk::DataNode*);
  void OnSearchButtonClicked(bool checked = false);
  void OnCloseButtonClicked(bool checked = false);
  void KeyWordTextChanged(const QString& text);

protected:

  bool eventFilter(QObject* obj, QEvent* event) override;

protected:

  QLineEdit* m_KeyWord;
  QPushButton* m_SearchButton;
  QTextBrowser* m_TextBrowser;

};

#endif // QMITKINFODIALOG_H
