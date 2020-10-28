/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#define SMW_INFO MITK_INFO("widget.stdmulti")

#include "QmitkStdMultiWidget.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkCameraController.h>
#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>
#include <mitkNodePredicateBase.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkStatusBar.h>
#include <mitkDisplayActionEventHandlerStd.h>
#include <mitkVtkLayerController.h>

// qt
#include <QList>
#include <QMouseEvent>
#include <QTimer>

// vtk
#include <vtkSmartPointer.h>

// c++
#include <iomanip>

QmitkStdMultiWidget::QmitkStdMultiWidget(QWidget *parent,
                                         Qt::WindowFlags f/* = 0*/,
                                         const QString &name/* = "stdmulti"*/)
  : QmitkAbstractMultiWidget(parent, f, name)
  , m_TimeNavigationController(nullptr)
  , m_PendingCrosshairPositionEvent(false)
{
  m_TimeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
}

QmitkStdMultiWidget::~QmitkStdMultiWidget()
{
  m_TimeNavigationController->Disconnect(GetRenderWindow1()->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(GetRenderWindow2()->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(GetRenderWindow3()->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(GetRenderWindow4()->GetSliceNavigationController());
}

void QmitkStdMultiWidget::InitializeMultiWidget()
{
  // yellow is default color for widget4
  m_DecorationColorWidget4[0] = 1.0f;
  m_DecorationColorWidget4[1] = 1.0f;
  m_DecorationColorWidget4[2] = 0.0f;

  SetLayout(2, 2);

  // transfer colors in WorldGeometry-Nodes of the associated Renderer
  mitk::IntProperty::Pointer layer;
  // of widget 1
  m_PlaneNode1 =
    mitk::BaseRenderer::GetInstance(GetRenderWindow1()->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode1->SetColor(GetDecorationColor(0));
  layer = mitk::IntProperty::New(1000);
  m_PlaneNode1->SetProperty("layer", layer);

  // of widget 2
  m_PlaneNode2 =
    mitk::BaseRenderer::GetInstance(GetRenderWindow2()->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode2->SetColor(GetDecorationColor(1));
  layer = mitk::IntProperty::New(1000);
  m_PlaneNode2->SetProperty("layer", layer);

  // of widget 3
  m_PlaneNode3 =
    mitk::BaseRenderer::GetInstance(GetRenderWindow3()->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetColor(GetDecorationColor(2));
  layer = mitk::IntProperty::New(1000);
  m_PlaneNode3->SetProperty("layer", layer);

  // the parent node
  m_ParentNodeForGeometryPlanes =
    mitk::BaseRenderer::GetInstance(GetRenderWindow4()->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  layer = mitk::IntProperty::New(1000);
  m_ParentNodeForGeometryPlanes->SetProperty("layer", layer);

  AddDisplayPlaneSubTree();

  SetDisplayActionEventHandler(std::make_unique<mitk::DisplayActionEventHandlerStd>());

  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions();
  }
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow(const QString& widgetName) const
{
  if ("axial" == widgetName)
  {
    return GetRenderWindow1();
  }

  if ("sagittal" == widgetName)
  {
    return GetRenderWindow2();
  }

  if ("coronal" == widgetName)
  {
    return GetRenderWindow3();
  }

  if ("3d" == widgetName)
  {
    return GetRenderWindow4();
  }


  return QmitkAbstractMultiWidget::GetRenderWindow(widgetName);
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow(const mitk::BaseRenderer::ViewDirection& viewDirection) const
{
  return GetRenderWindow(static_cast<unsigned int>(viewDirection));
}

void QmitkStdMultiWidget::SetSelectedPosition(const mitk::Point3D& newPosition, const QString& /*widgetName*/)
{
  GetRenderWindow1()->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
  GetRenderWindow2()->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
  GetRenderWindow3()->GetSliceNavigationController()->SelectSliceByPoint(newPosition);

  RequestUpdateAll();
}

const mitk::Point3D QmitkStdMultiWidget::GetSelectedPosition(const QString& /*widgetName*/) const
{
  const mitk::PlaneGeometry* plane1 = GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry* plane2 = GetRenderWindow2()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry* plane3 = GetRenderWindow3()->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ((plane1 != nullptr) && (plane2 != nullptr)
   && (plane1->IntersectionLine(plane2, line)))
  {
    mitk::Point3D point;
    if ((plane3 != nullptr) && (plane3->IntersectionPoint(line, point)))
    {
      return point;
    }
  }

  return mitk::Point3D();
}

void QmitkStdMultiWidget::SetCrosshairVisibility(bool visible)
{
  if (m_PlaneNode1.IsNotNull())
  {
    m_PlaneNode1->SetVisibility(visible);
  }
  if (m_PlaneNode2.IsNotNull())
  {
    m_PlaneNode2->SetVisibility(visible);
  }
  if (m_PlaneNode3.IsNotNull())
  {
    m_PlaneNode3->SetVisibility(visible);
  }

  RequestUpdateAll();
}

bool QmitkStdMultiWidget::GetCrosshairVisibility() const
{
  bool crosshairVisibility = true;

  if (m_PlaneNode1.IsNotNull())
  {
    bool visibilityProperty = false;
    m_PlaneNode1->GetVisibility(visibilityProperty, nullptr);
    crosshairVisibility &= visibilityProperty;
  }

  if (m_PlaneNode2.IsNotNull())
  {
    bool visibilityProperty = false;
    crosshairVisibility &= m_PlaneNode2->GetVisibility(visibilityProperty, nullptr);
    crosshairVisibility &= visibilityProperty;
  }

  if (m_PlaneNode3.IsNotNull())
  {
    bool visibilityProperty = false;
    crosshairVisibility &= m_PlaneNode3->GetVisibility(visibilityProperty, nullptr);
    crosshairVisibility &= visibilityProperty;
  }

  return crosshairVisibility;
}

void QmitkStdMultiWidget::ResetCrosshair()
{
  auto dataStorage = GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  auto geo = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeView(GetRenderWindow1()->GetRenderWindow(), geo);
  mitk::RenderingManager::GetInstance()->InitializeView(GetRenderWindow2()->GetRenderWindow(), geo);
  mitk::RenderingManager::GetInstance()->InitializeView(GetRenderWindow3()->GetRenderWindow(), geo);
  mitk::RenderingManager::GetInstance()->InitializeView(GetRenderWindow4()->GetRenderWindow(), geo);

  SetWidgetPlaneMode(mitk::InteractionSchemeSwitcher::MITKStandard);
}

void QmitkStdMultiWidget::SetWidgetPlaneMode(int userMode)
{
  MITK_DEBUG << "Changing crosshair mode to " << userMode;

  switch (userMode)
  {
  case 0:
    SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKStandard);
    break;
  case 1:
    SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKRotationUncoupled);
    break;
  case 2:
    SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKRotationCoupled);
    break;
  case 3:
    SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKSwivel);
    break;
  }
}

mitk::SliceNavigationController* QmitkStdMultiWidget::GetTimeNavigationController()
{
  return m_TimeNavigationController;
}

void QmitkStdMultiWidget::AddPlanesToDataStorage()
{
  auto dataStorage = GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull()
   && m_PlaneNode3.IsNotNull() && m_ParentNodeForGeometryPlanes.IsNotNull())
  {
    dataStorage->Add(m_ParentNodeForGeometryPlanes);
    dataStorage->Add(m_PlaneNode1, m_ParentNodeForGeometryPlanes);
    dataStorage->Add(m_PlaneNode2, m_ParentNodeForGeometryPlanes);
    dataStorage->Add(m_PlaneNode3, m_ParentNodeForGeometryPlanes);
  }
}

void QmitkStdMultiWidget::RemovePlanesFromDataStorage()
{
  auto dataStorage = GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull()
   && m_PlaneNode3.IsNotNull() && m_ParentNodeForGeometryPlanes.IsNotNull())
  {
    dataStorage->Remove(m_PlaneNode1);
    dataStorage->Remove(m_PlaneNode2);
    dataStorage->Remove(m_PlaneNode3);
    dataStorage->Remove(m_ParentNodeForGeometryPlanes);
  }
}

void QmitkStdMultiWidget::HandleCrosshairPositionEvent()
{
  if (!m_PendingCrosshairPositionEvent)
  {
    m_PendingCrosshairPositionEvent = true;
    QTimer::singleShot(0, this, SLOT(HandleCrosshairPositionEventDelayed()));
  }
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow(unsigned int number) const
{
  switch (number)
  {
  case 0:
    return GetRenderWindow1();
  case 1:
    return GetRenderWindow2();
  case 2:
    return GetRenderWindow3();
  case 3:
    return GetRenderWindow4();
  default:
    MITK_ERROR << "Requested unknown render window";
    break;
  }

  return nullptr;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow1() const
{
  return QmitkAbstractMultiWidget::GetRenderWindow(GetNameFromIndex(0, 0));
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow2() const
{
  return QmitkAbstractMultiWidget::GetRenderWindow(GetNameFromIndex(0, 1));
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow3() const
{
  return QmitkAbstractMultiWidget::GetRenderWindow(GetNameFromIndex(1, 0));
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow4() const
{
  return QmitkAbstractMultiWidget::GetRenderWindow(GetNameFromIndex(1, 1));
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane1() const
{
  return m_PlaneNode1;
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane2() const
{
  return m_PlaneNode2;
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane3() const
{
  return m_PlaneNode3;
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane(unsigned number) const
{
  switch (number)
  {
  case 1:
    return m_PlaneNode1;
  case 2:
    return m_PlaneNode2;
  case 3:
    return m_PlaneNode3;
  default:
    MITK_ERROR << "Requested unknown render window";
    break;
  }

  return nullptr;
}

void QmitkStdMultiWidget::SetDecorationColor(unsigned int widgetNumber, mitk::Color color)
{
  switch (widgetNumber)
  {
  case 0:
    if (m_PlaneNode1.IsNotNull())
    {
      m_PlaneNode1->SetColor(color);
    }
    break;
  case 1:
    if (m_PlaneNode2.IsNotNull())
    {
      m_PlaneNode2->SetColor(color);
    }
    break;
  case 2:
    if (m_PlaneNode3.IsNotNull())
    {
      m_PlaneNode3->SetColor(color);
    }
    break;
  case 3:
    m_DecorationColorWidget4 = color;
    break;
  default:
    MITK_ERROR << "Decoration color for unknown widget!";
    break;
  }
}

mitk::Color QmitkStdMultiWidget::GetDecorationColor(unsigned int widgetNumber)
{
  // The implementation looks a bit messy here, but it avoids
  // synchronization of the color of the geometry nodes and an
  // internal member here.
  // Default colors were chosen for decent visibility.
  // Feel free to change your preferences in the workbench.
  float tmp[3] = { 0.0f, 0.0f, 0.0f };
  switch (widgetNumber)
  {
  case 0:
  {
    if (m_PlaneNode1.IsNotNull())
    {
      if (m_PlaneNode1->GetColor(tmp))
      {
        return dynamic_cast<mitk::ColorProperty *>(m_PlaneNode1->GetProperty("color"))->GetColor();
      }
    }
    float red[3] = { 0.753f, 0.0f, 0.0f }; // This is #C00000 in hex
    return mitk::Color(red);
  }
  case 1:
  {
    if (m_PlaneNode2.IsNotNull())
    {
      if (m_PlaneNode2->GetColor(tmp))
      {
        return dynamic_cast<mitk::ColorProperty *>(m_PlaneNode2->GetProperty("color"))->GetColor();
      }
    }
    float green[3] = { 0.0f, 0.69f, 0.0f }; // This is #00B000 in hex
    return mitk::Color(green);
  }
  case 2:
  {
    if (m_PlaneNode3.IsNotNull())
    {
      if (m_PlaneNode3->GetColor(tmp))
      {
        return dynamic_cast<mitk::ColorProperty *>(m_PlaneNode3->GetProperty("color"))->GetColor();
      }
    }
    float blue[3] = { 0.0, 0.502f, 1.0f }; // This is #0080FF in hex
    return mitk::Color(blue);
  }
  case 3:
  {
    return m_DecorationColorWidget4;
  }
  default:
    MITK_ERROR << "Decoration color for unknown widget!";
    float black[3] = { 0.0f, 0.0f, 0.0f };
    return mitk::Color(black);
  }
}

void QmitkStdMultiWidget::mousePressEvent(QMouseEvent* e)
{
  if (QEvent::MouseButtonPress != e->type())
  {
    return;
  }

  auto renderWindowWidget = dynamic_cast<QmitkRenderWindowWidget*>(this->sender());
  if (nullptr == renderWindowWidget)
  {
    return;
  }

  auto renderWindowWidgetPointer = GetRenderWindowWidget(renderWindowWidget->GetWidgetName());
  SetActiveRenderWindowWidget(renderWindowWidgetPointer);
}

void QmitkStdMultiWidget::moveEvent(QMoveEvent* e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the Annotation as the StdMultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

void QmitkStdMultiWidget::wheelEvent(QWheelEvent* e)
{
  emit WheelMoved(e);
}

void QmitkStdMultiWidget::HandleCrosshairPositionEventDelayed()
{
  auto dataStorage = GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  m_PendingCrosshairPositionEvent = false;

  // find image with highest layer
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = dataStorage->GetSubset(isImageData).GetPointer();
  mitk::Point3D crosshairPos = GetSelectedPosition("");
  mitk::BaseRenderer* baseRenderer = GetRenderWindow1()->GetSliceNavigationController()->GetRenderer();
  auto globalCurrentTimePoint = baseRenderer->GetTime();
  mitk::DataNode::Pointer node = mitk::FindTopmostVisibleNode(nodes, crosshairPos, globalCurrentTimePoint, baseRenderer);

  mitk::DataNode::Pointer topSourceNode;
  mitk::Image::Pointer image;
  bool isBinary = false;
  int component = 0;

  if (node.IsNotNull())
  {
    node->GetBoolProperty("binary", isBinary);
    if (isBinary)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = dataStorage->GetSources(node, nullptr, true);
      if (!sourcenodes->empty())
      {
        topSourceNode = mitk::FindTopmostVisibleNode(sourcenodes, crosshairPos, globalCurrentTimePoint, baseRenderer);
      }
      if (topSourceNode.IsNotNull())
      {
        image = dynamic_cast<mitk::Image *>(topSourceNode->GetData());
        topSourceNode->GetIntProperty("Image.Displayed Component", component);
      }
      else
      {
        image = dynamic_cast<mitk::Image *>(node->GetData());
        node->GetIntProperty("Image.Displayed Component", component);
      }
    }
    else
    {
      image = dynamic_cast<mitk::Image *>(node->GetData());
      node->GetIntProperty("Image.Displayed Component", component);
    }
  }

  std::string statusText;
  std::stringstream stream;
  itk::Index<3> p;
  unsigned int timestep = baseRenderer->GetTimeStep();

  if (image.IsNotNull() && (image->GetTimeSteps() > timestep))
  {
    image->GetGeometry()->WorldToIndex(crosshairPos, p);
    stream.precision(2);
    stream << "Position: <" << std::fixed << crosshairPos[0] << ", " << std::fixed << crosshairPos[1] << ", "
      << std::fixed << crosshairPos[2] << "> mm";
    stream << "; Index: <" << p[0] << ", " << p[1] << ", " << p[2] << "> ";

    mitk::ScalarType pixelValue;

    mitkPixelTypeMultiplex5(mitk::FastSinglePixelAccess,
      image->GetChannelDescriptor().GetPixelType(),
      image,
      image->GetVolumeData(image->GetTimeGeometry()->TimePointToTimeStep(globalCurrentTimePoint)),
      p,
      pixelValue,
      component);

    if (fabs(pixelValue) > 1000000 || fabs(pixelValue) < 0.01)
    {
      stream << "; Time: " << globalCurrentTimePoint << " ms; Pixelvalue: " << std::scientific << pixelValue << "  ";
    }
    else
    {
      stream << "; Time: " << globalCurrentTimePoint << " ms; Pixelvalue: " << pixelValue << "  ";
    }
  }
  else
  {
    stream << "No image information at this position!";
  }

  statusText = stream.str();
  mitk::StatusBar::GetInstance()->DisplayGreyValueText(statusText.c_str());
}

void QmitkStdMultiWidget::Fit()
{
  vtkSmartPointer<vtkRenderer> vtkrenderer;
  vtkrenderer = mitk::BaseRenderer::GetInstance(GetRenderWindow1()->GetRenderWindow())->GetVtkRenderer();
  if (nullptr != vtkrenderer)
  {
    vtkrenderer->ResetCamera();
  }

  vtkrenderer = mitk::BaseRenderer::GetInstance(GetRenderWindow2()->GetRenderWindow())->GetVtkRenderer();
  if (nullptr != vtkrenderer)
  {
    vtkrenderer->ResetCamera();
  }

  vtkrenderer = mitk::BaseRenderer::GetInstance(GetRenderWindow3()->GetRenderWindow())->GetVtkRenderer();
  if (nullptr != vtkrenderer)
  {
    vtkrenderer->ResetCamera();
  }

  vtkrenderer = mitk::BaseRenderer::GetInstance(GetRenderWindow4()->GetRenderWindow())->GetVtkRenderer();
  if (nullptr != vtkrenderer)
  {
    vtkrenderer->ResetCamera();
  }

  mitk::BaseRenderer::GetInstance(GetRenderWindow1()->GetRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(GetRenderWindow2()->GetRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(GetRenderWindow3()->GetRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(GetRenderWindow4()->GetRenderWindow())->GetCameraController()->Fit();

  int w = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::AddDisplayPlaneSubTree()
{
  // add the displayed planes of the multiwidget to a node to which the subtree
  // @a planesSubTree points ...

  mitk::PlaneGeometryDataMapper2D::Pointer mapper;

  // ... of widget 1
  mitk::BaseRenderer* renderer1 = mitk::BaseRenderer::GetInstance(GetRenderWindow1()->GetRenderWindow());
  m_PlaneNode1 = renderer1->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode1->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode1->SetProperty("name", mitk::StringProperty::New(std::string(renderer1->GetName()) + ".plane"));
  m_PlaneNode1->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode1->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 2
  mitk::BaseRenderer* renderer2 = mitk::BaseRenderer::GetInstance(GetRenderWindow2()->GetRenderWindow());
  m_PlaneNode2 = renderer2->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode2->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("name", mitk::StringProperty::New(std::string(renderer2->GetName()) + ".plane"));
  m_PlaneNode2->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode2->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode2->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 3
  mitk::BaseRenderer *renderer3 = mitk::BaseRenderer::GetInstance(GetRenderWindow3()->GetRenderWindow());
  m_PlaneNode3 = renderer3->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("name", mitk::StringProperty::New(std::string(renderer3->GetName()) + ".plane"));
  m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode3->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  m_ParentNodeForGeometryPlanes = mitk::DataNode::New();
  m_ParentNodeForGeometryPlanes->SetProperty("name", mitk::StringProperty::New("Widgets"));
  m_ParentNodeForGeometryPlanes->SetProperty("helper object", mitk::BoolProperty::New(true));
}

void QmitkStdMultiWidget::EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D &p)
{
  mitk::Point2D pointOnDisplay;
  renderer->WorldToDisplay(p, pointOnDisplay);

  if (pointOnDisplay[0] < renderer->GetVtkRenderer()->GetOrigin()[0] ||
    pointOnDisplay[1] < renderer->GetVtkRenderer()->GetOrigin()[1] ||
    pointOnDisplay[0] > renderer->GetVtkRenderer()->GetOrigin()[0] + renderer->GetViewportSize()[0] ||
    pointOnDisplay[1] > renderer->GetVtkRenderer()->GetOrigin()[1] + renderer->GetViewportSize()[1])
  {
    mitk::Point2D pointOnPlane;
    renderer->GetCurrentWorldPlaneGeometry()->Map(p, pointOnPlane);
    renderer->GetCameraController()->MoveCameraToPoint(pointOnPlane);
  }
}

void QmitkStdMultiWidget::SetWidgetPlaneVisibility(const char *widgetName, bool visible, mitk::BaseRenderer *renderer)
{
  auto dataStorage = GetDataStorage();
  if (nullptr != dataStorage)
  {
    mitk::DataNode* dataNode = dataStorage->GetNamedNode(widgetName);
    if (dataNode != nullptr)
    {
      dataNode->SetVisibility(visible, renderer);
    }
  }
}

void QmitkStdMultiWidget::SetWidgetPlanesVisibility(bool visible, mitk::BaseRenderer *renderer)
{
  if (m_PlaneNode1.IsNotNull())
  {
    m_PlaneNode1->SetVisibility(visible, renderer);
  }
  if (m_PlaneNode2.IsNotNull())
  {
    m_PlaneNode2->SetVisibility(visible, renderer);
  }
  if (m_PlaneNode3.IsNotNull())
  {
    m_PlaneNode3->SetVisibility(visible, renderer);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkStdMultiWidget::SetLayoutImpl()
{
  CreateRenderWindowWidgets();
  GetMultiWidgetLayoutManager()->SetLayoutDesign(QmitkMultiWidgetLayoutManager::LayoutDesign::DEFAULT);

  ResetCrosshair();
}

void QmitkStdMultiWidget::CreateRenderWindowWidgets()
{
  // create axial render window (widget)
  QString renderWindowWidgetName = GetNameFromIndex(0, 0);
  RenderWindowWidgetPointer renderWindowWidget1 = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  auto renderWindow1 = renderWindowWidget1->GetRenderWindow();
  renderWindow1->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
  renderWindowWidget1->SetDecorationColor(GetDecorationColor(0));
  renderWindowWidget1->SetCornerAnnotationText("Axial");
  renderWindowWidget1->GetRenderWindow()->SetLayoutIndex(ViewDirection::AXIAL);
  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget1);

  // create sagittal render window (widget)
  renderWindowWidgetName = GetNameFromIndex(0, 1);
  RenderWindowWidgetPointer renderWindowWidget2 = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  auto renderWindow2 = renderWindowWidget2->GetRenderWindow();
  renderWindow2->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
  renderWindowWidget2->SetDecorationColor(GetDecorationColor(1));
  renderWindowWidget2->SetCornerAnnotationText("Sagittal");
  renderWindowWidget2->GetRenderWindow()->SetLayoutIndex(ViewDirection::SAGITTAL);
  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget2);

  // create coronal render window (widget)
  renderWindowWidgetName = GetNameFromIndex(1, 0);
  RenderWindowWidgetPointer renderWindowWidget3 = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  auto renderWindow3 = renderWindowWidget3->GetRenderWindow();
  renderWindow3->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
  renderWindowWidget3->SetDecorationColor(GetDecorationColor(2));
  renderWindowWidget3->SetCornerAnnotationText("Coronal");
  renderWindowWidget3->GetRenderWindow()->SetLayoutIndex(ViewDirection::CORONAL);
  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget3);

  // create 3D render window (widget)
  renderWindowWidgetName = GetNameFromIndex(1, 1);
  RenderWindowWidgetPointer renderWindowWidget4 = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  auto renderWindow4 = renderWindowWidget4->GetRenderWindow();
  renderWindow4->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Original);
  renderWindowWidget4->SetDecorationColor(GetDecorationColor(3));
  renderWindowWidget4->SetCornerAnnotationText("3D");
  renderWindowWidget4->GetRenderWindow()->SetLayoutIndex(ViewDirection::THREE_D);
  mitk::BaseRenderer::GetInstance(renderWindowWidget4->GetRenderWindow()->GetRenderWindow())->SetMapperID(mitk::BaseRenderer::Standard3D);
  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget4);

  SetActiveRenderWindowWidget(renderWindowWidget1);

  // connect to the "time navigation controller": send time via sliceNavigationControllers
  m_TimeNavigationController->ConnectGeometryTimeEvent(renderWindow1->GetSliceNavigationController(), false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(renderWindow2->GetSliceNavigationController(), false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(renderWindow3->GetSliceNavigationController(), false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(renderWindow4->GetSliceNavigationController(), false);
  renderWindow1->GetSliceNavigationController()->ConnectGeometrySendEvent(
    mitk::BaseRenderer::GetInstance(renderWindow4->GetRenderWindow()));

  // reverse connection between sliceNavigationControllers and timeNavigationController
  renderWindow1->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  renderWindow2->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  renderWindow3->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  //renderWindow4->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);

  auto layoutManager = GetMultiWidgetLayoutManager();
  connect(renderWindowWidget1.get(), &QmitkRenderWindowWidget::MouseEvent, this, &QmitkStdMultiWidget::mousePressEvent);
  connect(renderWindow1, &QmitkRenderWindow::ResetView, this, &QmitkStdMultiWidget::ResetCrosshair);
  connect(renderWindow1, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkStdMultiWidget::SetCrosshairVisibility);
  connect(renderWindow1, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkStdMultiWidget::SetWidgetPlaneMode);
  connect(renderWindow1, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);

  connect(renderWindowWidget2.get(), &QmitkRenderWindowWidget::MouseEvent, this, &QmitkStdMultiWidget::mousePressEvent);
  connect(renderWindow2, &QmitkRenderWindow::ResetView, this, &QmitkStdMultiWidget::ResetCrosshair);
  connect(renderWindow2, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkStdMultiWidget::SetCrosshairVisibility);
  connect(renderWindow2, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkStdMultiWidget::SetWidgetPlaneMode);
  connect(renderWindow2, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);

  connect(renderWindowWidget3.get(), &QmitkRenderWindowWidget::MouseEvent, this, &QmitkStdMultiWidget::mousePressEvent);
  connect(renderWindow3, &QmitkRenderWindow::ResetView, this, &QmitkStdMultiWidget::ResetCrosshair);
  connect(renderWindow3, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkStdMultiWidget::SetCrosshairVisibility);
  connect(renderWindow3, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkStdMultiWidget::SetWidgetPlaneMode);
  connect(renderWindow3, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);

  connect(renderWindowWidget4.get(), &QmitkRenderWindowWidget::MouseEvent, this, &QmitkStdMultiWidget::mousePressEvent);
  connect(renderWindow4, &QmitkRenderWindow::ResetView, this, &QmitkStdMultiWidget::ResetCrosshair);
  connect(renderWindow4, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkStdMultiWidget::SetCrosshairVisibility);
  connect(renderWindow4, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkStdMultiWidget::SetWidgetPlaneMode);
  connect(renderWindow4, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);
}
