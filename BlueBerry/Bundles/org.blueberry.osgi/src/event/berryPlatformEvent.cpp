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

#include "berryPlatformEvent.h"

namespace berry {

PlatformEvent::PlatformEvent(EventKind what)
  : m_What(what), m_HasException(false), m_Data(0)
{

}

PlatformEvent::PlatformEvent(EventKind what, IBundle::Pointer bundle)
 : m_Bundle(bundle), m_What(what), m_HasException(false), m_Data(0)
{

}

PlatformEvent::PlatformEvent(EventKind what, IBundle::Pointer bundle, std::exception exc)
 : m_Bundle(bundle), m_Exception(exc), m_What(what), m_HasException(true), m_Data(0)
{

}

PlatformEvent::EventKind
PlatformEvent::What() const
{
  return m_What;
}

const std::exception*
PlatformEvent::GetException() const
{
  if (m_HasException) return &m_Exception;
  return 0;
}

IBundle::Pointer
PlatformEvent::GetBundle()
{
  return m_Bundle;
}

void
PlatformEvent::SetData(Poco::Any* data)
{
  m_Data = data;
}

Poco::Any*
PlatformEvent::GetData()
{
  return m_Data;
}

const Poco::Any*
PlatformEvent::GetData() const
{
  return m_Data;
}

}
