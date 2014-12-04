/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_application")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* GetContext();
  static org_mitk_gui_qt_application_Activator* GetInstance();

  berry::IPreferencesService::Pointer GetPreferencesService();

private:

  static org_mitk_gui_qt_application_Activator* m_Instance;
  static ctkPluginContext* m_Context;

  QScopedPointer<ctkServiceTracker<berry::IPreferencesService*> > m_PrefServiceTracker;

}; // org_mitk_gui_common_Activator

typedef org_mitk_gui_qt_application_Activator PluginActivator;

}

#endif // MITKPLUGINACTIVATOR_H
