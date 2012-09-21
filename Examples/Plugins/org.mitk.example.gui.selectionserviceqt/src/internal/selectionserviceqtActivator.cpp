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


#include "selectionserviceqtActivator.h"

#include "SelectionServiceQT.h"
#include "ExtendedPerspective.h"
#include "ListenerView.h"
#include "SelectionView.h"

#include <QtPlugin>


void selectionserviceqtActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(SelectionServiceQT, context)
    BERRY_REGISTER_EXTENSION_CLASS(ExtendedPerspective, context)
    BERRY_REGISTER_EXTENSION_CLASS(ListenerView, context)
    BERRY_REGISTER_EXTENSION_CLASS(SelectionView, context)
}

void selectionserviceqtActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}


Q_EXPORT_PLUGIN2(org_mitk_example_gui_selectionserviceqt, selectionserviceqtActivator)


