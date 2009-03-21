#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>

// Forward declarations
class QComboBox;
class QString;
class QListView;
class QMenu;
class QAction;
class QModelIndex;
class QTableView;
class QSplitter;
class QPushButton;

class QmitkDataStorageTableModel;
class QmitkPropertiesTableEditor;
class QmitkPredicateEditor;

/**
 * \ingroup org_mitk_gui_qt_datamanager_internal
 *
 */
class QmitkDataManagerView : public QObject, public QmitkFunctionality
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
  void OnPredicateChanged();
  void NodeTableViewClicked( const QModelIndex & index );
  void NodeTableViewContextMenuRequested( const QPoint & index );
  void SaveActionTriggered( bool checked = false );
  void ActionRemoveTriggered( bool checked = false );
  void ActionReinitTriggered( bool checked = false );
  void ActionSaveToPacsTriggered ( bool checked = false );

  void BtnLoadClicked ( bool checked = false );
  void BtnGlobalReinitClicked ( bool checked = false );



protected:

  void CreateQtPartControl(QWidget* parent);
  void FileOpen( const char * fileName, mitk::DataTreeNode* parentIterator );

private:
  QWidget* m_BasePane;
  QComboBox* m_DataStorageSelectionComboBox;
  QmitkPredicateEditor* m_PredicateEditor;
  QmitkDataStorageTableModel* m_NodeTableModel;
  QTableView* m_NodeTableView;
  QMenu* m_NodeMenu;  
  QAction* m_SaveAction;
  QAction* m_ActionSaveToPacs;
  QAction* m_RemoveAction;
  QAction* m_ReinitAction;
  QPushButton* m_BtnLoad;
  QPushButton* m_BtnGlobalReinit;
  QmitkPropertiesTableEditor* m_NodePropertiesTableEditor;

};

#endif /*QMITKDATAMANAGERVIEW_H_*/
