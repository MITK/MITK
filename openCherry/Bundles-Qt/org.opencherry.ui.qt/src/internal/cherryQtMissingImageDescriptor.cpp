/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 16712 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtMissingImageDescriptor.h"

#include <QIcon>

namespace cherry {


void* QtMissingImageDescriptor::CreateImage(bool returnMissingImageOnError)
{
  return new QIcon(":/resources/icon_missing.png");
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


