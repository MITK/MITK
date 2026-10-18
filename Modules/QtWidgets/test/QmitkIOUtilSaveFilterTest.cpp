/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkIOUtil.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkIOMimeTypes.h>
#include <mitkImage.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkSurface.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <algorithm>

class QmitkIOUtilSaveFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkIOUtilSaveFilterTestSuite);
  MITK_TEST(GetDefaultMimeType_Surface_VtkPolyData);
  MITK_TEST(GetDefaultMimeType_TimeResolvedSurface_VtkHdf);
  MITK_TEST(GetDefaultMimeType_SurfaceWithTime_VtkHdf);
  MITK_TEST(GetMimeTypes_TimeResolvedSurface_OtherFormatsStillOffered);
  MITK_TEST(GetDefaultMimeType_Image_Nrrd);
  CPPUNIT_TEST_SUITE_END();

  static mitk::Surface::Pointer CreateSurface(unsigned int timeSteps)
  {
    auto surface = mitk::Surface::New();
    surface->Expand(timeSteps);

    for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
      surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), timeStep);

    return surface;
  }

  static QmitkIOUtil::SaveFilter CreateSaveFilter(const mitk::BaseData *data)
  {
    return QmitkIOUtil::SaveFilter(mitk::IOUtil::SaveInfo(data, mitk::MimeType(), std::string()));
  }

public:
  void GetDefaultMimeType_Surface_VtkPolyData()
  {
    const auto surface = CreateSurface(1);
    CPPUNIT_ASSERT_EQUAL(mitk::IOMimeTypes::VTK_POLYDATA_NAME(), CreateSaveFilter(surface).GetDefaultMimeType().GetName());
  }

  void GetDefaultMimeType_TimeResolvedSurface_VtkHdf()
  {
    const auto surface = CreateSurface(3);
    CPPUNIT_ASSERT_EQUAL(mitk::IOMimeTypes::VTKHDF_NAME(), CreateSaveFilter(surface).GetDefaultMimeType().GetName());
  }

  void GetDefaultMimeType_SurfaceWithTime_VtkHdf()
  {
    // A single time step that, unlike that of 3D images, is valid at its time only.
    const auto surface = CreateSurface(1);
    auto *timeGeometry = dynamic_cast<mitk::ProportionalTimeGeometry *>(surface->GetTimeGeometry());
    timeGeometry->SetFirstTimePoint(100.0);
    timeGeometry->SetStepDuration(50.0);

    CPPUNIT_ASSERT_EQUAL(mitk::IOMimeTypes::VTKHDF_NAME(), CreateSaveFilter(surface).GetDefaultMimeType().GetName());
  }

  void GetMimeTypes_TimeResolvedSurface_OtherFormatsStillOffered()
  {
    const auto surface = CreateSurface(3);
    const auto mimeTypes = CreateSaveFilter(surface).GetMimeTypes();

    for (const auto &name : { mitk::IOMimeTypes::VTK_POLYDATA_NAME(),
                              mitk::IOMimeTypes::VTK_POLYDATA_LEGACY_NAME(),
                              mitk::IOMimeTypes::STEREOLITHOGRAPHY_NAME() })
    {
      const auto offered = std::any_of(mimeTypes.begin(), mimeTypes.end(), [&name](const mitk::MimeType &mimeType) {
        return mimeType.GetName() == name;
      });

      CPPUNIT_ASSERT_MESSAGE(name, offered);
    }
  }

  void GetDefaultMimeType_Image_Nrrd()
  {
    const unsigned int dimensions[] = { 4, 4, 4 };

    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    CPPUNIT_ASSERT_EQUAL(mitk::IOMimeTypes::NRRD_MIMETYPE_NAME(), CreateSaveFilter(image).GetDefaultMimeType().GetName());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkIOUtilSaveFilter)
