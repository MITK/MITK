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
#include "internal/cherryInternalPlatform.h"

namespace cherry {

void
Plugin::Start(IBundleContext::Pointer context)
{
  m_Bundle = context->GetThisBundle();
}

void
Plugin::Stop(IBundleContext::Pointer /*context*/)
{
  m_Bundle = 0;
}

IBundle::Pointer
Plugin::GetBundle()
{
  return m_Bundle;
}


bool
Plugin::GetStatePath(Poco::Path& path)
{
  return InternalPlatform::GetInstance()->GetStatePath(path, m_Bundle, true);
}

}
