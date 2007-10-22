/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkOrganTypeProperty.h"
#include "mitkCoreObjectFactory.h"

/// ctest entry point
int mitkOrganTypePropertyTest(int /*argc*/, char* /*argv*/[])
{
    const char* NEW_ORGAN = "Der Knopf von der Eingangstuer von der Siebenschlaeferbehausung ist aus Messing damit es nicht so schnell rostet";

  // one big variable to tell if anything went wrong
    unsigned int numberFailed(0);
 
    std::cout << "Creating CoreObjectFactory" << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());

    std::cout << "Instantiation" << std::endl;

  // instantiation
    mitk::OrganTypeProperty::Pointer prop1 = new mitk::OrganTypeProperty;
    if (prop1.IsNotNull())
    {
      std::cout << "  (II) Instantiation works." << std::endl;
    }
    else
    {
      ++numberFailed;
      std::cout << numberFailed << " test failed, and it's the ugliest one!" << std::endl;
      return EXIT_FAILURE;
    }

    mitk::OrganTypeProperty::Pointer prop2 = new mitk::OrganTypeProperty;
    if (prop1.IsNotNull())
    {
      std::cout << "  (II) Instantiation still works." << std::endl;
    }
    else
    {
      ++numberFailed;
      std::cout << numberFailed << " test failed, and it's the ugliest one!" << std::endl;
      return EXIT_FAILURE;
    }
   
  // add an allowed value to prop1, test if it is now valid for prop2, too
    prop1->AddEnum( NEW_ORGAN, prop1->Size() );

    if ( !prop1->IsValidEnumerationValue( NEW_ORGAN ) )
    {
      ++numberFailed;
      std::cout << "  (EE) New organ is not a valid enumeration type after adding calling AddEnum()." << std::endl;
    }
    else
    {
      std::cout << "  (II) AddEnum() works." << std::endl;
    }

    if ( !prop2->IsValidEnumerationValue( NEW_ORGAN ) )
    {
      ++numberFailed;
      std::cout << "  (EE) New organ type did not propagate to a second instance of OrganTypeProperty." << std::endl;
    }

    std::cout << "Object destruction" << std::endl;

  // freeing
    prop1 = NULL;
    std::cout << "  (II) Freeing works for first property." << std::endl;
    prop2 = NULL;
      
    std::cout << "  (II) Freeing works for both properties." << std::endl;

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

