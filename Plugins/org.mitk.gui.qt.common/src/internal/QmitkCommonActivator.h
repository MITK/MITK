/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkCommonActivator_h
#define QmitkCommonActivator_h

#include <ctkPluginActivator.h>
#include "QmitkViewCoordinator.h"

namespace mitk
{
  class IPreferencesService;
}

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
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_common")
  Q_INTERFACES(ctkPluginActivator)

public:

  static ctkPluginContext* GetContext();
  static QmitkCommonActivator* GetInstance();

  mitk::IPreferencesService* GetPreferencesService();

  /**
   * Sets default StateMachine to EventMapper.
   */
  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

private:

  static QmitkCommonActivator* m_Instance;
  static ctkPluginContext* m_Context;

  QScopedPointer<QmitkViewCoordinator> m_ViewCoordinator;
};

#endif
