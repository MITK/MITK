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

#include "berryContributorFactory.h"

#include "internal/berryRegistryContributor.h"
#include "internal/berryCTKPluginUtils.h"

#include <ctkPlugin.h>

namespace berry {

IContributor::Pointer ContributorFactory::CreateContributor(const QSharedPointer<ctkPlugin>& contributor)
{
  QString id = QString::number(contributor->getPluginId());
  QString name = contributor->getSymbolicName();
  QString hostId;
  QString hostName;

//  // determine host properties, if any
//  if (OSGIUtils.getDefault().isFragment(contributor)) {
//    Bundle[] hosts = OSGIUtils.getDefault().getHosts(contributor);
//    if (hosts != null) {
//      Bundle hostBundle = hosts[0];
//      hostId = Long.toString(hostBundle.getBundleId());
//      hostName = hostBundle.getSymbolicName();
//    }
//  }

  IContributor::Pointer result(new RegistryContributor(id, name, hostId, hostName));
  return result;
}

QSharedPointer<ctkPlugin> ContributorFactory::Resolve(const IContributor::Pointer& contributor)
{
  if (contributor.IsNull())
    return QSharedPointer<ctkPlugin>();

  if (RegistryContributor::Pointer regContributor = contributor.Cast<RegistryContributor>())
  {
    QString symbolicName = regContributor->GetActualName();
    return CTKPluginUtils::GetDefault()->GetPlugin(symbolicName);
  }
  return QSharedPointer<ctkPlugin>();
}

}
