/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageStatisticsContainerNodeHelper_h
#define mitkImageStatisticsContainerNodeHelper_h

#include <MitkImageStatisticsExports.h>

#include <mitkImageStatisticsContainer.h>
#include <mitkDataNode.h>

namespace mitk
{
  /**
   * \brief Create a DataNode wrapping an ImageStatisticsContainer.
   *
   * This helper function creates a new DataNode and sets the given
   * ImageStatisticsContainer as its data. The node is configured as a
   * helper object (not directly visible in renderers).
   *
   * \param[in] statistic Pointer to the ImageStatisticsContainer to wrap.
   * \param[in] name Name for the DataNode. Defaults to "statistics".
   *
   * \return Pointer to the newly created DataNode.
   *
   * \sa ImageStatisticsContainer
   * \sa ImageStatisticsContainerManager
   */
  MITKIMAGESTATISTICS_EXPORT DataNode::Pointer CreateImageStatisticsNode(ImageStatisticsContainer::Pointer statistic, const std::string& name = "statistics");
}
#endif
