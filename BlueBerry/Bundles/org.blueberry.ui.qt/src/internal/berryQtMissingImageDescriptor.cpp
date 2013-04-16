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

#include "berryQtMissingImageDescriptor.h"

#include <QIcon>

namespace berry {


void* QtMissingImageDescriptor::CreateImage(bool  /*returnMissingImageOnError*/)
{
  return new QIcon(":/org.blueberry.ui.qt/icon_missing.png");
}

void QtMissingImageDescriptor::DestroyImage(void* img)
{
  QIcon* i = static_cast<QIcon*>(img);
  delete i;
}

bool QtMissingImageDescriptor::operator ==(const Object* o) const
{
  return dynamic_cast<const QtMissingImageDescriptor*>(o) != 0;
}

} // namespace


