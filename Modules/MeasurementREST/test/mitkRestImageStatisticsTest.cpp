/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFakeRestServer.h>

#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkImageStatisticsConstants.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkLocaleSwitch.h>
#include <mitkMultiLabelMaskGenerator.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkPlanarLine.h>
#include <mitkPlaneGeometry.h>
#include <mitkPointSet.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRestImageStatistics.h>
#include <mitkRestRoute.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>
#include <functional>
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  using Constants = mitk::ImageStatisticsConstants;
  using Object = mitk::ImageStatisticsContainer::ImageStatisticsObject;

  constexpr unsigned int Size = 8;
  constexpr unsigned int Slices = 3;

  /** The value of a pixel: its column and ten times its row, the same in all slices. */
  short ValueAt(unsigned int x, unsigned int y)
  {
    return static_cast<short>(x + 10 * y);
  }

  /** An image of 8 x 8 x 3 voxels of 1 x 1 x 2 mm. */
  mitk::Image::Pointer CreateImage()
  {
    unsigned int dimensions[] = { Size, Size, Slices };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);

    mitk::Vector3D spacing;
    spacing[0] = 1.0;
    spacing[1] = 1.0;
    spacing[2] = 2.0;
    image->GetGeometry()->SetSpacing(spacing);

    mitk::ImageWriteAccessor accessor(image);
    auto* pixels = static_cast<short*>(accessor.GetData());

    for (unsigned int z = 0; z < Slices; ++z)
    {
      for (unsigned int y = 0; y < Size; ++y)
      {
        for (unsigned int x = 0; x < Size; ++x)
          pixels[x + Size * (y + Size * z)] = ValueAt(x, y);
      }
    }

    return image;
  }

  mitk::Point2D P(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  mitk::DataNode::Pointer CreateNode(const std::string& name, mitk::BaseData* data)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(data);
    return node;
  }

  /** An image with the given number of time steps of 100 ms from 0 ms on. */
  mitk::Image::Pointer CreateTimeImage(unsigned int timeSteps)
  {
    unsigned int dimensions[] = { 2, 2, 2, timeSteps };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), timeSteps > 1 ? 4 : 3, dimensions);

    if (timeSteps > 1)
    {
      auto timeGeometry = mitk::ProportionalTimeGeometry::New();
      timeGeometry->Initialize(image->GetGeometry(), timeSteps);
      timeGeometry->SetFirstTimePoint(0.0);
      timeGeometry->SetStepDuration(100.0);
      image->SetTimeGeometry(timeGeometry);
    }

    return image;
  }

  mitk::RestTimeSelection At(double timePoint)
  {
    mitk::RestTimeSelection selection;
    selection.TimePoint = timePoint;
    return selection;
  }

  struct Failure
  {
    int Status = 0;
    std::string Code;
    std::string Message;
  };

  Failure GetFailureOf(const std::function<void()>& call)
  {
    try
    {
      call();
    }
    catch (const mitk::RestException& e)
    {
      const auto& error = e.GetError()["error"];
      return { error["status"].get<int>(), error["code"].get<std::string>(), error["message"].get<std::string>() };
    }

    return {};
  }

  Object MeasureLabel(const mitk::Image* image, const mitk::MultiLabelSegmentation* segmentation, mitk::MultiLabelSegmentation::LabelValueType value)
  {
    auto generator = mitk::MultiLabelMaskGenerator::New();
    generator->SetMultiLabelSegmentation(segmentation);

    auto calculator = mitk::ImageStatisticsCalculator::New();
    calculator->SetInputImage(image);
    calculator->SetMask(generator);

    return calculator->GetStatistics()->GetStatistics(value, 0);
  }

  /** The values of a figure, with the number of its pixels and the sum of their values counted from the mask. */
  struct FigureMeasurement
  {
    Object Values;
    unsigned long Count = 0;
    double Sum = 0.0;
  };

  FigureMeasurement MeasureFigure(const mitk::Image* image, mitk::PlanarFigure* figure)
  {
    auto generator = mitk::PlanarFigureMaskGenerator::New();
    generator->SetInputImage(image);
    generator->SetPlanarFigure(figure);

    FigureMeasurement measurement;
    const auto mask = generator->GetMask(0);
    mitk::ImageReadAccessor accessor(mask);
    const auto* pixels = static_cast<const unsigned short*>(accessor.GetData());
    const auto width = mask->GetDimension(0);

    for (unsigned int i = 0; i < width * mask->GetDimension(1); ++i)
    {
      if (pixels[i] != 0)
      {
        ++measurement.Count;
        measurement.Sum += ValueAt(i % width, i / width);
      }
    }

    auto calculator = mitk::ImageStatisticsCalculator::New();
    calculator->SetInputImage(image);
    calculator->SetMask(generator);

    const auto* container = calculator->GetStatistics();
    measurement.Values = container->GetStatistics(container->GetExistingLabelValues().front(), 0);
    return measurement;
  }

  /** Makes a German locale the one of C and C++ for a while, if the system has one. */
  class GermanLocale
  {
  public:
    GermanLocale()
    {
      for (const char* name : { "de_DE.UTF-8", "de_DE.utf8", "de-DE" })
      {
        try
        {
          const std::locale german(name);
          m_Switch = std::make_unique<mitk::LocaleSwitch>(name);
          m_Previous = std::locale::global(german);
          m_Installed = true;
          return;
        }
        catch (const std::runtime_error&)
        {
        }
      }
    }

    ~GermanLocale()
    {
      if (m_Installed)
        std::locale::global(m_Previous);
    }

    GermanLocale(const GermanLocale&) = delete;
    GermanLocale& operator=(const GermanLocale&) = delete;

    bool IsInstalled() const
    {
      return m_Installed;
    }

  private:
    std::unique_ptr<mitk::LocaleSwitch> m_Switch;
    std::locale m_Previous;
    bool m_Installed = false;
  };
}

class mitkRestImageStatisticsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestImageStatisticsTestSuite);
  MITK_TEST(TestReferenceImageIsTheImageSourceOfAMask);
  MITK_TEST(TestReferenceImageSkipsSourcesThatAreNoImages);
  MITK_TEST(TestReferenceImageOfAMaskWithoutSources);
  MITK_TEST(TestTimeStepsOfA3DImage);
  MITK_TEST(TestTimeStepsOfA4DImage);
  MITK_TEST(TestTimeStepsOutsideTheDataAreRefused);
  MITK_TEST(TestTimeStepErrorIsWrittenWithoutRegardToTheLocale);
  MITK_TEST(TestValuesWithAllStatistics);
  MITK_TEST(TestValuesWithoutScalarStatistics);
  MITK_TEST(TestValuesWithoutVolume);
  MITK_TEST(TestValuesOfALabel);
  MITK_TEST(TestValuesOfACircle);
  MITK_TEST(TestValuesOfALine);
  CPPUNIT_TEST_SUITE_END();

  static Object MakeObject(bool volume, bool scalar)
  {
    Object values;
    values.AddStatistic(Constants::NUMBEROFVOXELS(), mitk::ImageStatisticsContainer::VoxelCountType(5));

    if (volume)
      values.AddStatistic(Constants::VOLUME(), 2500.0);

    if (scalar)
    {
      values.AddStatistic(Constants::MEAN(), 1.5);
      values.AddStatistic(Constants::MEDIAN(), 2.0);
      values.AddStatistic(Constants::STANDARDDEVIATION(), 0.25);
      values.AddStatistic(Constants::MINIMUM(), -3.0);
      values.AddStatistic(Constants::MAXIMUM(), 8.0);
    }

    return values;
  }

public:
  void TestReferenceImageIsTheImageSourceOfAMask()
  {
    mitk::FakeRestServer server;
    const auto storage = mitk::StandaloneDataStorage::New();
    server.Storage = storage;

    const auto image = CreateNode("Image", CreateImage());
    const auto other = CreateNode("Other image", CreateImage());
    const auto mask = CreateNode("Mask", mitk::PlanarLine::New());
    storage->Add(other);
    storage->Add(image);
    storage->Add(mask, image);

    const auto reference = mitk::FindRestReferenceImage(server, storage, mask);
    CPPUNIT_ASSERT(reference != nullptr);
    CPPUNIT_ASSERT_EQUAL(image.GetPointer(), reference.get());
  }

  void TestReferenceImageSkipsSourcesThatAreNoImages()
  {
    mitk::FakeRestServer server;
    const auto storage = mitk::StandaloneDataStorage::New();
    server.Storage = storage;

    const auto points = CreateNode("Points", mitk::PointSet::New());
    const auto image = CreateNode("Image", CreateImage());
    const auto mask = CreateNode("Mask", mitk::PlanarLine::New());

    auto sources = mitk::DataStorage::SetOfObjects::New();
    sources->InsertElement(0, points);
    sources->InsertElement(1, image);
    storage->Add(points);
    storage->Add(image);
    storage->Add(mask, sources);

    const auto reference = mitk::FindRestReferenceImage(server, storage, mask);
    CPPUNIT_ASSERT(reference != nullptr);
    CPPUNIT_ASSERT_EQUAL(image.GetPointer(), reference.get());

    // Sources that no image is among give none.
    const auto lonelyMask = CreateNode("Lonely mask", mitk::PlanarLine::New());
    storage->Add(lonelyMask, points);
    CPPUNIT_ASSERT(mitk::FindRestReferenceImage(server, storage, lonelyMask) == nullptr);
  }

  void TestReferenceImageOfAMaskWithoutSources()
  {
    mitk::FakeRestServer server;
    const auto storage = mitk::StandaloneDataStorage::New();
    server.Storage = storage;

    // Only the direct sources count: the grandparent image is no reference.
    const auto image = CreateNode("Image", CreateImage());
    const auto middle = CreateNode("Middle", mitk::PointSet::New());
    const auto mask = CreateNode("Mask", mitk::PlanarLine::New());
    const auto free = CreateNode("Free", mitk::PlanarLine::New());
    storage->Add(image);
    storage->Add(middle, image);
    storage->Add(mask, middle);
    storage->Add(free);

    CPPUNIT_ASSERT(mitk::FindRestReferenceImage(server, storage, mask) == nullptr);
    CPPUNIT_ASSERT(mitk::FindRestReferenceImage(server, storage, free) == nullptr);
  }

  void TestTimeStepsOfA3DImage()
  {
    const auto node = CreateNode("Image", CreateTimeImage(1));
    const std::vector<mitk::TimeStepType> first = { 0 };

    CPPUNIT_ASSERT(first == mitk::GetRestTimeSteps(At(0.0), node, "/p"));

    mitk::RestTimeSelection all;
    all.All = true;
    CPPUNIT_ASSERT(first == mitk::GetRestTimeSteps(all, node, "/p"));
  }

  void TestTimeStepsOfA4DImage()
  {
    const auto node = CreateNode("Series", CreateTimeImage(4));
    using Steps = std::vector<mitk::TimeStepType>;

    CPPUNIT_ASSERT(Steps({ 0 }) == mitk::GetRestTimeSteps(At(0.0), node, "/p"));
    CPPUNIT_ASSERT(Steps({ 1 }) == mitk::GetRestTimeSteps(At(100.0), node, "/p"));
    CPPUNIT_ASSERT(Steps({ 1 }) == mitk::GetRestTimeSteps(At(150.0), node, "/p"));
    CPPUNIT_ASSERT(Steps({ 3 }) == mitk::GetRestTimeSteps(At(399.0), node, "/p"));

    mitk::RestTimeSelection all;
    all.All = true;
    CPPUNIT_ASSERT(Steps({ 0, 1, 2, 3 }) == mitk::GetRestTimeSteps(all, node, "/p"));
  }

  void TestTimeStepsOutsideTheDataAreRefused()
  {
    const auto node = CreateNode("Series", CreateTimeImage(4));

    // The end of the last step belongs to no step.
    for (const double timePoint : { 400.0, 5000.0, -1.0 })
    {
      const auto failure = GetFailureOf([&]() { mitk::GetRestTimeSteps(At(timePoint), node, "/api/v1/statistics"); });

      CPPUNIT_ASSERT_EQUAL_MESSAGE(std::to_string(timePoint), 422, failure.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("NO_DATA_AT_TIME_POINT"), failure.Code);
      CPPUNIT_ASSERT(failure.Message.find("\"Series\" has no data at ") == 0);
      CPPUNIT_ASSERT(failure.Message.find("Its time_steps say which time points it covers.") != std::string::npos);
    }

    const auto failure = GetFailureOf([&]() { mitk::GetRestTimeSteps(At(5000.0), node, "/p"); });
    CPPUNIT_ASSERT(failure.Message.find("has no data at 5000.0 ms.") != std::string::npos);
  }

  void TestTimeStepErrorIsWrittenWithoutRegardToTheLocale()
  {
    const auto node = CreateNode("Series", CreateTimeImage(4));
    const GermanLocale german;

    if (!german.IsInstalled())
      return;

    const auto failure = GetFailureOf([&]() { mitk::GetRestTimeSteps(At(1234.5), node, "/p"); });
    CPPUNIT_ASSERT(failure.Message.find("has no data at 1234.5 ms.") != std::string::npos);
  }

  void TestValuesWithAllStatistics()
  {
    nlohmann::json json = nlohmann::json::object();
    mitk::AddRestStatisticsValues(json, MakeObject(true, true));

    CPPUNIT_ASSERT_EQUAL(std::size_t(7), json.size());
    CPPUNIT_ASSERT_EQUAL(5ul, json["voxels"].get<unsigned long>());
    CPPUNIT_ASSERT_EQUAL(2.5, json["volume_ml"].get<double>());
    CPPUNIT_ASSERT_EQUAL(1.5, json["mean"].get<double>());
    CPPUNIT_ASSERT_EQUAL(2.0, json["median"].get<double>());
    CPPUNIT_ASSERT_EQUAL(0.25, json["standard_deviation"].get<double>());
    CPPUNIT_ASSERT_EQUAL(-3.0, json["min"].get<double>());
    CPPUNIT_ASSERT_EQUAL(8.0, json["max"].get<double>());
  }

  void TestValuesWithoutScalarStatistics()
  {
    // Images without scalar pixels have only voxels and volume; fields of the object stay.
    nlohmann::json json = { { "value", 3 } };
    mitk::AddRestStatisticsValues(json, MakeObject(true, false));

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), json.size());
    CPPUNIT_ASSERT_EQUAL(3, json["value"].get<int>());
    CPPUNIT_ASSERT_EQUAL(5ul, json["voxels"].get<unsigned long>());
    CPPUNIT_ASSERT_EQUAL(2.5, json["volume_ml"].get<double>());
  }

  void TestValuesWithoutVolume()
  {
    nlohmann::json json = nlohmann::json::object();
    mitk::AddRestStatisticsValues(json, MakeObject(false, true));

    CPPUNIT_ASSERT(!json.contains("volume_ml"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), json.size());

    json = nlohmann::json::object();
    mitk::AddRestStatisticsValues(json, MakeObject(false, false));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), json.size());
    CPPUNIT_ASSERT(json.contains("voxels"));
  }

  void TestValuesOfALabel()
  {
    const auto image = CreateImage();

    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(image);
    segmentation->AddLabel(mitk::Label::New(1, "Label"), 0);

    // The columns 0 and 1 of the rows 0 and 1 in all slices: 12 voxels of 2 mm3 each.
    {
      mitk::ImageWriteAccessor accessor(segmentation->GetGroupImage(0));
      auto* labels = static_cast<mitk::Label::PixelType*>(accessor.GetData());

      for (unsigned int z = 0; z < Slices; ++z)
      {
        for (unsigned int y = 0; y < 2; ++y)
        {
          for (unsigned int x = 0; x < 2; ++x)
            labels[x + Size * (y + Size * z)] = 1;
        }
      }
    }

    const auto values = MeasureLabel(image, segmentation, 1);

    nlohmann::json json = nlohmann::json::object();
    mitk::AddRestStatisticsValues(json, values);

    CPPUNIT_ASSERT_EQUAL(12ul, json["voxels"].get<unsigned long>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.024, json["volume_ml"].get<double>(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, json["mean"].get<double>(), 1e-9);
    CPPUNIT_ASSERT_EQUAL(0.0, json["min"].get<double>());
    CPPUNIT_ASSERT_EQUAL(11.0, json["max"].get<double>());
    CPPUNIT_ASSERT_EQUAL(values.GetValueConverted<double>(Constants::MEDIAN()), json["median"].get<double>());
    CPPUNIT_ASSERT_EQUAL(values.GetValueConverted<double>(Constants::STANDARDDEVIATION()), json["standard_deviation"].get<double>());
  }

  void TestValuesOfACircle()
  {
    const auto image = CreateImage();
    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(image->GetGeometry(), mitk::AnatomicalPlane::Axial, 1);

    auto circle = mitk::PlanarCircle::New();
    circle->SetPlaneGeometry(plane);
    circle->SetFromSpan(P(4, 4), P(6, 4));

    const auto measurement = MeasureFigure(image, circle);
    CPPUNIT_ASSERT(measurement.Count > 4);

    nlohmann::json json = nlohmann::json::object();
    mitk::AddRestStatisticsValues(json, measurement.Values);

    CPPUNIT_ASSERT_EQUAL(measurement.Count, json["voxels"].get<unsigned long>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(measurement.Sum / static_cast<double>(measurement.Count), json["mean"].get<double>(), 1e-9);
    CPPUNIT_ASSERT(json["min"].get<double>() <= json["mean"].get<double>());
    CPPUNIT_ASSERT(json["mean"].get<double>() <= json["max"].get<double>());
  }

  void TestValuesOfALine()
  {
    const auto image = CreateImage();
    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(image->GetGeometry(), mitk::AnatomicalPlane::Axial, 1);

    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(plane);
    line->SetFromSpan(P(1, 3), P(6, 3));

    const auto measurement = MeasureFigure(image, line);
    CPPUNIT_ASSERT_EQUAL(6ul, measurement.Count);

    nlohmann::json json = nlohmann::json::object();
    mitk::AddRestStatisticsValues(json, measurement.Values);

    CPPUNIT_ASSERT_EQUAL(6ul, json["voxels"].get<unsigned long>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(measurement.Sum / 6.0, json["mean"].get<double>(), 1e-9);

    // A row of the image: the values of a pixel along it differ by one.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, json["max"].get<double>() - json["min"].get<double>(), 1e-9);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestImageStatistics)
