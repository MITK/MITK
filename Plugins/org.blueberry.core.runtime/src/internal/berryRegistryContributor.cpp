/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryContributor.h"

namespace berry {

RegistryContributor::RegistryContributor(const QString& actualId, const QString& actualName,
                    const QString& hostId, const QString& hostName)
  : actualContributorId(actualId), actualContributorName(actualName)
{
  if (!hostId.isEmpty())
  {
    this->hostId = hostId;
    this->hostName = hostName;
  }
  else
  {
    this->hostId = actualId;
    this->hostName = actualName;
  }
}

QString RegistryContributor::GetActualId() const
{
  return actualContributorId;
}

QString RegistryContributor::GetActualName() const
{
  return actualContributorName;
}

QString RegistryContributor::GetId() const
{
  return hostId;
}

QString RegistryContributor::GetName() const
{
  return hostName;
}

}
