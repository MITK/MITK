/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include "mitkCommon.h"
#include <string>
#include <itkObject.h>

namespace mitk
{

class RenderWindow;
class RenderingManager;
class RenderingManagerFactory;

/**
 * \brief Manager for coordinating the rendering process.
 *
 * RenderingManager is a central instance retrieving and executing
 * RenderWindow update requests. Its main purpose is to coordinate
 * distributed requests which cannot be aware of each other - lacking the
 * knowledge of whether they are really necessary or not. For example, two
 * objects might determine that a specific RenderWindow needs to be updated.
 * This would result in one unnecessary update, if both executed the update
 * on their own.
 *
 * The RenderingManager addresses this by letting each such object
 * <em>request</em> an update, and waiting for other objects to possibly
 * issue the same request. The actual update will then only be executed at a
 * well-defined point in the main event loop (this may be each time after
 * event processing is done, or after a minimum user specified interval has
 * elapsed).
 *
 * Conviniency methods for updating all RenderWindows which have been
 * registered with the RenderingManager exist. If theses methods are not
 * used, it is not required to register (add) RenderWindows prior to using
 * the RenderingManager.
 *
 * The methods #ForceImmediateUpdate() and #ForceImmediateUpdateAll() can
 * be used to force the RenderWindow update execution without any delay,
 * bypassing the request functionality.
 *
 * The interface of RenderingManager is platform independent. Platform
 * specific subclasses have to be implemented, though, to supply an
 * appropriate timer for controlling the update execution process. See method
 * documentation for a description of how this can be done.
 *
 * \todo Individual timers for specific RenderWindows - the desired maximum
 * update frequency of a 3D window could be less then that of a 2D window
 * \ingroup Renderer
 */
class RenderingManager : public itk::Object
{
public:

  mitkClassMacro(RenderingManager,itk::Object);

  typedef std::vector< RenderWindow* > RenderWindowVector;

  static Pointer New();

  /** Set the object factory which produces the desired platform specific
   * RenderingManager singleton instance. */
  static void SetFactory( RenderingManagerFactory *factory );

  /** Get the object factory which produces the platform specific
   * RenderingManager instances. */
  static const RenderingManagerFactory * GetFactory();

  /** Returns true if a factory has already been set. */
  static bool HasFactory();

  /** Get the RenderingManager singleton instance. */
  static RenderingManager *GetInstance();

  /** Returns true if the singleton instance does already exist. */
  static bool IsInstantiated();

  /** Adds a RenderWindow. This is required if the methods #RequestUpdateAll
   * or #ForceImmediateUpdate are to be used. */
  void AddRenderWindow( RenderWindow *renderWindow );

  /** Removes a RenderWindow. */
  void RemoveRenderWindow( RenderWindow *renderWindow );

  /** Get a list of all registered RenderWindows */
  const RenderWindowVector& GetAllRegisteredRenderWindows();

  RenderWindow* GetRenderWindowByName(const std::string&);

  /** Requests an update for the specified RenderWindow. The time of
   * execution usually depends on the specified minimum interval. */
  void RequestUpdate( RenderWindow *renderWindow );

  /** Immediately executes an update of the specified RenderWindow. */
  void ForceImmediateUpdate( RenderWindow *renderWindow );

  /** Requests all currently registered RenderWindows to be updated. */
  void RequestUpdateAll( bool includeVtkActors = false ); // TODO temporary fix until bug 167 (new vtk-based rendering mechanism) is done
  
  /** Requests all currently registered RenderWindows which use 3DMappers to be updated. */
  void RequestUpdateAll3D(bool includeVtkActors = false);

  /** Immediately executes an update of all registered RenderWindows. */
  void ForceImmediateUpdateAll();
  
  /** Immediately executes an update of all registered RenderWindows.
   * If the renderer associated with a render window is an OpenGLRenderer,
   * its method UpdateIncludingVtkActors is called. Otherwise the
   * repaint method of the widget will be called.
   */
  void ForceImmediateUpdateIncludingVtkActors();
  
  /** Requests an Overlay for the specified RenderWindow. */
  void RequestOverlayUpdate( RenderWindow *renderWindow );

  /** Requests an overlay update all RenderWindows.*/
  void RequestOverlayUpdateAll();

  /** Sets the minimum time interval in msec. When requesting an update for
   * a specific RenderWindow for the first time, it will be executed only
   * after at least this time interval has elapsed. */
  void SetMinimumInterval( int msec );

  /** Gets the currently set minimum time interval. Default is 10. */
  int GetMinimumInterval() const;

  virtual ~RenderingManager();

  /** Executes all pending requests. This method has to be called by the
   * timer used by concrete RenderingManager implementations. */
  virtual void UpdateCallback();

  bool IsRendering() const;
  void AbortRendering( RenderWindow* renderWindow );

  virtual void DoStartRendering() {};
  virtual void DoMonitorRendering() {};
  virtual void DoFinishAbortRendering() {};

  int GetCurrentLOD();
  void SetCurrentLOD(int lod);
  void SetNumberOfLOD(int number);
  
  void SetShading(bool state, int lod);
  bool GetShading(int lod);
  
  void SetClippingPlaneStatus(bool status);
  bool GetClippingPlaneStatus();
  
protected:
  RenderingManager();

  /** Abstract method for restarting the timer (to be implemented in
   * subclasses). The timer is restarted whenever an update is requested
   * for the first time. */
  virtual void RestartTimer() = 0;

  /** Abstract method for stopping the timer (to be implemented in
   * subclasses). The timer is stopped whenever no more requests are
   * pending. */
  virtual void StopTimer() = 0;

  /** Checks whether there are still pending update requests and sets
   * m_UpdatePending accordingly. To be called when it is not clear that
   * m_UpdatePending is still correct, e.g. typically after a single forced 
   * after update. */
  virtual void CheckUpdatePending();

  bool m_UpdatePending;
  int m_Interval;
  int m_CurrentLOD;
  int m_MaxLOD;
  int m_Numberof3DRW;
  std::vector<bool> m_ShadingEnabled;
  bool m_ClippingPlaneStatus;

  void RenderingStartCallback( itk::Object* object, const itk::EventObject& event );
  void RenderingProgressCallback( itk::Object* object, const itk::EventObject& event );
  void RenderingEndCallback( itk::Object* object, const itk::EventObject& event );

private:
  typedef std::map< RenderWindow *, int > RenderWindowList;

  RenderWindowList m_RenderWindowList;
  RenderWindowList m_IsRendering;
  RenderWindowVector m_AllRenderWindows;

  static RenderWindowList s_RenderWindowList;

  static RenderingManager::Pointer s_Instance;
  static RenderingManagerFactory *s_RenderingManagerFactory;

};

} // namespace mitk

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
