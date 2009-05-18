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


#ifndef QMITKEXTasdfEXTACTIVATOR_H_ 
#define QMITKEXTasdfEXTACTIVATOR_H_ 

#include <cherryPlugin.h>

#include <QmitkExtDll.h>

/**
 * \ingroup org_mitk_core_sandbox_internal
 *
 * \brief The plug-in activator for the sandbox
 *
 * When the plug-in is started by the framework, it initialzes sandbox
 * specific things.
 */
class QMITKEXT_EXPORT QmitkExtActivator : public cherry::Plugin
{

public:

  /**
   * Registers sandbox core object factories.
   */
  void Start(cherry::IBundleContext::Pointer context);

};

#endif /* MBISANDBOXACTIVATOR_H_ */
