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


#include "berryPluginActivator.h"

#include "berryUITestApplication.h"
#include "util/berryEmptyPerspective.h"

#include <QtPlugin>

namespace berry {

org_blueberry_uitest_Activator::org_blueberry_uitest_Activator()
{

}

void org_blueberry_uitest_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(UITestApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(EmptyPerspective, context)
}

void org_blueberry_uitest_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_blueberry_uitest, berry::org_blueberry_uitest_Activator)
