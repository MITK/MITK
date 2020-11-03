/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
  void Initialize(IWorkbenchConfigurer::Pointer configurer) override;
};

}

#endif /* BERRYQTWORKBENCHADVISOR_H_ */
