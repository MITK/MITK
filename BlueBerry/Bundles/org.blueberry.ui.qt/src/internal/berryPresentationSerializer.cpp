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
  if (!okay) return IPresentablePart::Pointer(0);

  IPresentablePart::Pointer result;
  if (index < parts.size())
    result = parts[index];

  return result;
}

}
