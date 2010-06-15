/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKINFODIALOG_H_
#define QMITKINFODIALOG_H_

#include <vector>

#include <QDialog>

//class QmitkDataStorageComboBox;
namespace mitk
{
  class DataNode;
}
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
    QmitkInfoDialog( std::vector<mitk::DataNode*> _Nodes, QWidget * parent = 0, Qt::WindowFlags f = 0 );
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