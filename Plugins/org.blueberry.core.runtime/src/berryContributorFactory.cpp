/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
