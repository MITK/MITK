/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRestApiWorkbenchRoutes_h
#define QmitkRestApiWorkbenchRoutes_h

#include <mitkRestRoute.h>

#include <vector>

namespace mitk
{
  struct IRestServerService;

  /**
   * \brief The endpoints below /api/v1/workbench: the views, the selection
   *        of the Data Manager, undo and redo, and the running tasks.
   *
   * \param server Resolves the UIDs of nodes. Must outlive the routes.
   */
  std::vector<RestRoute> CreateWorkbenchRoutes(const IRestServerService* server);
}

#endif
