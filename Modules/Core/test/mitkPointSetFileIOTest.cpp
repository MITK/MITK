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
#include "mitkProportionalTimeGeometry.h"

#include <vector>
#include <itksys/SystemTools.hxx>
#include <time.h>
#include <random>

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

  mitk::PointSet::Pointer CreateTestPointSet(mitk::BaseGeometry* geometry = nullptr)
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    std::uniform_real_distribution<> r(0, 1);
    std::mt19937 gen;

    for (unsigned int t = 0; t < numberOfTimeSeries; t++)
    {
      for (unsigned int position = 0; position < 3; ++position)
      {
        mitk::Point3D point;
        mitk::FillVector3D(point, r(gen), r(gen), r(gen));
        pointSet->SetPoint(position, point, t);
      }
    }
    m_SavedPointSet = pointSet;

    // we set the geometry AFTER adding points. Else all points added via SetPoint would already bee transformed
    if (geometry != nullptr)
    {
      mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();
      timeGeometry->Initialize( geometry, numberOfTimeSeries );
      pointSet->SetTimeGeometry( timeGeometry );
    }

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

    // testing geometry
    MITK_TEST_CONDITION( mitk::Equal( *(pointSet1->GetGeometry()), *(pointSet2->GetGeometry()), 0.000001, true),
                         "Restored geometry must equal original one.");
  }

  bool PointSetWrite(mitk::BaseGeometry* geometry = nullptr)
  {
    try
    {
      m_SavedPointSet = NULL;

      std::ofstream tmpStream;
      m_FilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream) + ".mps";
      MITK_INFO << "PointSet test file at " << m_FilePath;
      mitk::IOUtil::Save(CreateTestPointSet(geometry), m_FilePath);
    }
    catch (std::exception& e)
    {
      MITK_ERROR << "Error during pointset creation: " << e.what();
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
    } catch (std::exception& e)
    {
      MITK_ERROR << "Error during pointset creation: " << e.what();
    }
  }

}; //mitkPointSetFileIOTestClass


int mitkPointSetFileIOTest(int, char*[])
{
  MITK_TEST_BEGIN("PointSet");

  // minimum test w/ identity geometry
  {
    mitkPointSetFileIOTestClass test;
    MITK_TEST_CONDITION(test.PointSetWrite(), "Testing if the PointSetWriter writes Data" );
    test.PointSetLoadAndCompareTest(); // load - compare
  }

  // case with a more complex geometry
  {
    mitkPointSetFileIOTestClass test;

    mitk::Geometry3D::Pointer g = mitk::Geometry3D::New();

    // define arbitrary transformation matrix
    // the number don't have much meaning - we just want them reproduced
    // by the writer/reader cycle
    mitk::BaseGeometry::BoundsArrayType bounds;
    bounds[0] = -918273645.18293746;
    bounds[1] = -52.723;
    bounds[2] = -1.002;
    bounds[3] = 918273645.18293746;
    bounds[4] = +1.002;
    bounds[5] = +52.723;
    g->SetBounds(bounds);

    mitk::ScalarType matrixCoeffs[9] = {
        0.0, 1.1, 2.2,
        3.3, 4.4, 5.5,
        6.6, 7.7, 8.8
    };

    mitk::AffineTransform3D::MatrixType matrix;
    matrix.GetVnlMatrix().set(matrixCoeffs);

    mitk::AffineTransform3D::OffsetType offset;
    offset[0] = -43.1829374;
    offset[1] = 0.0;
    offset[2] = +43.1829374;

    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(offset);
    g->SetIndexToWorldTransform(transform);

    MITK_TEST_CONDITION( test.PointSetWrite(g), "Testing if the PointSetWriter writes Data _with_ geometry" );
    test.PointSetLoadAndCompareTest(); // load - compare
  }


  MITK_TEST_END();
}

