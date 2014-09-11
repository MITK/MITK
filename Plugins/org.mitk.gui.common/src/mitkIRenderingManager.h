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

#ifndef MITKIRENDERINGMANAGER_H
#define MITKIRENDERINGMANAGER_H

#include <mitkRenderingManager.h>

#include <org_mitk_gui_common_Export.h>

#include <QList>
#include <QtPlugin>

namespace mitk {
/**
 * \ingroup org_mitk_gui_common
 *
 * \brief An interface for accessing a mitk::RenderingManager instance.
 *
 * This interface acts as a wrapper to a mitk::RenderingManager instance, hiding some
 * methods from the user.
 *
 * \see MakeRenderingManagerInterface
 */
struct IRenderingManager {
  virtual ~IRenderingManager() {}

  /** Get a list of all registered RenderWindows */
  virtual QList<vtkRenderWindow*> GetAllRegisteredVtkRenderWindows() const = 0;

  /**
   * Requests an update for the specified RenderWindow, to be executed as
   * soon as the main loop is ready for rendering.
   */
  virtual void RequestUpdate( vtkRenderWindow *renderWindow ) = 0;

  /** Immediately executes an update of the specified RenderWindow. */
  virtual void ForceImmediateUpdate( vtkRenderWindow *renderWindow ) = 0;

  /**
   * Requests all currently registered RenderWindows to be updated.
   * If only 2D or 3D windows should be updated, this can be specified
   * via the parameter requestType.
   */
  virtual void RequestUpdateAll( RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL ) = 0;

  /**
   * Immediately executes an update of all registered RenderWindows.
   * If only 2D or 3D windows should be updated, this can be specified
   * via the parameter requestType.
   */
  virtual void ForceImmediateUpdateAll( RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL ) = 0;

  /** Initializes the windows specified by requestType to the given geometry. */
  virtual bool InitializeViews( const BaseGeometry *geometry,
                                RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL,
                                bool preserveRoughOrientationInWorldSpace = false ) = 0;
  virtual bool InitializeViews( const TimeGeometry *geometry,
                                RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL,
                                bool preserveRoughOrientationInWorldSpace = false ) = 0;

  /**
   * Initializes the windows to the default viewing direction
   * (geomtry information is NOT changed).
   */
  virtual bool InitializeViews( RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL ) = 0;

  /**
   * Initializes the specified window to the given geometry. Set
   * "initializeGlobalTimeSNC" to true in order to use this geometry as
   * global TimeGeometry.
   */
  virtual bool InitializeView( vtkRenderWindow *renderWindow, const BaseGeometry *geometry,
                               bool initializeGlobalTimeSNC = false) = 0;

  /**
   * Initializes the specified window to the default viewing direction
   * (geomtry information is NOT changed).
   */
  virtual bool InitializeView( vtkRenderWindow *renderWindow ) = 0;

  /** Gets the SliceNavigationController responsible for time-slicing. */
  virtual const SliceNavigationController *GetTimeNavigationController() const = 0;

  /** Gets the SliceNavigationController responsible for time-slicing. */
  virtual SliceNavigationController *GetTimeNavigationController() = 0;

  virtual bool IsRendering() const = 0;

  virtual void AbortRendering() = 0;

  /** En-/Disable LOD increase globally. */
  virtual void SetLODIncreaseBlocked(bool blocked) = 0;

  /** Get LOD blocked status. */
  virtual bool GetLODIncreaseBlocked() const = 0;

  /** En-/Disable LOD abort mechanism. */
  virtual void SetLODAbortMechanismEnabled(bool abort) = 0;

  /** Get LOD abort mechanism status. */
  virtual bool GetLODAbortMechanismEnabled() const = 0;

  virtual int GetNextLOD( BaseRenderer* renderer ) const = 0;

  /** Set current LOD (NULL means all renderers)*/
  virtual void SetMaximumLOD( unsigned int max ) = 0;

  virtual void SetShading( bool state, unsigned int lod ) = 0;
  virtual bool GetShading( unsigned int lod ) = 0;

  virtual void SetClippingPlaneStatus( bool status ) = 0;
  virtual bool GetClippingPlaneStatus() = 0;

  virtual void SetShadingValues( float ambient, float diffuse,
                                 float specular, float specpower ) = 0;

  virtual QList<float> GetShadingValues() const = 0;
};
}

Q_DECLARE_INTERFACE(mitk::IRenderingManager, "org.mitk.ui.IRenderingManager")

namespace mitk {
/**
 * Create a IRenderManager interface for a given RenderingManager. Ownership of the
 * returned pointer is transferred to the caller of this function.
 *
 * \param manager The RenderingManager instance for which to create a interface.
 * \return A pointer to the interface object. The caller is responsible for deleting the pointer.
 */
MITK_GUI_COMMON_PLUGIN IRenderingManager* MakeRenderingManagerInterface(RenderingManager::Pointer manager);
}

#endif // MITKIRENDERINGMANAGER_H