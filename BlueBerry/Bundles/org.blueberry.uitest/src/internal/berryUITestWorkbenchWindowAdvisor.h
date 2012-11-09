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


#ifndef BERRYUITESTWORKBENCHWINDOWADVISOR_H_
#define BERRYUITESTWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>

namespace berry {

class UITestWorkbenchWindowAdvisor : public WorkbenchWindowAdvisor
{
public:

  UITestWorkbenchWindowAdvisor(IWorkbenchWindowConfigurer::Pointer configurer);
};

}

#endif /* BERRYUITESTWORKBENCHWINDOWADVISOR_H_ */
