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


#ifndef BERRYOBJECTINSPECTORACTIVATOR_H
#define BERRYOBJECTINSPECTORACTIVATOR_H

#include <ctkPluginActivator.h>

namespace berry {

class org_blueberry_ui_qt_objectinspector_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

};

typedef org_blueberry_ui_qt_objectinspector_Activator PluginActivator;

}

#endif // BERRYOBJECTINSPECTORACTIVATOR_H
