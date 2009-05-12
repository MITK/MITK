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

#include "mitkToolManager.h"
#include "mitkCoreObjectFactory.h"

/// ctest entry point
int mitkToolManagerTest(int /*argc*/, char* /*argv*/[])
{
  // one big variable to tell if anything went wrong
    unsigned int numberFailed(0);
 
    std::cout << "Creating CoreObjectFactory" << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());

    std::cout << "Instantiation" << std::endl;

  // instantiation
    mitk::ToolManager::Pointer filter = mitk::ToolManager::New();
    if (filter.IsNotNull())
    {
      std::cout << "  (II) Instantiation works." << std::endl;
    }
    else
    {
      ++numberFailed;
      std::cout << numberFailed << " test failed, and it's the ugliest one!" << std::endl;
      return EXIT_FAILURE;
    }
  
    std::cout << "Object destruction" << std::endl;

  // freeing
    filter = NULL;
      
    std::cout << "  (II) Freeing works." << std::endl;

    if (numberFailed > 0)
    {
      std::cout << numberFailed << " tests failed." << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      std::cout << "PASSED all tests." << std::endl;
      return EXIT_SUCCESS;
    }
}


