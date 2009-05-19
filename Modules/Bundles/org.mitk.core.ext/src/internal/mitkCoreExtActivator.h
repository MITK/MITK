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


#ifndef MBICOREEXTACTIVATOR_H_ 
#define MBICOREEXTACTIVATOR_H_ 

#include <cherryPlugin.h>

#include <mitkCoreExtDll.h>

namespace mitk
{

/**
 * \ingroup org_mitk_core_ext_internal
 *
 * \brief The plug-in activator for the CoreExt module
 *
 * When the plug-in is started by the framework, it initialzes CoreExt
 * specific things.
 */
class MITKCOREEXT_EXPORT CoreExtActivator : public cherry::Plugin
{

public:

  /**
   * Registers sandbox core object factories.
   */
  void Start(cherry::IBundleContext::Pointer context);

};

}

#endif /* MBICOREEXTACTIVATOR_H_ */
