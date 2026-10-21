/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPlanarFigureRestRoutes_h
#define QmitkPlanarFigureRestRoutes_h

#include <mitkRestRoute.h>

#include <vector>

namespace mitk
{
  struct IRestServerService;
}

/**
 * \brief REST endpoints that draw and list planar figures, the figures of the Measurement view.
 */
std::vector<mitk::RestRoute> CreatePlanarFigureRoutes(mitk::IRestServerService* server);

#endif
