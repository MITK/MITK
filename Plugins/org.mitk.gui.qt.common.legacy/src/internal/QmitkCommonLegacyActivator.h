/*=========================================================================

 Program:   MITK Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 17020 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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

