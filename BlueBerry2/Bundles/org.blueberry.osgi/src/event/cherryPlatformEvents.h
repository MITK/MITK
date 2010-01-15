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

#ifndef CHERRYPLATFORMEVENTS_H_
#define CHERRYPLATFORMEVENTS_H_

#include "../cherryOSGiDll.h"

#include "../osgi/framework/Message.h"

#include "cherryPlatformEvent.h"

namespace cherry {

struct CHERRY_OSGI PlatformEvents
{
  typedef Message1<const PlatformEvent&> EventType;

  EventType platformStarted;
  EventType platformError;
  EventType platformWarning;

  EventType logged;
};

}

#endif /*CHERRYPLATFORMEVENTS_H_*/
