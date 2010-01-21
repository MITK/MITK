/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkDataTreeNodeFactory.h"
#include "mitkStandardFileLocations.h"

#include "mitkTestingMacros.h"

#include <locale>
#include <locale.h>
  
void mitkDICOMLocaleTestChangeLocale(const std::string& locale)
{
  try
  {
    MITK_TEST_OUTPUT(<< " ** Changing locale to '" << locale << "'");
    setlocale(LC_ALL, locale.c_str());
    std::locale l( locale.c_str() );
    std::cin.imbue(l);
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG(<< "Could not activate locale " << locale);
  }

}

void mitkDICOMLocaleTestWithReferenceImage()
{
  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(locator.IsNotNull(),"Instantiating StandardFileLocations");
  std::string filename = locator->FindFile("spacing-ok.dcm", "Core/Code/Testing/Data/");

  mitk::Image::Pointer image;
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  factory->SetFileName( filename );
  factory->Update();
  MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfOutputs() > 0, "file loaded");

  mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
  image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNull())
  {
    MITK_TEST_FAILED_MSG(<< "File "<< filename << " is not an image - test will not be applied." );
  
    return;
  }  
    
  MITK_TEST_OUTPUT(<< "File "<< filename << " could be loaded." );

  MITK_TEST_OUTPUT(<< "MITK image reports pixel spacing of " << image->GetGeometry()->GetSpacing()[0] << " " << image->GetGeometry()->GetSpacing()[1] );
  MITK_TEST_CONDITION_REQUIRED(image->GetGeometry()->GetSpacing()[0] - 0.3141592 < 0.0000001, "correct x spacing");
  MITK_TEST_CONDITION_REQUIRED(image->GetGeometry()->GetSpacing()[1] - 0.3141592 < 0.0000001, "correct y spacing");
}

int mitkDICOMLocaleTest(int /*argc*/, char* /*argv*/ [])
{
  MITK_TEST_BEGIN("DICOMLocaleTest");

  // this should already work correctly on all machines
  mitkDICOMLocaleTestChangeLocale("C");
  mitkDICOMLocaleTestWithReferenceImage();
  // this should fail on all machines
  mitkDICOMLocaleTestChangeLocale("de_DE.utf8");
  mitkDICOMLocaleTestWithReferenceImage();
 
  MITK_TEST_END();
}

