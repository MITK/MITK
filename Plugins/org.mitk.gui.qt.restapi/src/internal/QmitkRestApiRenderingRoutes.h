/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRestApiRenderingRoutes_h
#define QmitkRestApiRenderingRoutes_h

#include <mitkRestRoute.h>

#include <vector>

namespace mitk
{
  struct IRestServerService;

  /**
   * \brief The endpoints below /api/v1/rendering/windows: the render windows
   *        of the standard display and their slices, screenshots that clients
   *        can point into, and what lies under such points.
   *
   * \param server Resolves the UIDs of nodes. Must outlive the routes.
   */
  std::vector<RestRoute> CreateRenderingRoutes(IRestServerService* server);
}

#endif
