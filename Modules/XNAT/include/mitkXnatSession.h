/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXnatSession_h
#define mitkXnatSession_h

#include <ctkXnatSession.h>

#include <usServiceInterface.h>

namespace mitk
{
  /**
   * \brief Type alias for ctkXnatSession, registered as a CppMicroServices service interface.
   *
   * This typedef makes ctkXnatSession available under the mitk namespace and registers it
   * as a micro service with the interface identifier "org.mitk.services.XnatSession". This
   * allows other MITK modules and plugins to discover and track XNAT sessions through the
   * CppMicroServices service registry.
   *
   * \sa mitk::XnatSessionTracker, ctkXnatSession
   */
  typedef ctkXnatSession XnatSession;
}
US_DECLARE_SERVICE_INTERFACE(mitk::XnatSession, "org.mitk.services.XnatSession")

#endif
