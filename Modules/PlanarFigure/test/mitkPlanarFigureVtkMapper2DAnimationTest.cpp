/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkVtkMapper.h>

#include <vtkActor2D.h>
#include <vtkMapper2D.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPropAssembly.h>
#include <vtkPropCollection.h>
#include <vtkTextActor.h>

#include <array>

/**
 * The animation time follows a steady clock, so the checks hold at any time
 * of a pulse: a pulsing figure is drawn larger than at rest, by no more than
 * the amplitude of the pulse.
 */
class mitkPlanarFigureVtkMapper2DAnimationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureVtkMapper2DAnimationTestSuite);
  MITK_TEST(Pulse_GrowsFigureWithinAmplitude);
  MITK_TEST(Pulse_KeepsLabelInPlace);
  CPPUNIT_TEST_SUITE_END();

  mitk::RenderingTestHelper m_RenderingTestHelper;
  mitk::DataNode::Pointer m_Node;

  vtkPropAssembly *Render()
  {
    m_RenderingTestHelper.Render();

    auto *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());
    auto *mapper = dynamic_cast<mitk::VtkMapper *>(m_Node->GetMapper(mitk::BaseRenderer::Standard2D));
    return vtkPropAssembly::SafeDownCast(mapper->GetVtkProp(renderer));
  }

  /** The horizontal extent of everything the figure draws, in device pixels. */
  static std::pair<double, double> GetFigureExtent(vtkPropAssembly *assembly)
  {
    auto *actor = vtkActor2D::SafeDownCast(assembly->GetParts()->GetItemAsObject(0));
    auto *polyData = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper())->GetInput();

    double bounds[6];
    polyData->GetBounds(bounds);
    return { bounds[0], bounds[1] };
  }

  double RenderAndGetWidth()
  {
    const auto [left, right] = GetFigureExtent(this->Render());
    return right - left;
  }

public:
  mitkPlanarFigureVtkMapper2DAnimationTestSuite() : m_RenderingTestHelper(300, 300) {}

  void setUp() override
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(300, 300);

    auto image = mitk::DataNode::New();
    image->SetData(mitk::IOUtil::Load(GetTestDataFilePath("Pic3D.nrrd"))[0]);
    m_RenderingTestHelper.AddNodeToStorage(image);
    m_RenderingTestHelper.Render();

    const auto *plane = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow())->GetCurrentWorldPlaneGeometry();

    mitk::Point2D center;
    center[0] = 0.5 * plane->GetExtentInMM(0);
    center[1] = 0.5 * plane->GetExtentInMM(1);

    auto rim = center;
    rim[0] += 0.25 * plane->GetExtentInMM(0);

    auto circle = mitk::CreatePlanarFigure("circle");
    circle->SetPlaneGeometry(plane);
    circle->SetFromSpan(center, rim);

    m_Node = mitk::DataNode::New();
    m_Node->SetData(circle);
    m_RenderingTestHelper.GetDataStorage()->Add(m_Node);
  }

  void tearDown() override
  {
    m_Node = nullptr;
  }

  void Pulse_GrowsFigureWithinAmplitude()
  {
    const auto widthAtRest = this->RenderAndGetWidth();

    m_Node->SetBoolProperty("animated.pulse", true);
    const auto pulsingWidth = this->RenderAndGetWidth();

    // At rest only at the exact start of a pulse.
    CPPUNIT_ASSERT_MESSAGE("The figure did not pulse", pulsingWidth > widthAtRest);
    CPPUNIT_ASSERT_MESSAGE("The figure grew beyond the amplitude", pulsingWidth <= 1.08 * widthAtRest);

    m_Node->SetBoolProperty("animated.pulse", false);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(widthAtRest, this->RenderAndGetWidth(), 1e-9);
  }

  void Pulse_KeepsLabelInPlace()
  {
    // Measured before the label is shown, as its plate is drawn with the figure.
    const auto [left, right] = GetFigureExtent(this->Render());
    const auto center = 0.5 * (left + right);
    const auto largestRight = center + 1.08 * (right - center);

    // Only the name, as the measurements would not fit right of the figure.
    m_Node->SetName("Circle");
    m_Node->SetBoolProperty("planarfigure.drawquantities", false);
    m_Node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(mitk::PlanarFigureVisibilityProperty::Visibility::Always));
    m_Node->SetBoolProperty("animated.pulse", true);

    auto *assembly = this->Render();
    auto *label = vtkTextActor::SafeDownCast(assembly->GetParts()->GetItemAsObject(1));
    CPPUNIT_ASSERT(nullptr != label && label->GetVisibility());

    const std::array<double, 2> position = { label->GetPosition()[0], label->GetPosition()[1] };

    // Every render is at a later time of the pulse.
    this->Render();
    CPPUNIT_ASSERT_EQUAL(position[0], label->GetPosition()[0]);
    CPPUNIT_ASSERT_EQUAL(position[1], label->GetPosition()[1]);

    CPPUNIT_ASSERT_MESSAGE("The figure reaches into its label", largestRight < position[0]);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureVtkMapper2DAnimation)
