/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkActionBarAdvisor.h"

#include <berryIActionBarConfigurer.h>

QmitkActionBarAdvisor::QmitkActionBarAdvisor(const berry::IActionBarConfigurer::Pointer& configurer)
 : berry::ActionBarAdvisor(configurer)
{

}

void QmitkActionBarAdvisor::FillMenuBar(berry::IMenuManager* )
{

}
