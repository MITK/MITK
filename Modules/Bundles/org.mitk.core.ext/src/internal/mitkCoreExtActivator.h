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


#ifndef MITKCOREEXTACTIVATOR_H_ 
#define MITKCOREEXTACTIVATOR_H_ 

#include <ctkPluginActivator.h>

#include <org_mitk_core_ext_Export.h>

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
    Q_INTERFACES(ctkPluginActivator)

  public:

    /**
    * Starts this plug-in and registers object factories.
    *
    * @param context 
    *        The context for the plug-in.
    */
    void start(ctkPluginContext* context);
    
    void stop(ctkPluginContext* context);

  private:

    /**
    * Activates the input device modules.
    */
    void StartInputDeviceModules();

  }; // end class CoreExtActivator
} //end namespace mitk

#endif /* MITKCOREEXTACTIVATOR_H_ */
