/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
