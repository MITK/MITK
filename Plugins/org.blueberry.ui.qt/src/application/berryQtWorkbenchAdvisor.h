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


#ifndef BERRYQTWORKBENCHADVISOR_H_
#define BERRYQTWORKBENCHADVISOR_H_

#include <org_blueberry_ui_qt_Export.h>

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
};

}

#endif /* BERRYQTWORKBENCHADVISOR_H_ */
