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


#ifndef QMITKABSTRACTRENDEREDITOR_H
#define QMITKABSTRACTRENDEREDITOR_H

#include <berryQtEditorPart.h>
#include <berryIBerryPreferences.h>
#include <berryISelectionProvider.h>

#include "mitkIRenderWindowPart.h"

#include <mitkIDataStorageReference.h>
#include <mitkDataStorage.h>

#include <org_mitk_gui_qt_common_Export.h>

class QmitkAbstractRenderEditorPrivate;

/**
 * \ingroup org_mitk_gui_qt_common
 *
 * \brief A convenient base class for MITK render window BlueBerry Editors.
 *
 * QmitkAbstractRenderEditor provides several convenience methods that ease the introduction of
 * a new editor for rendering a MITK DataStorage:
 *
 * <ol>
 *   <li> Access to the DataStorage (~ the shared data repository)
 *   <li> Access to and update notification for the editor's preferences
 *   <li> Default implementation of some mitk::IRenderWindowPart methods
 * </ol>
 *
 * When inheriting from QmitkAbstractRenderEditor, you must implement the following methods:
 * <ul>
 * <li>void CreateQtPartControl(QWidget* parent)
 * <li>void SetFocus()
 * </ul>
 *
 * You may reimplement the following private virtual methods to be notified about certain changes:
 * <ul>
 * <li>void OnPreferencesChanged(const berry::IBerryPreferences*)
 * </ul>
 *
 * \see IRenderWindowPart
 * \see ILinkedRenderWindowPart
 */
class MITK_QT_COMMON QmitkAbstractRenderEditor : public berry::QtEditorPart,
    public virtual mitk::IRenderWindowPart
{
  Q_OBJECT
  Q_INTERFACES(mitk::IRenderWindowPart)

public:

  QmitkAbstractRenderEditor();
  ~QmitkAbstractRenderEditor();

protected:

  /**
   * Initializes this editor by checking for a valid mitk::DataStorageEditorInput as <code>input</code>.
   *
   * \see berry::IEditorPart::Init
   */
  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

  /**
   * Get a reference to the DataStorage set by the editor input.
   */
  virtual mitk::IDataStorageReference::Pointer GetDataStorageReference() const;

  /**
   * Get the DataStorage set by the editor input.
   */
  virtual mitk::DataStorage::Pointer GetDataStorage() const;

  /**
   * Get the preferences for this editor.
   */
  virtual berry::IPreferences::Pointer GetPreferences() const;

  /**
  * Reimplement this method to set a custom selection provider. This method is
  * called once at initialization.
  *
  * The default implementation registers a mitk::PickedDataNodeSelectionProvider to support picking.
  */
  virtual void SetSelectionProvider();

  /**
   * Get the RenderingManager used by this editor. This default implementation uses the
   * global MITK RenderingManager provided by mitk::RenderingManager::GetInstance().
   *
   * \see mitk::IRenderWindowPart::GetRenderingManager
   */
  mitk::IRenderingManager* GetRenderingManager() const;

  /**
   * Request an update of this editor's render windows.
   * This implementation calls mitk::IRenderingManager::RequestUpdate on the rendering
   * manager interface returned by GetRenderingManager();
   *
   * \param requestType The type of render windows for which an update is requested.
   *
   * \see mitk::IRenderWindowPart::RequestUpdate
   */
  void RequestUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL);

  /**
   * Force an immediate update of this editor's render windows.
   * This implementation calls mitk::IRenderingManager::ForceImmediateUpdate() on the rendering
   * manager interface returned by GetRenderingManager();
   *
   * \param requestType The type of render windows for which an immedate update is forced.
   *
   * \see mitk::IRenderWindowPart::ForceImmediateUpdate
   */
  void ForceImmediateUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL);

  /**
   * Get the time navigation controller for this editor.
   * This implementation returns the SliceNavigationController returned by the mitk::IRenderingManager::GetTimeNavigationController()
   * method of the interface implementation returned by GetRenderingManager().
   *
   * \see mitk::IRenderingManager::GetTimeNavigationController
   */
  mitk::SliceNavigationController* GetTimeNavigationController() const;

  /** \see berry::IEditorPart::DoSave */
  void DoSave();

  /** \see berry::IEditorPart::DoSaveAs */
  void DoSaveAs();

  /** \see berry::IEditorPart::IsDirty */
  bool IsDirty() const;

  /** \see berry::IEditorPart::IsSaveAsAllowed */
  bool IsSaveAsAllowed() const;

private:

  virtual void OnPreferencesChanged(const berry::IBerryPreferences*);

private:

  QScopedPointer<QmitkAbstractRenderEditorPrivate> d;

};

#endif // QMITKABSTRACTRENDEREDITOR_H
