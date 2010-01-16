/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryQtIconImageDescriptor.h"

#include <QIcon>

namespace berry {

QtIconImageDescriptor::QtIconImageDescriptor(void* img)
: icon(static_cast<QIcon*>(img))
{

}

void* QtIconImageDescriptor::CreateImage(bool returnMissingImageOnError)
{
  if (icon) return icon;

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
