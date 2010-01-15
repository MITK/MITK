/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYUITESTWORKBENCHWINDOWADVISOR_H_
#define CHERRYUITESTWORKBENCHWINDOWADVISOR_H_

#include <cherryWorkbenchWindowAdvisor.h>

namespace cherry {

class UITestWorkbenchWindowAdvisor : public WorkbenchWindowAdvisor
{
public:

  UITestWorkbenchWindowAdvisor(IWorkbenchWindowConfigurer::Pointer configurer);
};

}

#endif /* CHERRYUITESTWORKBENCHWINDOWADVISOR_H_ */
