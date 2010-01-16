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


#ifndef BERRYQTWORKBENCHADVISOR_H_
#define BERRYQTWORKBENCHADVISOR_H_

#include "../berryUiQtDll.h"

#include <berryWorkbenchAdvisor.h>

namespace berry
{

class BERRY_UI_QT QtWorkbenchAdvisor : public WorkbenchAdvisor
{

public:

  /**
   * Creates the global QApplication object
   */
  void Initialize(IWorkbenchConfigurer::Pointer configurer);

  bool PreShutdown();
};

}

#endif /* BERRYQTWORKBENCHADVISOR_H_ */
