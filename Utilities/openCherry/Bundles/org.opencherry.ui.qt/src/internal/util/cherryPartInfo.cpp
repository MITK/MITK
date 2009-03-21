/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryPartInfo.h"

namespace cherry
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
