/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkFileReaderRegistry.h>
#include <mitkFileWriterRegistry.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkIOMimeTypes.h>
#include <mitkIOUtil.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlaneGeometry.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
  const std::vector<std::string> TypeIds = { "line", "arrow", "angle", "four_point_angle", "cross", "circle", "ellipse",
                                             "double_ellipse", "rectangle", "polygon", "path", "subdivision_polygon",
                                             "bezier_curve" };

  std::string GetMimeTypeName()
  {
    return mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".planarfigure";
  }

  mitk::Point2D P(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  mitk::Vector3D V(double x, double y, double z)
  {
    mitk::Vector3D vector;
    vector[0] = x;
    vector[1] = y;
    vector[2] = z;
    return vector;
  }

  mitk::PlaneGeometry::Pointer CreateAxialPlane()
  {
    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(100.0, 100.0, V(1.0, 1.0, 1.0));
    return plane;
  }

  mitk::PlaneGeometry::Pointer CreateAnisotropicPlane()
  {
    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(100.0, 100.0, V(0.3, 1.7, 2.5));
    plane->SetImageGeometry(true);
    return plane;
  }

  mitk::PlaneGeometry::Pointer CreateObliquePlane()
  {
    const auto spacing = V(0.75, 1.25, 1.0);

    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(V(60.0, 60.0, 0.0), V(-50.0, 50.0, 50.0), &spacing);

    mitk::Point3D origin;
    origin[0] = 5.5;
    origin[1] = -3.25;
    origin[2] = 7.125;
    plane->SetOrigin(origin);

    return plane;
  }

  /** Places a figure of every type with points that suit its constraints. */
  mitk::PlanarFigure::Pointer CreateFigure(const std::string &typeId, const mitk::PlaneGeometry *plane)
  {
    auto figure = mitk::CreatePlanarFigure(typeId);
    figure->SetPlaneGeometry(plane);

    if (mitk::PlanarFigure::PlacementKind::Span == figure->GetPlacementKind())
    {
      figure->SetFromSpan(P(10.25, 20.5), P(40.125, 35.75));
    }
    else
    {
      std::vector<mitk::Point2D> points = { P(10.25, 20.5), P(40.125, 35.75), P(30.5, 60.25), P(12.0, 50.0) };
      points.resize(std::min<std::size_t>(points.size(), figure->GetMaximumNumberOfControlPoints()));
      figure->SetControlPoints(points);
    }

    return figure;
  }

  void AssertEqual(const mitk::PlanarFigure *expected, const mitk::PlanarFigure *actual)
  {
    const auto message = expected->GetTypeId();

    CPPUNIT_ASSERT_MESSAGE(message, nullptr != actual);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected->GetTypeId(), actual->GetTypeId());
    CPPUNIT_ASSERT_MESSAGE(message, actual->IsComplete());
    CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(*expected->GetPlaneGeometry(), *actual->GetPlaneGeometry(), mitk::eps, true));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected->IsShapeLocked(), actual->IsShapeLocked());

    const auto expectedPoints = expected->GetControlPoints();
    const auto actualPoints = actual->GetControlPoints();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expectedPoints.size(), actualPoints.size());

    for (std::size_t i = 0; i < expectedPoints.size(); ++i)
      CPPUNIT_ASSERT_MESSAGE(message, expectedPoints[i] == actualPoints[i]);

    if (const auto *doubleEllipse = dynamic_cast<const mitk::PlanarDoubleEllipse *>(expected); nullptr != doubleEllipse)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE(message, doubleEllipse->GetThickness(),
        static_cast<const mitk::PlanarDoubleEllipse *>(actual)->GetThickness());
    }

    if (const auto *ellipse = dynamic_cast<const mitk::PlanarEllipse *>(expected); nullptr != ellipse)
    {
      const auto expectedDirection = ellipse->GetReferenceDirection();
      const auto actualDirection = static_cast<const mitk::PlanarEllipse *>(actual)->GetReferenceDirection();

      CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expectedDirection.has_value(), actualDirection.has_value());

      // Reading normalizes the direction again, which may change its last digit.
      if (expectedDirection.has_value())
        CPPUNIT_ASSERT_MESSAGE(message, (*expectedDirection - *actualDirection).GetNorm() < mitk::eps);
    }
  }

  std::string WriteToString(const mitk::PlanarFigure *figure)
  {
    mitk::FileWriterRegistry registry;
    auto writers = registry.GetWriters(figure, GetMimeTypeName());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), writers.size());

    std::ostringstream stream;
    writers.front()->SetOutputStream("", &stream);
    writers.front()->SetInput(figure);
    writers.front()->Write();

    return stream.str();
  }

  mitk::PlanarFigure::Pointer ReadFromString(const std::string &content)
  {
    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    mitk::FileReaderRegistry registry;
    auto readers = registry.GetReaders(mimeTypeProvider->GetMimeTypeForName(GetMimeTypeName()));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), readers.size());

    std::istringstream stream(content);
    readers.front()->SetInput("", &stream);
    const auto data = readers.front()->Read();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), data.size());

    return dynamic_cast<mitk::PlanarFigure *>(data.front().GetPointer());
  }

  nlohmann::json GetLineJson()
  {
    auto line = mitk::CreatePlanarFigure("line");
    line->SetPlaneGeometry(CreateAxialPlane());
    line->SetFromSpan(P(10, 20), P(30, 40));

    return nlohmann::json::parse(WriteToString(line));
  }
}

class mitkPlanarFigureIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureIOTestSuite);
  MITK_TEST(RoundTrip_EveryTypeOnAxialPlane);
  MITK_TEST(RoundTrip_EveryTypeOnAnisotropicPlane);
  MITK_TEST(RoundTrip_EveryTypeOnObliquePlane);
  MITK_TEST(RoundTrip_Parameters);
  MITK_TEST(RoundTrip_Stream);
  MITK_TEST(Write_IncompleteFigure_Throws);
  MITK_TEST(Read_UnknownKey_IsIgnored);
  MITK_TEST(Read_InvalidContent_Throws);
  MITK_TEST(MimeType_AppliesByContent);
  CPPUNIT_TEST_SUITE_END();

  std::vector<std::string> m_TemporaryFiles;

  std::string CreateTemporaryFile(const std::string &templateName)
  {
    m_TemporaryFiles.push_back(mitk::IOUtil::CreateTemporaryFile(templateName));
    return m_TemporaryFiles.back();
  }

  mitk::PlanarFigure::Pointer RoundTripThroughFile(const mitk::PlanarFigure *figure)
  {
    const auto path = this->CreateTemporaryFile("figure_XXXXXX.pf.json");
    mitk::IOUtil::Save(figure, path);

    const auto data = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), data.size());

    return dynamic_cast<mitk::PlanarFigure *>(data.front().GetPointer());
  }

  void RoundTripEveryType(const mitk::PlaneGeometry *plane)
  {
    for (const auto &typeId : TypeIds)
    {
      const auto figure = CreateFigure(typeId, plane);
      AssertEqual(figure, this->RoundTripThroughFile(figure));
    }
  }

public:
  void tearDown() override
  {
    for (const auto &file : m_TemporaryFiles)
      std::filesystem::remove(file);

    m_TemporaryFiles.clear();
  }

  void RoundTrip_EveryTypeOnAxialPlane()
  {
    this->RoundTripEveryType(CreateAxialPlane());
  }

  void RoundTrip_EveryTypeOnAnisotropicPlane()
  {
    this->RoundTripEveryType(CreateAnisotropicPlane());
  }

  void RoundTrip_EveryTypeOnObliquePlane()
  {
    this->RoundTripEveryType(CreateObliquePlane());
  }

  void RoundTrip_Parameters()
  {
    auto doubleEllipse = mitk::PlanarDoubleEllipse::New();
    doubleEllipse->SetPlaneGeometry(CreateObliquePlane());
    doubleEllipse->SetFromSpan(P(50, 50), P(70, 60));
    doubleEllipse->SetThickness(3.0 / 7.0);
    doubleEllipse->SetShapeLocked(true);
    AssertEqual(doubleEllipse, this->RoundTripThroughFile(doubleEllipse));

    auto ellipse = mitk::PlanarEllipse::New();
    ellipse->SetPlaneGeometry(CreateObliquePlane());
    ellipse->SetFromSpan(P(50, 50), P(70, 60));

    mitk::Vector2D direction;
    direction[0] = 1.0;
    direction[1] = 3.0;
    ellipse->SetReferenceDirection(direction);
    AssertEqual(ellipse, this->RoundTripThroughFile(ellipse));
  }

  void RoundTrip_Stream()
  {
    for (const auto &typeId : TypeIds)
    {
      const auto figure = CreateFigure(typeId, CreateObliquePlane());
      AssertEqual(figure, ReadFromString(WriteToString(figure)));
    }
  }

  void Write_IncompleteFigure_Throws()
  {
    auto line = mitk::CreatePlanarFigure("line");
    line->SetPlaneGeometry(CreateAxialPlane());
    line->BeginPlacement();
    line->SetFromSpan(P(10, 20), P(30, 40));

    CPPUNIT_ASSERT_THROW(WriteToString(line), mitk::Exception);
  }

  void Read_UnknownKey_IsIgnored()
  {
    auto json = GetLineJson();
    json["Comment"] = "Written by a newer writer";

    CPPUNIT_ASSERT_EQUAL(std::string("line"), ReadFromString(json.dump())->GetTypeId());
  }

  void Read_InvalidContent_Throws()
  {
    const auto valid = GetLineJson();

    auto json = valid;
    json["FileFormat"] = "MITK ROI";
    CPPUNIT_ASSERT_THROW_MESSAGE("Other format", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["Version"] = 2;
    CPPUNIT_ASSERT_THROW_MESSAGE("Newer version", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["Type"] = "square";
    CPPUNIT_ASSERT_THROW_MESSAGE("Unknown type", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json.erase("Plane");
    CPPUNIT_ASSERT_THROW_MESSAGE("Missing plane", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["Plane"]["Bounds"] = { 0, 0, 0, 100, 0, 1 };
    CPPUNIT_ASSERT_THROW_MESSAGE("Plane without extent", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["ControlPoints"] = { { 10, 20 } };
    CPPUNIT_ASSERT_THROW_MESSAGE("Too few control points", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["ControlPoints"] = { { 10, 20 }, { 30 } };
    CPPUNIT_ASSERT_THROW_MESSAGE("Malformed control point", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["Parameters"] = { { "Thickness", 2.0 } };
    CPPUNIT_ASSERT_THROW_MESSAGE("Parameter of another type", ReadFromString(json.dump()), mitk::Exception);

    json = valid;
    json["Type"] = "ellipse";
    json["ControlPoints"] = { { 50, 50 }, { 60, 50 }, { 55, 60 } };
    CPPUNIT_ASSERT_THROW_MESSAGE("Skewed axes", ReadFromString(json.dump()), mitk::Exception);

    CPPUNIT_ASSERT_THROW_MESSAGE("Not JSON", ReadFromString("<PlanarFigure />"), mitk::Exception);
  }

  void MimeType_AppliesByContent()
  {
    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());

    const auto appliesTo = [&](const std::string &path) {
      const auto mimeTypes = mimeTypeProvider->GetMimeTypesForFile(path);
      return std::any_of(mimeTypes.begin(), mimeTypes.end(), [](const mitk::MimeType &mimeType) {
        return GetMimeTypeName() == mimeType.GetName();
      });
    };

    const auto roiPath = this->CreateTemporaryFile("roi_XXXXXX.json");
    std::ofstream(roiPath) << R"({ "FileFormat": "MITK ROI", "Version": 2, "ROIs": [] })";
    CPPUNIT_ASSERT_MESSAGE("Another JSON format is not claimed", !appliesTo(roiPath));

    const auto figurePath = this->CreateTemporaryFile("figure_XXXXXX.json");
    std::ofstream(figurePath) << GetLineJson().dump();
    CPPUNIT_ASSERT_MESSAGE("A planar figure in a JSON file is recognized", appliesTo(figurePath));
    CPPUNIT_ASSERT_EQUAL(std::string("line"), mitk::IOUtil::Load<mitk::PlanarFigure>(figurePath)->GetTypeId());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureIO)
