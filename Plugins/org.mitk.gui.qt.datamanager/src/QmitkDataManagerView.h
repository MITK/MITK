/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

// BlueBerry includes
#include <berryIBerryPreferences.h>

/// Qmitk
#include <QmitkAbstractView.h>

/// Qt
#include <QItemSelection>

#include <org_mitk_gui_qt_datamanager_Export.h>

// Forward declarations
class QMenu;
class QAction;
class QComboBox;
class QWidgetAction;
class QSlider;
class QModelIndex;
class QTreeView;
class QPushButton;
class QToolBar;
class QMenu;
class QSignalMapper;

class QmitkDnDFrameWidget;
class QmitkDataStorageTreeModel;

///
/// \ingroup org_mitk_gui_qt_datamanager_internal
///
/// \brief A View class that can show all data tree nodes of a certain DataStorage
///
/// \TODO: complete PACS support, in save dialog show regular filename
///
class MITK_QT_DATAMANAGER QmitkDataManagerView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID; // = "org.mitk.extapp.defaultperspective"
  ///
  /// \brief Standard ctor.
  ///
  QmitkDataManagerView();

  ///
  /// \brief Standard dtor.
  ///
  virtual ~QmitkDataManagerView();

public slots:
  ///
  /// Invoked when the opacity slider changed
  ///
  void OpacityChanged(int value);
  ///
  /// Invoked when the opacity action changed
  /// In this function the the opacity slider is set to the selected nodes opacity value
  ///
  void OpacityActionChanged();
  ///
  /// Invoked when the color button is pressed
  ///
  void ColorChanged();
  ///
  /// Invoked when the color action changed
  ///
  void ColorActionChanged();
  ///
  /// Invoked when the color button is pressed
  ///
  void TextureInterpolationChanged();
  ///
  /// Invoked when the color action changed
  ///
  void TextureInterpolationToggled ( bool checked );
  ///
  /// SurfaceRepresentationActionToggled
  ///
  void SurfaceRepresentationMenuAboutToShow ();
  ///public
  /// SurfaceRepresentationActionToggled
  ///
  void SurfaceRepresentationActionToggled ( bool checked );
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
  void OnPreferencesChanged(const berry::IBerryPreferences*);
  ///
  /// \brief will be toggled when a extension point context menu action is toggled
  /// this is a proxy method which will load the corresponding extension class
  /// and run IContextMenuAction
  ///
  void ContextMenuActionTriggered( bool );

  ///
  /// Invoked when the MITK workbench selection changed
  ///
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
                          const QList<mitk::DataNode::Pointer>& selection);

  /// Invoked when the median action is invoked
  void OtsuFilter( bool checked = false );
  
  /// When rows are inserted auto expand them
  void NodeTreeViewRowsInserted ( const QModelIndex & parent, int start, int end );

  /// will setup m_CurrentRowCount
  void NodeTreeViewRowsRemoved ( const QModelIndex & parent, int start, int end );

  /// Whenever the selection changes set the "selected" property respectively
  void NodeSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );

  /// Opens the editor with the given id using the current data storage
  void ShowIn(const QString& editorId);

protected:

  ///
  /// \brief Create the view here.
  ///
  virtual void CreateQtPartControl(QWidget* parent);

  void SetFocus();

  ///
  /// \brief Shows a file open dialog.
  ///
  void FileOpen( const char * fileName, mitk::DataNode* parentNode );

protected:

  QWidget* m_Parent;
  QmitkDnDFrameWidget* m_DndFrameWidget;

  ///
  /// \brief A plain widget as the base pane.
  ///
  QmitkDataStorageTreeModel* m_NodeTreeModel;
  ///
  /// Holds the preferences for the datamanager. 
  ///
  berry::IBerryPreferences::Pointer m_DataManagerPreferencesNode;
  ///
  /// saves the configuration elements for the context menu actions from extension points
  ///
  std::map<QAction*, berry::IConfigurationElement::Pointer> m_ConfElements;
  ///
  /// \brief The Table view to show the selected nodes.
  ///
  QTreeView* m_NodeTreeView;
  ///
  /// \brief The context menu that shows up when right clicking on a node.
  ///
  QMenu* m_NodeMenu;
  ///
  /// \brief flag indicating whether a surface created from a selected decimation is decimated with vtkQuadricDecimation or not
  ///
  bool m_SurfaceDecimation;

  ///# Actions for the Context Menu
  /// Global Reinit Action
  QAction* m_GlobalReinitAction;
  /// Save Action
  QAction* m_SaveAction;
  /// Remove Action
  QAction* m_RemoveAction;
  /// Reinit Action
  QAction* m_ReinitAction;
  /// A Slider widget to change the opacity of a node
  QSlider* m_OpacitySlider;
  /// Opacity action
  QWidgetAction* m_OpacityAction;
  /// button to change the color of a node
  QPushButton* m_ColorButton;
  /// Color action
  QWidgetAction* m_ColorAction;
  /// TextureInterpolation action
  QAction* m_TextureInterpolation;
  /// TextureInterpolation action
  QAction* m_SurfaceRepresentation;
  /// Show only selected nodes
  QAction* m_ShowOnlySelectedNodes;
  /// Toggles visibility of selected nodes
  QAction* m_ToggleSelectedVisibility;
  /// Shows infos for selected nodes
  QAction* m_ActionShowInfoDialog;
  
  /// Special filter action for images
  QAction* m_OtsuFilterAction;

  /// Maps "Show in" actions to editor ids
  QSignalMapper* m_ShowInMapper;

  /// A list of "Show in" actions
  QList<QAction*> m_ShowInActions;

  /// saves the current amount of rows shown in the datamanager
  size_t m_CurrentRowCount;

private:

  QItemSelectionModel* GetDataNodeSelectionModel() const;

  /// Reopen multi widget editor if it has been closed
  mitk::IRenderWindowPart *OpenRenderWindowPart();
};

#endif /*QMITKDATAMANAGERVIEW_H_*/
