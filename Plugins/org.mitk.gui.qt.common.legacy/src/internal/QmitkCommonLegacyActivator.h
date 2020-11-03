/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKCOMMONLEGACYACTIVATOR_H_
#define QMITKCOMMONLEGACYACTIVATOR_H_

#include <ctkPluginActivator.h>

#include "QmitkFunctionalityCoordinator.h"

/**
 * \ingroup org_mitk_gui_qt_common_legacy_internal
 */
class QmitkCommonLegacyActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_common_legacy")
  Q_INTERFACES(ctkPluginActivator)

public:

  /**
   * Sets default StateMachine to EventMapper.
   */
  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  static ctkPluginContext* GetContext();

private:

  QmitkFunctionalityCoordinator m_FunctionalityCoordinator;

  static ctkPluginContext* m_Context;

};

#endif /* QMITKCOMMONLEGACYACTIVATOR_H_ */

