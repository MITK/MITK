/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyListReplacedObserver.h"

void mitk::PropertyListReplacedObserver::BeforePropertyListReplaced(const std::string & /*id*/,
                                                                    mitk::PropertyList * /*propertyList*/)
{
}

void mitk::PropertyListReplacedObserver::AfterPropertyListReplaced(const std::string & /*id*/,
                                                                   mitk::PropertyList * /*propertyList*/)
{
}
