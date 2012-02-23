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


#ifndef org_mitk_gui_qt_stdmultiwidgeteditor_Activator_H_
#define org_mitk_gui_qt_stdmultiwidgeteditor_Activator_H_

#include <ctkPluginActivator.h>

/**
 * \ingroup org_mitk_gui_qt_stdmultiwidgeteditor
 */
class org_mitk_gui_qt_stdmultiwidgeteditor_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

};

#endif /* org_mitk_gui_qt_stdmultiwidgeteditor_Activator_H_ */

