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

#include <mitkExclusiveInteraction.h>
#include <mitkIOUtil.h>
#include <mitkInteractionTestHelper.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionKeyReleaseEvent.h>
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
#include <mitkPlanarPath.h>
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
  MITK_TEST(PathInteractionCreate);
  MITK_TEST(RectangleInteractionCreate);
  MITK_TEST(HoverFigureThenLeave_EndsHovering);
  MITK_TEST(HoverPointThenLeave_EndsHovering);
  MITK_TEST(DragPointOutOfWindow_EndsHovering);
  MITK_TEST(QuickDragPointOutOfWindow_EndsHovering);
  MITK_TEST(ReleaseOnPointThenMoveAway_EndsHovering);
  MITK_TEST(LeaveWhileNotHovering_IsNotConsumed);
  MITK_TEST(PresetCircle_IsPlacedWhereClicked);
  MITK_TEST(DragLine_MovesIt);
  MITK_TEST(DragSelectedLine_KeepsSelection);
  MITK_TEST(ReleaseDragWithCtrlShift_EndsDrag);
  MITK_TEST(DragLineBeyondImage_StopsAtItsEdge);
  MITK_TEST(DragLinePointBeyondImage_StopsAtItsEdge);
  MITK_TEST(DragCircleInside_MovesIt);
  MITK_TEST(DragCircleCenterBeyondImage_StopsAtItsEdge);
  MITK_TEST(DragCircleRimBeyondImage_LeavesIt);
  MITK_TEST(PlaceLineBeyondImage_StopsAtItsEdge);
  MITK_TEST(PlaceCircleBeyondImage_LeavesIt);
  MITK_TEST(PlaceLineInsideCircleWhileClaimed_LeavesCircle);
  MITK_TEST(HoverNestedCircles_HoversSmallerInside);
  MITK_TEST(DragNestedCircles_MovesSmallerInside);
  MITK_TEST(HoverNestedCircles_IgnoresHiddenInner);
  MITK_TEST(LeaveNestedCircles_CtrlRevivesNoHover);
  MITK_TEST(HoverLineEndOnOtherStroke_HoversHandle);
  MITK_TEST(CtrlInOtherWindow_KeepsFeedbackUnderMouse);
  MITK_TEST(HoverHiddenFigure_DoesNothing);
  MITK_TEST(CtrlHoverPolygonEdge_PreviewsInsertion);
  MITK_TEST(CtrlClickPolygonEdge_InsertsDraggedPoint);
  MITK_TEST(CtrlClickPolygonPoint_RemovesIt);
  MITK_TEST(CtrlClickTrianglePoint_KeepsIt);
  MITK_TEST(PressAndReleaseCtrl_TogglesPreview);
  MITK_TEST(CtrlHoverLine_PreviewsNothing);
  MITK_TEST(HoverPolygon_ShowsCursorOfPress);
  MITK_TEST(DragDoubleEllipseInnerHandle_ThinsRing);
  MITK_TEST(PlacePolygonWithKeys_RemovesAndFinishes);
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
    m_Image = nullptr;
    m_InteractionTestHelper.reset();
  }

  void PlaybackOnNewFigure(mitk::PlanarFigure *figure, const std::string &interactionXmlPath)
  {
    this->SetUpNewFigure(figure, interactionXmlPath);
    m_InteractionTestHelper->PlaybackInteraction();
  }

  /** Prepares the render windows of the recording without playing it back. */
  void SetUpNewFigure(mitk::PlanarFigure *figure, const std::string &interactionXmlPath)
  {
    // Create DataNode as a container for our PlanarFigure
    m_FigureNode = mitk::DataNode::New();
    m_FigureNode->SetData(figure);

    m_InteractionTestHelper = std::make_unique<mitk::InteractionTestHelper>(GetTestDataFilePath(interactionXmlPath));

    // Load a bounding image
    m_Image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));

    mitk::DataNode::Pointer dn = mitk::DataNode::New();
    dn->SetData(m_Image);
    m_InteractionTestHelper->AddNodeToStorage(dn);
    m_InteractionTestHelper->GetDataStorage()->Add(m_FigureNode, dn);

    m_FigureNode->SetName("PLANAR FIGURE");
    // set as selected
    m_FigureNode->SetSelected(true);
    m_FigureNode->AddProperty("selected", mitk::BoolProperty::New(true));

    m_FigureInteractor = CreateInteractor(m_FigureNode);
  }

  static mitk::PlanarFigureInteractor::Pointer CreateInteractor(mitk::DataNode *node)
  {
    auto interactor = mitk::PlanarFigureInteractor::New();
    us::Module *planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");
    interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
    interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
    interactor->SetDataNode(node);
    return interactor;
  }

  /** Adds another figure with its own interactor, after the one of SetUpNewFigure(). */
  mitk::PlanarFigureInteractor::Pointer AddFigure(mitk::PlanarFigure *figure) const
  {
    auto node = mitk::DataNode::New();
    node->SetData(figure);
    m_InteractionTestHelper->GetDataStorage()->Add(node);
    return CreateInteractor(node);
  }

  void RunTest(mitk::PlanarFigure::Pointer figure, std::string interactionXmlPath, std::string referenceFigurePath)
  {
    this->PlaybackOnNewFigure(figure, interactionXmlPath);

    // Load reference PlanarFigure
    auto reference = mitk::IOUtil::Load<mitk::PlanarFigure>(GetTestDataFilePath(referenceFigurePath));

    // Some references were computed by the previous implementation, whose arithmetic differs in the last digits.
    CPPUNIT_ASSERT_MESSAGE("Compare figure with reference", mitk::Equal(*figure, *reference, 1e-6, true));
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
    RunTest(figure, "InteractionTestData/Interactions/Circle.xml", "InteractionTestData/ReferenceData/Circle.pf.json");
  }

  void DoubleEllipseInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarDoubleEllipse::New();
    RunTest(figure,
            "InteractionTestData/Interactions/DoubleEllipse.xml",
            "InteractionTestData/ReferenceData/DoubleEllipse.pf.json");
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
    RunTest(figure, "InteractionTestData/Interactions/Polygon.xml", "InteractionTestData/ReferenceData/Polygon.pf.json");
  }

  void PathInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarPath::New();
    RunTest(figure, "InteractionTestData/Interactions/Path.xml", "InteractionTestData/ReferenceData/Path.pf.json");
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
    CPPUNIT_ASSERT_MESSAGE("No handle is hovered", !this->GetFeedback().HoveredHandle.has_value());
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

  void PresetCircle_IsPlacedWhereClicked()
  {
    mitk::Point2D center;
    center.Fill(0.0);
    auto rim = center;
    rim[0] = 10.0;

    auto circle = mitk::PlanarCircle::New();
    circle->BeginPlacement();
    circle->SetFromSpan(center, rim);
    circle->SetShapeLocked(true);

    this->SetUpNewFigure(circle, "InteractionTestData/Interactions/Line.xml");
    this->InitializeViews();

    mitk::Point2D clickPosition;
    clickPosition[0] = 150;
    clickPosition[1] = 150;

    this->MoveTo(clickPosition);
    this->PressAt(clickPosition);
    this->ReleaseAt(clickPosition);

    CPPUNIT_ASSERT_MESSAGE("The preset is placed by one click", circle->IsComplete());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, *circle->GetMeasurement(mitk::PlanarFigure::MeasurementType::Radius), mitk::eps);

    const auto centerPosition = this->GetControlPointPosition(0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(clickPosition[0], centerPosition[0], 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(clickPosition[1], centerPosition[1], 0.5);

    // Its shape stays locked: dragging the rim does not change the radius.
    const auto rimPosition = this->GetControlPointPosition(1);
    this->DragFromTo(rimPosition, rimPosition + MakeOffset(30, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, *circle->GetMeasurement(mitk::PlanarFigure::MeasurementType::Radius), 1e-9);
  }

  void DragLine_MovesIt()
  {
    this->PlaybackLineAndMoveAway();

    const auto first = this->GetControlPointPosition(0);
    const auto second = this->GetControlPointPosition(1);
    const auto midpoint = this->GetLineMidpointPosition();

    this->MoveTo(midpoint);
    CPPUNIT_ASSERT_MESSAGE("Hovering over the stroke", this->IsHovering() && !this->GetFeedback().HoveredHandle.has_value());

    const auto offset = MakeOffset(20, 10);
    this->DragFromTo(midpoint, midpoint + offset);

    AssertDisplayPositionsEqual(first + offset, this->GetControlPointPosition(0));
    AssertDisplayPositionsEqual(second + offset, this->GetControlPointPosition(1));
  }

  void DragSelectedLine_KeepsSelection()
  {
    this->PlaybackLineAndMoveAway();

    const EventCounter selections(m_FigureNode->GetData(), mitk::SelectPlanarFigureEvent());

    // Selecting a selected figure again would collapse a selection of several nodes.
    m_FigureNode->SetSelected(true);
    const auto midpoint = this->GetLineMidpointPosition();
    this->DragFromTo(midpoint, midpoint + MakeOffset(20, 10));
    CPPUNIT_ASSERT_EQUAL(0u, selections.GetCount());

    m_FigureNode->SetSelected(false);
    const auto movedMidpoint = this->GetLineMidpointPosition();
    this->MoveTo(movedMidpoint);
    this->PressAt(movedMidpoint);
    this->ReleaseAt(movedMidpoint);
    CPPUNIT_ASSERT_EQUAL(1u, selections.GetCount());
  }

  void ReleaseDragWithCtrlShift_EndsDrag()
  {
    this->PlaybackLineAndMoveAway();

    const EventCounter endInteractions(m_FigureNode->GetData(), mitk::EndInteractionPlanarFigureEvent());

    const auto first = this->GetControlPointPosition(0);
    const auto midpoint = this->GetLineMidpointPosition();
    const auto offset = MakeOffset(20, 10);

    this->MoveTo(midpoint);
    this->PressAt(midpoint);
    this->DragTo(midpoint + offset);
    this->ReleaseAt(midpoint + offset, mitk::InteractionEvent::ShiftKey | mitk::InteractionEvent::ControlKey);

    AssertDisplayPositionsEqual(first + offset, this->GetControlPointPosition(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The release ends the drag", 1u, endInteractions.GetCount());

    // A drag that did not end would take no hover.
    this->MoveTo(GetPositionFarFromLine());
    this->MoveTo(this->GetLineMidpointPosition());
    CPPUNIT_ASSERT_MESSAGE("Hovering over the line after the drag", this->IsHovering());
  }

  void DragLineBeyondImage_StopsAtItsEdge()
  {
    this->PlaybackLineAndMoveAway();

    const auto *line = this->GetFigure();
    const auto length = this->GetWorldLength();
    const auto midpoint = this->GetLineMidpointPosition();

    this->DragFromTo(midpoint, GetPositionLeftOfImage(midpoint[1]));

    const auto left = std::min(line->GetWorldControlPoint(0)[0], line->GetWorldControlPoint(1)[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(this->GetLeftImageEdge(), left, 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(length, this->GetWorldLength(), 1e-6);
  }

  void DragLinePointBeyondImage_StopsAtItsEdge()
  {
    this->PlaybackLineAndMoveAway();

    const auto *line = this->GetFigure();
    const auto second = line->GetWorldControlPoint(1);
    const auto pointPosition = this->GetControlPointPosition(0);

    this->DragFromTo(pointPosition, GetPositionLeftOfImage(pointPosition[1]));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(this->GetLeftImageEdge(), line->GetWorldControlPoint(0)[0], 1e-6);
    CPPUNIT_ASSERT_MESSAGE("The other point stays", mitk::Equal(second, line->GetWorldControlPoint(1), 1e-6, true));
  }

  void DragCircleInside_MovesIt()
  {
    this->SetUpCircle(GetWindowCenter(), 60.0);
    const auto radius = this->GetWorldLength();
    const auto center = this->GetControlPointPosition(0);

    // Halfway between the center handle and the outline
    const auto inside = center + MakeOffset(30, 0);

    this->MoveTo(inside);
    CPPUNIT_ASSERT_MESSAGE("Hovering inside", this->IsHovering() && !this->GetFeedback().HoveredHandle.has_value());

    const auto offset = MakeOffset(20, 10);
    this->DragFromTo(inside, inside + offset);

    AssertDisplayPositionsEqual(center + offset, this->GetControlPointPosition(0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(radius, this->GetWorldLength(), 1e-6);
  }

  void DragCircleCenterBeyondImage_StopsAtItsEdge()
  {
    const auto circle = this->SetUpCircle(GetWindowCenter(), 60.0);
    const auto radius = this->GetWorldLength();
    const auto center = this->GetControlPointPosition(0);

    this->DragFromTo(center, GetPositionLeftOfImage(center[1]));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(this->GetLeftImageEdge(), circle->GetWorldControlPoint(0)[0], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(radius, this->GetWorldLength(), 1e-6);
  }

  void DragCircleRimBeyondImage_LeavesIt()
  {
    const auto circle = this->SetUpCircle(GetWindowCenter(), 60.0);
    const auto rim = this->GetControlPointPosition(1);

    this->DragFromTo(rim, GetPositionLeftOfImage(rim[1]));

    CPPUNIT_ASSERT_MESSAGE("The rim is left of the image", circle->GetWorldControlPoint(1)[0] < this->GetLeftImageEdge() - 1.0);
  }

  void PlaceLineBeyondImage_StopsAtItsEdge()
  {
    const auto line = mitk::PlanarLine::New();
    this->SetUpNewFigure(line, "InteractionTestData/Interactions/Line.xml");
    this->InitializeViews();

    const auto start = GetWindowCenter();
    this->DragFromTo(start, GetPositionLeftOfImage(start[1]));

    CPPUNIT_ASSERT_MESSAGE("The line is placed", line->IsComplete());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(this->GetLeftImageEdge(), line->GetWorldControlPoint(1)[0], 1e-6);
  }

  void PlaceCircleBeyondImage_LeavesIt()
  {
    const auto circle = mitk::PlanarCircle::New();
    this->SetUpNewFigure(circle, "InteractionTestData/Interactions/Line.xml");
    this->InitializeViews();

    const auto start = GetWindowCenter();
    this->DragFromTo(start, GetPositionLeftOfImage(start[1]));

    CPPUNIT_ASSERT_MESSAGE("The circle is placed", circle->IsComplete());
    CPPUNIT_ASSERT_MESSAGE("The rim is left of the image", circle->GetWorldControlPoint(1)[0] < this->GetLeftImageEdge() - 1.0);
  }

  void PlaceLineInsideCircleWhileClaimed_LeavesCircle()
  {
    const auto circle = this->SetUpCircle(GetWindowCenter(), 80.0);
    const auto center = circle->GetWorldControlPoint(0);
    const auto rim = circle->GetWorldControlPoint(1);

    const auto line = mitk::PlanarLine::New();
    this->AddFigure(line);

    // Like a view that places a new figure
    const auto claim = mitk::ExclusiveInteraction::Acquire([] { return true; });

    const auto start = GetWindowCenter() + MakeOffset(20, 0);
    this->DragFromTo(start, start + MakeOffset(40, 20));

    CPPUNIT_ASSERT_MESSAGE("The line is placed", line->IsComplete());
    CPPUNIT_ASSERT_MESSAGE("The circle stays",
      mitk::Equal(center, circle->GetWorldControlPoint(0), mitk::eps, true) &&
      mitk::Equal(rim, circle->GetWorldControlPoint(1), mitk::eps, true));
  }

  void HoverNestedCircles_HoversSmallerInside()
  {
    // The outer circle gets the events first.
    this->SetUpCircle(GetWindowCenter(), 80.0);

    const auto inner = mitk::PlanarCircle::New();
    this->PlaceCircle(inner, GetWindowCenter(), 30.0);
    const auto innerInteractor = this->AddFigure(inner);
    const auto *renderer = this->GetLineRenderer();

    this->MoveTo(GetWindowCenter() + MakeOffset(15, 0));
    CPPUNIT_ASSERT_MESSAGE("The inner circle is hovered", innerInteractor->GetFeedback(renderer).IsHovered);
    CPPUNIT_ASSERT_MESSAGE("The outer circle is not hovered", !this->IsHovering());

    this->MoveTo(GetWindowCenter() + MakeOffset(55, 0));
    CPPUNIT_ASSERT_MESSAGE("The outer circle is hovered", this->IsHovering());
    CPPUNIT_ASSERT_MESSAGE("The inner circle is no longer hovered", !innerInteractor->GetFeedback(renderer).IsHovered);
  }

  void HoverNestedCircles_IgnoresHiddenInner()
  {
    this->SetUpCircle(GetWindowCenter(), 80.0);

    const auto inner = mitk::PlanarCircle::New();
    this->PlaceCircle(inner, GetWindowCenter(), 30.0);
    this->AddFigure(inner)->GetDataNode()->SetVisibility(false);

    this->MoveTo(GetWindowCenter() + MakeOffset(15, 0));
    CPPUNIT_ASSERT_MESSAGE("A hidden inner circle does not take the hover from the outer one", this->IsHovering());
  }

  void LeaveNestedCircles_CtrlRevivesNoHover()
  {
    // The outer circle gets the events first.
    this->SetUpCircle(GetWindowCenter(), 80.0);

    const auto inner = mitk::PlanarCircle::New();
    this->PlaceCircle(inner, GetWindowCenter(), 30.0);
    const auto innerInteractor = this->AddFigure(inner);
    const auto *renderer = this->GetLineRenderer();

    // The inner circle sees none of the moves that leave it.
    this->MoveTo(GetWindowCenter() + MakeOffset(15, 0));
    this->MoveTo(GetWindowCenter() + MakeOffset(55, 0));
    this->MoveTo(GetWindowCenter() + MakeOffset(120, 0));
    CPPUNIT_ASSERT_MESSAGE("Precondition failed: the outer circle is still hovered", !this->IsHovering());

    this->PressControl();
    CPPUNIT_ASSERT_MESSAGE("The inner circle is not hovered again", !innerInteractor->GetFeedback(renderer).IsHovered);
    this->ReleaseControl();
  }

  void HoverLineEndOnOtherStroke_HoversHandle()
  {
    // The polygon gets the events first.
    this->SetUpPolygon(GetSquare());

    // A line that ends on the upper edge of the polygon.
    const auto lineEnd = MakePosition(150, 100);
    const auto *plane = this->GetLineRenderer()->GetSliceNavigationController()->GetCurrentPlaneGeometry();

    const auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(plane);
    line->SetControlPoints({ this->ToPlane(lineEnd, plane), this->ToPlane(MakePosition(150, 60), plane) });
    const auto lineInteractor = this->AddFigure(line);

    this->MoveTo(lineEnd);

    const auto lineFeedback = lineInteractor->GetFeedback(this->GetLineRenderer());
    CPPUNIT_ASSERT_MESSAGE("The end of the line is hovered", lineFeedback.IsHovered && lineFeedback.HoveredHandle == std::optional<std::size_t>(0));
    CPPUNIT_ASSERT_MESSAGE("The polygon is not hovered", !this->IsHovering());
  }

  void CtrlInOtherWindow_KeepsFeedbackUnderMouse()
  {
    this->PlaybackLineAndMoveAway();

    this->MoveTo(this->GetLineMidpointPosition());
    CPPUNIT_ASSERT_MESSAGE("Precondition failed: the line is not hovered", this->IsHovering());

    // Ctrl also reaches the window with the keyboard focus, here one without the mouse.
    auto *focusedWindow = m_InteractionTestHelper->GetRenderWindowByName("stdmulti.widget2");
    CPPUNIT_ASSERT_MESSAGE("Precondition failed: the recording has no second 2D window", nullptr != focusedWindow);
    Send(mitk::InteractionKeyEvent::New(focusedWindow->GetRenderer(), mitk::InteractionEvent::KeyControl, mitk::InteractionEvent::NoKey));

    CPPUNIT_ASSERT_MESSAGE("The line stays hovered where the mouse is", this->IsHovering());
    this->ReleaseControl();
  }

  void DragNestedCircles_MovesSmallerInside()
  {
    // The outer circle gets the events first.
    const auto outer = this->SetUpCircle(GetWindowCenter(), 80.0);
    const auto outerCenter = outer->GetWorldControlPoint(0);

    const auto inner = mitk::PlanarCircle::New();
    this->PlaceCircle(inner, GetWindowCenter(), 30.0);
    this->AddFigure(inner);
    const auto innerCenter = inner->GetWorldControlPoint(0);

    const auto start = GetWindowCenter() + MakeOffset(15, 0);
    this->DragFromTo(start, start + MakeOffset(20, 10));

    CPPUNIT_ASSERT_MESSAGE("The outer circle stays", mitk::Equal(outerCenter, outer->GetWorldControlPoint(0), mitk::eps, true));
    CPPUNIT_ASSERT_MESSAGE("The inner circle moves", !mitk::Equal(innerCenter, inner->GetWorldControlPoint(0), mitk::eps, false));
  }

  void HoverHiddenFigure_DoesNothing()
  {
    this->PlaybackLineAndMoveAway();
    m_FigureNode->SetVisibility(false);

    this->MoveTo(this->GetLineMidpointPosition());
    CPPUNIT_ASSERT_MESSAGE("A hidden figure is not hovered", !this->IsHovering());
  }

  void CtrlHoverPolygonEdge_PreviewsInsertion()
  {
    this->SetUpPolygon(GetSquare());
    const auto edgeMidpoint = MakePosition(150, 100);

    this->MoveTo(edgeMidpoint);
    CPPUNIT_ASSERT_MESSAGE("Without Ctrl, nothing is previewed", this->IsHovering() && !this->GetFeedback().InsertionPreview.has_value());

    this->MoveTo(edgeMidpoint, mitk::InteractionEvent::ControlKey);
    const auto feedback = this->GetFeedback();
    CPPUNIT_ASSERT_MESSAGE("With Ctrl, the insertion is previewed", feedback.InsertionPreview.has_value());
    CPPUNIT_ASSERT_MESSAGE("No handle is hovered", !feedback.HoveredHandle.has_value());
  }

  void CtrlClickPolygonEdge_InsertsDraggedPoint()
  {
    const auto polygon = this->SetUpPolygon(GetSquare());
    const auto edgeMidpoint = MakePosition(150, 100);
    const auto target = MakePosition(150, 80);

    this->MoveTo(edgeMidpoint, mitk::InteractionEvent::ControlKey);
    this->PressAt(edgeMidpoint, mitk::InteractionEvent::ControlKey);
    this->DragTo(target);
    this->ReleaseAt(target);

    CPPUNIT_ASSERT_EQUAL(std::size_t(5), polygon->GetNumberOfControlPoints());
    AssertDisplayPositionsEqual(target, this->GetControlPointPosition(1));
  }

  void CtrlClickPolygonPoint_RemovesIt()
  {
    const auto polygon = this->SetUpPolygon(GetSquare());
    const auto corner = GetSquare()[1];

    this->MoveTo(corner, mitk::InteractionEvent::ControlKey);
    const auto feedback = this->GetFeedback();
    CPPUNIT_ASSERT_MESSAGE("The removal is previewed", feedback.IsRemovalPreview && feedback.HoveredHandle == std::size_t(1));

    this->PressAt(corner, mitk::InteractionEvent::ControlKey);
    this->ReleaseAt(corner);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), polygon->GetNumberOfControlPoints());
  }

  void CtrlClickTrianglePoint_KeepsIt()
  {
    auto triangle = GetSquare();
    triangle.pop_back();

    const auto polygon = this->SetUpPolygon(triangle);
    const auto corner = triangle[1];

    this->MoveTo(corner, mitk::InteractionEvent::ControlKey);
    CPPUNIT_ASSERT_MESSAGE("No removal below the minimum is previewed", this->IsHovering() && !this->GetFeedback().IsRemovalPreview);

    this->PressAt(corner, mitk::InteractionEvent::ControlKey);
    this->ReleaseAt(corner);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), polygon->GetNumberOfControlPoints());
  }

  void PressAndReleaseCtrl_TogglesPreview()
  {
    this->SetUpPolygon(GetSquare());

    this->MoveTo(MakePosition(150, 100));
    CPPUNIT_ASSERT_MESSAGE("Nothing is previewed yet", this->IsHovering() && !this->GetFeedback().InsertionPreview.has_value());

    this->PressControl();
    CPPUNIT_ASSERT_MESSAGE("Pressing Ctrl previews the insertion", this->GetFeedback().InsertionPreview.has_value());

    this->ReleaseControl();
    CPPUNIT_ASSERT_MESSAGE("Releasing Ctrl ends the preview", this->IsHovering() && !this->GetFeedback().InsertionPreview.has_value());
  }

  void HoverPolygon_ShowsCursorOfPress()
  {
    this->SetUpPolygon(GetSquare());
    auto *renderWindow = this->GetLineRenderer()->GetRenderWindow();

    this->MoveTo(MakePosition(150, 100));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Over the stroke", VTK_CURSOR_SIZEALL, renderWindow->GetCurrentCursor());

    this->MoveTo(GetSquare()[1]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Over a handle", VTK_CURSOR_HAND, renderWindow->GetCurrentCursor());

    this->MoveTo(MakePosition(150, 100), mitk::InteractionEvent::ControlKey);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Over the stroke with Ctrl", VTK_CURSOR_CROSSHAIR, renderWindow->GetCurrentCursor());

    this->MoveTo(GetPositionFarFromLine());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Away from the polygon", VTK_CURSOR_DEFAULT, renderWindow->GetCurrentCursor());
  }

  void CtrlHoverLine_PreviewsNothing()
  {
    this->PlaybackLineAndMoveAway();

    this->MoveTo(this->GetLineMidpointPosition(), mitk::InteractionEvent::ControlKey);
    CPPUNIT_ASSERT_MESSAGE("Lines take no points", this->IsHovering() && !this->GetFeedback().InsertionPreview.has_value());
  }

  void DragDoubleEllipseInnerHandle_ThinsRing()
  {
    auto ring = mitk::PlanarDoubleEllipse::New();
    this->SetUpNewFigure(ring, "InteractionTestData/Interactions/Line.xml");
    this->InitializeViews();

    const auto *plane = this->GetLineRenderer()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    ring->SetPlaneGeometry(plane);
    ring->SetFromSpan(this->ToPlane(GetWindowCenter(), plane), this->ToPlane(GetWindowCenter() + MakeOffset(80, 0), plane));
    const auto thickness = ring->GetThickness();

    // The inner handle lies 40 pixels left of the center, opposite of the end of the first axis.
    mitk::Point3D innerHandle;
    plane->Map(ring->GetHandles()[3].Position, innerHandle);
    mitk::Point2D innerHandlePosition;
    this->GetLineRenderer()->WorldToDisplay(innerHandle, innerHandlePosition);

    this->DragFromTo(innerHandlePosition, innerHandlePosition - MakeOffset(20, 0));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5 * thickness, ring->GetThickness(), 0.02 * thickness);
    AssertDisplayPositionsEqual(GetWindowCenter() + MakeOffset(80, 0), this->GetControlPointPosition(1));
  }

  void PlacePolygonWithKeys_RemovesAndFinishes()
  {
    auto polygon = mitk::PlanarPolygon::New();
    this->SetUpNewFigure(polygon, "InteractionTestData/Interactions/Line.xml");
    this->InitializeViews();

    const auto square = GetSquare();

    for (const auto &corner : square)
    {
      this->MoveTo(corner);
      this->PressAt(corner);
      this->ReleaseAt(corner);
    }

    Send(mitk::InteractionKeyEvent::New(this->GetLineRenderer(), mitk::InteractionEvent::KeyBackspace, mitk::InteractionEvent::NoKey));
    Send(mitk::InteractionKeyEvent::New(this->GetLineRenderer(), mitk::InteractionEvent::KeyEnter, mitk::InteractionEvent::NoKey));

    CPPUNIT_ASSERT_MESSAGE("Enter finishes the placement", polygon->IsComplete());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Backspace removes the last point", std::size_t(3), polygon->GetNumberOfControlPoints());

    for (unsigned int i = 0; i < 3; ++i)
      AssertDisplayPositionsEqual(square[i], this->GetControlPointPosition(i));
  }

  // this is only for the OpenGL check
  mitkPlanarFigureInteractionTestSuite() : m_RenderingTestHelper(300, 300) {}
  private:
    mitk::BaseRenderer *GetLineRenderer() const
    {
      // Line.xml draws its line in this render window
      return m_InteractionTestHelper->GetRenderWindowByName("stdmulti.widget1")->GetRenderer();
    }

    mitk::PlanarFigure *GetFigure() const
    {
      return dynamic_cast<mitk::PlanarFigure *>(m_FigureNode->GetData());
    }

    void InitializeViews() const
    {
      // Without a playback, the views need to be initialized here.
      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_InteractionTestHelper->GetDataStorage());
      this->GetLineRenderer()->GetRenderWindow()->Render();
    }

    /** Places a circle in the render window of Line.xml, with center and radius in display pixels. */
    mitk::PlanarCircle::Pointer SetUpCircle(const mitk::Point2D &center, double radius)
    {
      auto circle = mitk::PlanarCircle::New();
      this->SetUpNewFigure(circle, "InteractionTestData/Interactions/Line.xml");
      this->InitializeViews();
      this->PlaceCircle(circle, center, radius);
      return circle;
    }

    /** Places a polygon in the render window of Line.xml, with points in display pixels. */
    mitk::PlanarPolygon::Pointer SetUpPolygon(const std::vector<mitk::Point2D> &displayPoints)
    {
      auto polygon = mitk::PlanarPolygon::New();
      this->SetUpNewFigure(polygon, "InteractionTestData/Interactions/Line.xml");
      this->InitializeViews();

      const auto *plane = this->GetLineRenderer()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
      polygon->SetPlaneGeometry(plane);

      std::vector<mitk::Point2D> points;

      for (const auto &displayPoint : displayPoints)
        points.push_back(this->ToPlane(displayPoint, plane));

      polygon->SetControlPoints(points);
      return polygon;
    }

    /** A square of 100 display pixels, clockwise from its upper left corner. */
    static std::vector<mitk::Point2D> GetSquare()
    {
      return { MakePosition(100, 100), MakePosition(200, 100), MakePosition(200, 200), MakePosition(100, 200) };
    }

    static mitk::Point2D MakePosition(double x, double y)
    {
      mitk::Point2D position;
      position[0] = x;
      position[1] = y;
      return position;
    }

    void PlaceCircle(mitk::PlanarCircle *circle, const mitk::Point2D &center, double radius) const
    {
      auto rim = center;
      rim[0] += radius;

      const auto *plane = this->GetLineRenderer()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
      circle->SetPlaneGeometry(plane);
      circle->SetFromSpan(this->ToPlane(center, plane), this->ToPlane(rim, plane));
    }

    mitk::Point2D ToPlane(const mitk::Point2D &displayPosition, const mitk::PlaneGeometry *plane) const
    {
      mitk::Point3D worldPosition;
      this->GetLineRenderer()->DisplayToWorld(displayPosition, worldPosition);

      mitk::Point2D position;
      plane->Map(worldPosition, position);
      return position;
    }

    mitk::Point2D GetControlPointPosition(unsigned int index) const
    {
      mitk::Point2D displayPosition;
      this->GetLineRenderer()->WorldToDisplay(this->GetFigure()->GetWorldControlPoint(index), displayPosition);
      return displayPosition;
    }

    /** The distance between the first two control points, i.e. the length of a line or the radius of a circle. */
    double GetWorldLength() const
    {
      const auto *figure = this->GetFigure();
      return figure->GetWorldControlPoint(0).EuclideanDistanceTo(figure->GetWorldControlPoint(1));
    }

    double GetLeftImageEdge() const
    {
      // The corners of the bounding box of an image are at the outer edges of its pixels.
      return m_Image->GetGeometry()->GetCornerPoint(0)[0];
    }

    static mitk::Point2D GetWindowCenter()
    {
      mitk::Point2D position;
      position[0] = 150;
      position[1] = 150;
      return position;
    }

    static mitk::Point2D GetPositionLeftOfImage(double y)
    {
      mitk::Point2D position;
      position[0] = -1000;
      position[1] = y;
      return position;
    }

    static mitk::Vector2D MakeOffset(double x, double y)
    {
      mitk::Vector2D offset;
      offset[0] = x;
      offset[1] = y;
      return offset;
    }

    static void AssertDisplayPositionsEqual(const mitk::Point2D &expected, const mitk::Point2D &actual)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[0], actual[0], 0.5);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[1], actual[1], 0.5);
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

    void MoveTo(const mitk::Point2D &position, mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey) const
    {
      Send(mitk::MouseMoveEvent::New(this->GetLineRenderer(), position, mitk::InteractionEvent::NoButton, modifiers));
    }

    void PressControl() const
    {
      Send(mitk::InteractionKeyEvent::New(this->GetLineRenderer(), mitk::InteractionEvent::KeyControl, mitk::InteractionEvent::NoKey));
    }

    void ReleaseControl() const
    {
      Send(mitk::InteractionKeyReleaseEvent::New(this->GetLineRenderer(), mitk::InteractionEvent::KeyControl, mitk::InteractionEvent::NoKey));
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

    void DragFromTo(const mitk::Point2D &from, const mitk::Point2D &to) const
    {
      this->MoveTo(from);
      this->PressAt(from);
      this->DragTo(to);
      this->ReleaseAt(to);
    }

    void PressAt(const mitk::Point2D &position, mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey) const
    {
      Send(mitk::MousePressEvent::New(this->GetLineRenderer(),
                                      position,
                                      mitk::InteractionEvent::LeftMouseButton,
                                      modifiers,
                                      mitk::InteractionEvent::LeftMouseButton));
    }

    void ReleaseAt(const mitk::Point2D &position, mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey) const
    {
      Send(mitk::MouseReleaseEvent::New(this->GetLineRenderer(),
                                        position,
                                        mitk::InteractionEvent::NoButton,
                                        modifiers,
                                        mitk::InteractionEvent::LeftMouseButton));
    }

    /** Counts the events of a kind that the figure sends while the counter lives. */
    class EventCounter
    {
    public:
      EventCounter(itk::Object *figure, const itk::EventObject &event)
        : m_Figure(figure),
          m_Tag(figure->AddObserver(event, [this](const itk::EventObject &) { ++m_Count; }))
      {
      }

      EventCounter(const EventCounter &) = delete;
      EventCounter &operator=(const EventCounter &) = delete;

      ~EventCounter() { m_Figure->RemoveObserver(m_Tag); }

      unsigned int GetCount() const { return m_Count; }

    private:
      itk::Object *m_Figure;
      unsigned int m_Count = 0;
      unsigned long m_Tag;
    };

    /** Returns whether an interactor consumed the event. */
    bool LeaveWindow() const
    {
      return Send(mitk::InternalEvent::New(this->GetLineRenderer(), nullptr, "LeaveRenderWindow"));
    }

    mitk::PlanarFigureInteractor::Feedback GetFeedback() const
    {
      return m_FigureInteractor->GetFeedback(this->GetLineRenderer());
    }

    bool IsHovering() const
    {
      return this->GetFeedback().IsHovered;
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
    mitk::Image::Pointer m_Image;
    mitk::DataNode::Pointer m_FigureNode;
    mitk::PlanarFigureInteractor::Pointer m_FigureInteractor;
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureInteraction)
