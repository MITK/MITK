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

#include "cherryBundleEvent.h"
#include "../cherryIBundle.h"

namespace cherry {

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
