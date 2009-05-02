#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

// Own includes
#include "cherryIPartListener.h"
#include "cherryISelection.h"
#include "cherryISelectionProvider.h"
/// Qmitk
#include "QmitkFunctionality.h"
#include "QmitkStandardViews.h"
#include "../mitkQtDataManagerDll.h"

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

///
/// \ingroup org_mitk_gui_qt_datamanager_internal
///
/// \brief A View class that is subdivided in three parts. 1. datastorage-selection, 2. node / other objects with properties-selection
/// and 3. properties table view with the possibility of editing them
/// 
/// \TODO: complete PACS support, complete datastorage selection change, outsource the properties table as an own view and use the Selection
/// of openCherry to intercommunicate, in save dialog show regular filename
///
class MITK_QT_DATAMANAGER QmitkDataManagerView : public QObject, public QmitkFunctionality//, virtual public ISelectionProvider
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
  /// \struct SingleNodeSelection
  /// \brief Represents a selection object that encapsulates the selection of a single node.
  ///
  struct SingleNodeSelection: public cherry::ISelection
  {
    ///
    /// \brief Make typdefs for pointer types.
    ///
    cherryObjectMacro(SingleNodeSelection);
    ///
    /// \brief Node setter.
    ///
    void SetNode(mitk::DataTreeNode* _SelectedNode);
    ///
    /// \brief Node getter.
    ///
    mitk::DataTreeNode* GetNode() const;
    ///
    /// \brief Checks if node is 0.
    ///
    virtual bool IsEmpty() const;

  protected:
    ///
    /// \brief Holds the node that is currently selected.
    ///
    mitk::DataTreeNode* m_Node;
  };

  ///
  /// Called when the part is activated.
  ///
  virtual void Activated();
  ///
  /// Called when the part is deactivated.
  ///
  virtual void Deactivated();

  ///
  /// \brief Shows a popup dialog with the currently selected node.
  ///
  void ShowNodeInfo();

protected slots:
  ///
  /// \brief Other datastorage selected, reset node selection and properties
  ///
  void DataStorageSelectionChanged(const QString & text);
  ///
  /// \brief When the user right clicks on a node, NodeTableViewClicked() makes sure it is also selected.
  ///
  void NodeTableViewClicked( const QModelIndex & index );
  ///
  /// \brief Should be called whenever the selection of the nodes table changes.
  ///
  void NodeTableViewSelectionChanged ( const QModelIndex & current, const QModelIndex & previous );
  ///
  /// \brief Shows a node context menu.
  ///
  void NodeTableViewContextMenuRequested( const QPoint & index );
  ///
  /// \brief Invoked when an element should be saved.
  ///
  void SaveActionTriggered( bool checked = false );
  ///
  /// \brief Invoked when an element should be removed.
  ///
  void ActionRemoveTriggered( bool checked = false );
  ///
  /// \brief Invoked when an element should be reinitiliased.
  ///
  void ActionReinitTriggered( bool checked = false );
  ///
  /// \brief Invoked when an element should be saved to the pacs.
  ///
  void ActionSaveToPacsTriggered ( bool checked = false );
  ///
  /// \brief Shows a load dialog.
  ///
  void BtnLoadClicked ( bool checked = false );
  ///
  /// \brief Reinits everything.
  ///
  void BtnGlobalReinitClicked ( bool checked = false );

protected:

  ///
  /// \brief Create the view here.
  ///
  void CreateQtPartControl(QWidget* parent);
  ///
  /// \brief Shows a file open dialog.
  ///
  void FileOpen( const char * fileName, mitk::DataTreeNode* parentNode );

private:
  ///
  /// \brief A plain widget as the base pane.
  ///
  QWidget* m_BasePane;
  ///
  /// \brief A combobox for the datastorage selection.
  ///
  QComboBox* m_DataStorageSelectionComboBox;
  ///
  /// \brief A plain widget as the base pane.
  ///
  QmitkDataStorageTableModel* m_NodeTableModel;
  ///
  /// \brief The Table view to show the selected nodes.
  ///
  QTableView* m_NodeTableView;
  ///
  /// \brief The context menu that shows up when right clicking on a node.
  ///
  QMenu* m_NodeMenu;
  ///
  /// \brief Action that is triggered when a node should be saved.
  ///
  QAction* m_SaveAction;
  ///
  /// \brief Action for saving to pacs (not implemented yet for qt4)
  ///
  QAction* m_ActionSaveToPacs;
  ///
  /// \brief Action that is triggered when a node should be removed.
  ///
  QAction* m_RemoveAction;
  ///
  /// \brief Action that is triggered when a node should be reinited.
  ///
  QAction* m_ReinitAction;
  ///
  /// \brief A button to load new nodes.
  ///
  QPushButton* m_BtnLoad;
  ///
  /// \brief A button to make a global reinit.
  ///
  QPushButton* m_BtnGlobalReinit;
  ///
  /// \brief The properties table editor.
  ///
  QmitkPropertiesTableEditor* m_NodePropertiesTableEditor;

};

#endif /*QMITKDATAMANAGERVIEW_H_*/
