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

#include "QmitkActionBarAdvisor.h"

#include <berryIActionBarConfigurer.h>

QmitkActionBarAdvisor::QmitkActionBarAdvisor(const berry::IActionBarConfigurer::Pointer& configurer)
 : berry::ActionBarAdvisor(configurer)
{

}

void QmitkActionBarAdvisor::FillMenuBar(berry::IMenuManager* )
{

}
