/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYUITESTWORKBENCHADVISOR_H_
#define BERRYUITESTWORKBENCHADVISOR_H_

#include <berryWorkbenchAdvisor.h>

namespace berry {

class UITestWorkbenchAdvisor : public WorkbenchAdvisor
{

public:

  UITestWorkbenchAdvisor();

  WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      IWorkbenchWindowConfigurer::Pointer configurer);

  QString GetInitialWindowPerspectiveId();

};

}

#endif /* BERRYUITESTWORKBENCHADVISOR_H_ */
