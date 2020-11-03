/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkImageStatisticsContainerNodeHelper.h"

#include <mitkProperties.h>

namespace mitk
{
  DataNode::Pointer CreateImageStatisticsNode(ImageStatisticsContainer::Pointer statistic, const std::string& name) {
    auto statisticsNode = mitk::DataNode::New();
    statisticsNode->SetName(name);
    statisticsNode->SetData(statistic);
    statisticsNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    return statisticsNode;
  }
}
