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

#include <vtkProperty.h>
#include "mitkEnumerationProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"


int mitkEnumerationPropertyTest( int /*argc*/, char* /*argv*/[] )
{
  mitk::EnumerationProperty::Pointer enumerationProperty(mitk::EnumerationProperty::New());

  std::cout << "Testing mitk::EnumerationProperty::AddEnum(...): ";
  bool success = true;
  success = success && enumerationProperty->AddEnum( "first", 1 );
  success = success && enumerationProperty->AddEnum( "second", 2 );
  success = success && enumerationProperty->AddEnum( "third", 3 );
  if ( ! success )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  
  
  
  
  std::cout << "Testing mitk::EnumerationProperty::Size(): " ;
  if ( enumerationProperty->Size() != 3 )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing mitk::EnumerationProperty::AddEnum() with invalid entries: ";
  if ( enumerationProperty->AddEnum( "first", 0 ) )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  
  
  
  
  std::cout << "Testing mitk::EnumerationProperty::SetValue(id): ";
  if ( ! enumerationProperty->SetValue( 2 ) )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( enumerationProperty->GetValueAsId() != 2 )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( enumerationProperty->GetValueAsString() != "second" )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  
  
  
  
  std::cout << "Testing mitk::EnumerationProperty::SetValue(name): ";
  if ( ! enumerationProperty->SetValue( "third" ) )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( enumerationProperty->GetValueAsId() != 3 )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( enumerationProperty->GetValueAsString() != "third" )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  
  
  
  
  std::cout << "Testing mitk::EnumerationProperty::SetValue(invalid id): ";
  if ( enumerationProperty->SetValue( 100 ) )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing mitk::EnumerationProperty::SetValue(invalid name): ";
  if ( enumerationProperty->SetValue( "madmax" ) )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;


  
  
  
  
  std::cout << "Testing mitk::VtkInterpolationType::SetInterpolationToPhong(): ";
  mitk::VtkInterpolationProperty::Pointer vtkInterpolationProperty(mitk::VtkInterpolationProperty::New());;
  vtkInterpolationProperty->SetInterpolationToPhong();
  if ( vtkInterpolationProperty->GetValueAsString() != "Phong" )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( vtkInterpolationProperty->GetValueAsId() != 2 )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( vtkInterpolationProperty->GetVtkInterpolation() != VTK_PHONG )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  
  
  
  
  std::cout << "Testing mitk::VtkRepresentationType::SetRepresentationToWireframe(): ";
  mitk::VtkRepresentationProperty::Pointer vtkRepresentationProperty(mitk::VtkRepresentationProperty::New());
  vtkRepresentationProperty->SetRepresentationToWireframe();
  if ( vtkRepresentationProperty->GetValueAsString() != "Wireframe" )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( vtkRepresentationProperty->GetValueAsId() != 1 )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  if ( vtkRepresentationProperty->GetVtkRepresentation() != VTK_WIREFRAME )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
