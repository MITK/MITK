/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef org_mitk_gui_qt_xnatinterface_Activator_h
#define org_mitk_gui_qt_xnatinterface_Activator_h

#include <ctkPluginActivator.h>
#include "QmitkXnatSessionManager.h"

namespace mitk {

class org_mitk_gui_qt_xnatinterface_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_xnatinterface")
  Q_INTERFACES(ctkPluginActivator)

public:

  static QmitkXnatSessionManager* GetXnatSessionManager();
  static ctkPluginContext* GetContext();
  static us::ModuleContext* GetXnatModuleContext();

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

private:

  static ctkPluginContext* m_Context;
  static us::ModuleContext* m_ModuleContext;

}; // org_mitk_gui_qt_xnatinterface_Activator

}

#endif // org_mitk_gui_qt_xnatinterface_Activator_h
