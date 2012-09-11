/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef org_mitk_gui_qt_cli_Activator_h
#define org_mitk_gui_qt_cli_Activator_h

#include <ctkPluginActivator.h>

namespace mitk {

/**
 * \class org_mitk_gui_qt_cli_Activator
 * \brief Blueberry plugin activator for CommandLineModulesView.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cli_internal
 */
class org_mitk_gui_qt_cli_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // org_mitk_gui_qt_cli_Activator

}

#endif // org_mitk_gui_qt_cli_Activator_h
