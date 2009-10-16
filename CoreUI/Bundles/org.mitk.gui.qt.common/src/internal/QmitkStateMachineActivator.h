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


#ifndef MBISTATEMACHINETACTIVATOR_H_ 
#define MBISTATEMACHINETACTIVATOR_H_ 

#include <cherryPlugin.h>

#include <mitkQtCommonDll.h>

/**
 * \ingroup org_mitk_core_ext_internal
 *
 * \brief The plug-in activator for the StateMachine
 *
 * When the plug-in is started by the framework, it initializes StateMachine
 * specific things.
 */
class MITK_QT_COMMON QmitkStateMachineActivator : public cherry::Plugin
{

public:

  /**
   * Sets default StateMachine to EventMapper.
   */
  void Start(cherry::IBundleContext::Pointer context);

};

#endif /* MBISTATEMACHINETACTIVATOR_H_ */

