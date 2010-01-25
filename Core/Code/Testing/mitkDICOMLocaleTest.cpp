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

/*
 This test is meant to reproduce the following error:
 
 - The machine or current user has a German locale.
 - This esp. means that stream IO expects the decimal separator as a comma: ","
 - DICOM files use a point "." as the decimal separator to be locale independent
 - The parser used by MITK (ITK's GDCM) seems to use the current locale instead of the "C" or "POSIX" locale
 - This leads to spacings (and probably other numbers) being trimmed/rounded,
   e.g. the correct spacing of 0.314 is read as 1.0 etc.

 MITK shold work around this behavior. This test is meant to verify any workaround
 and will be activated once such a workaround is available.

*/

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

  // load a reference DICOM file with the "C" locale being set
  mitkDICOMLocaleTestChangeLocale("C");
  mitkDICOMLocaleTestWithReferenceImage();
  // load a reference DICOM file with the German "de_DE.utf8" locale being set
  mitkDICOMLocaleTestChangeLocale("de_DE.utf8");
  mitkDICOMLocaleTestWithReferenceImage();
 
  MITK_TEST_END();
}

