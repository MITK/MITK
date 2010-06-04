/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkDataNodeFactory.h"
#include "mitkStandardFileLocations.h"

#include "mitkTestingMacros.h"

#include <list>
#include <locale>
#include <locale.h>
  
bool mitkDICOMLocaleTestChangeLocale(const std::string& locale)
{
  try
  {
    MITK_TEST_OUTPUT(<< " ** Changing locale from " << setlocale(LC_ALL, NULL) << " to '" << locale << "'");
    setlocale(LC_ALL, locale.c_str());
    std::locale l( locale.c_str() );
    std::cin.imbue(l);
    return true;
  }
  catch(...)
  {
    MITK_TEST_OUTPUT(<< "Could not activate locale " << locale);
    return false;
  }

}

void mitkDICOMLocaleTestWithReferenceImage()
{
  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(locator.IsNotNull(),"Instantiating StandardFileLocations");
  std::string filename = locator->FindFile("spacing-ok.dcm", "Core/Code/Testing/Data/");

  mitk::Image::Pointer image;
  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
  factory->SetFileName( filename );
  factory->Update();
  MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfOutputs() > 0, "file loaded");

  mitk::DataNode::Pointer node = factory->GetOutput( 0 );
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
  // load a reference DICOM file with German locales being set
  typedef std::list<std::string> StringList;
  StringList alllocales;
  alllocales.push_back("de_DE");
  alllocales.push_back("de_DE.utf8");
  alllocales.push_back("de_DE.UTF8");
  alllocales.push_back("de_DE@euro");
  alllocales.push_back("German_Germany");

  // QuickFix for MAC OS X
  // See for more the Bug #3894 comments
#ifdef __APPLE__ #if defined (__APPLE__) || defined(MACOSX)
  alllocales.push_back("C");
#endif

  unsigned int numberOfTestedGermanLocales(0);

  for (StringList::iterator iter = alllocales.begin();
       iter != alllocales.end();
       ++iter)
  {
    if ( mitkDICOMLocaleTestChangeLocale(*iter) )
    {
      ++numberOfTestedGermanLocales;
      mitkDICOMLocaleTestWithReferenceImage();
    }
  }

  MITK_TEST_CONDITION_REQUIRED( numberOfTestedGermanLocales > 0, "Verify that at least one German locale has been tested.");
 
  MITK_TEST_END();
}

