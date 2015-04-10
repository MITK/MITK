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

#ifndef BERRYSIMPLEEXTENSIONPOINTFILTER_H
#define BERRYSIMPLEEXTENSIONPOINTFILTER_H

#include <berryIExtensionPointFilter.h>

#include <QString>

namespace berry {

struct SimpleExtensionPointFilter : IExtensionPointFilter::Concept
{
  const QString m_Id;

  SimpleExtensionPointFilter(const QString& id);

  bool Matches(const IExtensionPoint* target) const;
};

}

#endif // BERRYSIMPLEEXTENSIONPOINTFILTER_H
