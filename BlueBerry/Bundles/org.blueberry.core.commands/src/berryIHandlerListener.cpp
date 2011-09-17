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

#include "berryIHandlerListener.h"

#include "berryIHandler.h"
#include "berryHandlerEvent.h"

namespace berry {

void
IHandlerListener::Events
::AddListener(IHandlerListener::Pointer l)
{
  if (l.IsNull()) return;

  handlerChanged += Delegate(l.GetPointer(), &IHandlerListener::HandlerChanged);
}

void
IHandlerListener::Events
::RemoveListener(IHandlerListener::Pointer l)
{
  if (l.IsNull()) return;

  handlerChanged -= Delegate(l.GetPointer(), &IHandlerListener::HandlerChanged);
}

}

