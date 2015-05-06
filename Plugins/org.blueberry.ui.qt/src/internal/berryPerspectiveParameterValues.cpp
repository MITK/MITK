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

#include "berryPerspectiveParameterValues.h"

#include <berryPlatformUI.h>
#include <berryIPerspectiveDescriptor.h>

#include <QHash>

namespace berry {

QHash<QString, QString> PerspectiveParameterValues::GetParameterValues() const
{
  QHash<QString,QString> values;

  const QList<IPerspectiveDescriptor::Pointer> perspectives = PlatformUI::GetWorkbench()
                                                              ->GetPerspectiveRegistry()->GetPerspectives();
  for (int i = 0; i < perspectives.size(); i++)
  {
    const IPerspectiveDescriptor::Pointer& perspective = perspectives[i];
    values.insert(perspective->GetLabel(), perspective->GetId());
  }

  return values;
}

}
