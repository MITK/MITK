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


#ifndef CHERRYQTWORKBENCHADVISOR_H_
#define CHERRYQTWORKBENCHADVISOR_H_

#include "../cherryUiQtDll.h"

#include <application/cherryWorkbenchAdvisor.h>

namespace cherry
{

class CHERRY_UI_QT QtWorkbenchAdvisor : public WorkbenchAdvisor
{

public:

  /**
   * Creates the global QApplication object
   */
  void Initialize(IWorkbenchConfigurer::Pointer configurer);
};

}

#endif /* CHERRYQTWORKBENCHADVISOR_H_ */
