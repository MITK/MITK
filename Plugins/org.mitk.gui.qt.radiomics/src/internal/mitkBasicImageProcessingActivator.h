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

#ifndef MITKRadiomicsStatisticACTIVATOR_H
#define MITKRadiomicsStatisticACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

class RadiomicsStatisticActivator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_radiomicsstatistic")
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_radiomicstransformation")
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_radiomicsarithmetric")
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_radiomicsmaskprocessing")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

}; // RadiomicsStatisticActivator

}

#endif // MITKRadiomicsStatisticACTIVATOR_H
