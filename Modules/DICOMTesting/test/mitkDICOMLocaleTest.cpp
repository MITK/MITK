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

/*
 This test is meant to reproduce the following error:

 - The machine or current user has a German locale.
 - This esp. means that stream IO expects the decimal separator as a comma: ","
 - DICOM files use a point "." as the decimal separator to be locale independent
 - The parser used by MITK (ITK's GDCM) seems to use the current locale instead of the "C" or "POSIX" locale
 - This leads to spacings (and probably other numbers) being trimmed/rounded,
   e.g. the correct spacing of 0.314 is read as 1.0 etc.

*/

#include "mitkDataNodeFactory.h"
#include "mitkStandardFileLocations.h"
#include "mitkDicomSeriesReader.h"

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

void mitkDICOMLocaleTestWithReferenceImage(std::string filename)
{
  mitk::Image::Pointer image;
  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
  factory->SetFileName( filename );
  factory->Update();
  MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfOutputs() > 0, "file " << filename << " loaded");

  mitk::DataNode::Pointer node = factory->GetOutput( 0 );
  image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNull())
  {
    MITK_TEST_FAILED_MSG(<< "File "<< filename << " is not an image - test will not be applied." );

    return;
  }

  // note importance of minor differences in spacings:
  // DICOM has order y-spacing, x-spacing, while in MITK we assume x-spacing, y-spacing (both meant for 0 and 1 index in array)
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(image->GetGeometry()->GetSpacing()[0], 0.3141592), "correct x spacing? found "
                               << image->GetGeometry()->GetSpacing()[0]);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(image->GetGeometry()->GetSpacing()[1], 0.3411592), "correct y spacing? found "
                               << image->GetGeometry()->GetSpacing()[1]);
}

int mitkDICOMLocaleTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DICOMLocaleTest");

  MITK_TEST_CONDITION_REQUIRED(argc >= 2, "File to load has been specified on commandline");

  MITK_TEST_OUTPUT(<< "Configuration: \n" << mitk::DicomSeriesReader::GetConfigurationString() );

  std::string filename = argv[1];

  // load a reference DICOM file with the "C" locale being set
  mitkDICOMLocaleTestChangeLocale("C");
  mitkDICOMLocaleTestWithReferenceImage(filename);
  // load a reference DICOM file with German locales being set
  typedef std::list<std::string> StringList;
  StringList alllocales;
  alllocales.push_back("de_DE");
  alllocales.push_back("de_DE.utf8");
  alllocales.push_back("de_DE.UTF8");
  alllocales.push_back("de_DE@euro");
  alllocales.push_back("German_Germany");

  // supressing this test to be run on MacOS X
  // See bug #3894
#if defined (__APPLE__) || defined(MACOSX)
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
      mitkDICOMLocaleTestWithReferenceImage(filename);
    }
  }

  if(numberOfTestedGermanLocales == 0)
  {
    MITK_TEST_OUTPUT(<< "Warning: No German locale was found on the system.");
  }
  //MITK_TEST_CONDITION_REQUIRED( numberOfTestedGermanLocales > 0, "Verify that at least one German locale has been tested.");

  MITK_TEST_END();
}

