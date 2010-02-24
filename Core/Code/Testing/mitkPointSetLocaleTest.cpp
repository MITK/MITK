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

#include "mitkPointSetReader.h"
#include "mitkPointSetWriter.h"
#include "mitkStandardFileLocations.h"
#include "mitkTestingMacros.h"
#include <list>
#include <fstream>

bool ChangeLocale(const std::string& locale)
{
  try
  {
    MITK_TEST_OUTPUT(<< "\n** Changing locale from " << setlocale(LC_ALL, NULL) << " to '" << locale << "'");
     setlocale(LC_ALL, locale.c_str());

    std::locale l( locale.c_str() );
    std::cin.imbue(l);
    std::cout.imbue(l);

     return true;

 }
  catch(...)
  {
    MITK_TEST_OUTPUT(<< "Could not activate locale" << locale << "\n");
    return false;
  }
}

void ReaderLocaleTest(mitk::Point3D & refPoint)
{
  MITK_TEST_OUTPUT(<< "---- Reader Test ---- ");
  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(locator.IsNotNull(),"Instantiating StandardFileLocations");
  std::string filename = locator->FindFile("pointSet.mps", "Core/Code/Testing/Data/");

  mitk::PointSetReader::Pointer reader = mitk::PointSetReader::New();
  reader -> SetFileName(filename);
  reader -> Update();
  mitk::PointSet::Pointer pointSet = reader -> GetOutput();

  mitk::Point3D point;
  if (pointSet->GetPointIfExists(0, &point))
  {
    MITK_TEST_CONDITION_REQUIRED(fabs(refPoint[0] - point[0]) < 0.00001, "read x correct");
    MITK_TEST_CONDITION_REQUIRED(fabs(refPoint[1] - point[1]) < 0.00001, "read y correct");
    MITK_TEST_CONDITION_REQUIRED(fabs(refPoint[2] - point[2]) < 0.00001, "read z correct");
  }else
  {
    MITK_TEST_FAILED_MSG(<< "File "<< filename << " can not be read - test will not applied." );
     return;
  }
}

void WriterLocaleTest(mitk::Point3D & refPoint)
{
  MITK_TEST_OUTPUT(<< "---- Writer Test---- ");
  //create pointset
  mitk::PointSet::Pointer refPointSet = mitk::PointSet::New();
  refPointSet->SetPoint(0, refPoint);

  //create locator
  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(locator.IsNotNull(),"Instantiating StandardFileLocations");
  std::string filename = locator->FindFile("pointSet.mps", "Core/Code/Testing/Data/");
  std::string testFileName = "testPointSet.mps";

  // write point set
  mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
  writer -> SetFileName(testFileName);
  writer -> SetInput(refPointSet);
  writer -> Write();

  //compare two .mps files
  std::ifstream refStream (filename.c_str());
  std::ifstream stream (testFileName.c_str());

  MITK_TEST_CONDITION_REQUIRED(refStream,"Read reference point set");
  MITK_TEST_CONDITION_REQUIRED(stream,"Read point set");

  refStream.seekg (0, ios::end);
  int length = refStream.tellg();
  refStream.seekg(0, ios::beg);

  char refBuffer[length];
  char buffer[length];

  refStream.read(refBuffer, length);
  stream.read(buffer, length);

  bool differ = false;
  for (int i = 0; i < length; i++)
    if (refBuffer[i] != buffer[i])
    {

      differ = true;
        return;
    }

  MITK_TEST_CONDITION_REQUIRED(!differ, "Writer Test");
}

int mitkPointSetLocaleTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PointSetLocaleTest");

  //create reference point set
  mitk::PointSet::Pointer refPointSet = mitk::PointSet::New();
  mitk::Point3D refPoint;
  refPoint[0] = 32.2946;
  refPoint[1] = -17.7359;
  refPoint[2] = 29.6502;
  refPointSet->SetPoint(0, refPoint);

  //create locale list
  std::ofstream stream;
  std::locale previousLocale(stream.getloc());

  typedef std::list<std::string> StringList;
  StringList alllocales;
  alllocales.push_back("de_DE");
  alllocales.push_back("de_DE.utf8");
  alllocales.push_back("de_DE.UTF-8");
  alllocales.push_back("de_DE@euro");
  alllocales.push_back("German_Germany");

  unsigned int numberOfTestedGermanLocales(0);
  for (StringList::iterator iter = alllocales.begin();
       iter != alllocales.end();
       ++iter)
  {
    if ( ChangeLocale(*iter) )
    {
      ++numberOfTestedGermanLocales;
     ReaderLocaleTest(refPoint);
     WriterLocaleTest(refPoint);
    }
  }
  MITK_TEST_CONDITION_REQUIRED( numberOfTestedGermanLocales > 0, "Verify that at least one German locale has been tested.");
  MITK_TEST_END();
}

