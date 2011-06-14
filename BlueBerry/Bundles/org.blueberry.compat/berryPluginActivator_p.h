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


#ifndef BERRYCOMPATIBILITYACTIVATOR_P_H
#define BERRYCOMPATIBILITYACTIVATOR_P_H

#include <ctkPluginActivator.h>

namespace berry {

class CTKPluginListener;

class org_blueberry_compat_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  org_blueberry_compat_Activator();
  ~org_blueberry_compat_Activator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

private:

  CTKPluginListener* pluginListener;

}; // org_blueberry_compat_Activator

typedef org_blueberry_compat_Activator PluginActivator;

}

#endif // BERRYCOMPATIBILITYACTIVATOR_P_H
