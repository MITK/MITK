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
  * When the plug-in is started by the framework, it initializes CoreExt module itself. <br> 
  * Furthermore through every other bundle, that is included in <code>mitk::CoreExtActivator::Start</code>,<br>
  * another module will be activated in the process. This can be done by adding a method to <br>
  * <code>mitk::CoreExtActivator::Start</code>,which initializes an Activator of the desired bundle.
  * 
  * E.g. <code>mitk::CoreExtActivator::StartInputDevicesModules</code>
  *
  * @brief The plug-in activator for the CoreExt module
  * @ingroup org_mitk_core_ext_internal
  */
  class CoreExtActivator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_INTERFACES(ctkPluginActivator)

  public:

    /**
    * Registers sandbox core object factories.
    *
    * @param context 
    *			the context for the bundle
    */
    void start(ctkPluginContext* context);
    
    void stop(ctkPluginContext* context);

    /**
    * Activates the input device modules.
    */
    void StartInputDeviceModules();

  }; // end class CoreExtActivator
} //end namespace mitk

#endif /* MITKCOREEXTACTIVATOR_H_ */
