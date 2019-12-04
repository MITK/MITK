/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKACTIONBARADVISOR_H_
#define QMITKACTIONBARADVISOR_H_

#include <berryActionBarAdvisor.h>


class QmitkActionBarAdvisor : public berry::ActionBarAdvisor
{
public:

  QmitkActionBarAdvisor(const berry::SmartPointer<berry::IActionBarConfigurer>& configurer);

protected:

  void FillMenuBar(berry::IMenuManager* menuBar) override;
};

#endif /*QMITKACTIONBARADVISOR_H_*/
