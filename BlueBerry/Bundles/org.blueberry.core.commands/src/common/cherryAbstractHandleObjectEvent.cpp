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

#include "cherryAbstractHandleObjectEvent.h"

namespace cherry
{

const int AbstractHandleObjectEvent::CHANGED_DEFINED = 1;
const int AbstractHandleObjectEvent::LAST_BIT_USED_ABSTRACT_HANDLE =
    AbstractHandleObjectEvent::CHANGED_DEFINED;

AbstractHandleObjectEvent::AbstractHandleObjectEvent(bool definedChanged)
{
  if (definedChanged)
  {
    changedValues |= CHANGED_DEFINED;
  }
}

bool AbstractHandleObjectEvent::IsDefinedChanged() const
{
  return ((changedValues & CHANGED_DEFINED) != 0);
}

}
