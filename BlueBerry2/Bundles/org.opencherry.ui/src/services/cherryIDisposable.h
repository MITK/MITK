/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYIDISPOSABLE_H_
#define CHERRYIDISPOSABLE_H_

#include "../cherryUiDll.h"

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

namespace cherry {

using namespace osgi::framework;

/**
 * <p>
 * The interface that should be implemented by services that make themselves
 * available through the <code>IAdaptable</code> mechanism. This is the
 * interface that drives the majority of services provided at the workbench
 * level.
 * </p>
 * <p>
 * A service has life-cycle. When the constructor completes, the service must be
 * fully functional. When it comes time for the service to go away, then the
 * service will receive a {@link #dispose()} call. At this point, the service
 * must release all resources and detach all listeners. A service can only be
 * disposed once; it cannot be reused.
 * </p>
 * <p>
 * This interface has nothing to do with OSGi services.
 * </p>
 * <p>
 * This interface can be extended or implemented by clients.
 * </p>
 *
 * @since 3.2
 */
struct CHERRY_UI IDisposable : public virtual Object
{

  osgiInterfaceMacro(cherry::IDisposable)

  /**
   * Disposes of this service. All resources must be freed. All listeners must
   * be detached. Dispose will only be called once during the life cycle of a
   * service.
   */
  virtual void Dispose() = 0;

};

}

#endif /* CHERRYIDISPOSABLE_H_ */
