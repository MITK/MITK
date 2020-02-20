/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKCOREEXTACTIVATOR_H_
#define MITKCOREEXTACTIVATOR_H_

#include <ctkPluginActivator.h>

#include "mitkInputDeviceRegistry.h"

namespace mitk
{

  /**
  * @brief The activator class for the org.mitk.core.ext plug-in.
  * @ingroup org_mitk_core_ext_internal
  *
  * When the plug-in is started by the framework, it calls a global function to initialize
  * the mitkCoreExt module.
  *
  */
  class CoreExtActivator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_core_ext")
    Q_INTERFACES(ctkPluginActivator)

  public:

    ~CoreExtActivator() override;

    /**
    * Starts this plug-in and registers object factories.
    *
    * @param context
    *        The context for the plug-in.
    */
    void start(ctkPluginContext* context) override;

    void stop(ctkPluginContext* context) override;

  private:

    /**
    * Activates the input device modules.
    */
    void StartInputDeviceModules(ctkPluginContext *context);

    QScopedPointer<InputDeviceRegistry> m_InputDeviceRegistry;

  }; // end class CoreExtActivator
} //end namespace mitk

#endif /* MITKCOREEXTACTIVATOR_H_ */
