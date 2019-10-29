/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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
