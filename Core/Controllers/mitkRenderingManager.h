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
#include <vector>
#include <itkObjectFactory.h>

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
 * knowledge of whether they are necessary or not. For example, two objects
 * might determine that a specific RenderWindow needs to be updated. This
 * would result in one unnecessary update, if both executed the update on
 * their own.
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
 * TODO: Individual timers for specific RenderWindows - the desired maximum
 * update frequency of a 3D window could be less then of a 2D window
 */
class RenderingManager : public itk::Object
{
public:
  mitkClassMacro( RenderingManager, itk::Object );

  static Pointer New();

  /** Set the object factory which produces the desired platform specific
   * RenderingManager singleton instance. */
  static void SetFactory( RenderingManagerFactory *factory );

  /** Get the RenderingManager singleton instance. */
  static Pointer GetInstance();

  /** Adds a RenderWindow. This is required if the methods #RequestUpdateAll
   * or #ForceImmediateUpdate are to be used. */
  void AddRenderWindow( RenderWindow *renderWindow );

  /** Removes a RenderWindow. */
  void RemoveRenderWindow( RenderWindow *renderWindow );

  /** Requests an update for the specified RenderWindow. The time of
   * execution usually depends on the specified minimum interval. */
  void RequestUpdate( RenderWindow *renderWindow );

  /** Immediately executes an update of the specified RenderWindow. */
  void ForceImmediateUpdate( RenderWindow *renderWindow );

  /** Requests all currently registered RenderWindows to be update. */
  void RequestUpdateAll();

  /** Immediately executes an update of all registered RenderWindows. */
  void ForceImmediateUpdateAll();

  /** Sets the minimum time interval in msec. When requesting an update for
   * a specific RenderWindow for the first time, it will be executed only
   * after at least this time interval has elapsed. */
  void SetMinimumInterval( int msec );

  /** Gets the currently set minimum time interval. Default is 10. */
  int GetMinimumInterval() const;

  ~RenderingManager();

  /** Executes all pending requests. This method has to be called by the
   * timer used by concrete RenderingManager implementations. */
  virtual void UpdateCallback();

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

  bool m_UpdatePending;
  int m_Interval;

private:
  typedef std::map< RenderWindow *, bool > RenderWindowList;

  RenderWindowList m_RenderWindowList;

  static RenderingManager::Pointer m_Instance;
  static RenderingManagerFactory *m_RenderingManagerFactory;

};

} // namespace mitk



#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
