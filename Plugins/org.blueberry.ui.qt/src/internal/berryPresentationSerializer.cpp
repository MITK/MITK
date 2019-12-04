/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPresentationSerializer.h"

namespace berry
{

PresentationSerializer::PresentationSerializer(
    const QList<IPresentablePart::Pointer>& presentableParts) :
  parts(presentableParts)
{

}

QString PresentationSerializer::GetId(IPresentablePart::Pointer part)
{
  int index = parts.indexOf(part);

  return QString::number(index);
}

IPresentablePart::Pointer PresentationSerializer::GetPart(const QString& id)
{
  bool okay = false;
  int index = id.toInt(&okay);
  if (!okay) return IPresentablePart::Pointer(nullptr);

  IPresentablePart::Pointer result;
  if (index < parts.size())
    result = parts[index];

  return result;
}

}
