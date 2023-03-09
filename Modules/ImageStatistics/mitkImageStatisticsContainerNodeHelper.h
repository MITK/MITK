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
  MITKIMAGESTATISTICS_EXPORT DataNode::Pointer CreateImageStatisticsNode(ImageStatisticsContainer::Pointer statistic, const std::string& name = "statistics");
}
#endif
