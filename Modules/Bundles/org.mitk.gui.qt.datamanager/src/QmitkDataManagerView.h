#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

// Own includes
#include "cherryIPartListener.h"
#include "cherryISelection.h"
#include "cherryISelectionProvider.h"
#include "cherryIPreferencesService.h"
#include "cherryICherryPreferences.h"

/// Qmitk
#include "QmitkFunctionality.h"
#include "QmitkDataTreeNodeSelectionProvider.h"
// #include "QmitkStandardViews.h"
#include "mitkQtDataManagerDll.h"

// Forward declarations
class QMenu;
class QAction;
class QModelIndex;
class QTableView;
class QPushButton;
class QToolBar;
class QMenu;

class QmitkDataStorageTableModel;
///
/// \ingroup org_mitk_gui_qt_datamanager_internal
///
/// \brief A View class that can show all data tree nodes of a certain DataStorage
///
/// \TODO: complete PACS support, in save dialog show regular filename
///
class MITK_QT_DATAMANAGER QmitkDataManagerView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:
  ///
  /// \brief Standard ctor.
  ///
  QmitkDataManagerView();
  ///
  /// \brief Standard dtor.
  ///
  virtual ~QmitkDataManagerView();
  ///
  /// \brief Returns all selected nodes in a vector
  ///
  std::vector<mitk::DataTreeNode*> GetSelectedNodes() const;
public slots:
  ///
  /// \brief Shows a node context menu.
  ///
  void NodeTableViewContextMenuRequested( const QPoint & index );
  ///
  /// \brief Invoked when an element should be saved.
  ///
  void SaveSelectedNodes( bool checked = false );
  ///
  /// \brief Invoked when an element should be removed.
  ///
  void RemoveSelectedNodes( bool checked = false );
  ///
  /// \brief Invoked when an element should be reinitiliased.
  ///
  void ReinitSelectedNodes( bool checked = false );
  ///
  /// \brief Invoked when the visibility of the selected nodes should be toggled.
  ///
  void MakeAllNodesInvisible ( bool checked = false );
  ///
  /// \brief Makes all selected nodes visible, all other nodes invisible.
  ///
  void ShowOnlySelectedNodes ( bool checked = false );
  ///
  /// \brief Invoked when the visibility of the selected nodes should be toggled.
  ///
  void ToggleVisibilityOfSelectedNodes ( bool checked = false );
  ///
  /// \brief Invoked when infos of the selected nodes should be shown in a dialog.
  ///
  void ShowInfoDialogForSelectedNodes ( bool checked = false );
  ///
  /// \brief Shows a load dialog.
  ///
  void Load ( bool checked = false );
  ///
  /// \brief Reinits everything.
  ///
  void GlobalReinit ( bool checked = false );
  ///
  /// Invoked when the preferences were changed
  ///
  void OnPreferencesChanged(const cherry::ICherryPreferences*);
protected:
  ///
  /// \brief Create the view here.
  ///
  void CreateQtPartControl(QWidget* parent);
  ///
  /// \brief Shows a file open dialog.
  ///
  void FileOpen( const char * fileName, mitk::DataTreeNode* parentNode );
protected:
  ///
  /// \brief A plain widget as the base pane.
  ///
  QmitkDataStorageTableModel* m_NodeTableModel;
  ///
  /// \brief The openCherry selection provider
  ///
  QmitkDataTreeNodeSelectionProvider::Pointer m_SelectionProvider;
  ///
  /// Holds the preferences for the datamanager. 
  ///
  cherry::ICherryPreferences::Pointer m_DataManagerPreferencesNode;
  ///
  /// \brief The Table view to show the selected nodes.
  ///
  QTableView* m_NodeTableView;
  ///
  /// \brief The context menu that shows up when right clicking on a node.
  ///
  QMenu* m_NodeMenu;

};

#endif /*QMITKDATAMANAGERVIEW_H_*/
