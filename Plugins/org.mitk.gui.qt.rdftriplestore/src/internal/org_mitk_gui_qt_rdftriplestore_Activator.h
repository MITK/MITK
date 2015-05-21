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

#ifndef org_mitk_gui_qt_rdftriplestore_Activator_h
#define org_mitk_gui_qt_rdftriplestore_Activator_h

#include <ctkPluginActivator.h>
#include <mitkExportMacros.h>

namespace mitk {

  class MITK_LOCAL org_mitk_gui_qt_rdftriplestore_Activator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_rdftriplestore")
#endif
    Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

  }; // org_mitk_gui_qt_rdftriplestore_Activator

} // end of namespace mitk

#endif // org_mitk_gui_qt_rdftriplestore_Activator_h
