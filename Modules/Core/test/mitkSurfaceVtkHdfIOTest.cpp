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
#include <mitkIOMimeTypes.h>
#include <mitkIOUtil.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkSurface.h>

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkHDFReader.h>
#include <vtkHDFWriter.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLinearTransform.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTransformFilter.h>

#include <itksys/SystemTools.hxx>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace
{
  // One triangle per time step, shifted along x by the index of the time step.
  vtkSmartPointer<vtkPolyData> CreateTriangle(double offset, int pointDataType = VTK_FLOAT)
  {
    auto points = vtkSmartPointer<vtkPoints>::New(pointDataType);
    points->InsertNextPoint(offset, 0.0, 0.0);
    points->InsertNextPoint(offset + 1.0, 0.0, 0.0);
    points->InsertNextPoint(offset, 1.0, 0.5);

    const vtkIdType ids[] = { 0, 1, 2 };
    auto polys = vtkSmartPointer<vtkCellArray>::New();
    polys->InsertNextCell(3, ids);

    auto normals = vtkSmartPointer<vtkFloatArray>::New();
    normals->SetName("Normals");
    normals->SetNumberOfComponents(3);

    for (int i = 0; i < 3; ++i)
      normals->InsertNextTuple3(0.0, -0.4472136, 0.8944272);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);
    polyData->GetPointData()->SetNormals(normals);
    return polyData;
  }

  mitk::Surface::Pointer CreateSurface(mitk::TimeGeometry *timeGeometry)
  {
    const auto timeSteps = static_cast<unsigned int>(timeGeometry->CountTimeSteps());

    auto surface = mitk::Surface::New();
    surface->SetTimeGeometry(timeGeometry);
    surface->Expand(timeSteps);

    for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
      surface->SetVtkPolyData(CreateTriangle(timeStep), timeStep);

    return surface;
  }

  mitk::Surface::Pointer CreateSurface(mitk::TimePointType firstTimePoint,
                                       mitk::TimePointType stepDuration,
                                       mitk::TimeStepType timeSteps)
  {
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::Geometry3D::New(), timeSteps);
    timeGeometry->SetFirstTimePoint(firstTimePoint);
    timeGeometry->SetStepDuration(stepDuration);

    return CreateSurface(timeGeometry);
  }

  double GetWorldMinimumX(const mitk::Surface *surface, mitk::TimeStepType timeStep)
  {
    auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputData(surface->GetVtkPolyData(static_cast<unsigned int>(timeStep)));
    transformFilter->SetTransform(surface->GetGeometry(static_cast<int>(timeStep))->GetVtkTransform());
    transformFilter->Update();

    return transformFilter->GetPolyDataOutput()->GetBounds()[0];
  }

  /** Writes VTKHDF files the way another application would. */
  class TimeStepSource : public vtkPolyDataAlgorithm
  {
  public:
    static TimeStepSource *New();
    vtkTypeMacro(TimeStepSource, vtkPolyDataAlgorithm);

    std::vector<vtkSmartPointer<vtkPolyData>> PolyDatas;
    std::vector<double> TimeValues;

  protected:
    TimeStepSource() { this->SetNumberOfInputPorts(0); }

    int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *outputVector) override
    {
      auto *outInfo = outputVector->GetInformationObject(0);
      outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), TimeValues.data(), static_cast<int>(TimeValues.size()));

      const double timeRange[] = { TimeValues.front(), TimeValues.back() };
      outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
      return 1;
    }

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *outputVector) override
    {
      auto *outInfo = outputVector->GetInformationObject(0);
      const auto timeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
      const auto timeStep = std::distance(TimeValues.begin(), std::find(TimeValues.begin(), TimeValues.end(), timeValue));

      vtkPolyData::GetData(outInfo)->ShallowCopy(PolyDatas[timeStep]);
      return 1;
    }
  };

  vtkStandardNewMacro(TimeStepSource);

  void WriteWithVtk(const std::string &fileName, const std::vector<vtkSmartPointer<vtkPolyData>> &polyDatas, const std::vector<double> &timeValues)
  {
    auto source = vtkSmartPointer<TimeStepSource>::New();
    source->PolyDatas = polyDatas;
    source->TimeValues = timeValues;

    auto writer = vtkSmartPointer<vtkHDFWriter>::New();
    writer->SetInputConnection(source->GetOutputPort());
    writer->SetFileName(fileName.c_str());
    writer->Write();
  }
}

class mitkSurfaceVtkHdfIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkHdfIOTestSuite);
  MITK_TEST(SaveAndLoad_ProportionalTimeSteps_OneFile);
  MITK_TEST(SaveAndLoad_IrregularTimeBounds_Restored);
  MITK_TEST(SaveAndLoad_TimeBoundsBeyondSinglePrecision_Restored);
  MITK_TEST(SaveAndLoad_EqualStartTimePoints_NumberedTimeSteps);
  MITK_TEST(SaveAndLoad_SingleTimeStep_Restored);
  MITK_TEST(SaveAndLoad_ValidAtEveryTimePoint_Kept);
  MITK_TEST(Load_WithoutTimeSteps_ValidAtEveryTimePoint);
  MITK_TEST(SaveAndLoad_EmptyTimeStep_Restored);
  MITK_TEST(SaveAndLoad_EmptyFirstTimeStep_PointDataTypeKept);
  MITK_TEST(Save_ArrayMissingInNonEmptyTimeStep_Throws);
  MITK_TEST(Save_TimeStepsToMissingDirectory_Throws);
  MITK_TEST(Load_NoStoredTimeBounds_TimeBoundsFromTimeValues);
  MITK_TEST(Load_TimeValuesChangedElsewhere_StoredTimeBoundsIgnored);
  MITK_TEST(Load_OverlappingStoredTimeBounds_TimeBoundsFromTimeValues);
  MITK_TEST(Load_NoHdf5File_Throws);
  MITK_TEST(Load_TruncatedFile_Throws);
  MITK_TEST(GetWriterConfidenceLevel_TimeSteps_Supported);
  MITK_TEST(GetDefaultWriter_ValidAtEveryTimePoint_VtkXmlPolyData);
  CPPUNIT_TEST_SUITE_END();

  std::string m_Directory;

public:
  void setUp() override
  {
    m_Directory = mitk::IOUtil::CreateTemporaryDirectory("SurfaceVtkHdfIOTest_XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_Directory);
  }

  void SaveAndLoad_ProportionalTimeSteps_OneFile()
  {
    auto surface = CreateSurface(50.0, 100.0, 3);

    mitk::Point3D origin;
    mitk::FillVector3D(origin, 10.0, 0.0, 0.0);

    for (int timeStep = 0; timeStep < 3; ++timeStep)
      surface->GetGeometry(timeStep)->SetOrigin(origin);

    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(surface, fileName);

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(3), loadedSurface->GetTimeGeometry()->CountTimeSteps());

    const auto *timeGeometry = dynamic_cast<const mitk::ProportionalTimeGeometry *>(loadedSurface->GetTimeGeometry());
    CPPUNIT_ASSERT(timeGeometry != nullptr);
    CPPUNIT_ASSERT_EQUAL(50.0, timeGeometry->GetFirstTimePoint());
    CPPUNIT_ASSERT_EQUAL(100.0, timeGeometry->GetStepDuration());

    for (mitk::TimeStepType timeStep = 0; timeStep < 3; ++timeStep)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0 + timeStep, GetWorldMinimumX(loadedSurface, timeStep), mitk::eps);

      auto *normals = loadedSurface->GetVtkPolyData(static_cast<unsigned int>(timeStep))->GetPointData()->GetNormals();
      CPPUNIT_ASSERT(normals != nullptr);
      CPPUNIT_ASSERT_EQUAL(vtkIdType(3), normals->GetNumberOfTuples());
    }
  }

  void SaveAndLoad_IrregularTimeBounds_Restored()
  {
    SaveAndLoadTimeBounds({ { 0.0, 10.0 }, { 20.0, 25.5 }, { 25.5, 100.0 } });
  }

  void SaveAndLoad_TimeBoundsBeyondSinglePrecision_Restored()
  {
    // Time points of dynamic DICOM series, in milliseconds.
    SaveAndLoadTimeBounds({ { 0.0, 1234.567 }, { 1234.567, 2469.134 }, { 2469.134, 3700.0 } });
  }

  void SaveAndLoad_EqualStartTimePoints_NumberedTimeSteps()
  {
    // Like a DICOM series with two time steps acquired within the same second.
    auto timeGeometry = mitk::ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();
    timeGeometry->AppendNewTimeStep(mitk::Geometry3D::New(), 0.0, 1000.0);
    timeGeometry->AppendNewTimeStep(mitk::Geometry3D::New(), 1000.0, 1000.0);
    timeGeometry->AppendNewTimeStep(mitk::Geometry3D::New(), 1000.0, 2000.0);
    timeGeometry->Update();

    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(CreateSurface(timeGeometry), fileName);

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(3), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT_EQUAL(2.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(2)[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, GetWorldMinimumX(loadedSurface, 2), mitk::eps);
  }

  void SaveAndLoadTimeBounds(const std::vector<std::pair<mitk::TimePointType, mitk::TimePointType>> &timeBounds)
  {
    auto timeGeometry = mitk::ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();

    for (const auto &[minimum, maximum] : timeBounds)
      timeGeometry->AppendNewTimeStep(mitk::Geometry3D::New(), minimum, maximum);

    timeGeometry->Update();

    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(CreateSurface(timeGeometry), fileName);

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(timeBounds.size(), loadedSurface->GetTimeGeometry()->CountTimeSteps());

    for (mitk::TimeStepType timeStep = 0; timeStep < timeBounds.size(); ++timeStep)
    {
      const auto bounds = loadedSurface->GetTimeGeometry()->GetTimeBounds(timeStep);
      CPPUNIT_ASSERT_EQUAL(timeBounds[timeStep].first, bounds[0]);
      CPPUNIT_ASSERT_EQUAL(timeBounds[timeStep].second, bounds[1]);
    }
  }

  void SaveAndLoad_SingleTimeStep_Restored()
  {
    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 1), fileName);

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(1), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT_EQUAL(1.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(0)[1]);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(3), loadedSurface->GetVtkPolyData()->GetNumberOfPoints());
  }

  void SaveAndLoad_ValidAtEveryTimePoint_Kept()
  {
    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(CreateTriangle(0.0));

    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(surface, fileName);

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(1), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT(loadedSurface->GetTimeGeometry()->IsValidTimePoint(1e6));
    CPPUNIT_ASSERT(loadedSurface->GetVtkPolyData()->GetFieldData()->GetArray("MITKTimeBounds") == nullptr);

    // MITK's reader removes stored time bounds, so look at the file the way
    // other applications see it.
    auto reader = vtkSmartPointer<vtkHDFReader>::New();
    reader->SetFileName(fileName.c_str());
    CPPUNIT_ASSERT(reader->Update());

    auto *polyData = vtkPolyData::SafeDownCast(reader->GetOutputDataObject(0));
    CPPUNIT_ASSERT(polyData != nullptr);
    CPPUNIT_ASSERT(polyData->GetFieldData()->GetArray("MITKTimeBounds") == nullptr);
  }

  void Load_WithoutTimeSteps_ValidAtEveryTimePoint()
  {
    const auto fileName = m_Directory + "/surface.vtkhdf";

    auto writer = vtkSmartPointer<vtkHDFWriter>::New();
    writer->SetInputData(CreateTriangle(0.0));
    writer->SetFileName(fileName.c_str());
    writer->Write();

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(1), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT(loadedSurface->GetTimeGeometry()->IsValidTimePoint(1e6));
  }

  void SaveAndLoad_EmptyTimeStep_Restored()
  {
    // Polygon models of segmentations get empty poly data for time steps in
    // which a label is absent, without the arrays of the other time steps.
    auto surface = CreateSurface(0.0, 1.0, 3);
    surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), 1);

    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(surface, fileName);

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(3), loadedSurface->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT_EQUAL(vtkIdType(3), loadedSurface->GetVtkPolyData(0)->GetNumberOfPoints());
    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), loadedSurface->GetVtkPolyData(1)->GetNumberOfPoints());
    CPPUNIT_ASSERT_EQUAL(vtkIdType(3), loadedSurface->GetVtkPolyData(2)->GetNumberOfPoints());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, loadedSurface->GetVtkPolyData(2)->GetBounds()[0], mitk::eps);

    auto *normals = loadedSurface->GetVtkPolyData(2)->GetPointData()->GetNormals();
    CPPUNIT_ASSERT(normals != nullptr);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(3), normals->GetNumberOfTuples());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8944272, normals->GetComponent(2, 2), 1e-6);
  }

  void SaveAndLoad_EmptyFirstTimeStep_PointDataTypeKept()
  {
    // The writer chooses the data type of the points of all time steps from
    // the first time step.
    for (const int pointDataType : { VTK_FLOAT, VTK_DOUBLE })
    {
      auto surface = CreateSurface(0.0, 1.0, 3);
      surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), 0);

      for (unsigned int timeStep = 1; timeStep < 3; ++timeStep)
        surface->SetVtkPolyData(CreateTriangle(timeStep, pointDataType), timeStep);

      const auto fileName = m_Directory + "/surface.vtkhdf";
      mitk::IOUtil::Save(surface, fileName);

      const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
      CPPUNIT_ASSERT_EQUAL(vtkIdType(0), loadedSurface->GetVtkPolyData(0)->GetNumberOfPoints());
      CPPUNIT_ASSERT_EQUAL(pointDataType, loadedSurface->GetVtkPolyData(2)->GetPoints()->GetDataType());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, loadedSurface->GetVtkPolyData(2)->GetBounds()[0], mitk::eps);
    }
  }

  void Save_ArrayMissingInNonEmptyTimeStep_Throws()
  {
    auto surface = CreateSurface(0.0, 1.0, 2);
    surface->GetVtkPolyData(1)->GetPointData()->RemoveArray("Normals");

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(surface, m_Directory + "/surface.vtkhdf"), mitk::Exception);
  }

  void Save_TimeStepsToMissingDirectory_Throws()
  {
    // A time step that vtkHDFWriter failed to write used to make it loop forever.
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 3), m_Directory + "/missing/surface.vtkhdf"),
                         mitk::Exception);
  }

  void Load_NoStoredTimeBounds_TimeBoundsFromTimeValues()
  {
    const auto fileName = m_Directory + "/surface.vtkhdf";
    WriteWithVtk(fileName, { CreateTriangle(0.0), CreateTriangle(1.0), CreateTriangle(2.0) }, { 5.0, 15.0, 30.0 });

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    const auto *timeGeometry = loadedSurface->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(3), timeGeometry->CountTimeSteps());

    CPPUNIT_ASSERT_EQUAL(5.0, timeGeometry->GetTimeBounds(0)[0]);
    CPPUNIT_ASSERT_EQUAL(15.0, timeGeometry->GetTimeBounds(0)[1]);
    CPPUNIT_ASSERT_EQUAL(30.0, timeGeometry->GetTimeBounds(1)[1]);
    CPPUNIT_ASSERT_EQUAL(45.0, timeGeometry->GetTimeBounds(2)[1]);
  }

  void Load_TimeValuesChangedElsewhere_StoredTimeBoundsIgnored()
  {
    std::vector<vtkSmartPointer<vtkPolyData>> polyDatas;

    for (int timeStep = 0; timeStep < 2; ++timeStep)
    {
      auto timeBounds = vtkSmartPointer<vtkDoubleArray>::New();
      timeBounds->SetName("MITKTimeBounds");
      timeBounds->SetNumberOfComponents(2);
      timeBounds->InsertNextTuple2(timeStep, timeStep + 1);

      polyDatas.push_back(CreateTriangle(timeStep));
      polyDatas.back()->GetFieldData()->AddArray(timeBounds);
    }

    const auto fileName = m_Directory + "/surface.vtkhdf";
    WriteWithVtk(fileName, polyDatas, { 100.0, 200.0 });

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(100.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(0)[0]);
    CPPUNIT_ASSERT_EQUAL(300.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(1)[1]);
    CPPUNIT_ASSERT(loadedSurface->GetVtkPolyData(0)->GetFieldData()->GetArray("MITKTimeBounds") == nullptr);
  }

  void Load_OverlappingStoredTimeBounds_TimeBoundsFromTimeValues()
  {
    // The start points agree with the time values, but the time steps overlap.
    const std::vector<std::pair<double, double>> storedTimeBounds = { { 10.0, 50.0 }, { 20.0, 30.0 } };
    std::vector<vtkSmartPointer<vtkPolyData>> polyDatas;

    for (const auto &[minimum, maximum] : storedTimeBounds)
    {
      auto timeBounds = vtkSmartPointer<vtkDoubleArray>::New();
      timeBounds->SetName("MITKTimeBounds");
      timeBounds->SetNumberOfComponents(2);
      timeBounds->InsertNextTuple2(minimum, maximum);

      polyDatas.push_back(CreateTriangle(static_cast<double>(polyDatas.size())));
      polyDatas.back()->GetFieldData()->AddArray(timeBounds);
    }

    const auto fileName = m_Directory + "/surface.vtkhdf";
    WriteWithVtk(fileName, polyDatas, { 10.0, 20.0 });

    const auto loadedSurface = mitk::IOUtil::Load<mitk::Surface>(fileName);
    CPPUNIT_ASSERT_EQUAL(10.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(0)[0]);
    CPPUNIT_ASSERT_EQUAL(20.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(0)[1]);
    CPPUNIT_ASSERT_EQUAL(30.0, loadedSurface->GetTimeGeometry()->GetTimeBounds(1)[1]);
  }

  void Load_NoHdf5File_Throws()
  {
    const auto fileName = m_Directory + "/surface.vtkhdf";
    std::ofstream(fileName) << "not an HDF5 file\n";

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(fileName), mitk::Exception);
  }

  void Load_TruncatedFile_Throws()
  {
    const auto fileName = m_Directory + "/surface.vtkhdf";
    mitk::IOUtil::Save(CreateSurface(0.0, 1.0, 3), fileName);

    std::string content;

    {
      std::ifstream file(fileName, std::ios::binary);
      content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    std::ofstream(fileName, std::ios::binary | std::ios::trunc) << content.substr(0, content.size() * 9 / 10);

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(fileName), mitk::Exception);
  }

  void GetWriterConfidenceLevel_TimeSteps_Supported()
  {
    const auto surface = CreateSurface(0.0, 1.0, 3);
    mitk::IOUtil::SaveInfo saveInfo(surface, mitk::MimeType(), std::string());

    const auto writers = saveInfo.m_WriterSelector.Get(std::string());
    const auto vtkHdfWriter = std::find_if(writers.begin(), writers.end(), [](const auto &writer) {
      return writer.GetMimeType().GetName() == mitk::IOMimeTypes::VTKHDF_NAME();
    });

    CPPUNIT_ASSERT(vtkHdfWriter != writers.end());
    CPPUNIT_ASSERT_EQUAL(mitk::IFileWriter::Supported, vtkHdfWriter->GetConfidenceLevel());
    CPPUNIT_ASSERT_EQUAL(mitk::IOMimeTypes::VTKHDF_NAME(), saveInfo.m_WriterSelector.GetDefault().GetMimeType().GetName());
  }

  void GetDefaultWriter_ValidAtEveryTimePoint_VtkXmlPolyData()
  {
    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(CreateTriangle(0.0));

    mitk::IOUtil::SaveInfo saveInfo(surface, mitk::MimeType(), std::string());
    CPPUNIT_ASSERT_EQUAL(mitk::IOMimeTypes::VTK_POLYDATA_NAME(), saveInfo.m_WriterSelector.GetDefault().GetMimeType().GetName());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkHdfIO)
