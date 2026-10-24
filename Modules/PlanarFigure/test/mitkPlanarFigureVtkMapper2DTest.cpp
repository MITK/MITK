/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkPlanarFigure.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkRenderWindow.h>

namespace
{
  const std::vector<std::string> TypeIds = { "line", "arrow", "angle", "four_point_angle", "cross", "circle", "ellipse",
                                             "double_ellipse", "rectangle", "polygon", "path", "subdivision_polygon",
                                             "bezier_curve" };
}

class mitkPlanarFigureVtkMapper2DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureVtkMapper2DTestSuite);
  MITK_TEST(RenderEveryType);
  MITK_TEST(RenderLineWidths);
  MITK_TEST(RenderDarkTranslucentFigures);
  MITK_TEST(RenderLabels);
  MITK_TEST(RenderHighDpi);
  CPPUNIT_TEST_SUITE_END();

  mitk::RenderingTestHelper m_RenderingTestHelper;
  mitk::PlaneGeometry::ConstPointer m_Plane;

  /** Point of the plane, relative to a cell of a grid of 4 x 4 cells across the plane. */
  mitk::Point2D P(std::size_t cell, double x, double y) const
  {
    const auto cellWidth = m_Plane->GetExtentInMM(0) / 4.0;
    const auto cellHeight = m_Plane->GetExtentInMM(1) / 4.0;

    mitk::Point2D point;
    point[0] = (cell % 4 + x) * cellWidth;
    point[1] = (cell / 4 + y) * cellHeight;
    return point;
  }

  mitk::PlanarFigure::Pointer CreateFigure(const std::string &typeId, std::size_t cell) const
  {
    auto figure = mitk::CreatePlanarFigure(typeId);
    figure->SetPlaneGeometry(m_Plane);

    const auto center = P(cell, 0.5, 0.5);
    const auto rim = P(cell, 0.85, 0.5);

    if ("line" == typeId || "arrow" == typeId)
      figure->SetFromSpan(P(cell, 0.15, 0.2), P(cell, 0.85, 0.8));
    else if ("angle" == typeId)
      figure->SetControlPoints({ P(cell, 0.2, 0.8), P(cell, 0.2, 0.2), P(cell, 0.8, 0.45) });
    else if ("four_point_angle" == typeId)
      figure->SetControlPoints({ P(cell, 0.15, 0.2), P(cell, 0.85, 0.5), P(cell, 0.2, 0.8), P(cell, 0.8, 0.25) });
    else if ("cross" == typeId)
      figure->SetFromSpan(P(cell, 0.15, 0.5), P(cell, 0.85, 0.5));
    else if ("circle" == typeId)
      figure->SetFromSpan(center, rim);
    else if ("ellipse" == typeId || "double_ellipse" == typeId)
    {
      figure->SetFromSpan(center, rim);
      figure->MoveHandle(2, P(cell, 0.5, 0.75));
    }
    else if ("rectangle" == typeId)
      figure->SetFromSpan(P(cell, 0.15, 0.25), P(cell, 0.85, 0.75));
    else if ("bezier_curve" == typeId)
      figure->SetControlPoints({ P(cell, 0.1, 0.8), P(cell, 0.3, 0.1), P(cell, 0.7, 0.9), P(cell, 0.9, 0.2) });
    else
      figure->SetControlPoints({ P(cell, 0.15, 0.2), P(cell, 0.85, 0.3), P(cell, 0.7, 0.85), P(cell, 0.25, 0.7) });

    return figure;
  }

  /** Adds every type, one per cell, and returns their nodes. */
  std::vector<mitk::DataNode::Pointer> AddEveryType()
  {
    std::vector<mitk::DataNode::Pointer> nodes;

    for (std::size_t i = 0; i < TypeIds.size(); ++i)
    {
      auto node = mitk::DataNode::New();
      node->SetName(TypeIds[i]);
      node->SetData(this->CreateFigure(TypeIds[i], i));

      // Added without a new initialization of the views, which would move the slice.
      m_RenderingTestHelper.GetDataStorage()->Add(node);
      nodes.push_back(node);
    }

    return nodes;
  }

  void CompareWithReference(const std::string &name)
  {
    std::vector<std::string> arguments = {
      "-V", GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/PlanarFigureVtkMapper2D_" + name + ".png")
    };

    mitk::RenderingTestHelper::ArgcHelperClass arg(arguments);
    CPPUNIT_ASSERT_MESSAGE(name, m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()));
  }

public:
  mitkPlanarFigureVtkMapper2DTestSuite() : m_RenderingTestHelper(400, 400) {}

  void setUp() override
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(400, 400);

    auto image = mitk::DataNode::New();
    image->SetData(mitk::IOUtil::Load(GetTestDataFilePath("Pic3D.nrrd"))[0]);
    m_RenderingTestHelper.AddNodeToStorage(image);
    m_RenderingTestHelper.Render();

    auto *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());
    m_Plane = renderer->GetCurrentWorldPlaneGeometry();
  }

  void tearDown() override
  {
    m_Plane = nullptr;
  }

  void RenderEveryType()
  {
    this->AddEveryType();
    this->CompareWithReference("EveryType");
  }

  void RenderLineWidths()
  {
    const auto nodes = this->AddEveryType();

    for (std::size_t i = 0; i < nodes.size(); ++i)
      nodes[i]->SetFloatProperty("line width", 0 == i % 2 ? 1.0f : 5.0f);

    this->CompareWithReference("LineWidths");
  }

  void RenderDarkTranslucentFigures()
  {
    for (auto &node : this->AddEveryType())
    {
      node->SetColor(0.1f, 0.1f, 0.5f);
      node->SetOpacity(0.5f);
    }

    this->CompareWithReference("DarkTranslucent");
  }

  void RenderLabels()
  {
    // The label of the polygon at the right border flips to its left side.
    for (const auto &[typeId, cell] : std::vector<std::pair<std::string, std::size_t>>{ { "line", 0 }, { "circle", 9 }, { "polygon", 7 } })
    {
      auto node = mitk::DataNode::New();
      node->SetName(typeId);
      node->SetData(this->CreateFigure(typeId, cell));
      node->SetBoolProperty("planarfigure.drawname", true);
      node->SetBoolProperty("planarfigure.drawquantities", true);
      m_RenderingTestHelper.GetDataStorage()->Add(node);
    }

    this->CompareWithReference("Labels");
  }

  void RenderHighDpi()
  {
    m_RenderingTestHelper.GetVtkRenderWindow()->SetDPI(144);
    this->AddEveryType();
    this->CompareWithReference("HighDpi");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureVtkMapper2D)
