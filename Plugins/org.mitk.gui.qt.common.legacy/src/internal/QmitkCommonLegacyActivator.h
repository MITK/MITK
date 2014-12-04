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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_common_legacy")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  /**
   * Sets default StateMachine to EventMapper.
   */
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

private:

  QmitkFunctionalityCoordinator::Pointer m_FunctionalityCoordinator;

};

#endif /* QMITKCOMMONLEGACYACTIVATOR_H_ */

