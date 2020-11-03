/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryAbstractGroupMarker.h"

namespace berry {

AbstractGroupMarker::AbstractGroupMarker()
{
}

AbstractGroupMarker::AbstractGroupMarker(const QString& groupName)
  : ContributionItem(groupName)
{
  Q_ASSERT(!groupName.isEmpty());
}

QString AbstractGroupMarker::GetGroupName() const
{
    return GetId();
}

bool AbstractGroupMarker::IsGroupMarker() const
{
  return !(GetId().isEmpty());
}

}
