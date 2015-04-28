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


#include "org_mitk_gui_qt_pointsetinteractionmultispectrum_Activator.h"

#include <QtPlugin>

#include "PointSetInteractionMultispectrum.h"

namespace mitk {

void org_mitk_gui_qt_pointsetinteractionmultispectrum_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(PointSetInteractionMultispectrum, context)
}

void org_mitk_gui_qt_pointsetinteractionmultispectrum_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_pointsetinteractionmultispectrum, mitk::org_mitk_gui_qt_pointsetinteractionmultispectrum_Activator)
#endif
