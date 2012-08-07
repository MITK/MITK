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


#include "multipleperspectivesActivator.h"

#include "MultiplePerspectives.h"
#include "MinimalPerspective.h"
#include "ExtendedPerspective.h"
#include "MinimalView.h"
#include "SelectionView.h"

#include <QtPlugin>


void multipleperspectivesActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(MultiplePerspectives, context)
    BERRY_REGISTER_EXTENSION_CLASS(MinimalPerspective, context)
    BERRY_REGISTER_EXTENSION_CLASS(ExtendedPerspective, context)
    BERRY_REGISTER_EXTENSION_CLASS(MinimalView, context)
    BERRY_REGISTER_EXTENSION_CLASS(SelectionView, context)
}

void multipleperspectivesActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}


Q_EXPORT_PLUGIN2(org_mitk_example_gui_multipleperspectives, multipleperspectivesActivator)


