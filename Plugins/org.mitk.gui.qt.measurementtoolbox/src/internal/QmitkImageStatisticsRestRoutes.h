/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageStatisticsRestRoutes_h
#define QmitkImageStatisticsRestRoutes_h

#include <mitkRestRoute.h>

#include <vector>

namespace mitk
{
  struct IRestServerService;
}

/**
 * \brief REST endpoints that measure images as the Image Statistics view does.
 */
std::vector<mitk::RestRoute> CreateImageStatisticsRoutes(mitk::IRestServerService* server);

#endif
