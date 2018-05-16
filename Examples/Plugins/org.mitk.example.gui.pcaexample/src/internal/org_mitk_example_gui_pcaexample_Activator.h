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


#ifndef org_mitk_example_gui_pcaexample_Activator_h
#define org_mitk_example_gui_pcaexample_Activator_h

#include <ctkPluginActivator.h>

namespace mitk
{
  class org_mitk_example_gui_pcaexample_Activator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_example_gui_pcaexample")
    Q_INTERFACES(ctkPluginActivator)

  public:
    void start(ctkPluginContext *context);
    void stop(ctkPluginContext *context);

  }; // org_mitk_example_gui_pcaexample_Activator
}

#endif // org_mitk_example_gui_pcaexample_Activator_h
