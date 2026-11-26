/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRestApiManualRoutes_h
#define QmitkRestApiManualRoutes_h

#include <mitkRestRoute.h>

#include <vector>

class ctkPluginContext;

namespace mitk
{
  struct IRestServerService;

  /**
   * \brief The endpoints below /api/v1/manual, which search and read the
   *        user manual that the help view shows.
   *
   * \param server Runs the work in the GUI thread. Must outlive the routes.
   * \param context The plugin context, whose data directory holds a copy of
   *        the help files. Must outlive the routes.
   */
  std::vector<RestRoute> CreateManualRoutes(const IRestServerService* server, ctkPluginContext* context);
}

#endif
