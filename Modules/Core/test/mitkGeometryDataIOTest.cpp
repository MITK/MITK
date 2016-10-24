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

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkGeometry3D.h"
#include "mitkGeometryData.h"
#include "mitkProportionalTimeGeometry.h"

#include "mitkIOMimeTypes.h"
#include "mitkIOUtil.h"
/**
 \brief Reader/Writer test for GeometryData (via IOUtil).

 Tests whether the reader and writer for GeometryData
 - is existent and accessible via IOUtil
 - is able to restore some geometries via a loop of original geometry, writer, file, reader, restored geometry

*/
class mitkGeometryDataIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGeometryDataIOTestSuite);
  MITK_TEST(StoreDefaultGeometry3D);
  MITK_TEST(StoreImageGeometry);
  MITK_TEST(StoreFrameOfReference);
  MITK_TEST(StoreOrigin);
  MITK_TEST(StoreSpacing);
  MITK_TEST(StoreBounds);
  MITK_TEST(StoreTransform);
  MITK_TEST(StoreProportionalTimeGeometry);
  CPPUNIT_TEST_SUITE_END();

  mitk::GeometryData::Pointer m_GeometryData;
  mitk::Geometry3D::Pointer m_Geom3D;
  mitk::ProportionalTimeGeometry::Pointer m_TimeGeometry;

  mitk::GeometryData::Pointer m_ReadGeometryData;
  mitk::Geometry3D::Pointer m_ReadGeom3D;
  mitk::ProportionalTimeGeometry::Pointer m_ReadProportionalTimeGeom;

public:
  void setUp() override
  {
    m_GeometryData = mitk::GeometryData::New();
    m_Geom3D = mitk::Geometry3D::New();

    m_GeometryData->SetGeometry(m_Geom3D); // does copy!? doc says otherwise
    // --> we reference and use the clone, not the original!
    m_Geom3D = static_cast<mitk::Geometry3D *>(m_GeometryData->GetGeometry());

    m_TimeGeometry = dynamic_cast<mitk::ProportionalTimeGeometry *>(m_GeometryData->GetTimeGeometry());
    CPPUNIT_ASSERT(m_TimeGeometry.IsNotNull());
  }

  void tearDown() override
  {
    m_GeometryData = nullptr;
    m_Geom3D = nullptr;
    m_ReadGeometryData = nullptr;
    m_ReadGeom3D = nullptr;
    m_ReadProportionalTimeGeom = nullptr;
  }

  void StoreDefaultGeometry3D()
  {
    // m_Geom3D already prepared in setUp()
    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void StoreImageGeometry()
  {
    m_Geom3D->SetImageGeometry(true);
    ASSERT_Geometry3D_WriteReadLoop_Works();

    m_Geom3D->SetImageGeometry(false);
    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void StoreFrameOfReference()
  {
    m_Geom3D->SetFrameOfReferenceID(513);
    ASSERT_Geometry3D_WriteReadLoop_Works();

    m_Geom3D->SetFrameOfReferenceID(std::numeric_limits<unsigned int>::max());
    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void StoreOrigin()
  {
    mitk::Point3D origin;
    mitk::FillVector3D(origin, std::numeric_limits<mitk::ScalarType>::min(), -52.723, +0.002);
    m_Geom3D->SetOrigin(origin);
    ASSERT_Geometry3D_WriteReadLoop_Works();

    mitk::FillVector3D(origin, -0.0015, 78.81, std::numeric_limits<mitk::ScalarType>::max());
    m_Geom3D->SetOrigin(origin);
    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void StoreSpacing()
  {
    mitk::Vector3D spacing;
    // tempted to test with spacing of std::numeric_limits<mitk::ScalarType>::max()
    // BUT that puts TinyXML into a pretty long loop counting the required numbers needed
    // So we test a _big_ spacing
    mitk::FillVector3D(spacing, 0.0000001, 0.0017, 918273645.18293746);
    m_Geom3D->SetSpacing(spacing);
    ASSERT_Geometry3D_WriteReadLoop_Works();

    // tempted to test negative spacings? they should be serialized, but
    // there are assertions somewhere in the code that verify positive
    // spacings, so we can skip this test.
  }

  void StoreBounds()
  {
    mitk::BaseGeometry::BoundsArrayType bounds;
    bounds[0] = std::numeric_limits<mitk::ScalarType>::min();
    bounds[1] = -52.723;
    bounds[2] = -0.002;
    bounds[3] = 918273645.18293746;
    bounds[4] = -0.002;
    bounds[5] = +52.723;
    m_Geom3D->SetBounds(bounds);
    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void StoreTransform()
  {
    mitk::ScalarType matrixCoeffs[9] = {0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};

    mitk::AffineTransform3D::MatrixType matrix;
    matrix.GetVnlMatrix().set(matrixCoeffs);

    mitk::AffineTransform3D::OffsetType offset;
    offset[0] = -918273645.1829374; // don't want limits::min(), there is a minimal difference for this value.
    // offset[1] = -0.0; // this causes problems with mitk::Equal
    offset[1] = 0.0;
    offset[2] = +918273645.1829374;

    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(offset);
    m_Geom3D->SetIndexToWorldTransform(transform);

    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void WriteAndRead_m_GeometryData()
  {
    // temp path
    std::string tmpPath = mitk::IOUtil::CreateTemporaryDirectory("GeomDataIOTest_XXXXXX");

    // let IOUtil find a good file extension
    std::string filename("geometrydata_geometry3d");
    std::string extension(mitk::IOMimeTypes::GEOMETRY_DATA_MIMETYPE().GetExtensions().front());

    // don't specify the extension, expect that there is no other writer
    // TODO Other than documented, Save does not add the extension. Anything missing in my MIMETYPE??
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(m_GeometryData, tmpPath + "/" + filename + "." + extension));

    // read into member
    std::vector<mitk::BaseData::Pointer> loadedData = mitk::IOUtil::Load(tmpPath + "/" + filename + "." + extension);
    CPPUNIT_ASSERT_MESSAGE("IOUtil could read something (and just one)", loadedData.size() == 1);

    m_ReadGeometryData = dynamic_cast<mitk::GeometryData *>(loadedData.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("IOUtil could read _some_ GeometryData", m_ReadGeometryData.IsNotNull());

    m_ReadGeom3D = dynamic_cast<mitk::Geometry3D *>(m_ReadGeometryData->GetGeometry());
    CPPUNIT_ASSERT_MESSAGE("IOUtil could read _some_ Geometry3D", m_ReadGeom3D.IsNotNull());

    m_ReadProportionalTimeGeom = dynamic_cast<mitk::ProportionalTimeGeometry *>(m_ReadGeometryData->GetTimeGeometry());
    CPPUNIT_ASSERT_MESSAGE("IOUtil could read _some_ ProportionalTimeGeometry", m_ReadProportionalTimeGeom.IsNotNull());
  }

  void ASSERT_Geometry3D_WriteReadLoop_Works()
  {
    WriteAndRead_m_GeometryData();
    // Doc of mitk::Equal for BaseGeometry says
    // "The function compares the spacing, origin, axis vectors, extents, the matrix of the
    //  IndexToWorldTransform(element wise), the bounding(element wise) and the ImageGeometry flag."
    // This seems pretty much everything that we can have in a Geometry3D..
    CPPUNIT_ASSERT_MESSAGE("Geometry3D > file > Geometry3D keeps geometry",
                           mitk::Equal(*m_Geom3D, *m_ReadGeom3D, 0.000001, true));
    // Tolerance: Storing huge values 9.18274e+008 does not work at the precision of mitk::eps
    //            So the author of this test judged above tolerance sufficient. If more is
    //            required we need to inspect in more detail.
  }

  void StoreProportionalTimeGeometry()
  {
    // Set Time Geometry
    m_TimeGeometry->ClearAllGeometries(); // remove default filling from setUp();

    for (int t = 0; t < 4; ++t)
    {
      // add new time steps
      mitk::Geometry3D::Pointer timestepGeometry = mitk::Geometry3D::New();
      vtkSmartPointer<vtkMatrix4x4> vtk_matrix = vtkSmartPointer<vtkMatrix4x4>::New();
      for (int i = 0; i != 4; ++i)
      {
        for (int j = 0; j != 4; ++j)
        {
          vtk_matrix->SetElement(i, j, t + (i + j) / 8.0); // just insignificant values
        }
      }

      if (t % 2 == 0) // invert every second one
        vtk_matrix->Invert();

      timestepGeometry->SetIndexToWorldTransformByVtkMatrix(vtk_matrix);
      m_TimeGeometry->SetTimeStepGeometry(timestepGeometry, t);
    }

    // time steps are handled as ScalarType, so use some negative values
    m_TimeGeometry->SetFirstTimePoint(-5017.20);
    m_TimeGeometry->SetStepDuration(2743.83);

    // fill m_GeometryData with something that has multiple time steps
    ASSERT_ProportionalTimeGeometry_WriteReadLoop_Works();
  }

  void ASSERT_ProportionalTimeGeometry_WriteReadLoop_Works()
  {
    WriteAndRead_m_GeometryData();

    CPPUNIT_ASSERT_MESSAGE("ProportionalTimeGeometry > file > ProportionalTimeGeometry keeps geometry",
                           mitk::Equal(*m_TimeGeometry, *m_ReadProportionalTimeGeom, 0.000001, true));
    // Tolerance: Storing huge values 9.18274e+008 does not work at the precision of mitk::eps
    //            So the author of this test judged above tolerance sufficient. If more is
    //            required we need to inspect in more detail.
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkGeometryDataIO)
