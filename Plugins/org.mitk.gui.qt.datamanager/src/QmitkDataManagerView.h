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

#ifndef QMITKDATAMANAGERVIEW_H
#define QMITKDATAMANAGERVIEW_H

#include <org_mitk_gui_qt_datamanager_Export.h>

// mitk core
#include <mitkNodePredicateBase.h>

// berry plugin
#include <berryIBerryPreferences.h>

// mitk gui qt common plugin
#include <QmitkAbstractView.h>

// mitk gui qt application
#include <QmitkDataNodeContextMenu.h>

// qt
#include <QItemSelection>

// forward declarations
class QAction;
class QModelIndex;
class QTreeView;
class QSignalMapper;

class QmitkDnDFrameWidget;
class QmitkDataStorageTreeModel;
class QmitkDataManagerItemDelegate;
class QmitkDataStorageFilterProxyModel;

/**
* @brief A view that shows all data nodes of the data storage in a qt tree view.
*
*/
class MITK_QT_DATAMANAGER QmitkDataManagerView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const QString VIEW_ID; // = "org.mitk.views.datamanager"

  QmitkDataManagerView();

  ~QmitkDataManagerView() override;

public Q_SLOTS:

  // invoked when the berry preferences were changed
  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  //////////////////////////////////////////////////////////////////////////
  // Slots for Qt node tree signals
  //////////////////////////////////////////////////////////////////////////
  /// When rows are inserted auto expand them
  void NodeTreeViewRowsInserted(const QModelIndex& parent, int start, int end);

  /// will setup m_CurrentRowCount
  void NodeTreeViewRowsRemoved(const QModelIndex& parent, int start, int end);

  /// Whenever the selection changes set the "selected" property respectively
  void NodeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void OnNodeVisibilityChanged();

  /// Opens the editor with the given id using the current data storage
  void ShowIn(const QString& editorId);

protected:

  void CreateQtPartControl(QWidget* parent) override;

  void SetFocus() override;
  ///
  /// React to node changes. Overridden from QmitkAbstractView.
  ///
  void NodeChanged(const mitk::DataNode* node) override;

protected:

  QWidget* m_Parent;
  QmitkDnDFrameWidget* m_DnDFrameWidget;

  ///
  /// \brief A plain widget as the base pane.
  ///
  QmitkDataStorageTreeModel* m_NodeTreeModel;
  QmitkDataStorageFilterProxyModel* m_FilterModel;
  mitk::NodePredicateBase::Pointer m_HelperObjectFilterPredicate;
  mitk::NodePredicateBase::Pointer m_NodeWithNoDataFilterPredicate;
  ///
  /// Holds the preferences for the data manager.
  ///
  berry::IBerryPreferences::Pointer m_DataManagerPreferencesNode;
  ///
  /// \brief The Table view to show the selected nodes.
  ///
  QTreeView* m_NodeTreeView;
  ///
  /// \brief The context menu that shows up when right clicking on a node.
  ///
  QmitkDataNodeContextMenu* m_DataNodeContextMenu;
  ///
  /// \brief flag indicating whether a surface created from a selected decimation is decimated with vtkQuadricDecimation or not
  ///
  bool m_SurfaceDecimation;

  /// Maps "Show in" actions to editor ids
  QSignalMapper* m_ShowInMapper;

  /// A list of "Show in" actions
  QList<QAction*> m_ShowInActions;

  /// saves the current amount of rows shown in the data manager
  size_t m_CurrentRowCount;

  QmitkDataManagerItemDelegate* m_ItemDelegate;

private:

  QItemSelectionModel* GetDataNodeSelectionModel() const override;

};

#endif // QMITKDATAMANAGERVIEW_H
