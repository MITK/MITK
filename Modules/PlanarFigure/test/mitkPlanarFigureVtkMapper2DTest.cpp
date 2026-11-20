/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCameraController.h>
#include <mitkIOUtil.h>
#include <mitkMouseMoveEvent.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkVtkMapper.h>
#include <mitkProperties.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <usModuleRegistry.h>

#include <vtkActor2D.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPropAssembly.h>
#include <vtkPropCollection.h>
#include <vtkRenderLargeImage.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkWindowToImageFilter.h>

#include <algorithm>
#include <array>
#include <utility>

namespace
{
  const std::vector<std::string> TypeIds = { "line", "arrow", "angle", "four_point_angle", "cross", "circle", "ellipse",
                                             "double_ellipse", "rectangle", "polygon", "path", "subdivision_polygon",
                                             "bezier_curve" };

  /** The bounding box {min x, max x, min y, max y} of the magenta pixels of an image; all -1 without any. */
  std::array<int, 4> GetBoxOfMagenta(vtkImageData *image)
  {
    int dimensions[3];
    image->GetDimensions(dimensions);

    const auto components = image->GetNumberOfScalarComponents();
    const auto *pixels = static_cast<const unsigned char *>(image->GetScalarPointer());
    std::array<int, 4> box = { -1, -1, -1, -1 };

    for (int y = 0; y < dimensions[1]; ++y)
    {
      for (int x = 0; x < dimensions[0]; ++x)
      {
        const auto *pixel = pixels + (static_cast<std::size_t>(y) * dimensions[0] + x) * components;

        if (pixel[0] < 200 || pixel[1] > 60 || pixel[2] < 200)
          continue;

        box = box[0] < 0
          ? std::array<int, 4>{ x, x, y, y }
          : std::array<int, 4>{ std::min(box[0], x), std::max(box[1], x), std::min(box[2], y), std::max(box[3], y) };
      }
    }

    return box;
  }

  /** The white pixels of an image left of its magenta box and right of it. */
  std::pair<int, int> CountLabelPixelsBesideFigure(vtkImageData *image)
  {
    const auto box = GetBoxOfMagenta(image);

    int dimensions[3];
    image->GetDimensions(dimensions);

    const auto components = image->GetNumberOfScalarComponents();
    const auto *pixels = static_cast<const unsigned char *>(image->GetScalarPointer());
    std::pair<int, int> counts = { 0, 0 };

    for (int y = 0; y < dimensions[1]; ++y)
    {
      for (int x = 0; x < dimensions[0]; ++x)
      {
        const auto *pixel = pixels + (static_cast<std::size_t>(y) * dimensions[0] + x) * components;

        if (pixel[0] < 200 || pixel[1] < 200 || pixel[2] < 200)
          continue;

        if (x < box[0])
          ++counts.first;
        else if (x > box[1])
          ++counts.second;
      }
    }

    return counts;
  }
}

class mitkPlanarFigureVtkMapper2DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureVtkMapper2DTestSuite);
  MITK_TEST(RenderEveryType);
  MITK_TEST(RenderLineWidths);
  MITK_TEST(RenderDarkTranslucentFigures);
  MITK_TEST(RenderWithoutContrastOutline);
  MITK_TEST(RenderLabels);
  MITK_TEST(Label_FollowsItsProperty);
  MITK_TEST(RenderHighDpi);
  MITK_TEST(RenderStates);
  MITK_TEST(RenderInsertionPreview);
  MITK_TEST(RenderRemovalPreview);
  MITK_TEST(HelperLines_FollowTheirProperty);
  MITK_TEST(Handles_OfSelectedBezierCurves_AreShown);
  MITK_TEST(MovedPlane_IsDrawnAgain);
  MITK_TEST(UnchangedFigure_IsNotDrawnAgain);
  MITK_TEST(MagnifiedImage_ShowsTheFigureInItsPlace);
  MITK_TEST(MagnifiedImage_ShowsTheLabelOnceOnItsSide);
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

    if ("line" == typeId)
      figure->SetFromSpan(P(cell, 0.15, 0.2), P(cell, 0.85, 0.8));
    else if ("arrow" == typeId)
      figure->SetFromSpan(P(cell, 0.85, 0.8), P(cell, 0.15, 0.2));
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

  /** Adds a figure without a label, which tests of states and previews do not need. */
  mitk::DataNode::Pointer AddFigure(const std::string &typeId, std::size_t cell)
  {
    auto node = mitk::DataNode::New();
    node->SetData(this->CreateFigure(typeId, cell));
    node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(mitk::PlanarFigureVisibilityProperty::Visibility::Never));
    m_RenderingTestHelper.GetDataStorage()->Add(node);
    return node;
  }

  /** Whether a figure is drawn at all. */
  bool RenderAndIsShown(mitk::DataNode *node)
  {
    m_RenderingTestHelper.Render();

    auto *mapper = dynamic_cast<mitk::VtkMapper *>(node->GetMapper(mitk::BaseRenderer::Standard2D));
    return mapper->GetVtkProp(this->GetRenderer())->GetVisibility();
  }

  /** Whether the label of a figure is drawn. */
  bool RenderAndIsLabelShown(mitk::DataNode *node)
  {
    m_RenderingTestHelper.Render();

    auto *mapper = dynamic_cast<mitk::VtkMapper *>(node->GetMapper(mitk::BaseRenderer::Standard2D));
    auto *assembly = vtkPropAssembly::SafeDownCast(mapper->GetVtkProp(this->GetRenderer()));
    auto *textActor = vtkTextActor::SafeDownCast(assembly->GetParts()->GetItemAsObject(1));
    return nullptr != textActor && textActor->GetVisibility();
  }

  static mitk::PlanarFigureInteractor::Pointer AddInteractor(mitk::DataNode *node)
  {
    auto interactor = mitk::PlanarFigureInteractor::New();
    auto *planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");
    interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
    interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
    interactor->SetDataNode(node);
    return interactor;
  }

  mitk::BaseRenderer *GetRenderer()
  {
    return mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());
  }

  /** Moves the mouse over a point of the plane, through the dispatcher like a real mouse. */
  void MoveTo(const mitk::Point2D &point, mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey)
  {
    auto *renderer = this->GetRenderer();

    mitk::Point3D worldPoint;
    m_Plane->Map(point, worldPoint);

    mitk::Point2D displayPoint;
    renderer->WorldToDisplay(worldPoint, displayPoint);

    auto event = mitk::MouseMoveEvent::New(renderer, displayPoint, mitk::InteractionEvent::NoButton, modifiers);
    renderer->GetDispatcher()->ProcessEvent(event);
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

  void RenderWithoutContrastOutline()
  {
    for (auto &node : this->AddEveryType())
      node->SetBoolProperty("planarfigure.drawcontrastoutline", false);

    this->CompareWithReference("NoContrastOutline");
  }

  void RenderLabels()
  {
    // The label of the polygon at the right border flips to its left side.
    for (const auto &[typeId, cell] : std::vector<std::pair<std::string, std::size_t>>{ { "line", 0 }, { "circle", 9 }, { "polygon", 7 }, { "angle", 14 } })
    {
      auto node = mitk::DataNode::New();
      node->SetName(typeId);
      node->SetData(this->CreateFigure(typeId, cell));
      node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(mitk::PlanarFigureVisibilityProperty::Visibility::Always));
      m_RenderingTestHelper.GetDataStorage()->Add(node);
    }

    this->CompareWithReference("Labels");
  }

  void Label_FollowsItsProperty()
  {
    using Visibility = mitk::PlanarFigureVisibilityProperty::Visibility;

    auto node = mitk::DataNode::New();
    node->SetName("circle");
    node->SetData(this->CreateFigure("circle", 5));
    m_RenderingTestHelper.GetDataStorage()->Add(node);

    bool drawName = false;
    bool drawQuantities = false;
    CPPUNIT_ASSERT_MESSAGE("The name is in the label by default", node->GetBoolProperty("planarfigure.drawname", drawName) && drawName);
    CPPUNIT_ASSERT_MESSAGE("The measurements are in the label by default", node->GetBoolProperty("planarfigure.drawquantities", drawQuantities) && drawQuantities);

    const auto *byDefault = dynamic_cast<mitk::PlanarFigureVisibilityProperty *>(node->GetProperty("planarfigure.label"));
    CPPUNIT_ASSERT_MESSAGE("The label is shown on hover by default", nullptr != byDefault && Visibility::OnHover == byDefault->GetVisibility());

    CPPUNIT_ASSERT_MESSAGE("The label is hidden at rest", !this->RenderAndIsLabelShown(node));

    AddInteractor(node);
    this->MoveTo(P(5, 0.85, 0.5));
    CPPUNIT_ASSERT_MESSAGE("The label is shown on hover", this->RenderAndIsLabelShown(node));

    node->SetBoolProperty("helper object", true);
    CPPUNIT_ASSERT_MESSAGE("A helper object has no label", !this->RenderAndIsLabelShown(node));

    node->SetBoolProperty("helper object", false);
    node->SetBoolProperty("hidden object", true);
    CPPUNIT_ASSERT_MESSAGE("A hidden object has no label", !this->RenderAndIsLabelShown(node));

    node->SetBoolProperty("hidden object", false);
    CPPUNIT_ASSERT_MESSAGE("The label is shown on hover again", this->RenderAndIsLabelShown(node));

    node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(Visibility::Never));
    CPPUNIT_ASSERT_MESSAGE("The label is hidden if never shown", !this->RenderAndIsLabelShown(node));
  }

  void RenderHighDpi()
  {
    m_RenderingTestHelper.GetVtkRenderWindow()->SetDPI(144);
    this->AddEveryType();
    this->CompareWithReference("HighDpi");
  }

  void RenderStates()
  {
    this->AddFigure("polygon", 4);
    this->AddFigure("polygon", 5)->SetSelected(true);

    const auto hoveredInteractor = AddInteractor(this->AddFigure("circle", 6));
    this->MoveTo(P(6, 0.65, 0.5));
    CPPUNIT_ASSERT_MESSAGE("The circle is hovered", hoveredInteractor->GetFeedback(this->GetRenderer()).IsHovered);

    this->CompareWithReference("StatesSoftHalo");
  }

  void RenderInsertionPreview()
  {
    const auto node = this->AddFigure("polygon", 5);
    node->SetSelected(true);

    // Midway along the first edge
    const auto interactor = AddInteractor(node);
    this->MoveTo(P(5, 0.5, 0.25), mitk::InteractionEvent::ControlKey);
    CPPUNIT_ASSERT_MESSAGE("An insertion is previewed", interactor->GetFeedback(this->GetRenderer()).InsertionPreview.has_value());

    this->CompareWithReference("InsertionPreviewSoftHalo");
  }

  void RenderRemovalPreview()
  {
    const auto interactor = AddInteractor(this->AddFigure("polygon", 5));
    this->MoveTo(P(5, 0.85, 0.3), mitk::InteractionEvent::ControlKey);
    CPPUNIT_ASSERT_MESSAGE("A removal is previewed", interactor->GetFeedback(this->GetRenderer()).IsRemovalPreview);

    this->CompareWithReference("RemovalPreviewSoftHalo");
  }

  void HelperLines_FollowTheirProperty()
  {
    using Visibility = mitk::PlanarFigureVisibilityProperty::Visibility;

    const auto curve = this->AddFigure("bezier_curve", 5);
    const auto angle = this->AddFigure("angle", 6);

    const auto *byDefault = dynamic_cast<mitk::PlanarFigureVisibilityProperty *>(curve->GetProperty("planarfigure.helperlines"));
    CPPUNIT_ASSERT_MESSAGE("The control polygon follows the handles by default", nullptr != byDefault && Visibility::OnHoverOrWhenSelected == byDefault->GetVisibility());

    const auto curveAtRest = this->RenderAndCountPoints(curve);

    curve->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(Visibility::Always));
    CPPUNIT_ASSERT_MESSAGE("The control polygon is shown always", this->RenderAndCountPoints(curve) > curveAtRest);

    // Selection adds a halo as well, so the selected curve is compared with and without its control polygon.
    curve->SetSelected(true);
    curve->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(Visibility::WhenSelected));
    const auto selectedCurve = this->RenderAndCountPoints(curve);

    curve->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(Visibility::Never));
    CPPUNIT_ASSERT_MESSAGE("The control polygon is shown when selected", this->RenderAndCountPoints(curve) < selectedCurve);

    // Handles are drawn on hover as well, so the hovered curve is compared with and without its control polygon.
    curve->SetSelected(false);
    curve->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(Visibility::OnHover));
    AddInteractor(curve);
    this->MoveTo(P(5, 0.1, 0.8));
    const auto hoveredCurve = this->RenderAndCountPoints(curve);

    curve->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(Visibility::Never));
    CPPUNIT_ASSERT_MESSAGE("The control polygon is shown on hover", this->RenderAndCountPoints(curve) < hoveredCurve);

    const auto angleWithArc = this->RenderAndCountPoints(angle);
    angle->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(Visibility::Never));
    CPPUNIT_ASSERT_MESSAGE("The arc of an angle is shown by default", this->RenderAndCountPoints(angle) < angleWithArc);
  }

  void Handles_OfSelectedBezierCurves_AreShown()
  {
    const auto curve = this->AddFigure("bezier_curve", 5);
    const auto polygon = this->AddFigure("polygon", 6);

    // A locked shape hides the handles of its points, so it tells whether they are drawn.
    const auto countWithAndWithoutHandles = [this](mitk::DataNode *node) {
      node->SetProperty("planarfigure.helperlines", mitk::PlanarFigureVisibilityProperty::New(mitk::PlanarFigureVisibilityProperty::Visibility::Never));
      node->SetSelected(true);
      const auto withHandles = this->RenderAndCountPoints(node);
      static_cast<mitk::PlanarFigure *>(node->GetData())->SetShapeLocked(true);
      return std::make_pair(withHandles, this->RenderAndCountPoints(node));
    };

    const auto [curveWithHandles, curveWithoutHandles] = countWithAndWithoutHandles(curve);
    CPPUNIT_ASSERT_MESSAGE("A selected Bezier curve shows its handles", curveWithoutHandles < curveWithHandles);

    const auto [polygonWithHandles, polygonWithoutHandles] = countWithAndWithoutHandles(polygon);
    CPPUNIT_ASSERT_MESSAGE("A selected polygon shows no handles", polygonWithoutHandles == polygonWithHandles);
  }

  void MovedPlane_IsDrawnAgain()
  {
    const auto node = this->AddFigure("circle", 5);
    CPPUNIT_ASSERT_MESSAGE("The figure is shown on its slice", this->RenderAndIsShown(node));

    // Moved in place by a whole slice, which leaves the modification time of the figure as it is.
    auto *plane = node->GetData()->GetGeometry();
    plane->SetOrigin(plane->GetOrigin() + plane->GetAxisVector(2));
    CPPUNIT_ASSERT_MESSAGE("The figure is hidden off its slice", !this->RenderAndIsShown(node));
  }

  void UnchangedFigure_IsNotDrawnAgain()
  {
    const auto node = this->AddFigure("circle", 5);
    const auto drawTime = this->RenderAndGetDrawing(node)->GetMTime();
    CPPUNIT_ASSERT_MESSAGE("An unchanged figure is not drawn again", drawTime == this->RenderAndGetDrawing(node)->GetMTime());

    this->GetRenderer()->GetCameraController()->Zoom(2.0, P(5, 0.5, 0.5));
    CPPUNIT_ASSERT_MESSAGE("A zoomed figure is drawn again", drawTime != this->RenderAndGetDrawing(node)->GetMTime());
  }

  void MagnifiedImage_ShowsTheFigureInItsPlace()
  {
    const auto node = this->AddFigure("rectangle", 5);
    node->SetColor(1.0f, 0.0f, 1.0f);
    node->SetBoolProperty("planarfigure.drawcontrastoutline", false);
    m_RenderingTestHelper.Render();

    auto window = vtkSmartPointer<vtkWindowToImageFilter>::New();
    window->SetInput(m_RenderingTestHelper.GetVtkRenderWindow());
    window->ReadFrontBufferOff();
    window->Update();
    const auto box = GetBoxOfMagenta(window->GetOutput());

    // Tiles the camera, as the magnified screenshots of the Screenshot Maker and the REST API do.
    auto magnifier = vtkSmartPointer<vtkRenderLargeImage>::New();
    magnifier->SetInput(m_RenderingTestHelper.GetVtkRenderer());
    magnifier->SetMagnification(2);
    magnifier->Update();
    const auto magnifiedBox = GetBoxOfMagenta(magnifier->GetOutput());

    CPPUNIT_ASSERT_MESSAGE("The figure is drawn", box[0] >= 0);

    // A tile that reused the drawing of another one would put the figure in a place of its own, far off. Within a
    // pixel or two, as drivers rasterize lines differently.
    for (std::size_t i = 0; i < 4; ++i)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Side " + std::to_string(i) + " of the box of the figure",
        2.0 * box[i], static_cast<double>(magnifiedBox[i]), 2.0);
    }
  }

  void MagnifiedImage_ShowsTheLabelOnceOnItsSide()
  {
    // Black around the figure, so that the white of the text is the label's alone.
    m_RenderingTestHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("Image"))->SetVisibility(false);

    // So close to the right edge that the label goes left of the figure, while the last of the tiles
    // of a fourfold image has room for it on the right.
    auto figure = mitk::CreatePlanarFigure("rectangle");
    figure->SetPlaneGeometry(m_Plane);
    mitk::Point2D first, second;
    first[0] = 0.70 * m_Plane->GetExtentInMM(0);
    first[1] = 0.40 * m_Plane->GetExtentInMM(1);
    second[0] = 0.93 * m_Plane->GetExtentInMM(0);
    second[1] = 0.60 * m_Plane->GetExtentInMM(1);
    figure->SetFromSpan(first, second);

    auto node = mitk::DataNode::New();
    node->SetName("Liver");
    node->SetData(figure);
    node->SetColor(1.0f, 0.0f, 1.0f);
    node->SetBoolProperty("planarfigure.drawcontrastoutline", false);
    node->SetBoolProperty("planarfigure.drawquantities", false);
    node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(mitk::PlanarFigureVisibilityProperty::Visibility::Always));
    m_RenderingTestHelper.GetDataStorage()->Add(node);
    m_RenderingTestHelper.Render();

    auto window = vtkSmartPointer<vtkWindowToImageFilter>::New();
    window->SetInput(m_RenderingTestHelper.GetVtkRenderWindow());
    window->ReadFrontBufferOff();
    window->Update();

    const auto [leftOfFigure, rightOfFigure] = CountLabelPixelsBesideFigure(window->GetOutput());
    CPPUNIT_ASSERT_MESSAGE("In the window, the label is left of the figure", leftOfFigure > 0 && 0 == rightOfFigure);

    auto magnifier = vtkSmartPointer<vtkRenderLargeImage>::New();
    magnifier->SetInput(m_RenderingTestHelper.GetVtkRenderer());
    magnifier->SetMagnification(4);
    magnifier->Update();

    const auto [magnifiedLeft, magnifiedRight] = CountLabelPixelsBesideFigure(magnifier->GetOutput());
    CPPUNIT_ASSERT_MESSAGE("In the magnified image, the label is left of the figure", magnifiedLeft > 0);
    CPPUNIT_ASSERT_MESSAGE("In the magnified image, no tile puts a label right of the figure", 0 == magnifiedRight);
  }

  /** The geometry drawn for a figure. */
  vtkPolyData *RenderAndGetDrawing(mitk::DataNode *node)
  {
    m_RenderingTestHelper.Render();

    auto *mapper = dynamic_cast<mitk::VtkMapper *>(node->GetMapper(mitk::BaseRenderer::Standard2D));
    auto *assembly = vtkPropAssembly::SafeDownCast(mapper->GetVtkProp(this->GetRenderer()));
    auto *actor = vtkActor2D::SafeDownCast(assembly->GetParts()->GetItemAsObject(0));
    return vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper())->GetInput();
  }

  /** The number of points of the geometry drawn for a figure. */
  vtkIdType RenderAndCountPoints(mitk::DataNode *node)
  {
    return this->RenderAndGetDrawing(node)->GetNumberOfPoints();
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureVtkMapper2D)
