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


#ifndef MITKTESTMODULEA2SERVICE_H
#define MITKTESTMODULEA2SERVICE_H

#include <mitkServiceInterface.h>

namespace mitk {

struct TestModuleA2Service
{
  virtual ~TestModuleA2Service() {}
};

}

MITK_DECLARE_SERVICE_INTERFACE(mitk::TestModuleA2Service, "org.mitk.TestModuleA2Service")

#endif // MITKTESTMODULEA2SERVICE_H
