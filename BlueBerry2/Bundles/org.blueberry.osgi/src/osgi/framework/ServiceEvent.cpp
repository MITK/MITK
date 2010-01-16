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

#include "ServiceEvent.h"

namespace osgi
{
namespace framework
{

ServiceEvent::ServiceEvent(Type type, ServiceReference::Pointer reference) :
  reference(reference), type(type)
{

}

ServiceReference::Pointer ServiceEvent::GetServiceReference() const
{
  return reference;
}

ServiceEvent::Type ServiceEvent::GetType() const
{
  return type;
}

}
}
