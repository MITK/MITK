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

#include "cherryPlugin.h"

namespace cherry {

void
Plugin::Start(IBundleContext::Pointer /*context*/)
{

}

void
Plugin::Stop(IBundleContext::Pointer /*context*/)
{

}

IBundle::Pointer
Plugin::GetBundle()
{
  return m_Bundle;
}


Poco::Path*
Plugin::GetStatePath()
{
  return 0;
}

}
