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

class QObject;

namespace mitk
{
  struct IRestServerService;
}

/**
 * \brief REST endpoints that draw, list and restyle planar figures, the figures of the Measurement view.
 *
 * \param[in] server The server that offers the endpoints.
 * \param[in] owner A QObject of the GUI thread, the parent of the timers that
 *            end pulses after a given duration. Destroy it after the routes
 *            are removed; the pulses that it still times end with it.
 */
std::vector<mitk::RestRoute> CreatePlanarFigureRoutes(mitk::IRestServerService* server, QObject* owner);

#endif
