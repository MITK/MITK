/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPartInfo.h"

namespace berry
{

PartInfo::PartInfo()
{
}

PartInfo::PartInfo(IPresentablePart::Pointer part)
{
  this->Set(part);
}

void PartInfo::Set(IPresentablePart::Pointer part)
{
  name = part->GetName();
  title = part->GetTitle();
  contentDescription = part->GetTitleStatus();
  image = part->GetTitleImage();
  toolTip = part->GetTitleToolTip();
  dirty = part->IsDirty();
}

}
