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


#ifndef BERRYPLUGINACTIVATOR_H
#define BERRYPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace berry {

class org_blueberry_core_expressions_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:
  org_blueberry_core_expressions_Activator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);
};

typedef org_blueberry_core_expressions_Activator PluginActivator;

}

#endif // BERRYPLUGINACTIVATOR_H
