/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef QMITKABSTRACTVIEW_H_
#define QMITKABSTRACTVIEW_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

//# blueberry stuff
#include <berryQtViewPart.h>
#include <berryIPreferencesService.h>
#include <berryISelectionListener.h>

#include <berryIPreferences.h>

//# mitk stuff
#include <org_mitk_gui_qt_common_Export.h>

#include "mitkDataNodeSelection.h"
#include "mitkIRenderWindowPart.h"

#include <mitkDataStorage.h>
#include <mitkRenderingManager.h>
#include <mitkIDataStorageReference.h>

class QItemSelectionModel;

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
   * Describes the strategies to be used for getting a mitk::IRenderWindowPart
   * instance.
   */
  enum IRenderWindowPartStrategy {

    /** Do nothing. */
    NONE           = 0x00000000,
    /** Bring the most recently activated mitk::IRenderWindowPart instance to the front. */
    BRING_TO_FRONT = 0x00000001,
    /** Activate a mitk::IRenderWindowPart part (implies bringing it to the front). */
    ACTIVATE       = 0x00000002,
    /** Create a mitk::IRenderWindowPart if none is alredy opened. */
    OPEN           = 0x00000004
  };

  Q_DECLARE_FLAGS(IRenderWindowPartStrategies, IRenderWindowPartStrategy)

  /**
   * Creates smartpointer typedefs
   */
  berryObjectMacro(QmitkAbstractView)

  /**
   * Nothing to do in the standard ctor. <b>Initiliaze your GUI in CreateQtPartControl(QWidget*)</b>
   * \see berry::QtViewPart::CreateQtPartControl(QWidget*)
   */
  QmitkAbstractView();

  /**
   * Disconnects all standard event listeners
   */
  virtual ~QmitkAbstractView();

protected:

  /**
   * Informs other parts of the workbench that node is selected via the blueberry selection service.
   */
  void FireNodeSelected(mitk::DataNode::Pointer node);

  /**
   * Informs other parts of the workbench that the nodes are selected via the blueberry selection service.
   */
  virtual void FireNodesSelected(const QList<mitk::DataNode::Pointer>& nodes);

  /**
   * \return the selection of the currently active part of the workbench or an empty list
   *         if nothing is selected
   */
  QList<mitk::DataNode::Pointer> GetCurrentSelection() const;

  /**
   * Returns the current selection made in the datamanager bundle or an empty list
   * if nothing`s selected or if the data manager view does not exist
   */
  QList<mitk::DataNode::Pointer> GetDataManagerSelection() const;

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
  mitk::IRenderWindowPart* GetRenderWindowPart(IRenderWindowPartStrategies strategies = NONE) const;

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
  void HandleException( std::exception& e, QWidget* parent = NULL, bool showDialog = true ) const;

  /**
   * Calls HandleException ( std::exception&, QWidget*, bool ) internally
   * \see HandleException ( std::exception&, QWidget*, bool )
   */
  void HandleException( const char* str, QWidget* parent = NULL, bool showDialog = true ) const;

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
   */
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

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
  void CreatePartControl(void* parent);

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

Q_DECLARE_OPERATORS_FOR_FLAGS(QmitkAbstractView::IRenderWindowPartStrategies)

#endif /*QMITKABSTRACTVIEW_H_*/
