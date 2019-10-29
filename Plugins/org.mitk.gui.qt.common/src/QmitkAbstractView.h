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

#ifndef QMITKABSTRACTVIEW_H_
#define QMITKABSTRACTVIEW_H_

//# blueberry stuff
#include <berryQtViewPart.h>
#include <berryIPreferencesService.h>
#include <berryISelectionListener.h>

#include <berryIPreferences.h>

//# mitk stuff
#include <org_mitk_gui_qt_common_Export.h>

#include "mitkDataNodeSelection.h"
#include "mitkIRenderWindowPart.h"

#include <mitkWorkbenchUtil.h>

#include <mitkDataStorage.h>
#include <mitkRenderingManager.h>
#include <mitkIDataStorageReference.h>

#include <QItemSelectionModel>

namespace mitk {
  class DataNode;
}

namespace berry {
  struct IBerryPreferences;
}

class QmitkAbstractViewPrivate;
class QmitkAbstractViewSelectionProvider;

/**
 * \ingroup org_mitk_gui_qt_common
 *
 * \brief A convenient base class for MITK related BlueBerry Views.
 *
 * QmitkAbstractView provides several convenience methods that ease the introduction of a new view:
 *
 * <ol>
 *   <li> Access to the DataStorage (~ the shared data repository)
 *   <li> Access to the active IRenderWindowPart
 *   <li> Access to and update notification for the view's preferences
 *   <li> Access to and update notification for the current DataNode selection / to DataNode selection events send through the SelectionService
 *   <li> Access to and update notification for DataNode events (added/removed/modified)
 *   <li> Methods to send DataNode selections through the SelectionService
 *   <li> Some minor important convenience methods (like changing the mouse cursor/exception handling)
 * </ol>
 *
 * Usually all MITK Views inherit from QmitkAbstractView to achieve a consistent Workbench behavior.
 *
 * When inheriting from QmitkAbstractView, you must implement the following methods:
 * <ul>
 * <li>void CreateQtPartControl(QWidget* parent)
 * <li>void SetFocus()
 * </ul>
 *
 * You may reimplement the following private virtual methods to customize your View's behavior:
 * <ul>
 * <li>void SetSelectionProvider()
 * <li>QItemSelectionModel* GetDataNodeSelectionModel() const
 * </ul>
 *
 * You may reimplement the following private virtual methods to be notified about certain changes:
 * <ul>
 * <li>void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes)
 * <li>void OnNullSelection(berry::IWorkbenchPart::Pointer part)
 * <li>void OnPreferencesChanged(const berry::IBerryPreferences*)
 * <li>void NodeAdded(const mitk::DataNode* node)
 * <li>void NodeChanged(const mitk::DataNode* node)
 * <li>void NodeRemoved(const mitk::DataNode* node)
 * <li>void DataStorageModified()
 * <li>void DataStorageChanged(mitk::IDataStorageReference::Pointer dsRef)
 * </ul>
 *
 * \see mitk::ILifecycleAwarePart
 * \see mitk::IZombieViewPart
 * \see mitk::IRenderWindowPartListener
 */
class MITK_QT_COMMON QmitkAbstractView : public berry::QtViewPart
{

public:

  /**
   * Creates smartpointer typedefs
   */
  berryObjectMacro(QmitkAbstractView);

  /**
   * Nothing to do in the standard ctor. <b>Initiliaze your GUI in CreateQtPartControl(QWidget*)</b>
   * \see berry::QtViewPart::CreateQtPartControl(QWidget*)
   */
  QmitkAbstractView();

  /**
   * Disconnects all standard event listeners
   */
  ~QmitkAbstractView() override;

protected:

  /**
   * Informs other parts of the workbench that node is selected via the blueberry selection service.
   *
   * \note This method should not be used if you have set your own selection provider via
   * SetSelectionProvider() or your own QItemSelectionModel via GetDataNodeSelectionModel().
   */
  void FireNodeSelected(mitk::DataNode::Pointer node);

  /**
   * Informs other parts of the workbench that the nodes are selected via the blueberry selection service.
   *
   * \note This method should not be used if you have set your own selection provider via
   * SetSelectionProvider() or your own QItemSelectionModel via GetDataNodeSelectionModel().
   */
  virtual void FireNodesSelected(const QList<mitk::DataNode::Pointer>& nodes);

  /**
   * \return The selection of the currently active part of the workbench or an empty list
   *         if there is no selection or if it is empty.
   *
   * \see IsCurrentSelectionValid
   */
  QList<mitk::DataNode::Pointer> GetCurrentSelection() const;

  /**
   * Queries the state of the current selection.
   *
   * \return If the current selection is <code>nullptr</code>, this method returns
   * <code>false</code> and <code>true</code> otherwise.
   */
  bool IsCurrentSelectionValid() const;

  /**
   * Returns the current selection made in the datamanager bundle or an empty list
   * if there is no selection or if it is empty.
   *
   * \see IsDataManagerSelectionValid
   */
  QList<mitk::DataNode::Pointer> GetDataManagerSelection() const;

  /**
   * Queries the state of the current selection of the data manager view.
   *
   * \return If the current data manager selection is <code>nullptr</code>, this method returns
   * <code>false</code> and <code>true</code> otherwise.
   */
  bool IsDataManagerSelectionValid() const;

  /**
   * Sets the selection of the data manager view if available.
   *
   * \param selection The new selection for the data manager.
   * \param flags The Qt selection flags for controlling the way how the selection is updated.
   */
  void SetDataManagerSelection(const berry::ISelection::ConstPointer& selection,
                               QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect) const;

  /**
   * Takes the current selection and sets it on the data manager. Only matching nodes in the
   * data manager view will be selected.
   */
  void SynchronizeDataManagerSelection() const;

  /**
   * Returns the Preferences object for this View.
   * <b>Important</b>: When refering to this preferences, e.g. in a PreferencePage: The ID
   * for this preferences object is "/<VIEW-ID>", e.g. "/org.mitk.views.datamanager"
   */
  berry::IPreferences::Pointer GetPreferences() const;

  /**
   * Returns a reference to the currently active DataStorage.
   */
  mitk::IDataStorageReference::Pointer GetDataStorageReference() const;

  /**
   * Returns the currently active DataStorage.
   */
  mitk::DataStorage::Pointer GetDataStorage() const;

  /**
   * Returns the currently active mitk::IRenderWindowPart.
   *
   * \param strategies Strategies for returning a mitk::IRenderWindowPart instance if there
   *        is currently no active one.
   * \return The active mitk::IRenderWindowPart.
   */
  mitk::IRenderWindowPart* GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategies strategies = mitk::WorkbenchUtil::NONE) const;

  /**
   * Request an update of all render windows of the currently active IRenderWindowPart.
   *
   * \param requestType Specifies the type of render windows for which an update
   *        will be requested.
   */
  void RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL);

  /**
   * Outputs an error message to the console and displays a message box containing
   * the exception description.
   * \param e the exception which should be handled
   * \param showDialog controls, whether additionally a message box should be
   *        displayed to inform the user that something went wrong
   */
  void HandleException( std::exception& e, QWidget* parent = nullptr, bool showDialog = true ) const;

  /**
   * Calls HandleException ( std::exception&, QWidget*, bool ) internally
   * \see HandleException ( std::exception&, QWidget*, bool )
   */
  void HandleException( const char* str, QWidget* parent = nullptr, bool showDialog = true ) const;

  /**
   * Convenient method to set and reset a wait cursor ("hourglass")
   */
  void WaitCursorOn();

  /**
   * Convenient method to restore the standard cursor
   */
  void WaitCursorOff();

  /**
   * Convenient method to set and reset a busy cursor
   */
  void BusyCursorOn();

  /**
   * Convenient method to restore the standard cursor
   */
  void BusyCursorOff();

  /**
   * Convenient method to restore the standard cursor
   */
  void RestoreOverrideCursor();

private:

  /**
   * Reimplement this method to set a custom selection provider. This method is
   * called once after CreateQtPartControl().
   *
   * The default implementation registers a QmitkDataNodeSelectionProvider with
   * a QItemSelectionModel returned by GetDataNodeSelectionModel().
   */
  virtual void SetSelectionProvider();

  /**
   * Reimplement this method to supply a custom Qt selection model. The custom
   * model will be used with the default selection provider QmitkDataNodeSelectionProvider
   * to inform the MITK Workbench about selection changes.
   *
   * If you reimplement this method, the methods FireNodeSelected() and FireNodesSelected()
   * will have no effect. Use your custom selection model to notify the MITK Workbench
   * about selection changes.
   *
   * The Qt item model used with the custom selection model must return mitk::DataNode::Pointer
   * objects for model indexes when the role is QmitkDataNodeRole.
   */
  virtual QItemSelectionModel* GetDataNodeSelectionModel() const;

  /**
   * Called when the selection in the workbench changed.
   * May be reimplemented by deriving classes.
   *
   * \param part The source part responsible for the selection change.
   * \param nodes A list of selected nodes.
   *
   * \see OnNullSelection
   */
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

  /**
   * Called when a <code>nullptr</code> selection occurs.
   *
   * \param part The source part responsible for the selection change.
   */
  virtual void OnNullSelection(berry::IWorkbenchPart::Pointer part);

  /**
   * Called when the preferences object of this view changed.
   * May be reimplemented by deriving classes.
   *
   * \see GetPreferences()
   */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*);

  /**
   * Called when a DataStorage Add event was thrown. May be reimplemented
   * by deriving classes.
   */
  virtual void NodeAdded(const mitk::DataNode* node);

  /**
   * Called when a DataStorage Changed event was thrown. May be reimplemented
   * by deriving classes.
   */
  virtual void NodeChanged(const mitk::DataNode* node);

  /**
   * Called when a DataStorage Remove event was thrown. May be reimplemented
   * by deriving classes.
   */
  virtual void NodeRemoved(const mitk::DataNode* node);

  /**
   * Called when a DataStorage add *or* remove *or* change event from the currently active
   * data storage is thrown.
   *
   * May be reimplemented by deriving classes.
   */
  virtual void DataStorageModified();

  /**
   * Called when the currently active DataStorage changed.
   * May be reimplemented by deriving classes.
   *
   * \param dsRef A reference to the new active DataStorage.
   */
  virtual void DataStorageChanged(mitk::IDataStorageReference::Pointer dsRef);

  /**
   * Creates a scroll area for this view and calls CreateQtPartControl then
   */
  void CreatePartControl(QWidget* parent) override;

  /**
   * Called immediately after CreateQtPartControl().
   * Here standard event listeners for a QmitkAbstractView are registered
   */
  void AfterCreateQtPartControl();

private:

  friend class QmitkAbstractViewPrivate;
  friend class QmitkViewCoordinator;

  Q_DISABLE_COPY(QmitkAbstractView)

  const QScopedPointer<QmitkAbstractViewPrivate> d;

};

#endif /*QMITKABSTRACTVIEW_H_*/
