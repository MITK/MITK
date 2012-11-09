/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKACTIONBARADVISOR_H_
#define QMITKACTIONBARADVISOR_H_

#include <berryActionBarAdvisor.h>


class QmitkActionBarAdvisor : public berry::ActionBarAdvisor
{
public:

  QmitkActionBarAdvisor(berry::IActionBarConfigurer::Pointer configurer);

protected:

  void FillMenuBar(void* menuBar);
};

#endif /*QMITKACTIONBARADVISOR_H_*/
