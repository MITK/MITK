/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef org_mitk_gui_qt_cmdlinemodules_Activator_h
#define org_mitk_gui_qt_cmdlinemodules_Activator_h

#include <ctkPluginActivator.h>

namespace mitk {

/**
 * \class org_mitk_gui_qt_cmdlinemodules_Activator
 * \brief Blueberry plugin activator for CommandLineModulesView.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 */
class org_mitk_gui_qt_cmdlinemodules_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_cmdlinemodules")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // org_mitk_gui_qt_cmdlinemodules_Activator

}

#endif // org_mitk_gui_qt_cmdlinemodules_Activator_h
