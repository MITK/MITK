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
#include <mitkTestingMacros.h>
#include "mitkTestingConfig.h"
#include <mitkContourModelElement.h>
#include <iostream>
#include <fstream>
#include <locale>
#include <time.h>


typedef long long int64; typedef unsigned long long uint64;

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */

int64 GetTimeMs64()
{
#ifdef WIN32
 /* Windows */
 FILETIME ft;
 LARGE_INTEGER li;

 /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
  * to a LARGE_INTEGER structure. */
 GetSystemTimeAsFileTime(&ft);
 li.LowPart = ft.dwLowDateTime;
 li.HighPart = ft.dwHighDateTime;

 uint64 ret = li.QuadPart;
 ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
 //ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */
 ret /= 10; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

 return ret;
#else
 /* Linux */
 struct timeval tv;

 gettimeofday(&tv, NULL);

 uint64 ret = tv.tv_usec;
 /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
 ret /= 1000;

 /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
 ret += (tv.tv_sec * 1000);

 return ret;
#endif
}




#define ClockVariables int64 startJob, stopJob;\
  int64 durationJob;

#define CalculateExecution(expr) startJob = GetTimeMs64();\
  expr;\
  stopJob = GetTimeMs64();\
  durationJob = (stopJob - startJob);



//Test ContourModelElement performance
static bool TestANN()
{
  int64 startJob, stopJob, durationJob;

  std::string fileName = std::string( MITK_TEST_OUTPUT_DIR ) + "/ANN-test.csv";

  std::ofstream out( fileName.c_str() );
  if ( !out.good() )
  {
    out.close();
    return false;
  }

  std::locale previousLocale(out.getloc());
  std::locale I("C");
  out.imbue(I);



  mitk::ContourModelElement::Pointer contour = mitk::ContourModelElement::New();


  //worst case query
  mitk::Point3D query;
  query[0] = query[1] = query[2] = -100;

  out << "BruteForce" << std::endl;

  for(int step = 0; step < 20000; step++)
  {
    out << "Step: " << step << std::endl;

    for(int i=0; i<10; i++)
    {
      startJob = GetTimeMs64();
      mitk::ContourModelElement::VertexType* v = contour->BruteForceGetVertexAt(query,0.001);
      stopJob = GetTimeMs64();
      durationJob = (stopJob - startJob);
      out << durationJob << " ";
    }
    mitk::Point3D p;
    p[0] = p[1] = p[2] = 1;
    contour->AddVertex(p,false);

    out << std::endl;
  }


  out.imbue(previousLocale);

    if ( !out.good() ) // some error during output
    {
      out.close();
      throw std::ios_base::failure("Some error during point set writing.");
    }
 
    out.close();

    return true;
}



int mitkContourModelElementTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkContourModelElementTest")

    MITK_TEST_CONDITION( TestANN(), "write test");

  MITK_TEST_END()
}