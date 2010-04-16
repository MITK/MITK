/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fri, 12 Mar 2010) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTestingMacros.h"
#include "mitkCoreExtObjectFactory.h"
#include "mitkCoreObjectFactory.h"

int mitkCoreExtObjectFactoryTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("CoreExtObjectFactory")
  
  mitk::CoreObjectFactory::Pointer instance = mitk::CoreObjectFactory::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(instance.IsNotNull(),"Testing instantiation");
  MITK_TEST_CONDITION(strcmp(instance->GetNameOfClass(),"CoreObjectFactory") == 0,"Is this a CoreObjectFactory?");

  instance->RegisterExtraFactory(mitk::CoreExtObjectFactory::New());
 
  MITK_TEST_END()
}
