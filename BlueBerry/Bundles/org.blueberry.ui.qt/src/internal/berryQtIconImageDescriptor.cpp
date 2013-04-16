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

#include "berryQtIconImageDescriptor.h"

#include <QIcon>

namespace berry {

QtIconImageDescriptor::QtIconImageDescriptor(void* img)
: icon(static_cast<QIcon*>(img))
{

}

QtIconImageDescriptor::~QtIconImageDescriptor()
{
  delete icon;
}

void* QtIconImageDescriptor::CreateImage(bool returnMissingImageOnError)
{
  if (icon) return new QIcon(*icon);

  if (returnMissingImageOnError)
    return GetMissingImageDescriptor()->CreateImage();

  return 0;
}

void QtIconImageDescriptor::DestroyImage(void* img)
{
  QIcon* i = static_cast<QIcon*>(img);
  delete i;
}

bool QtIconImageDescriptor::operator ==(const Object* o) const
{
  if (const QtIconImageDescriptor* obj = dynamic_cast<const QtIconImageDescriptor*>(o))
  {
    return this->icon == obj->icon;
  }

  return false;
}

}
