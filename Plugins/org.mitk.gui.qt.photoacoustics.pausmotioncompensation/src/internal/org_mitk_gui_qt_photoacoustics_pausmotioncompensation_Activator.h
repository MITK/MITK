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


#ifndef org_mitk_gui_qt_photoacoustics_pausmotioncompensation_Activator_h
#define org_mitk_gui_qt_photoacoustics_pausmotioncompensation_Activator_h

#include <ctkPluginActivator.h>

namespace mitk
{
  class org_mitk_gui_qt_photoacoustics_pausmotioncompensation_Activator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_photoacoustics_pausmotioncompensation")
    Q_INTERFACES(ctkPluginActivator)

  public:
    void start(ctkPluginContext *context) override;
    void stop(ctkPluginContext *context) override;

  }; // org_mitk_gui_qt_photoacoustics_pausmotioncompensation_Activator
}

#endif // org_mitk_gui_qt_photoacoustics_pausmotioncompensation_Activator_h
