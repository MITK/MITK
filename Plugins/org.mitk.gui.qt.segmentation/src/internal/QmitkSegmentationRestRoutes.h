/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationRestRoutes_h
#define QmitkSegmentationRestRoutes_h

#include <mitkRestRoute.h>

#include <vector>

namespace mitk
{
  struct IRestServerService;

  /**
   * \brief The endpoints below /api/v1/segmentation, which segment the way
   *        the Segmentation view does: choose the image and the
   *        segmentation, create segmentations and labels, use the tools, and
   *        confirm their results.
   *
   * The endpoints work through the Segmentation view and open it when they
   * need it, so that the user sees what they do.
   *
   * \param server Resolves the UIDs of nodes and runs the work in the GUI
   *        thread. Must outlive the routes.
   */
  std::vector<RestRoute> CreateSegmentationRoutes(IRestServerService* server);
}

#endif
