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

#include "mitkPointSet.h"
#include "mitkStandardFileLocations.h"
#include "mitkIOUtil.h"
#include "mitkTestingMacros.h"
#include <list>
#include <fstream>
#include <iostream>
#include <string>

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

void ReaderLocaleTest(mitk::Point3D & refPoint, std::string filename)
{
  MITK_TEST_OUTPUT(<< "---- Reader Test ---- ");

  mitk::PointSet::Pointer pointSet = mitk::IOUtil::LoadPointSet(filename);

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

void WriterLocaleTest(mitk::Point3D & refPoint, std::string filename)
{
  MITK_TEST_OUTPUT(<< "---- Writer Test---- ");
  //create pointset
  mitk::PointSet::Pointer refPointSet = mitk::PointSet::New();
  refPointSet->InsertPoint(0,refPoint);
  //SetPoint(0, refPoint);

  std::ofstream tmpStream;
  std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream);

  // write point set
  mitk::IOUtil::Save(refPointSet, tmpFilePath);
  tmpStream.close();

  std::ifstream stream(tmpFilePath.c_str());

  //compare two .mps files
  std::ifstream refStream (filename.c_str());

  MITK_TEST_CONDITION_REQUIRED(refStream,"Read reference point set");
  MITK_TEST_CONDITION_REQUIRED(stream,"Read point set");

  bool differ = false;
  if (stream.is_open() && refStream.is_open())
  {
   std::string streamLine;
   std::string refStreamLine;
    while(!stream.eof() && ! refStream.eof())
    {
      getline(stream, streamLine);
      getline(refStream, refStreamLine);
      if(streamLine.compare(refStreamLine) != 0)
      {
        differ = true;
          break;
       }
    }
    stream.close();
    refStream.close();
  }
  MITK_TEST_CONDITION_REQUIRED(!differ, "Write point set correct");

}

int mitkPointSetLocaleTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("PointSetLocaleTest");

  if (argc<2) {MITK_TEST_FAILED_MSG(<<"Error: test file name is needed as second argument.");}


  std::string filename = argv[1];

  MITK_INFO << filename;

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

  // QuickFix for MAC OS X
  // See for more the Bug #3894 comments
#if defined (__APPLE__) || defined(MACOSX)
  alllocales.push_back("C");
#endif

  unsigned int numberOfTestedGermanLocales(0);
  for (StringList::iterator iter = alllocales.begin();
       iter != alllocales.end();
       ++iter)
  {
    if ( ChangeLocale(*iter) )
    {
     ++numberOfTestedGermanLocales;
     WriterLocaleTest(refPoint,filename);
     ReaderLocaleTest(refPoint,filename);
    }
  }

  if(numberOfTestedGermanLocales == 0)
  {
    MITK_TEST_OUTPUT(<< "Warning: No German locale was found on the system.");
  }
  //MITK_TEST_CONDITION_REQUIRED( numberOfTestedGermanLocales > 0, "Verify that at least one German locale has been tested.");
  MITK_TEST_END();
}

