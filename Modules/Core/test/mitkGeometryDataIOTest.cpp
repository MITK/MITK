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
#include "mitkException.h"
#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"

#include "mitkGeometry3D.h"
#include "mitkGeometryDataReaderService.h"
#include "mitkCustomMimeType.h"

#include "mitkIOUtil.h"
#include "mitkIOMimeTypes.h"

#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

#ifdef WIN32
#include "process.h"
#else
#include <unistd.h>
#endif

class mitkGeometryDataIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGeometryDataIOTestSuite);
  MITK_TEST(StoreDefaultGeometry3D);
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
    m_GeometryData->SetGeometry( m_Geom3D );
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
    CPPUNIT_ASSERT( Geometry3D_WriteReadLoop_Restores() );
  }

  bool Geometry3D_WriteReadLoop_Restores()
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
      CPPUNIT_ASSERT( loadedData.size() == 1 ); // IOUtil could read something (and just one)

      m_ReadGeometryData = dynamic_cast< mitk::GeometryData* >( loadedData.front().GetPointer() );
      CPPUNIT_ASSERT( m_ReadGeometryData.IsNotNull() ); // IOUtil could read _some_ GeometryData

      m_ReadGeom3D = dynamic_cast< mitk::Geometry3D* >( m_ReadGeometryData->GetGeometry() );
      CPPUNIT_ASSERT( m_ReadGeom3D.IsNotNull() ); // IOUtil could read _some_ Geometry3D

      // Doc of mitk::Equal for BaseGeometry says
      // "The function compares the spacing, origin, axisvectors, extents, the matrix of the
      //  IndexToWorldTransform(elementwise), the bounding(elementwise) and the ImageGeometry flag."
      // This seems pretty much everything that we can have in a Geometry3D..
      MITK_ASSERT_EQUAL( m_Geom3D, m_ReadGeom3D, "Geometry3D > file > Geometry3D keeps geometry");
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkGeometryDataIO)
