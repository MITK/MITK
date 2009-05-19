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


#ifndef QMITKEXTEXTACTIVATOR_H_ 
#define QMITKEXTEXTACTIVATOR_H_ 

#include <cherryPlugin.h>

#include <QmitkExtDll.h>

/**
 * \ingroup org_mitk_gui_qt_ext_internal
 *
 * \brief The plug-in activator for the sandbox
 *
 * When the plug-in is started by the framework, it initialzes sandbox
 * specific things.
 */
class ORG_MITK_GUI_QT_EXT_EXPORT QmitkExtActivator : public cherry::Plugin
{

public:

  /**
   * Registers sandbox core object factories.
   */
  void Start(cherry::IBundleContext::Pointer context);

};

#endif /* QMITKEXTEXTACTIVATOR_H_ */
