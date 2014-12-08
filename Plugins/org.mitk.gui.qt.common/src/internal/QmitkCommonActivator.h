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


#ifndef QMITKCOMMONACTIVATOR_H_
#define QMITKCOMMONACTIVATOR_H_

#include <ctkPluginActivator.h>
#include <ctkServiceTracker.h>

#include <berryIPreferencesService.h>

#include "QmitkViewCoordinator.h"

/**
 * \ingroup org_mitk_gui_qt_common_internal
 *
 * \brief The plug-in activator for the StateMachine
 *
 * When the plug-in is started by the framework, it initializes StateMachine
 * specific things.
 */
class QmitkCommonActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_common")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  static ctkPluginContext* GetContext();
  static QmitkCommonActivator* GetInstance();

  berry::IPreferencesService::Pointer GetPreferencesService();

  /**
   * Sets default StateMachine to EventMapper.
   */
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

private:

  static QmitkCommonActivator* m_Instance;
  static ctkPluginContext* m_Context;

  QmitkViewCoordinator::Pointer m_ViewCoordinator;
  QScopedPointer<ctkServiceTracker<berry::IPreferencesService*> > m_PrefServiceTracker;

};

#endif /* QMITKCOMMONACTIVATOR_H_ */

