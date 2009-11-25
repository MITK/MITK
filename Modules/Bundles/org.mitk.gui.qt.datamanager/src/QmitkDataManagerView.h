#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

// Own includes
#include <cherryIPartListener.h>
#include <cherryISelection.h>
#include <cherryISelectionProvider.h>
#include <cherryIPreferencesService.h>
#include <cherryICherryPreferences.h>
#include <cherryISelectionListener.h>

/// Qmitk
#include <cherryQtViewPart.h>
#include <QmitkDataTreeNodeSelectionProvider.h>
#include <QmitkDnDFrameWidget.h>

#include <cherryIWorkbenchPartReference.h>
#include <cherryIPartListener.h>
#include <cherryIPreferencesService.h>

#include <mitkDataStorage.h>
#include "mitkQtCommonDll.h"
#include "mitkQtDataManagerDll.h"

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

class QmitkDataStorageTreeModel;
///
/// \ingroup org_mitk_gui_qt_datamanager_internal
///
/// \brief A View class that can show all data tree nodes of a certain DataStorage
///
/// \TODO: complete PACS support, in save dialog show regular filename
///
class MITK_QT_DATAMANAGER QmitkDataManagerView : public QObject, public cherry::QtViewPart
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
  void OnPreferencesChanged(const cherry::ICherryPreferences*);

  ///
  /// Invoked when the DataManager selection changed
  ///
  virtual void SelectionChanged(cherry::IWorkbenchPart::Pointer part
    , cherry::ISelection::ConstPointer selection);

  /// Invoked when the median action is invoked
  void OtsuFilter( bool checked = false );
  
  /// When rows are inserted auto expand them
  void NodeTreeViewRowsInserted ( const QModelIndex & parent, int start, int end );

  /// Whenever the selection changes set the "selected" property respectively
  void NodeSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );

protected:
  ///
  /// \brief Create the view here.
  ///
  virtual void CreateQtPartControl(QWidget* parent);
  void SetFocus();
  mitk::DataStorage::Pointer GetDataStorage() const;
  ///
  /// \brief Shows a file open dialog.
  ///
  void FileOpen( const char * fileName, mitk::DataTreeNode* parentNode );
protected:
  QWidget* m_Parent;
  QmitkDnDFrameWidget* m_DndFrameWidget;

  ///
  /// \brief A plain widget as the base pane.
  ///
  QmitkDataStorageTreeModel* m_NodeTreeModel;
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
  QTreeView* m_NodeTreeView;
  ///
  /// \brief The context menu that shows up when right clicking on a node.
  ///
  QMenu* m_NodeMenu;

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
  /// A selection listener for datatreenode events
  cherry::ISelectionListener::Pointer m_SelectionListener;
  /// cherry::SelectionChangedAdapter<QmitkPropertyListView> must be a friend to call
  friend struct cherry::SelectionChangedAdapter<QmitkDataManagerView>;

};

#endif /*QMITKDATAMANAGERVIEW_H_*/
