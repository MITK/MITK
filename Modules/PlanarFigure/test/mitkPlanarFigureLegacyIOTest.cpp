/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkFileReaderRegistry.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkIOMimeTypes.h>
#include <mitkIOUtil.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlaneGeometry.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <array>
#include <cmath>
#include <numbers>
#include <sstream>

namespace
{
  /* Every planar figure file of MITK-Data as read by the XML reader of file
   * format version 1 before the redesign of planar figures: world control
   * points after the reader replayed the figure constraints, and the active
   * measurements in feature order. Polygons that are not closed are read as
   * paths since.
   */
  struct LegacyFigure
  {
    std::string File;
    std::string ClassName;
    bool Closed;
    std::vector<std::array<double, 3>> WorldControlPoints;
    std::vector<std::pair<std::string, double>> Measurements;
  };

  const std::vector<LegacyFigure> LegacyFigures = {
    {"DiffusionImaging/Fiberfox/Fiducial_0.pf", "PlanarEllipse", true,
      {{3, 10.8669, 3}, {3, 12.4668636681, 1.97772320887}, {3, 11.8892, 4.6}, {3, 12.7656093748, 3}},
      {{"Major Axis", 3.79733252525}, {"Minor Axis", 3.79741874962}, {"Area", 11.3254899988}}},
    {"DiffusionImaging/Fiberfox/Fiducial_1.pf", "PlanarEllipse", true,
      {{10.7028743976, 13.1600675508, 2.73333}, {10.2302374068, 14.827695956, 2.2000386559}, {10.5574549242, 13.6731582511, 4.46667}, {10.9024753505, 12.4558056647, 4.39256208174}},
      {{"Major Axis", 3.62699317932}, {"Minor Axis", 3.62705194096}, {"Area", 10.332142687}}},
    {"DiffusionImaging/Fiberfox/Fiducial_2.pf", "PlanarEllipse", true,
      {{18.9520937516, 8.91519565969, 2.68889}, {20.0322835423, 12.7204519484, 1.35556563372}, {19.1188354823, 9.50258782445, 4.50037}, {19.7157906963, 11.6055215879, 5.32907e-15}},
      {{"Major Axis", 8.34854221344}, {"Minor Axis", 3.82324058903}, {"Area", 25.0687198506}}},
    {"DiffusionImaging/ROI1.pf", "PlanarCircle", true,
      {{62.1189, 94.5958, 3.8523}, {72.4772, 94.5958, -6.81053}},
      {{"Radius", 14.8657432542}, {"Diameter", 29.7314865083}, {"Area", 694.261573677}}},
    {"DiffusionImaging/ROI2.pf", "PlanarCircle", true,
      {{78.2656, 33.5958, 3.243}, {86.4912, 33.5958, -6.50588}},
      {{"Radius", 12.7554363553}, {"Diameter", 25.5108727106}, {"Area", 511.14075835}}},
    {"DiffusionImaging/ROI3.pf", "PlanarCircle", true,
      {{126.0966, 86.5958, 3.243}, {135.8446, 86.5958, -10.161722}},
      {{"Radius", 16.574380106}, {"Diameter", 33.1487602119}, {"Area", 863.027156306}}},
    {"ImageStatisticsTestData/Pic3DAxialPlanarFigure.pf", "PlanarRectangle", true,
      {{164.706951027, 120.827014218, 72}, {106.469984202, 120.827014218, 72}, {106.469984202, 175.424170616, 72}, {164.706951027, 175.424170616, 72}},
      {{"Circumference", 225.668246445}, {"Area", 3179.57278588}}},
    {"ImageStatisticsTestData/Pic3DCoronalPlanarFigure.pf", "PlanarRectangle", true,
      {{232.263076923, 128, 138.166153846}, {167.278461538, 128, 138.166153846}, {167.278461538, 128, 114.535384615}, {232.263076923, 128, 114.535384615}},
      {{"Circumference", 177.230769231}, {"Area", 1535.6364497}}},
    {"ImageStatisticsTestData/Pic3DSagittalPlanarFigure.pf", "PlanarRectangle", true,
      {{128, 180.948382126, 86.0030816641}, {128, 114.285824345, 86.0030816641}, {128, 114.285824345, 30.7796610169}, {128, 180.948382126, 30.7796610169}},
      {{"Circumference", 243.771956857}, {"Area", 3681.33446977}}},
    {"ImageStatisticsTestData/Pic3D_croppedPF.pf", "PlanarRectangle", true,
      {{150.50000048, 139, 115.499944982}, {148.5, 139, 115.499944982}, {148.5, 139, 112.500000435}, {150.50000048, 139, 112.500000435}},
      {{"Circumference", 9.99989005554}, {"Area", 5.99989053587}}},
    {"ImageStatisticsTestData/Pic3D_croppedPF_PartiallyOutOfImage.pf", "PlanarCircle", true,
      {{148.706557377, 137.842138153, 114}, {149.493442623, 138.638859465, 114}},
      {{"Radius", 1.11980053509}, {"Diameter", 2.23960107017}, {"Area", 3.93941028163}}},
    {"ImageStatisticsTestData/Pic3D_croppedPF_PartiallyOutOfImage2.pf", "PlanarCircle", true,
      {{151.136065574, 140.126466779, 114}, {150.093442623, 139.860893009, 114}},
      {{"Radius", 1.07591451573}, {"Diameter", 2.15182903145}, {"Area", 3.63668266489}}},
    {"ImageStatisticsTestData/US4DAxialPlanarFigure.pf", "PlanarRectangle", true,
      {{144.053846154, 129.992307692, 19}, {91.4384615385, 129.992307692, 19}, {91.4384615385, 173.192307692, 19}, {144.053846154, 173.192307692, 19}},
      {{"Circumference", 191.630769231}, {"Area", 2272.98461538}}},
    {"ImageStatisticsTestData/US4DCoronalPlanarFigure.pf", "PlanarPolygon", true,
      {{228.238461538, 170, 7.47692307692}, {258.146153846, 170, 28.5230769231}, {83.6846153846, 170, 26.8615384615}, {110.823076923, 170, 8.58461538462}, {163.992307692, 170, 18.5538461538}},
      {{"Circumference", 363.049055308}, {"Area", 2254.27313609}}},
    {"ImageStatisticsTestData/US4DSagittalPlanarFigure.pf", "PlanarRectangle", true,
      {{147, 117.835901387, 28.4530046225}, {147, 68.5292758089, 28.4530046225}, {147, 68.5292758089, 13.0693374422}, {147, 117.835901387, 13.0693374422}},
      {{"Circumference", 129.380585516}, {"Area", 758.516717672}}},
    {"ImageStatisticsTestData/US4D_croppedPF.pf", "PlanarRectangle", true,
      {{104, 135.499997097, 27.4969915733}, {104, 134.50028994, 27.4969915733}, {104, 134.50028994, 25.5000018722}, {104, 135.499997097, 25.5000018722}},
      {{"Circumference", 5.99339371479}, {"Area", 1.99640489524}}},
    {"InteractionTestData/ReferenceData/Angle.pf", "PlanarAngle", false,
      {{16.13, 15.8711, -44.012077}, {23.0511, 51.6303, -44.012077}, {113.026, 63.7423, -44.012077}},
      {{"Angle", 108.620808495}}},
    {"InteractionTestData/ReferenceData/Angle1.pf", "PlanarAngle", false,
      {{128, 163.14556962, 110.075949367}, {128, 68.3607594937, 35.5443037975}, {128, 175.297468354, 8.81012658228}},
      {{"Angle", 52.2150911031}}},
    {"InteractionTestData/ReferenceData/Angle2.pf", "PlanarAngle", false,
      {{238.541009464, 127, 103.09148265}, {140.017350158, 127, 60.2902208202}, {184.433753943, 127, 9.41324921136}},
      {{"Angle", 72.3599151855}}},
    {"InteractionTestData/ReferenceData/Bezier.pf", "PlanarBezierCurve", false,
      {{99.6388012618, 202.604100946, 72}, {104.484227129, 151.727129338, 72}, {135.979495268, 186.452681388, 72}, {131.134069401, 134.768138801, 72}, {135.17192429, 121.847003155, 72}, {145.670347003, 59.6640378549, 72}, {203.007886435, 132.345425868, 72}, {219.966876972, 90.3517350158, 72}, {136.787066246, 21.7082018927, 72}, {109.329652997, 50.7807570978, 72}},
      {{"Length", 230.49357557}}},
    {"InteractionTestData/ReferenceData/Circle.pf", "PlanarCircle", true,
      {{56.8375394322, 129.922712934, 72}, {57.6451104101, 144.458990536, 72}},
      {{"Radius", 14.5586928473}, {"Diameter", 29.1173856946}, {"Area", 665.877959253}}},
    {"InteractionTestData/ReferenceData/DoubleEllipse.pf", "PlanarDoubleEllipse", true,
      {{81.0646687697, 79.8533123028, 72}, {103.676656151, 117.809148265, 72}, {65.4737793356, 89.141501753, 72}, {64.7608187212, 52.4861354356, 72}},
      {{"Major Axis", 88.3616988731}, {"Minor Axis", 36.2958012233}, {"Thickness", 12.3252707087}}},
    {"InteractionTestData/ReferenceData/Line.pf", "PlanarLine", false,
      {{72.5374639769, 86.1858789625, 72}, {185.413544669, 171.027377522, 72}},
      {{"Length", 141.205840779}}},
    {"InteractionTestData/ReferenceData/Path.pf", "PlanarPath", false,
      {{137.594637224, 189.6829653, 72}, {102.061514196, 202.604100946, 72}, {47.1466876972, 174.339116719, 72}, {43.9164037855, 171.108832808, 72}, {30.1876971609, 128.307570978, 72}, {42.3012618297, 75.0078864353, 72}, {47.9542586751, 67.7397476341, 72}, {87.5252365931, 45.9353312303, 72}, {103.676656151, 44.3201892744, 72}, {149.708201893, 44.3201892744, 72}, {167.474763407, 48.358044164, 72}, {205.430599369, 66.9321766562, 72}, {215.121451104, 77.4305993691, 72}, {229.657728707, 134.768138801, 72}, {226.427444795, 146.074132492, 72}, {191.701892744, 183.222397476, 72}, {184.433753943, 184.837539432, 72}, {148.900630915, 188.875394322, 72}, {154.55362776, 151.727129338, 72}},
      {{"Length", 597.708097081}}},
    {"InteractionTestData/ReferenceData/Planar4PointAngle.pf", "PlanarFourPointAngle", false,
      {{148.900630915, 64.5094637224, 72}, {69.7586750789, 170.30126183, 72}, {138.402208202, 47.5504731861, 72}, {81.0646687697, 176.761829653, 72}},
      {{"Angle", 12.8705991731}}},
    {"InteractionTestData/ReferenceData/Polygon.pf", "PlanarPolygon", true,
      {{203.007886435, 97.619873817, 72}, {166.667192429, 75.0078864353, 72}, {102.061514196, 102.465299685, 72}, {100.44637224, 106.503154574, 72}, {98.8312302839, 155.764984227, 72}, {188.471608833, 164.648264984, 72}, {120.635646688, 185.64511041, 72}, {165.052050473, 198.566246057, 72}, {179.588328076, 200.181388013, 72}, {237.733438486, 170.30126183, 72}, {237.733438486, 165.455835962, 72}, {172.320189274, 135.575709779, 72}, {215.929022082, 122.654574132, 72}},
      {{"Circumference", 604.399586637}, {"Area", 9792.99799978}}},
    {"InteractionTestData/ReferenceData/Rectangle.pf", "PlanarRectangle", true,
      {{207.045741325, 129.922712934, 72}, {161.821766562, 129.922712934, 72}, {161.821766562, 214.717665615, 72}, {207.045741325, 214.717665615, 72}},
      {{"Circumference", 260.03785489}, {"Area", 3834.76480013}}},
    {"InteractionTestData/ReferenceData/SubDivision.pf", "PlanarSubdivisionPolygon", true,
      {{93.1782334385, 74.2003154574, 72}, {64.1056782334, 153.342271293, 72}, {191.701892744, 172.723974763, 72}, {190.894321767, 91.1593059937, 72}, {124.673501577, 132.345425868, 72}},
      {{"Circumference", 457.575361618}, {"Area", 9177.66657025}}},
    {"RenderingTestData/PlanarFigures/FourPointAngle1.pf", "PlanarFourPointAngle", false,
      {{102.636, 153.591, 0}, {155.73, 206.685, 0}, {102.636, 206.579, 0}, {155.623, 180.563, 0}},
      {{"Angle", 71.1505007215}}},
    {"RenderingTestData/PlanarFigures/Line1.pf", "PlanarLine", false,
      {{307.589, 153.901, 0}, {307.589, 206.64, 0}},
      {{"Length", 52.739}}},
    {"RenderingTestData/PlanarFigures/Path1.pf", "PlanarPath", false,
      {{102.796, 306.639, 0}, {102.453, 256.64, 0}, {156.22, 256.64, 0}, {156.22, 306.639, 0}, {205.192, 306.639, 0}},
      {{"Length", 202.7381765}}},
    {"RenderingTestData/PlanarFigures/Polygon1.pf", "PlanarPolygon", true,
      {{166.532, 345.49, 0}, {166.532, 319.292, 0}, {192.826, 319.581, 0}, {192.729, 345.49, 0}},
      {{"Circumference", 104.599769743}, {"Area", 683.7941425}}},
    {"RenderingTestData/PlanarFigures/Rectangle1.pf", "PlanarRectangle", true,
      {{345.592, 319.875, 0}, {319.778, 319.875, 0}, {319.778, 345.56, 0}, {345.592, 345.56, 0}},
      {{"Circumference", 102.998}, {"Area", 663.03259}}}
  };

  // The golden values carry 12 significant digits.
  constexpr double Epsilon = 1e-6;

  /* Rectangles keep two opposite corners since: corners 1 and 3, where the
   * placement started and ended. Ellipses and double ellipses keep the
   * center and the ends of both outer axes; the fourth point only gives the
   * reference direction or the thickness.
   */
  std::vector<std::array<double, 3>> GetExpectedWorldControlPoints(const LegacyFigure &figure)
  {
    if (figure.ClassName == "PlanarRectangle")
      return { figure.WorldControlPoints[1], figure.WorldControlPoints[3] };

    if (figure.ClassName == "PlanarEllipse" || figure.ClassName == "PlanarDoubleEllipse")
      return { figure.WorldControlPoints[0], figure.WorldControlPoints[1], figure.WorldControlPoints[2] };

    return figure.WorldControlPoints;
  }

  mitk::Point2D MapToPlane(const mitk::PlanarFigure *figure, const std::array<double, 3> &worldPoint)
  {
    mitk::Point3D point;
    point[0] = worldPoint[0];
    point[1] = worldPoint[1];
    point[2] = worldPoint[2];

    mitk::Point2D planePoint;
    figure->GetPlaneGeometry()->Map(point, planePoint);
    return planePoint;
  }

  /* The measurements of the current API equal the features above, except
   * that the major axis of an ellipse is the longer one. The features named
   * the axis of control point 1 the major axis, whatever its length.
   */
  std::vector<std::pair<std::string, double>> GetExpectedMeasurements(const LegacyFigure &figure)
  {
    auto measurements = figure.Measurements;

    if (figure.ClassName == "PlanarEllipse" || figure.ClassName == "PlanarDoubleEllipse")
    {
      auto &majorAxis = measurements[0].second;
      auto &minorAxis = measurements[1].second;

      if (majorAxis < minorAxis)
        std::swap(majorAxis, minorAxis);
    }

    return measurements;
  }

  const std::string AxialGeometry =
    R"(<Geometry>)"
    R"(<transformParam param0="1" param1="0" param2="0" param3="0" param4="1" param5="0" param6="0" param7="0" param8="1" param9="0" param10="0" param11="0" />)"
    R"(<boundsParam bound0="0" bound1="100" bound2="0" bound3="100" bound4="0" bound5="1" />)"
    R"(<Spacing x="1" y="1" z="1" />)"
    R"(<Origin x="0" y="0" z="0" />)"
    R"(</Geometry>)";

  std::string Vertices(const std::vector<std::array<double, 2>> &points)
  {
    std::ostringstream vertices;

    for (std::size_t i = 0; i < points.size(); ++i)
      vertices << R"(<Vertex id=")" << i << R"(" x=")" << points[i][0] << R"(" y=")" << points[i][1] << R"(" />)";

    return vertices.str();
  }

  std::string BoolProperty(const std::string &key, bool value)
  {
    return R"(<property key=")" + key + R"(" type="BoolProperty"><bool value=")" + (value ? "true" : "false") + R"(" /></property>)";
  }

  std::string Figure(const std::string &type, const std::string &content)
  {
    return R"(<PlanarFigure type=")" + type + R"(">)" + content + "</PlanarFigure>";
  }

  std::string File(const std::string &figures, const std::string &version = R"(<Version FileVersion="1" />)")
  {
    return R"(<?xml version="1.0" ?>)" + version + figures;
  }

  std::string ControlPoints(const std::vector<std::array<double, 2>> &points)
  {
    return "<ControlPoints>" + Vertices(points) + "</ControlPoints>";
  }

  std::vector<mitk::BaseData::Pointer> ReadLegacy(const std::string &content)
  {
    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    mitk::FileReaderRegistry registry;
    auto readers = registry.GetReaders(mimeTypeProvider->GetMimeTypeForName(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".planarfigure.legacy"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), readers.size());

    std::istringstream stream(content);
    readers.front()->SetInput("", &stream);
    return readers.front()->Read();
  }

  mitk::PlanarFigure::Pointer ReadLegacyFigure(const std::string &type, const std::string &content)
  {
    const auto data = ReadLegacy(File(Figure(type, content + AxialGeometry)));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), data.size());

    return dynamic_cast<mitk::PlanarFigure *>(data.front().GetPointer());
  }
}

class mitkPlanarFigureLegacyIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureLegacyIOTestSuite);

  MITK_TEST(ReadsEveryLegacyFileOfTestData);
  MITK_TEST(FixedSizeFigures_HaveRadiusZero);
  MITK_TEST(SingleLineCrosses_AreLines);
  MITK_TEST(Cross_KeepsBothLines);
  MITK_TEST(Cross_RoundedCoordinates_Load);
  MITK_TEST(Cross_ReplaysOldConstraints);
  MITK_TEST(Arrow_IsRead);
  MITK_TEST(SeveralFigures_AreRead);
  MITK_TEST(InvalidFiles_Throw);

  CPPUNIT_TEST_SUITE_END();

public:
  void FixedSizeFigures_HaveRadiusZero()
  {
    const auto circle = ReadLegacyFigure("PlanarCircle", ControlPoints({ { 10, 20 } }));
    CPPUNIT_ASSERT_EQUAL(std::string("circle"), circle->GetTypeId());
    CPPUNIT_ASSERT_EQUAL(0.0, *circle->GetMeasurement(mitk::PlanarFigure::MeasurementType::Radius));

    const auto doubleEllipse = ReadLegacyFigure("PlanarDoubleEllipse", ControlPoints({ { 10, 20 } }));
    CPPUNIT_ASSERT_EQUAL(std::string("double_ellipse"), doubleEllipse->GetTypeId());
    CPPUNIT_ASSERT_EQUAL(3u, doubleEllipse->GetNumberOfControlPoints());
    CPPUNIT_ASSERT_EQUAL(0.0, static_cast<const mitk::PlanarDoubleEllipse *>(doubleEllipse.GetPointer())->GetThickness());
  }

  void SingleLineCrosses_AreLines()
  {
    const auto singleLine = ReadLegacyFigure("PlanarCross", BoolProperty("SingleLineMode", true) + ControlPoints({ { 10, 20 }, { 50, 20 } }));
    CPPUNIT_ASSERT_EQUAL(std::string("line"), singleLine->GetTypeId());

    const auto withoutSecondLine = ReadLegacyFigure("PlanarCross", ControlPoints({ { 10, 20 }, { 50, 20 } }));
    CPPUNIT_ASSERT_EQUAL(std::string("line"), withoutSecondLine->GetTypeId());
    CPPUNIT_ASSERT_EQUAL(40.0, *withoutSecondLine->GetMeasurement(mitk::PlanarFigure::MeasurementType::Length));
  }

  void Cross_KeepsBothLines()
  {
    const auto cross = ReadLegacyFigure("PlanarCross",
      BoolProperty("SingleLineMode", false) + ControlPoints({ { 10, 20 }, { 50, 20 }, { 30, 30 }, { 30, 15 } }));

    CPPUNIT_ASSERT_EQUAL(std::string("cross"), cross->GetTypeId());
    CPPUNIT_ASSERT_EQUAL(40.0, *cross->GetMeasurement(mitk::PlanarFigure::MeasurementType::LongAxis));
    CPPUNIT_ASSERT_EQUAL(15.0, *cross->GetMeasurement(mitk::PlanarFigure::MeasurementType::ShortAxis));
  }

  void Cross_RoundedCoordinates_Load()
  {
    // Files written before late 2013 stored six significant digits, like Vertices() does.
    const auto angle = 23.7 * std::numbers::pi / 180.0;
    const std::array<double, 2> direction = { std::cos(angle), std::sin(angle) };
    const std::array<double, 2> normal = { -direction[1], direction[0] };

    const auto at = [&](double along, double across) {
      return std::array<double, 2>{ 51.37 + direction[0] * along + normal[0] * across, 48.91 + direction[1] * along + normal[1] * across };
    };

    const auto cross = ReadLegacyFigure("PlanarCross", ControlPoints({ at(-20, 0), at(20, 0), at(3, 7), at(3, -5) }));

    CPPUNIT_ASSERT_EQUAL(std::string("cross"), cross->GetTypeId());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, *cross->GetMeasurement(mitk::PlanarFigure::MeasurementType::LongAxis), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, *cross->GetMeasurement(mitk::PlanarFigure::MeasurementType::ShortAxis), 1e-3);
  }

  void Cross_ReplaysOldConstraints()
  {
    const auto assertControlPoint = [](const mitk::PlanarFigure *figure, std::size_t index, double x, double y) {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(x, figure->GetControlPoint(index)[0], Epsilon);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(y, figure->GetControlPoint(index)[1], Epsilon);
    };

    // The crossing is clipped to the end of the first line.
    const auto clipped = ReadLegacyFigure("PlanarCross", ControlPoints({ { 10, 20 }, { 50, 20 }, { 60, 30 }, { 55, 15 } }));
    assertControlPoint(clipped, 2, 50, 30);
    assertControlPoint(clipped, 3, 50, 15);

    // A second point on the side of the first one moves onto the first line.
    const auto tShaped = ReadLegacyFigure("PlanarCross", ControlPoints({ { 10, 20 }, { 50, 20 }, { 30, 30 }, { 32, 25 } }));
    assertControlPoint(tShaped, 2, 30, 30);
    assertControlPoint(tShaped, 3, 30, 20);
  }

  void Arrow_IsRead()
  {
    const auto arrow = ReadLegacyFigure("PlanarArrow", ControlPoints({ { 10, 20 }, { 50, 50 } }));

    CPPUNIT_ASSERT_EQUAL(std::string("arrow"), arrow->GetTypeId());
    CPPUNIT_ASSERT_EQUAL(50.0, *arrow->GetMeasurement(mitk::PlanarFigure::MeasurementType::Length));
  }

  void SeveralFigures_AreRead()
  {
    const auto data = ReadLegacy(File(
      Figure("PlanarLine", ControlPoints({ { 10, 20 }, { 50, 20 } }) + AxialGeometry) +
      Figure("PlanarCircle", ControlPoints({ { 10, 20 }, { 15, 20 } }) + AxialGeometry)));

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), data.size());
  }

  void InvalidFiles_Throw()
  {
    const auto line = ControlPoints({ { 10, 20 }, { 50, 20 } }) + AxialGeometry;

    CPPUNIT_ASSERT_THROW_MESSAGE("Version 2",
      ReadLegacy(File(Figure("PlanarLine", line), R"(<Version FileVersion="2" />)")), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("No version", ReadLegacy(File(Figure("PlanarLine", line), "")), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("Unknown type", ReadLegacy(File(Figure("PlanarSquare", line))), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("No figure", ReadLegacy(File("")), mitk::Exception);

    CPPUNIT_ASSERT_THROW_MESSAGE("No geometry",
      ReadLegacy(File(Figure("PlanarLine", ControlPoints({ { 10, 20 }, { 50, 20 } })))), mitk::Exception);

    CPPUNIT_ASSERT_THROW_MESSAGE("Incomplete geometry",
      ReadLegacy(File(Figure("PlanarLine", ControlPoints({ { 10, 20 }, { 50, 20 } }) +
        R"(<Geometry><boundsParam bound0="0" bound1="100" bound2="0" bound3="100" bound4="0" bound5="1" /></Geometry>)"))),
      mitk::Exception);

    CPPUNIT_ASSERT_THROW_MESSAGE("Vertex without y",
      ReadLegacy(File(Figure("PlanarLine",
        R"(<ControlPoints><Vertex id="0" x="10" y="20" /><Vertex id="1" x="50" /></ControlPoints>)" + AxialGeometry))),
      mitk::Exception);

    CPPUNIT_ASSERT_THROW_MESSAGE("Gap in the ids",
      ReadLegacy(File(Figure("PlanarLine",
        R"(<ControlPoints><Vertex id="0" x="10" y="20" /><Vertex id="2" x="50" y="20" /></ControlPoints>)" + AxialGeometry))),
      mitk::Exception);

    CPPUNIT_ASSERT_THROW_MESSAGE("Too few control points",
      ReadLegacy(File(Figure("PlanarLine", ControlPoints({ { 10, 20 } }) + AxialGeometry))), mitk::Exception);
  }

  void ReadsEveryLegacyFileOfTestData()
  {
    for (const auto& expected : LegacyFigures)
    {
      const auto data = mitk::IOUtil::Load(GetTestDataFilePath(expected.File));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, std::size_t(1), data.size());

      auto figure = dynamic_cast<mitk::PlanarFigure*>(data.front().GetPointer());
      CPPUNIT_ASSERT_MESSAGE(expected.File, figure != nullptr);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expected.ClassName, std::string(figure->GetNameOfClass()));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expected.Closed, figure->IsClosed());

      // Properties of the file only steer the conversion.
      CPPUNIT_ASSERT_MESSAGE(expected.File, nullptr == figure->GetPropertyList()->GetProperty("subdivision"));

      const auto expectedWorldControlPoints = GetExpectedWorldControlPoints(expected);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expectedWorldControlPoints.size(), static_cast<std::size_t>(figure->GetNumberOfControlPoints()));

      for (unsigned int i = 0; i < expectedWorldControlPoints.size(); ++i)
      {
        const auto point = figure->GetWorldControlPoint(i);

        for (unsigned int j = 0; j < 3; ++j)
          CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(expected.File, expectedWorldControlPoints[i][j], point[j], Epsilon);
      }

      if (expected.ClassName == "PlanarEllipse")
      {
        const auto direction = static_cast<const mitk::PlanarEllipse *>(figure)->GetReferenceDirection();
        CPPUNIT_ASSERT_MESSAGE(expected.File, direction.has_value());

        auto expectedDirection = MapToPlane(figure, expected.WorldControlPoints[3]) - MapToPlane(figure, expected.WorldControlPoints[0]);
        expectedDirection.Normalize();

        for (unsigned int j = 0; j < 2; ++j)
          CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(expected.File, expectedDirection[j], (*direction)[j], Epsilon);
      }

      figure->EvaluateFeatures();

      std::vector<std::pair<std::string, double>> measurements;

      for (unsigned int i = 0; i < figure->GetNumberOfFeatures(); ++i)
      {
        if (figure->IsFeatureActive(i))
          measurements.emplace_back(figure->GetFeatureName(i), figure->GetQuantity(i));
      }

      CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expected.Measurements.size(), measurements.size());

      for (std::size_t i = 0; i < measurements.size(); ++i)
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expected.Measurements[i].first, measurements[i].first);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(expected.File, expected.Measurements[i].second, measurements[i].second, Epsilon);
      }

      const auto expectedMeasurements = GetExpectedMeasurements(expected);
      const auto currentMeasurements = figure->GetMeasurements();

      CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expectedMeasurements.size(), currentMeasurements.size());

      for (std::size_t i = 0; i < currentMeasurements.size(); ++i)
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(expected.File, expectedMeasurements[i].first,
          mitk::PlanarFigure::GetDisplayName(currentMeasurements[i].Type));
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(expected.File, expectedMeasurements[i].second, currentMeasurements[i].Value, Epsilon);
      }
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureLegacyIO)
