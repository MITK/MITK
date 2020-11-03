/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSIMPLEEXTENSIONPOINTFILTER_H
#define BERRYSIMPLEEXTENSIONPOINTFILTER_H

#include <berryIExtensionPointFilter.h>

#include <QString>

namespace berry {

struct SimpleExtensionPointFilter : IExtensionPointFilter::Concept
{
  const QString m_Id;

  SimpleExtensionPointFilter(const QString& id);

  bool Matches(const IExtensionPoint* target) const override;
};

}

#endif // BERRYSIMPLEEXTENSIONPOINTFILTER_H
