/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkArbitraryTimeGeometry.h>
#include <mitkGeometry3D.h>
#include <mitkIOUtil.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkSurface.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkLinearTransform.h>
#include <vtkSmartPointer.h>
#include <vtkTransformFilter.h>
#include <vtkXMLPolyDataReader.h>

#include <itksys/Directory.hxx>
#include <itksys/SystemTools.hxx>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace
{
  // One triangle per time step, shifted along x by the index of the time step.
  vtkSmartPointer<vtkPolyData> CreateTriangle(double offset)
  {
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(offset, 0.0, 0.0);
    points->InsertNextPoint(offset + 1.0, 0.0, 0.0);
    points->InsertNextPoint(offset, 1.0, 0.5);

    const vtkIdType ids[] = { 0, 1, 2 };
    auto polys = vtkSmartPointer<vtkCellArray>::New();
    polys->InsertNextCell(3, ids);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);
    return polyData;
  }

  mitk::Surface::Pointer CreateSurface(mitk::TimePointType firstTimePoint,
                                       mitk::TimePointType stepDuration,
                                       mitk::TimeStepType timeSteps)
  {
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::Geometry3D::New(), timeSteps);
    timeGeometry->SetFirstTimePoint(firstTimePoint);
    timeGeometry->SetStepDuration(stepDuration);

    auto surface = mitk::Surface::New();
    surface->SetTimeGeometry(timeGeometry);
    surface->Expand(static_cast<unsigned int>(timeSteps));

    for (mitk::TimeStepType timeStep = 0; timeStep < timeSteps; ++timeStep)
      surface->SetVtkPolyData(CreateTriangle(static_cast<double>(timeStep)), static_cast<unsigned int>(timeStep));

    return surface;
  }

  vtkSmartPointer<vtkPolyData> ReadWithVtk(const std::string &fileName)
  {
    const auto extension = itksys::SystemTools::GetFilenameLastExtension(fileName);

    if (extension == ".vtp")
    {
      auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
      reader->SetFileName(fileName.c_str());
      reader->Update();
      return reader->GetOutput();
    }

    if (extension == ".vtk")
    {
      auto reader = vtkSmartPointer<vtkPolyDataReader>::New();
      reader->SetFileName(fileName.c_str());
      reader->Update();
      return reader->GetOutput();
    }

    if (extension == ".stl")
    {
      auto reader = vtkSmartPointer<vtkSTLReader>::New();
      reader->SetFileName(fileName.c_str());
      reader->Update();
      return reader->GetOutput();
    }

    CPPUNIT_FAIL("ReadWithVtk does not support " + extension);
    return nullptr;
  }

  void Translate(mitk::Surface *surface, double x)
  {
    mitk::Point3D origin;
    mitk::FillVector3D(origin, x, 0.0, 0.0);

    for (mitk::TimeStepType timeStep = 0; timeStep < surface->GetTimeGeometry()->CountTimeSteps(); ++timeStep)
      surface->GetGeometry(static_cast<int>(timeStep))->SetOrigin(origin);
  }

  double GetWorldMinimumX(const mitk::Surface *surface, mitk::TimeStepType timeStep)
  {
    auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputData(surface->GetVtkPolyData(static_cast<unsigned int>(timeStep)));
    transformFilter->SetTransform(surface->GetGeometry(static_cast<int>(timeStep))->GetVtkTransform());
    transformFilter->Update();

    return transformFilter->GetPolyDataOutput()->GetBounds()[0];
  }

  // Each time step holds the triangle created for it, at its place in the world.
  void AssertTimeSteps(const mitk::Surface *surface, mitk::TimeStepType timeSteps, double x = 0.0)
  {
    CPPUNIT_ASSERT(surface != nullptr);
    CPPUNIT_ASSERT_EQUAL(timeSteps, surface->GetTimeGeometry()->CountTimeSteps());

    for (mitk::TimeStepType timeStep = 0; timeStep < timeSteps; ++timeStep)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(x + timeStep, GetWorldMinimumX(surface, timeStep), mitk::eps);
  }

  unsigned long CountFiles(const std::string &directory)
  {
    itksys::Directory listing;
    listing.Load(directory);

    unsigned long count = 0;

    for (unsigned long i = 0; i < listing.GetNumberOfFiles(); ++i)
    {
      if (!itksys::SystemTools::FileIsDirectory(directory + '/' + listing.GetFile(i)))
        ++count;
    }

    return count;
  }
}

class mitkSurfaceVtkIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkIOTestSuite);
  MITK_TEST(Write_SingleTimeStep_KeepsFileName);
  MITK_TEST(Write_TimeSteps_OneFilePerTimeStepInEachFormat);
  MITK_TEST(Write_TimeBounds_ExactInFileNames);
  MITK_TEST(Write_DotsInFileName_SuffixBeforeExtension);
  MITK_TEST(Write_Geometry_BakedIntoPoints);
  MITK_TEST(Read_SingleFile_ValidAtEveryTimePoint);
  MITK_TEST(Read_OneFileOfSeries_AllTimeStepsInEachFormat);
  MITK_TEST(SaveAndLoad_EmptyTimeStep_RestoredInEachFormat);
  MITK_TEST(Read_SeveralFilesOfSeries_OneSurfaceNamedAfterSeries);
  MITK_TEST(Read_IrregularTimeBounds_Restored);
  MITK_TEST(Read_EmptyTimeBounds_NumberedTimeSteps);
  MITK_TEST(Read_OverlappingTimeBounds_NumberedTimeSteps);
  MITK_TEST(Read_TimeBoundsWithGap_ArbitraryTimeGeometry);
  MITK_TEST(Read_TwoFilesForOneTimeStep_Throws);
  MITK_TEST(Read_TruncatedFileOfSeries_ThrowsNamingFile);
  MITK_TEST(Read_SingleFileFollowingPattern_ValidAtEveryTimePoint);
  MITK_TEST(Read_ExtensionsDifferingInCase_OneSeries);
  MITK_TEST(Write_EarlierSeriesWithOtherTimeBounds_ThrowsNamingFile);
  CPPUNIT_TEST_SUITE_END();

  std::string m_Directory;

public:
  void setUp() override
  {
    m_Directory = mitk::IOUtil::CreateTemporaryDirectory("SurfaceVtkIOTest_XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_Directory);
  }

  void Write_SingleTimeStep_KeepsFileName()
  {
    const auto fileName = m_Directory + "/surface.vtp";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 1), fileName);

    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(fileName));
    CPPUNIT_ASSERT_EQUAL(1ul, CountFiles(m_Directory));
  }

  void Write_TimeSteps_OneFilePerTimeStepInEachFormat()
  {
    const auto surface = CreateSurface(0.0, 100.0, 3);

    for (const std::string extension : { ".vtp", ".vtk", ".stl" })
    {
      const auto directory = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_Directory);
      mitk::IOUtil::Save(surface, directory + "/surface" + extension);

      CPPUNIT_ASSERT_EQUAL_MESSAGE(extension, 3ul, CountFiles(directory));

      for (int timeStep = 0; timeStep < 3; ++timeStep)
      {
        const auto fileName = directory + "/surface_S" + std::to_string(timeStep * 100) + "_E" +
                              std::to_string((timeStep + 1) * 100) + "_T" + std::to_string(timeStep) + extension;

        CPPUNIT_ASSERT_MESSAGE(fileName, itksys::SystemTools::FileExists(fileName));

        const auto polyData = ReadWithVtk(fileName);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(fileName, timeStep, polyData->GetBounds()[0], mitk::eps);
      }
    }
  }

  void Write_TimeBounds_ExactInFileNames()
  {
    mitk::IOUtil::Save(CreateSurface(1000.5, 150.0, 2), m_Directory + "/surface.vtp");

    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(m_Directory + "/surface_S1000.5_E1150.5_T0.vtp"));
    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(m_Directory + "/surface_S1150.5_E1300.5_T1.vtp"));
  }

  void Write_DotsInFileName_SuffixBeforeExtension()
  {
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 2), m_Directory + "/my.surface.vtp");

    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(m_Directory + "/my.surface_S0_E1_T0.vtp"));
    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(m_Directory + "/my.surface_S1_E2_T1.vtp"));
  }

  void Write_Geometry_BakedIntoPoints()
  {
    auto surface = CreateSurface(0.0, 1.0, 1);

    mitk::Point3D origin;
    mitk::FillVector3D(origin, 10.0, 20.0, 30.0);
    surface->GetGeometry()->SetOrigin(origin);

    const auto fileName = m_Directory + "/surface.vtp";
    mitk::IOUtil::Save(surface, fileName);

    const auto polyData = ReadWithVtk(fileName);
    const auto *bounds = polyData->GetBounds();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, bounds[0], mitk::eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, bounds[2], mitk::eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, bounds[4], mitk::eps);
  }

  void Read_SingleFile_ValidAtEveryTimePoint()
  {
    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(CreateTriangle(0.0));

    const auto fileName = m_Directory + "/surface.vtp";
    mitk::IOUtil::Save(surface, fileName);

    // Like a 3D image, so that it is shown along with time-resolved data.
    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(1), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT(loadedSurface->GetTimeGeometry()->IsValidTimePoint(1e6));
  }

  void Read_OneFileOfSeries_AllTimeStepsInEachFormat()
  {
    const auto surface = CreateSurface(0.0, 100.0, 3);
    Translate(surface, 10.0);

    // PLY is provided by MitkIOExt, which is loaded together with MitkCore.
    for (const std::string extension : { ".vtp", ".vtk", ".stl", ".ply" })
    {
      const auto directory = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_Directory);
      mitk::IOUtil::Save(surface, directory + "/surface" + extension);

      const auto data = mitk::IOUtil::Load(directory + "/surface_S100_E200_T1" + extension);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(extension, std::size_t(1), data.size());

      const auto *loadedSurface = dynamic_cast<const mitk::Surface *>(data.front().GetPointer());
      AssertTimeSteps(loadedSurface, 3, 10.0);

      const auto *timeGeometry = dynamic_cast<const mitk::ProportionalTimeGeometry *>(loadedSurface->GetTimeGeometry());
      CPPUNIT_ASSERT_MESSAGE(extension, timeGeometry != nullptr);
      CPPUNIT_ASSERT_EQUAL(0.0, timeGeometry->GetFirstTimePoint());
      CPPUNIT_ASSERT_EQUAL(100.0, timeGeometry->GetStepDuration());
    }
  }

  void SaveAndLoad_EmptyTimeStep_RestoredInEachFormat()
  {
    // Polygon models of segmentations get poly data without points for time
    // steps in which a label is absent.
    auto surface = CreateSurface(0.0, 1.0, 3);
    surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), 1);

    for (const std::string extension : { ".vtp", ".vtk", ".stl", ".ply" })
    {
      const auto directory = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_Directory);
      mitk::IOUtil::Save(surface, directory + "/surface" + extension);

      const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(directory + "/surface_S0_E1_T0" + extension);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(extension, mitk::TimeStepType(3), loadedSurface->GetTimeGeometry()->CountTimeSteps());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(extension, vtkIdType(0), loadedSurface->GetVtkPolyData(1)->GetNumberOfPoints());
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(extension, 2.0, loadedSurface->GetVtkPolyData(2)->GetBounds()[0], mitk::eps);
    }
  }

  void Read_SeveralFilesOfSeries_OneSurfaceNamedAfterSeries()
  {
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 3), m_Directory + "/surface.vtp");

    auto storage = mitk::StandaloneDataStorage::New();
    const auto nodes = mitk::IOUtil::Load({ m_Directory + "/surface_S2_E3_T2.vtp",
                                            m_Directory + "/surface_S0_E1_T0.vtp",
                                            m_Directory + "/surface_S1_E2_T1.vtp" },
                                          *storage);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), static_cast<std::size_t>(nodes->Size()));

    const auto node = nodes->GetElement(0);
    CPPUNIT_ASSERT_EQUAL(std::string("surface"), node->GetName());
    AssertTimeSteps(dynamic_cast<const mitk::Surface *>(node->GetData()), 3);
  }

  void Read_IrregularTimeBounds_Restored()
  {
    const std::vector<std::pair<mitk::TimePointType, mitk::TimePointType>> timeBounds = {
      { 0.0, 10.0 }, { 20.0, 25.5 }, { 25.5, 100.0 }
    };

    auto timeGeometry = mitk::ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();

    for (const auto &[minimum, maximum] : timeBounds)
      timeGeometry->AppendNewTimeStep(mitk::Geometry3D::New(), minimum, maximum);

    timeGeometry->Update();

    auto surface = mitk::Surface::New();
    surface->SetTimeGeometry(timeGeometry);
    surface->Expand(3);

    for (unsigned int timeStep = 0; timeStep < 3; ++timeStep)
      surface->SetVtkPolyData(CreateTriangle(timeStep), timeStep);

    mitk::IOUtil::Save(surface, m_Directory + "/surface.vtp");

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(m_Directory + "/surface_S0_E10_T0.vtp");
    AssertTimeSteps(loadedSurface, 3);

    for (mitk::TimeStepType timeStep = 0; timeStep < 3; ++timeStep)
    {
      const auto bounds = loadedSurface->GetTimeGeometry()->GetTimeBounds(timeStep);
      CPPUNIT_ASSERT_EQUAL(timeBounds[timeStep].first, bounds[0]);
      CPPUNIT_ASSERT_EQUAL(timeBounds[timeStep].second, bounds[1]);
    }
  }

  void Read_EmptyTimeBounds_NumberedTimeSteps()
  {
    // Older versions of MITK printed the time bounds with a single
    // significant digit, so that later time steps collapse.
    const auto loadedSurface = LoadSeriesOfCopies({ "old_S1e+03_E1e+03_T0.vtp", "old_S1e+03_E1e+03_T1.vtp" });
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(2), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT_EQUAL(1.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(1)[0]);
  }

  void Read_OverlappingTimeBounds_NumberedTimeSteps()
  {
    const auto loadedSurface = LoadSeriesOfCopies({ "overlap_S0_E2_T0.vtp", "overlap_S1_E3_T1.vtp" });
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(2), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT_EQUAL(1.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(0)[1]);
    CPPUNIT_ASSERT_EQUAL(2.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(1)[1]);
  }

  void Read_TimeBoundsWithGap_ArbitraryTimeGeometry()
  {
    const auto loadedSurface = LoadSeriesOfCopies({ "gap_S0_E1_T0.vtp", "gap_S5_E6_T1.vtp" });
    CPPUNIT_ASSERT_EQUAL(1.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(0)[1]);
    CPPUNIT_ASSERT_EQUAL(5.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(1)[0]);
    CPPUNIT_ASSERT_EQUAL(6.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(1)[1]);
  }

  /** Loads a time series whose files are copies of a single time step. */
  mitk::Surface::Pointer LoadSeriesOfCopies(const std::vector<std::string> &fileNames)
  {
    const auto fileName = m_Directory + "/single.vtp";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 1), fileName);

    for (const auto &seriesFileName : fileNames)
      itksys::SystemTools::CopyFileAlways(fileName, m_Directory + "/" + seriesFileName);

    return mitk::IOUtil::Load<mitk::Surface>(m_Directory + "/" + fileNames.front());
  }

  void Read_TwoFilesForOneTimeStep_Throws()
  {
    const auto fileName = m_Directory + "/single.vtp";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 1), fileName);

    itksys::SystemTools::CopyFileAlways(fileName, m_Directory + "/surface_S0_E1_T0.vtp");
    itksys::SystemTools::CopyFileAlways(fileName, m_Directory + "/surface_S0_E2_T0.vtp");

    try
    {
      mitk::IOUtil::Load(m_Directory + "/surface_S0_E1_T0.vtp");
      CPPUNIT_FAIL("Two files for one time step must not load.");
    }
    catch (const mitk::Exception &e)
    {
      const std::string message = e.what();
      CPPUNIT_ASSERT_MESSAGE(message, message.find("surface_S0_E1_T0.vtp") != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE(message, message.find("surface_S0_E2_T0.vtp") != std::string::npos);
    }
  }

  void Read_SingleFileFollowingPattern_ValidAtEveryTimePoint()
  {
    // Named by a user, not by MITK, which writes a time series only for
    // several time steps.
    const auto fileName = m_Directory + "/liver_S1_E2_T1.stl";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 1), m_Directory + "/single.stl");
    std::filesystem::rename(m_Directory + "/single.stl", fileName);

    auto storage = mitk::StandaloneDataStorage::New();
    const auto nodes = mitk::IOUtil::Load(fileName, *storage);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), static_cast<std::size_t>(nodes->Size()));

    const auto node = nodes->GetElement(0);
    CPPUNIT_ASSERT_EQUAL(std::string("liver_S1_E2_T1"), node->GetName());
    CPPUNIT_ASSERT(node->GetData()->GetTimeGeometry()->IsValidAtEveryTimePoint());
  }

  void Read_ExtensionsDifferingInCase_OneSeries()
  {
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 2), m_Directory + "/surface.vtp");
    std::filesystem::rename(m_Directory + "/surface_S1_E2_T1.vtp", m_Directory + "/surface_S1_E2_T1.VTP");

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(m_Directory + "/surface_S0_E1_T0.vtp");
    AssertTimeSteps(loadedSurface, 2);
  }

  void Write_EarlierSeriesWithOtherTimeBounds_ThrowsNamingFile()
  {
    const auto fileName = m_Directory + "/surface.vtp";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 2), fileName);

    // The same time bounds overwrite the files.
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 2), fileName);
    CPPUNIT_ASSERT_EQUAL(2ul, CountFiles(m_Directory));

    try
    {
      mitk::IOUtil::Save(CreateSurface(0.0, 50.0, 2), fileName);
      CPPUNIT_FAIL("Files of an earlier time series must not stay next to a new one.");
    }
    catch (const mitk::Exception &e)
    {
      CPPUNIT_ASSERT_MESSAGE(e.what(), std::string(e.what()).find("surface_S1_E2_T1.vtp") != std::string::npos);
    }

    CPPUNIT_ASSERT_EQUAL(2ul, CountFiles(m_Directory));
  }

  void Read_TruncatedFileOfSeries_ThrowsNamingFile()
  {
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 3), m_Directory + "/surface.vtp");

    // Like an interrupted copy.
    const auto truncatedFileName = m_Directory + "/surface_S1_E2_T1.vtp";
    std::string content;

    {
      std::ifstream file(truncatedFileName, std::ios::binary);
      content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    std::ofstream(truncatedFileName, std::ios::binary | std::ios::trunc) << content.substr(0, content.size() / 2);

    try
    {
      mitk::IOUtil::Load(m_Directory + "/surface_S0_E1_T0.vtp");
      CPPUNIT_FAIL("A truncated file of the time series must not load as an empty time step.");
    }
    catch (const mitk::Exception &e)
    {
      CPPUNIT_ASSERT_MESSAGE(e.what(), std::string(e.what()).find("surface_S1_E2_T1.vtp") != std::string::npos);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkIO)
