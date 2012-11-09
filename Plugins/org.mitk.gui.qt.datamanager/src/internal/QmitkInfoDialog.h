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

#ifndef QMITKINFODIALOG_H_
#define QMITKINFODIALOG_H_

#include <mitkDataNode.h>

#include <QDialog>

class QTextBrowser;
class QLineEdit;

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class QmitkInfoDialog : public QDialog
{
  Q_OBJECT

  public:
    QmitkInfoDialog(const QList<mitk::DataNode::Pointer>& _Nodes, QWidget * parent = 0, Qt::WindowFlags f = 0 );
  public slots:
    void OnSelectionChanged(const mitk::DataNode*);
    void OnSearchButtonClicked ( bool checked = false );
    void OnCancelButtonClicked ( bool checked = false );
    void KeyWordTextChanged(const QString & text);
  protected:
    bool eventFilter(QObject *obj, QEvent *event);
  protected:
    QLineEdit* m_KeyWord;
    QPushButton* m_SearchButton;
    QTextBrowser* m_TextBrowser;
};

#endif // QMITKINFODIALOG_H_
