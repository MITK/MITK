/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYIDISPOSABLE_H_
#define BERRYIDISPOSABLE_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryObject.h>
#include <berryMacros.h>

namespace berry {

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
 */
struct IDisposable : public virtual Object
{

  berryObjectMacro(berry::IDisposable)

  ~IDisposable();

  /**
   * Disposes of this service. All resources must be freed. All listeners must
   * be detached. Dispose will only be called once during the life cycle of a
   * service.
   */
  virtual void Dispose() = 0;

};

}

#endif /* BERRYIDISPOSABLE_H_ */
