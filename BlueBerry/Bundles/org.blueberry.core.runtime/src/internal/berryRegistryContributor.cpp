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
