/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKPLUGINACTIVATOR_H
#define MITKPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>
#include <ctkServiceTracker.h>

#include <berryIPreferencesService.h>


namespace mitk {

class org_mitk_gui_qt_application_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_application")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  static ctkPluginContext* GetContext();
  static org_mitk_gui_qt_application_Activator* GetInstance();

  berry::IPreferencesService* GetPreferencesService();

private:

  static org_mitk_gui_qt_application_Activator* m_Instance;
  static ctkPluginContext* m_Context;

  QScopedPointer<ctkServiceTracker<berry::IPreferencesService*> > m_PrefServiceTracker;

}; // org_mitk_gui_common_Activator

typedef org_mitk_gui_qt_application_Activator PluginActivator;

}

#endif // MITKPLUGINACTIVATOR_H
