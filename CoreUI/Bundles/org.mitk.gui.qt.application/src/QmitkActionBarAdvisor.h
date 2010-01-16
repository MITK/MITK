/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QMITKACTIONBARADVISOR_H_
#define QMITKACTIONBARADVISOR_H_

#include <berryActionBarAdvisor.h>

#include "mitkQtAppDll.h"

class MITK_QT_APP QmitkActionBarAdvisor : public berry::ActionBarAdvisor
{
public:

  QmitkActionBarAdvisor(berry::IActionBarConfigurer::Pointer configurer);

protected:

  void FillMenuBar(void* menuBar);
};

#endif /*QMITKACTIONBARADVISOR_H_*/
