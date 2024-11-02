/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkINodeSelectionListener_h
#define mitkINodeSelectionListener_h

#include <MitkCoreExports.h>

namespace mitk
{
  class MITKCORE_EXPORT INodeSelectionListener
  {
  public:
    virtual void OnSelectionReceived(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) = 0;
  };
}

#endif
