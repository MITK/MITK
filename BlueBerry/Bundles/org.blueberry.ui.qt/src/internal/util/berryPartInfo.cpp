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

#include "berryPartInfo.h"

namespace berry
{

PartInfo::PartInfo() :
  image(0)
{

}

PartInfo::PartInfo(IPresentablePart::Pointer part)
{
  this->Set(part);
}

void PartInfo::Set(IPresentablePart::Pointer part)
{
  name = part->GetName().c_str();
  title = part->GetTitle().c_str();
  contentDescription = part->GetTitleStatus().c_str();
  image = static_cast<QIcon*> (part->GetTitleImage());
  toolTip = part->GetTitleToolTip().c_str();
  dirty = part->IsDirty();
}

}
