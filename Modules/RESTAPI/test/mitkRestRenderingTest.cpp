/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkDataNode.h>
#include <mitkException.h>
#include <mitkImage.h>
#include <mitkProperties.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRestRendering.h>
#include <mitkRestRoute.h>
#include <mitkVtkPropRenderer.h>

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace
{
  /** The status, the code and the message of the error that a call throws; status 0 if it throws none. */
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

  mitk::RestRequest MakeRequest(const std::string& target)
  {
    mitk::RestRequest request;
    request.Method = "GET";
    request.SetTarget(target);
    return request;
  }

  /** The options that a query asks for, or the failure when it is refused. */
  Failure ParseFailure(const std::string& query)
  {
    return GetFailureOf([&query]() { mitk::ParseRestScreenshotOptions(MakeRequest("/api/v1/x?" + query)); });
  }

  mitk::RestScreenshotOptions Parse(const std::string& query)
  {
    return mitk::ParseRestScreenshotOptions(MakeRequest("/api/v1/x?" + query));
  }

  /** An image of unsigned chars of the given size that holds x + 4 * y + 16 * z in the voxel (x, y, z), or `fill` per time step. */
  mitk::Image::Pointer MakeImage(unsigned int timeSteps = 1, bool rotated = false)
  {
    unsigned int dimensions[] = { 4, 4, 4, timeSteps };
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

    if (rotated)
    {
      // A half turn around the x axis lets the z axis of the image run against that of the world.
      auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
      matrix->SetElement(1, 1, -1.0);
      matrix->SetElement(2, 2, -1.0);
      image->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(matrix);
    }

    for (unsigned int t = 0; t < timeSteps; ++t)
    {
      std::vector<unsigned char> voxels(64);

      for (unsigned int i = 0; i < 64; ++i)
        voxels[i] = static_cast<unsigned char>(timeSteps > 1 ? 7 + 2 * t : i);

      image->SetVolume(voxels.data(), t);
    }

    return image;
  }

  /** A renderer in a window that is never shown, with a fixed size. */
  class TestRenderer
  {
  public:
    /** The name of a renderer tells its renderer specific properties from those of the others. */
    TestRenderer(int width, int height, MapperSlotId mapper = mitk::BaseRenderer::Standard2D)
      : m_RenderWindow(vtkSmartPointer<vtkRenderWindow>::New())
    {
      m_RenderWindow->SetSize(width, height);
      m_Renderer = mitk::VtkPropRenderer::New(
        mapper == mitk::BaseRenderer::Standard2D ? "RestRendering2D" : "RestRendering3D", m_RenderWindow);
      m_Renderer->SetMapperID(mapper);
      m_Renderer->SetConstrainZoomingAndPanning(false);
    }

    mitk::VtkPropRenderer* Get() const
    {
      return m_Renderer;
    }

    /** Shows the image in the plane, with its first slice. */
    void Show(const mitk::Image* image, mitk::AnatomicalPlane plane)
    {
      auto* navigation = m_Renderer->GetSliceNavigationController();
      navigation->SetInputWorldTimeGeometry(image->GetTimeGeometry());
      navigation->SetViewDirection(plane);
      navigation->Update();
    }

  private:
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
    mitk::VtkPropRenderer::Pointer m_Renderer;
  };

  /** The index of the slice that a renderer shows, as clients count them. */
  int ShownSlice(mitk::BaseRenderer* renderer)
  {
    const auto* stepper = renderer->GetSliceNavigationController()->GetStepper();
    return mitk::ConvertRestSliceNumber(renderer, static_cast<int>(stepper->GetSteps()), static_cast<int>(stepper->GetPos()));
  }

  mitk::Point3D MakePoint(double x, double y, double z)
  {
    mitk::Point3D point;
    point[0] = x;
    point[1] = y;
    point[2] = z;
    return point;
  }

  /** The eight corners of a box. */
  std::vector<mitk::Point3D> MakeBox(const mitk::Point3D& lower, const mitk::Point3D& upper)
  {
    std::vector<mitk::Point3D> corners;

    for (int corner = 0; corner < 8; ++corner)
    {
      corners.push_back(MakePoint(
        corner & 1 ? upper[0] : lower[0], corner & 2 ? upper[1] : lower[1], corner & 4 ? upper[2] : lower[2]));
    }

    return corners;
  }
}

class mitkRestRenderingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestRenderingTestSuite);
  MITK_TEST(SliceNumbersFollowTheImageIndexOfInvertedSlices);
  MITK_TEST(WindowsAreDescribedWithTheSliceOfTheImageIndex);
  MITK_TEST(CrosshairShowsWhereAllPlanesAreVisible);
  MITK_TEST(QueryFlagsAreTrueOrFalse);
  MITK_TEST(PixelCountsAreWholeNumbers);
  MITK_TEST(ScreenshotOptionsHaveDefaults);
  MITK_TEST(ScreenshotOptionsAreClamped);
  MITK_TEST(ScreenshotOptionsRefuseJunk);
  MITK_TEST(GridLinesRunEveryFiftyPixelsInsideTheImage);
  MITK_TEST(TimeStepsFollowTheTimePointOfTheData);
  MITK_TEST(CenterIsTheMiddleOfTheBox);
  MITK_TEST(Focus2DShowsTheSliceAndFillsTheWindow);
  MITK_TEST(Focus2DKeepsTheZoomOfDataWithoutExtent);
  MITK_TEST(Focus3DMovesTheCameraAlongItsDirection);
  MITK_TEST(Focus3DKeepsTheDistanceForDataWithoutExtent);
  MITK_TEST(ImageValuesAreReadInsideTheImageOnly);
  CPPUNIT_TEST_SUITE_END();

public:
  void SliceNumbersFollowTheImageIndexOfInvertedSlices()
  {
    // The axial stepper of an image in the standard orientation runs against its slice index, and
    // the one of an image turned around the x axis runs along it.
    const auto standard = MakeImage();
    TestRenderer standardRenderer(100, 100);
    standardRenderer.Show(standard, mitk::AnatomicalPlane::Axial);

    const auto turned = MakeImage(1, true);
    TestRenderer turnedRenderer(100, 100);
    turnedRenderer.Show(turned, mitk::AnatomicalPlane::Axial);

    for (int number = 0; number < 4; ++number)
    {
      CPPUNIT_ASSERT_EQUAL(3 - number, mitk::ConvertRestSliceNumber(standardRenderer.Get(), 4, number));
      CPPUNIT_ASSERT_EQUAL(number, mitk::ConvertRestSliceNumber(turnedRenderer.Get(), 4, number));

      // Its own inverse.
      CPPUNIT_ASSERT_EQUAL(number,
        mitk::ConvertRestSliceNumber(standardRenderer.Get(), 4, mitk::ConvertRestSliceNumber(standardRenderer.Get(), 4, number)));
    }

    // The count is that of the caller: a window with other counts converts as well.
    CPPUNIT_ASSERT_EQUAL(0, mitk::ConvertRestSliceNumber(standardRenderer.Get(), 1, 0));
    CPPUNIT_ASSERT_EQUAL(9, mitk::ConvertRestSliceNumber(standardRenderer.Get(), 10, 0));

    CPPUNIT_ASSERT_THROW(mitk::ConvertRestSliceNumber(nullptr, 4, 1), mitk::Exception);
  }

  void WindowsAreDescribedWithTheSliceOfTheImageIndex()
  {
    // The stepper of the standard orientation runs against the slice index: position 1 of 4 is slice 2.
    const auto image = MakeImage();
    TestRenderer window(300, 200);
    window.Show(image, mitk::AnatomicalPlane::Axial);
    window.Get()->GetSliceNavigationController()->GetStepper()->SetPos(1);

    const auto json = mitk::DescribeRestWindow("axial", window.Get());

    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), json["kind"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Axial"), json["view_direction"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(4, json["slice"]["count"].get<int>());
    CPPUNIT_ASSERT_EQUAL(2, json["slice"]["index"].get<int>());
    CPPUNIT_ASSERT(json.contains("timepoint_ms"));
    CPPUNIT_ASSERT(json["size"].is_array() && json["size"].size() == 2);

    // A window without data has no slices to number.
    TestRenderer empty(100, 100);
    const auto emptyJson = mitk::DescribeRestWindow("sagittal", empty.Get());
    CPPUNIT_ASSERT(emptyJson.contains("view_direction"));
    CPPUNIT_ASSERT(!emptyJson.contains("slice"));

    // The 3D window has neither a view direction nor slices.
    TestRenderer window3D(100, 100, mitk::BaseRenderer::Standard3D);
    const auto json3D = mitk::DescribeRestWindow("3d", window3D.Get());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json3D["kind"].get<std::string>());
    CPPUNIT_ASSERT(!json3D.contains("view_direction"));
    CPPUNIT_ASSERT(!json3D.contains("slice"));
    CPPUNIT_ASSERT(!mitk::IsRestRenderer2D(window3D.Get()));
    CPPUNIT_ASSERT(mitk::IsRestRenderer2D(window.Get()));
  }

  void CrosshairShowsWhereAllPlanesAreVisible()
  {
    TestRenderer window2D(100, 100);
    TestRenderer window3D(100, 100, mitk::BaseRenderer::Standard3D);

    std::vector<mitk::DataNode::Pointer> nodes;

    for (int i = 0; i < 3; ++i)
      nodes.push_back(mitk::DataNode::New());

    const auto describe = [&]() {
      return mitk::DescribeRestCrosshair({ nodes[0], nodes[1], nodes[2] }, window2D.Get(), window3D.Get());
    };

    auto json = describe();
    CPPUNIT_ASSERT(json["visible"].get<bool>());
    CPPUNIT_ASSERT(json["visible_3d"].get<bool>());

    // One hidden plane hides the crosshair, in the windows it is hidden in.
    nodes[1]->SetBoolProperty("visible", false, window3D.Get());
    json = describe();
    CPPUNIT_ASSERT(json["visible"].get<bool>());
    CPPUNIT_ASSERT(!json["visible_3d"].get<bool>());

    nodes[2]->SetBoolProperty("visible", false);
    json = describe();
    CPPUNIT_ASSERT(!json["visible"].get<bool>());
    CPPUNIT_ASSERT(!json["visible_3d"].get<bool>());

    // A plane that does not exist hides it as well.
    nodes[1]->SetBoolProperty("visible", true, window3D.Get());
    nodes[2]->SetBoolProperty("visible", true);
    json = mitk::DescribeRestCrosshair({ nodes[0].GetPointer(), nullptr, nodes[2].GetPointer() }, window2D.Get(), window3D.Get());
    CPPUNIT_ASSERT(!json["visible"].get<bool>());
    CPPUNIT_ASSERT(!json["visible_3d"].get<bool>());
  }

  void QueryFlagsAreTrueOrFalse()
  {
    const auto flag = [](const std::string& target, bool defaultValue) {
      return mitk::GetRestQueryFlag(MakeRequest(target), "flag", defaultValue);
    };

    CPPUNIT_ASSERT(flag("/p", true));
    CPPUNIT_ASSERT(!flag("/p", false));
    CPPUNIT_ASSERT(flag("/p?flag=true", false));
    CPPUNIT_ASSERT(!flag("/p?flag=false", true));

    // The first value counts.
    CPPUNIT_ASSERT(flag("/p?flag=true&flag=false", false));

    for (const std::string value : { "", "TRUE", "False", "1", "0", "yes", "true%20", "null" })
    {
      const auto failure = GetFailureOf([&]() { flag("/p?flag=" + value, true); });
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flag=" + value, 400, failure.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("flag must be true or false."), failure.Message);
    }
  }

  void PixelCountsAreWholeNumbers()
  {
    const std::vector<std::pair<std::string, int>> valid = {
      { "0", 0 }, { "100", 100 }, { "-5", -5 }, { "+7", 7 }, { "007", 7 }, { " 12", 12 }, { "2147483647", 2147483647 }
    };

    for (const auto& [text, count] : valid)
      CPPUNIT_ASSERT_EQUAL_MESSAGE(text, count, mitk::ParseRestPixelCount("size", text, "/p"));

    for (const std::string text : { "", " ", "abc", "12px", "1.5", "1e3", "0x10", "12 ", "--1", "2147483648", "99999999999999999999" })
    {
      const auto failure = GetFailureOf([&]() { mitk::ParseRestPixelCount("size", text, "/p"); });
      CPPUNIT_ASSERT_EQUAL_MESSAGE("\"" + text + "\"", 400, failure.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("size must be a number of pixels."), failure.Message);
    }
  }

  void ScreenshotOptionsHaveDefaults()
  {
    const auto options = mitk::ParseRestScreenshotOptions(MakeRequest("/api/v1/x"));

    CPPUNIT_ASSERT_EQUAL(1000u, options.MaxSize);
    CPPUNIT_ASSERT(options.BackgroundMode == mitk::RestScreenshotOptions::Background::AsShown);
    CPPUNIT_ASSERT(options.Decorations);
    CPPUNIT_ASSERT_EQUAL(1u, options.Magnification);
    CPPUNIT_ASSERT(!options.CropToContent);
    CPPUNIT_ASSERT_EQUAL(0u, options.Padding);

    // The same as the struct has them, and parameters that the query leaves out stay.
    const mitk::RestScreenshotOptions defaults;
    CPPUNIT_ASSERT_EQUAL(defaults.MaxSize, options.MaxSize);
    CPPUNIT_ASSERT_EQUAL(500u, Parse("max_size=500").MaxSize);
    CPPUNIT_ASSERT(Parse("max_size=500").Decorations);
  }

  void ScreenshotOptionsAreClamped()
  {
    CPPUNIT_ASSERT_EQUAL(100u, Parse("max_size=100").MaxSize);
    CPPUNIT_ASSERT_EQUAL(100u, Parse("max_size=99").MaxSize);
    CPPUNIT_ASSERT_EQUAL(100u, Parse("max_size=0").MaxSize);
    CPPUNIT_ASSERT_EQUAL(100u, Parse("max_size=-7").MaxSize);
    CPPUNIT_ASSERT_EQUAL(4000u, Parse("max_size=4000").MaxSize);
    CPPUNIT_ASSERT_EQUAL(4000u, Parse("max_size=4001").MaxSize);

    const auto transparent = Parse("background=transparent");
    CPPUNIT_ASSERT(transparent.BackgroundMode == mitk::RestScreenshotOptions::Background::Transparent);

    // Digits in either case.
    const auto color = Parse("background=%23FFaa00");
    CPPUNIT_ASSERT(color.BackgroundMode == mitk::RestScreenshotOptions::Background::Color);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, color.BackgroundColor[0], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(170.0 / 255.0, color.BackgroundColor[1], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, color.BackgroundColor[2], 1e-6);

    for (unsigned int scale = 1; scale <= 4; ++scale)
      CPPUNIT_ASSERT_EQUAL(scale, Parse("scale=" + std::to_string(scale)).Magnification);

    CPPUNIT_ASSERT(!Parse("decorations=false").Decorations);
    CPPUNIT_ASSERT(Parse("crop=true").CropToContent);

    // Padding is limited by half of the size, which is the clamped one.
    CPPUNIT_ASSERT_EQUAL(0u, Parse("crop=true&padding=0").Padding);
    CPPUNIT_ASSERT_EQUAL(49u, Parse("crop=true&max_size=100&padding=49").Padding);
    CPPUNIT_ASSERT_EQUAL(49u, Parse("crop=true&max_size=1&padding=49").Padding);
    CPPUNIT_ASSERT_EQUAL(499u, Parse("crop=true&padding=499").Padding);
  }

  void ScreenshotOptionsRefuseJunk()
  {
    const std::vector<std::pair<std::string, std::string>> refused = {
      { "max_size=abc", "max_size must be a number of pixels." },
      { "max_size=", "max_size must be a number of pixels." },
      { "max_size=1.5", "max_size must be a number of pixels." },
      { "background=red", "background must be transparent or a color like #ffffff." },
      { "background=%23fff", "background must be transparent or a color like #ffffff." },
      { "background=ffffff", "background must be transparent or a color like #ffffff." },
      { "background=Transparent", "background must be transparent or a color like #ffffff." },
      { "background=", "background must be transparent or a color like #ffffff." },
      { "decorations=0", "decorations must be true or false." },
      { "crop=yes", "crop must be true or false." },
      { "scale=0", "scale must be 1, 2, 3 or 4." },
      { "scale=5", "scale must be 1, 2, 3 or 4." },
      { "scale=2.0", "scale must be 1, 2, 3 or 4." },
      { "scale=", "scale must be 1, 2, 3 or 4." },
      { "padding=5", "padding needs crop=true." },
      { "crop=false&padding=0", "padding needs crop=true." },
      { "crop=true&padding=-1", "padding must be 0 or more and less than half of max_size." },
      { "crop=true&padding=500", "padding must be 0 or more and less than half of max_size." },
      { "crop=true&max_size=100&padding=50", "padding must be 0 or more and less than half of max_size." },
      { "crop=true&padding=x", "padding must be a number of pixels." },
      // The first refused parameter in the order that the options are read gives the message.
      { "scale=9&max_size=abc", "max_size must be a number of pixels." },
      { "padding=1&scale=9", "scale must be 1, 2, 3 or 4." }
    };

    for (const auto& [query, message] : refused)
    {
      const auto failure = ParseFailure(query);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(query, 400, failure.Status);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(query, std::string("INVALID_REQUEST"), failure.Code);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(query, message, failure.Message);
    }
  }

  void GridLinesRunEveryFiftyPixelsInsideTheImage()
  {
    using Lines = std::vector<mitk::RestGridLine>;

    for (const int length : { -10, 0, 1, 50 })
      CPPUNIT_ASSERT(mitk::GetRestGridLines(length).empty());

    CPPUNIT_ASSERT((Lines{ { 50, false } } == mitk::GetRestGridLines(51)));
    CPPUNIT_ASSERT((Lines{ { 50, false } } == mitk::GetRestGridLines(100)));
    CPPUNIT_ASSERT((Lines{ { 50, false }, { 100, true } } == mitk::GetRestGridLines(101)));
    CPPUNIT_ASSERT((Lines{ { 50, false }, { 100, true }, { 150, false }, { 200, true }, { 250, false } } == mitk::GetRestGridLines(260)));
  }

  void TimeStepsFollowTheTimePointOfTheData()
  {
    auto node = mitk::DataNode::New();
    node->SetName("series");
    node->SetData(MakeImage(4));

    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(0), mitk::GetRestTimeStep(node, 0.0, "/p"));
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(1), mitk::GetRestTimeStep(node, 100.0, "/p"));
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(2), mitk::GetRestTimeStep(node, 250.0, "/p"));
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(3), mitk::GetRestTimeStep(node, 399.0, "/p"));

    // The end of the last step belongs to no step.
    for (const double timePoint : { 400.0, 1000.0, -1.0 })
    {
      const auto failure = GetFailureOf([&]() { mitk::GetRestTimeStep(node, timePoint, "/p"); });
      CPPUNIT_ASSERT_EQUAL(422, failure.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("NO_DATA_AT_TIME_POINT"), failure.Code);
      CPPUNIT_ASSERT(failure.Message.find("\"series\"") != std::string::npos);
      CPPUNIT_ASSERT(failure.Message.find(std::to_string(timePoint)) != std::string::npos);
    }

    // Data with one time step covers every time point.
    node->SetData(MakeImage());
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(0), mitk::GetRestTimeStep(node, 5000.0, "/p"));
  }

  void CenterIsTheMiddleOfTheBox()
  {
    const auto single = mitk::GetRestCenter({ MakePoint(1.0, -2.0, 3.0) });
    CPPUNIT_ASSERT(mitk::Equal(MakePoint(1.0, -2.0, 3.0), single));

    // The order and the number of the points do not matter, only the extremes.
    const auto center = mitk::GetRestCenter({ MakePoint(10.0, 0.0, -4.0), MakePoint(-2.0, 4.0, 0.0), MakePoint(3.0, 1.0, 6.0) });
    CPPUNIT_ASSERT(mitk::Equal(MakePoint(4.0, 2.0, 1.0), center));

    const auto box = mitk::GetRestCenter(MakeBox(MakePoint(0.0, 0.0, 0.0), MakePoint(2.0, 4.0, 6.0)));
    CPPUNIT_ASSERT(mitk::Equal(MakePoint(1.0, 2.0, 3.0), box));
  }

  void Focus2DShowsTheSliceAndFillsTheWindow()
  {
    const auto image = MakeImage();
    TestRenderer window(200, 100);
    window.Show(image, mitk::AnatomicalPlane::Axial);
    auto* renderer = window.Get();

    // A box of 20 by 10 mm in the plane, in the slice of the third voxel row of z.
    const auto corners = MakeBox(MakePoint(0.0, 0.0, 2.0), MakePoint(20.0, 10.0, 2.0));
    mitk::FocusRestRenderer2D(renderer, corners, 0.0);

    CPPUNIT_ASSERT(mitk::Equal(2.0, renderer->GetCurrentWorldPlaneGeometry()->GetOrigin()[2]));
    CPPUNIT_ASSERT_EQUAL(2, ShownSlice(renderer));

    // 20 mm over 200 pixels and 10 mm over 100 pixels: a tenth of a millimeter per pixel.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, renderer->GetScaleFactorMMPerDisplayUnit(), 1e-6);

    // The margin is a share of the size.
    mitk::FocusRestRenderer2D(renderer, corners, 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.15, renderer->GetScaleFactorMMPerDisplayUnit(), 1e-6);

    // The longer side in proportion to the window decides.
    mitk::FocusRestRenderer2D(renderer, MakeBox(MakePoint(0.0, 0.0, 2.0), MakePoint(10.0, 20.0, 2.0)), 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, renderer->GetScaleFactorMMPerDisplayUnit(), 1e-6);

    // The window shows the center in its middle.
    const auto middle = renderer->GetCameraController()->GetCameraPositionOnPlane();
    mitk::Point2D expected;
    renderer->GetCurrentWorldPlaneGeometry()->Map(MakePoint(5.0, 10.0, 2.0), expected);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[0], middle[0], 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[1], middle[1], 1e-3);
  }

  void Focus2DKeepsTheZoomOfDataWithoutExtent()
  {
    const auto image = MakeImage();
    TestRenderer window(200, 100);
    window.Show(image, mitk::AnatomicalPlane::Axial);
    auto* renderer = window.Get();

    mitk::FocusRestRenderer2D(renderer, MakeBox(MakePoint(0.0, 0.0, 1.0), MakePoint(20.0, 10.0, 1.0)), 0.0);
    const auto scale = renderer->GetScaleFactorMMPerDisplayUnit();

    // A single point, and a line that runs through the slice, have no size in the plane.
    mitk::FocusRestRenderer2D(renderer, { MakePoint(3.0, 2.0, 3.0), MakePoint(3.0, 2.0, 3.0) }, 0.2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(scale, renderer->GetScaleFactorMMPerDisplayUnit(), 1e-9);
    CPPUNIT_ASSERT_EQUAL(3, ShownSlice(renderer));

    mitk::Point2D expected;
    renderer->GetCurrentWorldPlaneGeometry()->Map(MakePoint(3.0, 2.0, 3.0), expected);
    const auto middle = renderer->GetCameraController()->GetCameraPositionOnPlane();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[0], middle[0], 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[1], middle[1], 1e-3);

    mitk::FocusRestRenderer2D(renderer, { MakePoint(1.0, 1.0, 0.0), MakePoint(1.0, 1.0, 3.0) }, 0.2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(scale, renderer->GetScaleFactorMMPerDisplayUnit(), 1e-9);
    CPPUNIT_ASSERT(std::isfinite(renderer->GetScaleFactorMMPerDisplayUnit()));
  }

  /** A renderer whose camera looks down the z axis from 100 mm, with a view angle of 30 degrees. */
  static void PrepareCamera(mitk::BaseRenderer* renderer, bool parallel)
  {
    auto* camera = renderer->GetVtkRenderer()->GetActiveCamera();
    camera->SetParallelProjection(parallel);
    camera->SetViewAngle(30.0);
    camera->SetUseHorizontalViewAngle(false);
    camera->SetPosition(0.0, 0.0, 100.0);
    camera->SetFocalPoint(0.0, 0.0, 0.0);
    camera->SetViewUp(0.0, 1.0, 0.0);
    camera->SetParallelScale(50.0);
  }

  void Focus3DMovesTheCameraAlongItsDirection()
  {
    TestRenderer window(200, 200, mitk::BaseRenderer::Standard3D);
    auto* camera = window.Get()->GetVtkRenderer()->GetActiveCamera();

    // The corners of a cube of 20 mm around (5, 0, 0), in the view of a window of the same width and height.
    const auto cube = MakeBox(MakePoint(-5.0, -10.0, -10.0), MakePoint(15.0, 10.0, 10.0));
    const auto tanHalfAngle = std::tan(15.0 * 3.14159265358979323846 / 180.0);

    PrepareCamera(window.Get(), false);
    mitk::FocusRestRenderer3D(window.Get(), cube, 0.0);

    // The nearest corner is 10 mm toward the camera, and 10 mm up or to the side.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0 + 10.0 / tanHalfAngle, camera->GetDistance(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, camera->GetFocalPoint()[0], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, camera->GetFocalPoint()[1], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, camera->GetPosition()[0], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, camera->GetPosition()[1], 1e-9);
    CPPUNIT_ASSERT(camera->GetPosition()[2] > 0.0);

    // A margin makes the cube appear larger.
    PrepareCamera(window.Get(), false);
    mitk::FocusRestRenderer3D(window.Get(), cube, 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0 + 15.0 / tanHalfAngle, camera->GetDistance(), 1e-6);

    // A parallel projection zooms through its scale and keeps the distance.
    PrepareCamera(window.Get(), true);
    mitk::FocusRestRenderer3D(window.Get(), cube, 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, camera->GetParallelScale(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, camera->GetDistance(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, camera->GetFocalPoint()[0], 1e-9);

    // The direction stays.
    double direction[3];
    camera->GetDirectionOfProjection(direction);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, direction[2], 1e-9);
  }

  void Focus3DKeepsTheDistanceForDataWithoutExtent()
  {
    TestRenderer window(200, 200, mitk::BaseRenderer::Standard3D);
    auto* camera = window.Get()->GetVtkRenderer()->GetActiveCamera();

    for (const bool parallel : { false, true })
    {
      PrepareCamera(window.Get(), parallel);
      mitk::FocusRestRenderer3D(window.Get(), { MakePoint(1.0, 2.0, 3.0) }, 0.2);

      CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, camera->GetParallelScale(), 1e-9);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, camera->GetDistance(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, camera->GetFocalPoint()[0], 1e-9);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, camera->GetFocalPoint()[1], 1e-9);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, camera->GetFocalPoint()[2], 1e-9);
    }
  }

  void ImageValuesAreReadInsideTheImageOnly()
  {
    const auto image = MakeImage();
    const auto* geometry = image->GetGeometry();

    const auto centerOf = [&](double x, double y, double z) {
      mitk::Point3D index;
      index[0] = x;
      index[1] = y;
      index[2] = z;
      mitk::Point3D world;
      geometry->IndexToWorld(index, world);
      return world;
    };

    // The value of voxel (x, y, z) is x + 4 y + 16 z.
    CPPUNIT_ASSERT_EQUAL(0.0, mitk::GetRestImageValue(image, centerOf(0, 0, 0), 0.0).value());
    CPPUNIT_ASSERT_EQUAL(1.0 + 8.0 + 48.0, mitk::GetRestImageValue(image, centerOf(1, 2, 3), 0.0).value());
    CPPUNIT_ASSERT_EQUAL(63.0, mitk::GetRestImageValue(image, centerOf(3, 3, 3), 0.0).value());

    // The voxels reach half a voxel beyond their centers; beyond that is outside.
    CPPUNIT_ASSERT_EQUAL(63.0, mitk::GetRestImageValue(image, centerOf(3.4, 3.4, 3.4), 0.0).value());
    CPPUNIT_ASSERT_EQUAL(0.0, mitk::GetRestImageValue(image, centerOf(-0.4, -0.4, -0.4), 0.0).value());

    for (const auto& [x, y, z] : std::vector<std::array<double, 3>>{ { 3.6, 0, 0 }, { 0, 3.6, 0 }, { 0, 0, 3.6 }, { -0.6, 0, 0 }, { 0, -0.6, 0 }, { 0, 0, -0.6 } })
      CPPUNIT_ASSERT(!mitk::GetRestImageValue(image, centerOf(x, y, z), 0.0).has_value());

    // Images of the time steps hold 7, 9, 11 and 13; 100 ms long steps from 0 ms on.
    const auto series = MakeImage(4);
    const auto inside = series->GetGeometry()->GetCenter();

    CPPUNIT_ASSERT_EQUAL(7.0, mitk::GetRestImageValue(series, inside, 0.0).value());
    CPPUNIT_ASSERT_EQUAL(9.0, mitk::GetRestImageValue(series, inside, 150.0).value());
    CPPUNIT_ASSERT_EQUAL(13.0, mitk::GetRestImageValue(series, inside, 399.0).value());
    CPPUNIT_ASSERT(!mitk::GetRestImageValue(series, inside, 400.0).has_value());
    CPPUNIT_ASSERT(!mitk::GetRestImageValue(series, inside, -1.0).has_value());

    // An image of two dimensions has no value at a point in space.
    unsigned int dimensions[] = { 4, 4 };
    auto flat = mitk::Image::New();
    flat->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 2, dimensions);
    CPPUNIT_ASSERT(!mitk::GetRestImageValue(flat, MakePoint(1.0, 1.0, 0.0), 0.0).has_value());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestRendering)
