/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>

#include <mitkIOUtil.h>
#include <mitkInteractionTestHelper.h>
#include <mitkInternalEvent.h>
#include <mitkMouseMoveEvent.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseReleaseEvent.h>
#include <mitkRenderingTestHelper.h>
#include <mitkPlanarFigureInteractor.h>

#include <mitkPlanarAngle.h>
#include <mitkPlanarArrow.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarSubdivisionPolygon.h>

#include <vtkDebugLeaks.h>

#include <usModuleRegistry.h>

#include <memory>

class mitkPlanarFigureInteractionTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureInteractionTestSuite);
  MITK_TEST(AngleInteractionCreate);
  MITK_TEST(Angle2InteractionCreate);
  MITK_TEST(BezierCurveInteractionCreate);
  MITK_TEST(CircleInteractionCreate);
  MITK_TEST(DoubleEllipseInteractionCreate);
  MITK_TEST(PlanarFourPointAngleInteractionCreate);
  MITK_TEST(PlanarLineInteractionCreate);
  MITK_TEST(PlanarPolygonInteractionCreate);
  MITK_TEST(NonClosedPlanarPolygonInteractionCreate);
  MITK_TEST(RectangleInteractionCreate);
  MITK_TEST(HoverFigureThenLeave_EndsHovering);
  MITK_TEST(HoverPointThenLeave_EndsHovering);
  MITK_TEST(DragPointOutOfWindow_EndsHovering);
  MITK_TEST(QuickDragPointOutOfWindow_EndsHovering);
  MITK_TEST(ReleaseOnPointThenMoveAway_EndsHovering);
  MITK_TEST(LeaveWhileNotHovering_IsNotConsumed);
  // BUG 19304
  // MITK_TEST(PlanarSubdivisionInteractionCreate);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    /// \todo Fix leaks of vtkObjects. Bug 18095.
    vtkDebugLeaks::SetExitError(0);
  }

  void tearDown()
  {
    m_FigureInteractor = nullptr;
    m_FigureNode = nullptr;
    m_InteractionTestHelper.reset();
  }

  void PlaybackOnNewFigure(mitk::PlanarFigure *figure, const std::string &interactionXmlPath)
  {
    // Create DataNode as a container for our PlanarFigure
    m_FigureNode = mitk::DataNode::New();
    m_FigureNode->SetData(figure);

    m_InteractionTestHelper = std::make_unique<mitk::InteractionTestHelper>(GetTestDataFilePath(interactionXmlPath));

    // Load a bounding image
    mitk::Image::Pointer testImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
    figure->SetGeometry(testImage->GetGeometry());

    mitk::DataNode::Pointer dn = mitk::DataNode::New();
    dn->SetData(testImage);
    m_InteractionTestHelper->AddNodeToStorage(dn);
    m_InteractionTestHelper->GetDataStorage()->Add(m_FigureNode, dn);

    m_FigureNode->SetName("PLANAR FIGURE");
    // set as selected
    m_FigureNode->SetSelected(true);
    m_FigureNode->AddProperty("selected", mitk::BoolProperty::New(true));

    // Load state machine
    m_FigureInteractor = mitk::PlanarFigureInteractor::New();
    us::Module *planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");
    m_FigureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
    m_FigureInteractor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
    m_FigureInteractor->SetDataNode(m_FigureNode);

    // Start Interaction
    m_InteractionTestHelper->PlaybackInteraction();
  }

  void RunTest(mitk::PlanarFigure::Pointer figure, std::string interactionXmlPath, std::string referenceFigurePath)
  {
    this->PlaybackOnNewFigure(figure, interactionXmlPath);

    // Load reference PlanarFigure
    auto reference = mitk::IOUtil::Load<mitk::PlanarFigure>(GetTestDataFilePath(referenceFigurePath));

    // Compare figures
    MITK_ASSERT_EQUAL(figure, reference, "Compare figure with reference");
  }

  void AngleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarAngle::New();
    RunTest(figure, "InteractionTestData/Interactions/Angle1.xml", "InteractionTestData/ReferenceData/Angle1.pf");
  }

  void Angle2InteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarAngle::New();
    RunTest(figure, "InteractionTestData/Interactions/Angle2.xml", "InteractionTestData/ReferenceData/Angle2.pf");
  }

  void BezierCurveInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarBezierCurve::New();
    RunTest(figure, "InteractionTestData/Interactions/Bezier.xml", "InteractionTestData/ReferenceData/Bezier.pf");
  }

  void CircleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarCircle::New();
    RunTest(figure, "InteractionTestData/Interactions/Circle.xml", "InteractionTestData/ReferenceData/Circle.pf");
  }

  void DoubleEllipseInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarDoubleEllipse::New();
    RunTest(figure,
            "InteractionTestData/Interactions/DoubleEllipse.xml",
            "InteractionTestData/ReferenceData/DoubleEllipse.pf");
  }

  void PlanarSubdivisionInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarSubdivisionPolygon::New();
    RunTest(figure,
            "InteractionTestData/Interactions/SubdivisionPolygon.xml",
            "InteractionTestData/ReferenceData/SubDivision.pf");
  }

  void PlanarFourPointAngleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarFourPointAngle::New();
    RunTest(figure,
            "InteractionTestData/Interactions/Planar4PointAngle.xml",
            "InteractionTestData/ReferenceData/Planar4PointAngle.pf");
  }

  void PlanarLineInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarLine::New();
    RunTest(figure, "InteractionTestData/Interactions/Line.xml", "InteractionTestData/ReferenceData/Line.pf");
  }

  void PlanarPolygonInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarPolygon::New();
    RunTest(figure, "InteractionTestData/Interactions/Polygon.xml", "InteractionTestData/ReferenceData/Polygon.pf");
  }

  void NonClosedPlanarPolygonInteractionCreate()
  {
    mitk::PlanarPolygon::Pointer figure;
    figure = mitk::PlanarPolygon::New();
    figure->ClosedOff();
    RunTest(
      figure.GetPointer(), "InteractionTestData/Interactions/Path.xml", "InteractionTestData/ReferenceData/Path.pf");
  }

  void RectangleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarRectangle::New();
    RunTest(figure, "InteractionTestData/Interactions/Rectangle.xml", "InteractionTestData/ReferenceData/Rectangle.pf");
  }

  void HoverFigureThenLeave_EndsHovering()
  {
    this->PlaybackLineAndMoveAway();

    this->MoveTo(this->GetLineMidpointPosition());
    CPPUNIT_ASSERT_MESSAGE("Hovering over the line", this->IsHovering());

    this->LeaveWindow();
    CPPUNIT_ASSERT_MESSAGE("Hovering ends when leaving the window", !this->IsHovering());
  }

  void HoverPointThenLeave_EndsHovering()
  {
    this->PlaybackLineAndMoveAway();
    this->HoverOverControlPoint(0);

    this->LeaveWindow();
    CPPUNIT_ASSERT_MESSAGE("Hovering ends when leaving the window", !this->IsHovering());

    const auto figure = dynamic_cast<mitk::PlanarFigure *>(m_FigureNode->GetData());
    CPPUNIT_ASSERT_EQUAL(-1, figure->GetSelectedControlPoint());
  }

  void DragPointOutOfWindow_EndsHovering()
  {
    this->PlaybackLineAndMoveAway();
    this->HoverOverControlPoint(0);

    const auto pointPosition = this->GetControlPointPosition(0);
    const auto outsidePosition = GetPositionOutsideWindow(pointPosition[1]);
    this->PressAt(pointPosition);
    this->DragTo(outsidePosition);
    this->ReleaseAt(outsidePosition);
    this->LeaveWindow();

    CPPUNIT_ASSERT_MESSAGE("Hovering ends after releasing outside the window", !this->IsHovering());
  }

  void QuickDragPointOutOfWindow_EndsHovering()
  {
    this->PlaybackLineAndMoveAway();

    // Pressing right after the first move reaches the drag from the figure state, not from the point state
    const auto pointPosition = this->GetControlPointPosition(0);
    this->MoveTo(pointPosition);
    CPPUNIT_ASSERT_MESSAGE("Hovering over the line", this->IsHovering());

    const auto outsidePosition = GetPositionOutsideWindow(pointPosition[1]);
    this->PressAt(pointPosition);
    this->DragTo(outsidePosition);
    this->ReleaseAt(outsidePosition);
    this->LeaveWindow();

    CPPUNIT_ASSERT_MESSAGE("Hovering ends after releasing outside the window", !this->IsHovering());
  }

  void ReleaseOnPointThenMoveAway_EndsHovering()
  {
    this->PlaybackLineAndMoveAway();
    this->HoverOverControlPoint(0);

    const auto pointPosition = this->GetControlPointPosition(0);
    this->PressAt(pointPosition);
    this->ReleaseAt(pointPosition);
    CPPUNIT_ASSERT_MESSAGE("Still hovering after releasing on the control point", this->IsHovering());

    this->MoveTo(GetPositionFarFromLine());
    CPPUNIT_ASSERT_MESSAGE("Hovering ends with the first move away from the line", !this->IsHovering());
  }

  void LeaveWhileNotHovering_IsNotConsumed()
  {
    this->PlaybackLineAndMoveAway();

    // Other handlers, e.g. the crosshair rotation, only see the leave event if no interactor consumes it
    CPPUNIT_ASSERT_MESSAGE("Leave event is not consumed", !this->LeaveWindow());
  }

  // this is only for the OpenGL check
  mitkPlanarFigureInteractionTestSuite() : m_RenderingTestHelper(300, 300) {}
  private:
    mitk::BaseRenderer *GetLineRenderer() const
    {
      // Line.xml draws its line in this render window
      return m_InteractionTestHelper->GetRenderWindowByName("stdmulti.widget1")->GetRenderer();
    }

    mitk::Point2D GetControlPointPosition(unsigned int index) const
    {
      const auto figure = dynamic_cast<mitk::PlanarFigure *>(m_FigureNode->GetData());

      mitk::Point2D displayPosition;
      this->GetLineRenderer()->WorldToDisplay(figure->GetWorldControlPoint(index), displayPosition);
      return displayPosition;
    }

    mitk::Point2D GetLineMidpointPosition() const
    {
      const auto first = this->GetControlPointPosition(0);
      const auto second = this->GetControlPointPosition(1);
      return first + (second - first) * 0.5;
    }

    static mitk::Point2D GetPositionFarFromLine()
    {
      mitk::Point2D position;
      position[0] = 10;
      position[1] = 10;
      return position;
    }

    static mitk::Point2D GetPositionOutsideWindow(double y)
    {
      mitk::Point2D position;
      position[0] = -20;
      position[1] = y;
      return position;
    }

    static bool Send(mitk::InteractionEvent *event)
    {
      return event->GetSender()->GetDispatcher()->ProcessEvent(event);
    }

    void MoveTo(const mitk::Point2D &position) const
    {
      Send(mitk::MouseMoveEvent::New(
        this->GetLineRenderer(), position, mitk::InteractionEvent::NoButton, mitk::InteractionEvent::NoKey));
    }

    void HoverOverControlPoint(unsigned int index) const
    {
      // The first move only reaches the figure state; a real mouse sends more moves, which reach the point state
      const auto pointPosition = this->GetControlPointPosition(index);
      this->MoveTo(pointPosition);
      this->MoveTo(pointPosition);
      CPPUNIT_ASSERT_MESSAGE("Hovering over the control point", this->IsHovering());
    }

    void DragTo(const mitk::Point2D &position) const
    {
      Send(mitk::MouseMoveEvent::New(
        this->GetLineRenderer(), position, mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::NoKey));
    }

    void PressAt(const mitk::Point2D &position) const
    {
      Send(mitk::MousePressEvent::New(this->GetLineRenderer(),
                                      position,
                                      mitk::InteractionEvent::LeftMouseButton,
                                      mitk::InteractionEvent::NoKey,
                                      mitk::InteractionEvent::LeftMouseButton));
    }

    void ReleaseAt(const mitk::Point2D &position) const
    {
      Send(mitk::MouseReleaseEvent::New(this->GetLineRenderer(),
                                        position,
                                        mitk::InteractionEvent::NoButton,
                                        mitk::InteractionEvent::NoKey,
                                        mitk::InteractionEvent::LeftMouseButton));
    }

    /** Returns whether an interactor consumed the event. */
    bool LeaveWindow() const
    {
      return Send(mitk::InternalEvent::New(this->GetLineRenderer(), nullptr, "LeaveRenderWindow"));
    }

    bool IsHovering() const
    {
      bool isHovering = false;
      m_FigureNode->GetBoolProperty("planarfigure.ishovering", isHovering);
      return isHovering;
    }

    void PlaybackLineAndMoveAway()
    {
      this->PlaybackOnNewFigure(mitk::PlanarLine::New(), "InteractionTestData/Interactions/Line.xml");

      // Whatever state the playback ended in, moving away from the line leaves the interactor idle
      this->MoveTo(GetPositionFarFromLine());
      CPPUNIT_ASSERT_MESSAGE("Not hovering before the test starts", !this->IsHovering());
    }

    mitk::RenderingTestHelper m_RenderingTestHelper;
    std::unique_ptr<mitk::InteractionTestHelper> m_InteractionTestHelper;
    mitk::DataNode::Pointer m_FigureNode;
    mitk::PlanarFigureInteractor::Pointer m_FigureInteractor;
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureInteraction)
