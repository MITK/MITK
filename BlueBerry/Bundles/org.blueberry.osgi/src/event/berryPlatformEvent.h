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

#ifndef BERRYPLATFORMEVENT_H_
#define BERRYPLATFORMEVENT_H_

#include "../berryOSGiDll.h"

#include "../berryIBundle.h"

#include "Poco/Any.h"

#include <exception>

namespace berry {

class BERRY_OSGI PlatformEvent
{
public:
  enum EventKind { EV_PLATFORM_STARTED, EV_PLATFORM_ERROR, EV_PLATFORM_WARNING,
                   EV_LOGGED};
  
  PlatformEvent(EventKind what);
  PlatformEvent(EventKind what, IBundle::Pointer bundle);
  PlatformEvent(EventKind what, IBundle::Pointer bundle, std::exception exc);
  
  EventKind What() const;
  const std::exception* GetException() const;
  IBundle::Pointer GetBundle();
  
  void SetData(Poco::Any* data);
  Poco::Any* GetData();
  const Poco::Any* GetData() const;
  
private:
  
  IBundle::Pointer m_Bundle;
  std::exception m_Exception;
  EventKind m_What;
  bool m_HasException;
  
  
  Poco::Any* m_Data;
};

}

#endif /*BERRYPLATFORMEVENT_H_*/
