/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


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
