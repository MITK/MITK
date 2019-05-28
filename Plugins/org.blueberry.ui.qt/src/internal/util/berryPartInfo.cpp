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

#include <QApplication>

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
  name = QApplication::translate("Plugin Title", part->GetName().toStdString().c_str());
  title = part->GetTitle();
  contentDescription = part->GetTitleStatus();
  image = part->GetTitleImage();
  toolTip = part->GetTitleToolTip();

  dirty = part->IsDirty();
}

}
