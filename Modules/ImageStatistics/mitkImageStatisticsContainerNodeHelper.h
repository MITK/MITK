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

#ifndef MITKIMAGESTATISTICSCONTAINERNODEHELPER
#define MITKIMAGESTATISTICSCONTAINERNODEHELPER

#include <MitkImageStatisticsExports.h>

#include <mitkImageStatisticsContainer.h>
#include <mitkDataNode.h>

namespace mitk
{
  MITKIMAGESTATISTICS_EXPORT DataNode::Pointer CreateImageStatisticsNode(ImageStatisticsContainer::Pointer statistic, const std::string& name = "statistics");
}
#endif // MITKIMAGESTATISTICSCONTAINERNODEHELPER

