/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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

  std::string GetInitialWindowPerspectiveId();

};

}

#endif /* BERRYUITESTWORKBENCHADVISOR_H_ */
