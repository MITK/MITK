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
#include "mitkTestingMacros.h"
#include "mitkFileWriterRegistry.h"
#include "mitkIOUtil.h"

#include <vector>
#include <itksys/SystemTools.hxx>
#include <time.h>

//unsigned int numberOfTestPointSets = 1;
unsigned int numberOfTimeSeries = 5;

// create one test PointSet
class mitkPointSetFileIOTestClass
{
public:

  mitk::PointSet::Pointer m_SavedPointSet;
  std::string m_FilePath;

  mitkPointSetFileIOTestClass()
  {

  }

  ~mitkPointSetFileIOTestClass()
  {
    if (!m_FilePath.empty())
    {
      std::remove(m_FilePath.c_str());
    }
  }

  mitk::PointSet::Pointer CreateTestPointSet()
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

    for (unsigned int t = 0; t < numberOfTimeSeries; t++)
    {
      unsigned int position(0);
      mitk::Point3D point;
      mitk::FillVector3D(point, (rand()%1000) /1000.0 , (rand()%1000) /1000.0, (rand()%1000)/1000.0);
      pointSet->SetPoint(position, point, t);

      mitk::FillVector3D(point, (rand()%1000) /1000.0 , (rand()%1000) /1000.0, (rand()%1000)/1000.0);
      ++position;
      pointSet->SetPoint(position, point, t);

      mitk::FillVector3D(point, (rand()%1000) /1000.0 , (rand()%1000) /1000.0, (rand()%1000)/1000.0);
      ++position;
      pointSet->SetPoint(position, point, t);
    }
    m_SavedPointSet = pointSet;

    return pointSet;

  }

  void PointSetCompare(mitk::PointSet::Pointer pointSet2,
      mitk::PointSet::Pointer pointSet1, bool& /*identical*/)
  {

    MITK_TEST_CONDITION(pointSet1->GetSize() == pointSet2->GetSize(), "Testing if PointSet size is correct" );

    for (unsigned int t = 0; t < numberOfTimeSeries; t++)
    {
      for (unsigned int i = 0; i < (unsigned int) pointSet1->GetSize(t); ++i)
      {
        mitk::Point3D p1 = pointSet1->GetPoint(i);
        mitk::Point3D p2 = pointSet2->GetPoint(i);

          //test
          std::cout << "r point: " << p2 << std::endl;
          std::cout << "w point: " << p1 << std::endl;

          //test end

        MITK_TEST_CONDITION((p1[0] - p2[0]) <= 0.0001, "Testing if X coordinates of the Point are at the same Position" );
        MITK_TEST_CONDITION((p1[1] - p2[1]) <= 0.0001, "Testing if Y coordinates of the Point are at the same Position" );
        MITK_TEST_CONDITION((p1[2] - p2[2]) <= 0.0001, "Testing if Z coordinates of the Point are at the same Position" );

      }
    }

  }

  bool PointSetWrite()
  {
    try
    {
      m_SavedPointSet = NULL;

      std::ofstream tmpStream;
      m_FilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream);
      mitk::IOUtil::Save(CreateTestPointSet(), m_FilePath);
    }
    catch (std::exception& /*e*/)
    {
      return false;
    }

    return true;
  }

  void PointSetLoadAndCompareTest()
  {
    try
    {
      mitk::PointSet::Pointer pointSet = mitk::IOUtil::LoadPointSet(m_FilePath);
      MITK_TEST_CONDITION(pointSet.IsNotNull(), "Testing if the loaded Data are NULL" );

      bool identical(true);
      PointSetCompare(pointSet.GetPointer(), m_SavedPointSet.GetPointer(),
                      identical);
    } catch (std::exception& /*e*/)
    {
    }
  }

}; //mitkPointSetFileIOTestClass


int mitkPointSetFileIOTest(int, char*[])
{
  MITK_TEST_BEGIN("PointSet");

  mitkPointSetFileIOTestClass* test = new mitkPointSetFileIOTestClass();

  // write
  MITK_TEST_CONDITION(test->PointSetWrite(), "Testing if the PointSetWriter writes Data" );

  // load - compare
  test->PointSetLoadAndCompareTest();

  //Delete correctly
  delete test;

  MITK_TEST_END();
}

