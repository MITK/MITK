/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
