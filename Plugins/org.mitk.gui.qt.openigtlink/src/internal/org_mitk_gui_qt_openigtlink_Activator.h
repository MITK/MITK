/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef org_mitk_gui_qt_openigtlink_Activator_h
#define org_mitk_gui_qt_openigtlink_Activator_h

#include <ctkPluginActivator.h>

namespace mitk {

class org_mitk_gui_qt_openigtlink_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_openigtlink")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

}; // org_mitk_gui_qt_igtlplugin_Activator

}

#endif // org_mitk_gui_qt_igtlplugin_Activator_h
