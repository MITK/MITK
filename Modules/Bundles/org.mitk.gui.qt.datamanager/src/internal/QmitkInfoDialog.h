#ifndef QMITKINFODIALOG_H_
#define QMITKINFODIALOG_H_

#include <vector>

#include <QDialog>

//class QmitkDataStorageComboBox;
namespace mitk
{
  class DataTreeNode;
}
class QTextBrowser;

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class QmitkInfoDialog : public QDialog
{
  Q_OBJECT

  public:
    QmitkInfoDialog( std::vector<mitk::DataTreeNode*> _Nodes, QWidget * parent = 0, Qt::WindowFlags f = 0 );
  public slots:
    void OnSelectionChanged(const mitk::DataTreeNode*);
    void OnCancelButtonClicked ( bool checked = false );
  protected:
    QTextBrowser* m_TextBrowser;
};

#endif // QMITKINFODIALOG_H_