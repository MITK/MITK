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

#include "mitkIOUtil.h"
#include "mitkIOMimeTypes.h"
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
  CPPUNIT_TEST_SUITE_END();

  mitk::GeometryData::Pointer m_GeometryData;
  mitk::Geometry3D::Pointer m_Geom3D;

  mitk::GeometryData::Pointer m_ReadGeometryData;
  mitk::Geometry3D::Pointer m_ReadGeom3D;

public:

  void setUp() override
  {
    m_GeometryData = mitk::GeometryData::New();
    m_Geom3D = mitk::Geometry3D::New();

    m_GeometryData->SetGeometry( m_Geom3D ); // does copy!? doc says otherwise
    // --> we reference and use the clone, not the original!
    m_Geom3D = static_cast<mitk::Geometry3D*>( m_GeometryData->GetGeometry() );
  }

  void tearDown() override
  {
    m_GeometryData = nullptr;
    m_Geom3D = nullptr;
    m_ReadGeometryData = nullptr;
    m_ReadGeom3D = nullptr;
  }

  void StoreDefaultGeometry3D()
  {
    // TODO
    //m_Geom3D-> this and that
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

    m_Geom3D->SetFrameOfReferenceID( std::numeric_limits<unsigned int>::max() );
    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void StoreOrigin()
  {
    mitk::Point3D origin;
    mitk::FillVector3D(origin, std::numeric_limits<mitk::ScalarType>::min(), -52.723, +0.002 );
    m_Geom3D->SetOrigin(origin);
    ASSERT_Geometry3D_WriteReadLoop_Works();

    mitk::FillVector3D(origin, -0.0015, 78.81, std::numeric_limits<mitk::ScalarType>::max() );
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
    mitk::ScalarType matrixCoeffs[9] = {
      0.0, 1.1, 2.2,
      3.3, 4.4, 5.5,
      6.6, 7.7, 8.8
    };

    mitk::AffineTransform3D::MatrixType matrix;
    matrix.GetVnlMatrix().set( matrixCoeffs );

    mitk::AffineTransform3D::OffsetType offset;
    offset[0] = -918273645.1829374; // don't want limits::min(), there is a minimal difference for this value.
    //offset[1] = -0.0; // this causes problems with mitk::Equal
    offset[1] = 0.0;
    offset[2] = +918273645.1829374;

    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(offset);
    m_Geom3D->SetIndexToWorldTransform(transform);

    ASSERT_Geometry3D_WriteReadLoop_Works();
  }

  void ASSERT_Geometry3D_WriteReadLoop_Works()
  {
      // temp path
      std::string tmpPath = mitk::IOUtil::CreateTemporaryDirectory("GeomDataIOTest_XXXXXX");

      // let IOUtil find a good file extension
      std::string filename("geometrydata_geometry3d");
      std::string extension( mitk::IOMimeTypes::GEOMETRY_DATA_MIMETYPE().GetExtensions().front() );

      // don't specify the extension, expect that there is no other writer
      // TODO Other than documented, Save does not add the extension. Anything missing in my MIMETYPE??
      CPPUNIT_ASSERT_NO_THROW( mitk::IOUtil::Save( m_GeometryData, tmpPath + "/" + filename + "." + extension ) );

      // read into member
      std::vector<mitk::BaseData::Pointer> loadedData = mitk::IOUtil::Load( tmpPath + "/" + filename + "." + extension );
      CPPUNIT_ASSERT_MESSAGE("IOUtil could read something (and just one)", loadedData.size() == 1 );

      m_ReadGeometryData = dynamic_cast< mitk::GeometryData* >( loadedData.front().GetPointer() );
      CPPUNIT_ASSERT_MESSAGE("IOUtil could read _some_ GeometryData", m_ReadGeometryData.IsNotNull() );

      m_ReadGeom3D = dynamic_cast< mitk::Geometry3D* >( m_ReadGeometryData->GetGeometry() );
      CPPUNIT_ASSERT_MESSAGE("IOUtil could read _some_ Geometry3D", m_ReadGeom3D.IsNotNull() );

      // Doc of mitk::Equal for BaseGeometry says
      // "The function compares the spacing, origin, axis vectors, extents, the matrix of the
      //  IndexToWorldTransform(element wise), the bounding(element wise) and the ImageGeometry flag."
      // This seems pretty much everything that we can have in a Geometry3D..
      CPPUNIT_ASSERT_MESSAGE("Geometry3D > file > Geometry3D keeps geometry", mitk::Equal(*m_Geom3D, *m_ReadGeom3D, 0.000001, true));
      // Tolerance: Storing huge values 9.18274e+008 does not work at the precision of mitk::eps
      //            So the author of this test judged above tolerance sufficient. If more is
      //            required we need to inspect in more detail.
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkGeometryDataIO)
