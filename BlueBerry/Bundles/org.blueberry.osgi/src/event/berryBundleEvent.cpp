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

#include "berryBundleEvent.h"
#include "berryIBundle.h"

namespace berry {

BundleEvent::BundleEvent(IBundle::Pointer bundle, EventKind what) :
   m_Bundle(bundle), m_What(what)
{

}

BundleEvent::BundleEvent(IBundle* bundle, EventKind what) :
  m_Bundle(bundle), m_What(what)
{

}

BundleEvent::BundleEvent(const BundleEvent& event) :
  m_Bundle(event.GetBundle()), m_What(event.What())
{

}

BundleEvent::~BundleEvent()
{

}

BundleEvent&
BundleEvent::operator= (const BundleEvent& event)
{
  m_Bundle = event.GetBundle();
  m_What = event.What();
  return *this;
}

IBundle::ConstPointer
BundleEvent::GetBundle() const
{
  return m_Bundle;
}

BundleEvent::EventKind
BundleEvent::What() const
{
  return m_What;
}

}
