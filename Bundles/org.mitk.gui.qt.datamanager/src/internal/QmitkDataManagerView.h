#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>

#include "../mitkQtDataManagerDll.h"

// Forward declarations
class QComboBox;
class QString;
class QListView;
class QMenu;
class QAction;
class QModelIndex;
class QTableView;
class QmitkDataStorageTableModel;
class QmitkPropertiesTableEditor;

/**
 * \ingroup org_mitk_gui_qt_datamanager_internal
 *
 */
class MITK_QT_DATAMANAGER QmitkDataManagerView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:

  ///
  /// Called when the part is activated.
  ///
  virtual void Activated();
  ///
  /// Called when the part is deactivated.
  ///
  virtual void Deactivated();
  ///
  /// Called when a DataStorage Add Event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeAdded(const mitk::DataTreeNode* node);

  virtual ~QmitkDataManagerView();

protected slots:
  void DataStorageSelectionChanged(const QString & text);
  void NodeSelectionModeChanged(const QString & text);
  void NodeTableViewClicked( const QModelIndex & index );
  void NodeTableViewContextMenuRequested( const QPoint & index );
  void ShowDerivedNodesClicked(bool checked = false);

protected:

  void CreateQtPartControl(QWidget* parent);

private:
  QWidget* m_BasePane;
  QComboBox* m_DataStorageSelectionComboBox;
  QComboBox* m_NodeSelectionModeComboBox;
  QmitkDataStorageTableModel* m_NodeTableModel;
  QTableView* m_NodeTableView;
  QMenu* m_ShowDerivedNodesMenu;
  QAction* m_ShowDerivedNodesAction;
  QmitkPropertiesTableEditor* m_NodePropertiesTableEditor;

};

#endif /*QMITKDATAMANAGERVIEW_H_*/
