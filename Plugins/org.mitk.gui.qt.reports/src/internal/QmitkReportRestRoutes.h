/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportRestRoutes_h
#define QmitkReportRestRoutes_h

#include <mitkRestRoute.h>

#include <vector>

namespace mitk
{
  struct IRestServerService;
}

/**
 * \brief REST endpoints below /api/v1/reports, which let clients write reports
 *        that open in editors of their own, with images of the render windows.
 */
std::vector<mitk::RestRoute> CreateReportRoutes(mitk::IRestServerService* server);

#endif
