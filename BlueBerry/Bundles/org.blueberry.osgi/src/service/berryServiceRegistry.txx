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

#ifndef __BERRY_SERVICE_REGISTRY_TXX__
#define __BERRY_SERVICE_REGISTRY_TXX__

namespace berry {

template<class S>
typename S::Pointer ServiceRegistry::GetServiceById(const std::string& id)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  Service::Pointer servicePtr;

  std::map<const std::string, Service::Pointer>::const_iterator serviceIt =
      m_ServiceMap.find(id);

  if (serviceIt != m_ServiceMap.end())
  {
    servicePtr = serviceIt->second;
  }

  if (servicePtr.IsNull()) return SmartPointer<S>();
  
  if (servicePtr->IsA(typeid(S)))
  {
    SmartPointer<S> castService = servicePtr.Cast<S>();
    return castService;
  }
  else throw Poco::BadCastException("The service could not be cast to: ", typeid(S).name());
}

} // namespace berry

#endif // __BERRY_SERVICE_REGISTRY_TXX__
