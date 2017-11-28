/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#define SMW_INFO MITK_INFO("widget.custommulti")

#include "QmitkCustomMultiWidget.h"

#include <QList>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>

#include "mitkImagePixelReadAccessor.h"
#include "mitkPixelTypeMultiplex.h"
#include <mitkManualPlacementAnnotationRenderer.h>
#include <mitkCameraController.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>
#include <mitkNodePredicateBase.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkStatusBar.h>
#include <mitkVtkLayerController.h>
#include <vtkSmartPointer.h>
#include <vtkQImageToImageSource.h>

#include <iomanip>

// qt
#include <QGridLayout>

QmitkCustomMultiWidget::QmitkCustomMultiWidget(QWidget* parent, Qt::WindowFlags f, mitk::RenderingManager* renderingManager, mitk::BaseRenderer::RenderingMode::Type renderingMode, const QString& multWidgetName)
  : QWidget(parent, f)
  , m_CustomMultiWidgetLayout(nullptr)
  , m_PlaneMode(PLANE_MODE_SLICING)
  , m_RenderingManager(renderingManager)
  , m_RenderingMode(renderingMode)
  , m_MultiWidgetName(multWidgetName)
  , m_TimeNavigationController(nullptr)
  , m_PendingCrosshairPositionEvent(false)
  , m_CrosshairNavigationEnabled(false)
{
  if (nullptr == m_RenderingManager)
  {
    m_RenderingManager = mitk::RenderingManager::GetInstance();
  }
  m_TimeNavigationController = m_RenderingManager->GetTimeNavigationController();

  // create widget manually
  // create and set layout
  InitializeGUI();
  // create at least one render window initially
  AddRenderWindowWidget();

  resize(QSize(364, 477).expandedTo(minimumSizeHint()));

  InitializeWidget();
  ActivateAllRenderWindowMenus(true);
}

QmitkCustomMultiWidget::~QmitkCustomMultiWidget()
{
  for (const auto& mapEntry : m_RenderWindowWidgets)
  {
    m_TimeNavigationController->Disconnect(mapEntry.second->GetRenderWindow()->GetSliceNavigationController());
  }
}

void QmitkCustomMultiWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (dataStorage == m_DataStorage)
  {
    return;
  }

  m_DataStorage = dataStorage;
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->SetDataStorage(m_DataStorage);
  }
}

std::map<std::string, QmitkRenderWindowWidget*> QmitkCustomMultiWidget::GetRenderWindowWidgets() const
{
  return m_RenderWindowWidgets;
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetRenderWindowWidget(unsigned int id) const
{
  return m_RenderWindowWidgets.find(std::to_string(id))->second;
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetActiveRenderWindowWidget() const
{
  return m_ActiveRenderWindowWidget;
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetFirstRenderWindowWidget() const
{
  return m_RenderWindowWidgets.begin()->second;
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetLastRenderWindowWidget() const
{
  return m_RenderWindowWidgets.rbegin()->second;
}

unsigned int QmitkCustomMultiWidget::GetNumberOfRenderWindowWidgets() const
{
  return m_RenderWindowWidgets.size();
}

void QmitkCustomMultiWidget::InitializeWidget()
{
  // #TODO: some things have to be handled globally (hold for all render window (widgets)
  // analyse those things and design a controlling mechanism

  // Set plane mode (slicing/rotation behavior) to slicing (default)
  m_PlaneMode = PLANE_MODE_SLICING;

  m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();

  // setup the department logo rendering
  /*
  m_LogoRendering = mitk::LogoOverlay::New();
  mitk::BaseRenderer::Pointer renderer4 = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow());
  m_LogoRendering->SetOpacity(0.5);
  mitk::Point2D offset;
  offset.Fill(0.03);
  m_LogoRendering->SetOffsetVector(offset);
  m_LogoRendering->SetRelativeSize(0.15);
  m_LogoRendering->SetCornerPosition(1);
  m_LogoRendering->SetLogoImagePath("DefaultLogo");
  renderer4->GetOverlayManager()->AddOverlay(m_LogoRendering.GetPointer(), renderer4);
  */
}

const mitk::Point3D QmitkCustomMultiWidget::GetCrossPosition() const
{
  /*
  const mitk::PlaneGeometry *plane1 = mitkWidget1->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane2 = mitkWidget2->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane3 = mitkWidget3->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ((plane1 != NULL) && (plane2 != NULL) && (plane1->IntersectionLine(plane2, line)))
  {
    mitk::Point3D point;
    if ((plane3 != NULL) && (plane3->IntersectionPoint(line, point)))
    {
      return point;
    }
  }
  // TODO BUG POSITIONTRACKER;
  mitk::Point3D p;
  return p;
  // return m_LastLeftClickPositionSupplier->GetCurrentPoint();
  */
  return mitk::Point3D();
}

void QmitkCustomMultiWidget::RequestUpdate(unsigned int widgetNumber)
{
  GetRenderWindowWidget(widgetNumber)->RequestUpdate();
}

void QmitkCustomMultiWidget::RequestUpdateAll()
{
  // #TODO: Update only render windows that show the same image?
  // #TODO: Update only type specific render windows (2D / 3D)?
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    RequestUpdate(i);
  }
}

void QmitkCustomMultiWidget::ForceImmediateUpdate(unsigned int widgetNumber)
{
  GetRenderWindowWidget(widgetNumber)->ForceImmediateUpdate();
}

void QmitkCustomMultiWidget::ForceImmediateUpdateAll()
{
  // #TODO: Update only render windows that show the same image?
  // #TODO: Update only type specific render windows (2D / 3D)?
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ForceImmediateUpdate(i);
  }
}

void QmitkCustomMultiWidget::EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D &p)
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

void QmitkCustomMultiWidget::MoveCrossToPosition(const mitk::Point3D &newPosition)
{
  GetRenderWindow()->GetSliceNavigationController()->SelectSliceByPoint(newPosition);

  m_RenderingManager->RequestUpdateAll();
}

void QmitkCustomMultiWidget::HandleCrosshairPositionEvent()
{
  if (!m_PendingCrosshairPositionEvent)
  {
    m_PendingCrosshairPositionEvent = true;
    QTimer::singleShot(0, this, SLOT(HandleCrosshairPositionEventDelayed()));
  }
}

mitk::DataNode::Pointer QmitkCustomMultiWidget::GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes)
{
  mitk::Point3D crosshairPos = GetCrossPosition();
  mitk::DataNode::Pointer node;
  int maxlayer = -32768;

  if (nodes.IsNotNull())
  {
    mitk::BaseRenderer *baseRenderer = GetRenderWindow()->GetSliceNavigationController()->GetRenderer();
    // find node with largest layer, that is the node shown on top in the render window
    for (unsigned int x = 0; x < nodes->size(); x++)
    {
      if ((nodes->at(x)->GetData()->GetGeometry() != NULL) &&
           nodes->at(x)->GetData()->GetGeometry()->IsInside(crosshairPos))
      {
        int layer = 0;
        if (!(nodes->at(x)->GetIntProperty("layer", layer)))
        {
          continue;
        }
        if (layer > maxlayer)
        {
          if (static_cast<mitk::DataNode::Pointer>(nodes->at(x))->IsVisible(baseRenderer))
          {
            node = nodes->at(x);
            maxlayer = layer;
          }
        }
      }
    }
  }
  return node;
}

void QmitkCustomMultiWidget::HandleCrosshairPositionEventDelayed()
{
  m_PendingCrosshairPositionEvent = false;

  // find image with highest layer
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->m_DataStorage->GetSubset(isImageData).GetPointer();

  mitk::DataNode::Pointer node;
  mitk::DataNode::Pointer topSourceNode;
  mitk::Image::Pointer image;
  bool isBinary = false;
  node = this->GetTopLayerNode(nodes);
  int component = 0;
  if (node.IsNotNull())
  {
    node->GetBoolProperty("binary", isBinary);
    if (isBinary)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = m_DataStorage->GetSources(node, NULL, true);
      if (!sourcenodes->empty())
      {
        topSourceNode = this->GetTopLayerNode(sourcenodes);
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

  mitk::Point3D crosshairPos = this->GetCrossPosition();
  std::string statusText;
  std::stringstream stream;
  itk::Index<3> p;
  mitk::BaseRenderer *baseRenderer = GetRenderWindow()->GetSliceNavigationController()->GetRenderer();
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
                            image->GetVolumeData(baseRenderer->GetTimeStep()),
                            p,
                            pixelValue,
                            component);

    if (fabs(pixelValue) > 1000000 || fabs(pixelValue) < 0.01)
    {
      stream << "; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: " << std::scientific << pixelValue << "  ";
    }
    else
    {
      stream << "; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: " << pixelValue << "  ";
    }
  }
  else
  {
    stream << "No image information at this position!";
  }

  statusText = stream.str();
  mitk::StatusBar::GetInstance()->DisplayGreyValueText(statusText.c_str());
}

void QmitkCustomMultiWidget::Fit()
{
  vtkSmartPointer<vtkRenderer> vtkrenderer;
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    vtkRenderer* renderer = GetRenderWindow(widgetNumber)->GetRenderer()->GetVtkRenderer();
    mitk::BaseRenderer* baseRenderer = mitk::BaseRenderer::GetInstance(GetRenderWindow(i)->GetRenderWindow());
    vtkrenderer = baseRenderer->GetVtkRenderer();
    if (nullptr != vtkrenderer)
    {
      vtkrenderer->ResetCamera();
    }

    baseRenderer->GetCameraController()->Fit();
  }



  int w = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();
  vtkObject::SetGlobalWarningDisplay(w);
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
//////////////////////////////////////////////////////////////////////////
// TODO: Use widget directly instead of number?
void QmitkCustomMultiWidget::ShowLevelWindowWidget(unsigned int widgetNumber, bool show)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Level window widget can not be shown for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->ShowLevelWindowWidget(show);
}

// TODO: Use foreach on widgets map instead of iterator i and "widget number"?
void QmitkCustomMultiWidget::ShowAllLevelWindowWidgets(bool show)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ShowLevelWindowWidget(i, show);
  }
}

void QmitkCustomMultiWidget::SetBackgroundColorGradient(const mitk::Color& upper, const mitk::Color& lower, unsigned int widgetNumber)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Background color gradient can not be set for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->SetBackgroundColorGradient(upper, lower);
}

void QmitkCustomMultiWidget::SetAllBackgroundColorGradients(const mitk::Color& upper, const mitk::Color& lower)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    SetBackgroundColorGradient(upper, lower, i);
  }
}

void QmitkCustomMultiWidget::FillAllBackgroundColorGradientsWithBlack()
{
  float black[3] = { 0.0f, 0.0f, 0.0f };
  SetAllBackgroundColorGradients(black, black);
}

void QmitkCustomMultiWidget::ShowBackgroundColorGradient(unsigned int widgetNumber, bool show)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Background color gradient can not be shown for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->ShowBackgroundColorGradient(show);
}

void QmitkCustomMultiWidget::ShowAllBackgroundColorGradients(bool show)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ShowBackgroundColorGradient(i, show);
  }
}

std::pair<mitk::Color, mitk::Color> QmitkCustomMultiWidget::GetBackgroundColorGradient(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Background color gradient can not be retrieved for an unknown widget. Returning black color pair.";
    float black[3] = { 0.0f, 0.0f, 0.0f };
    return std::make_pair(mitk::Color(black), mitk::Color(black));
  }

  return GetRenderWindowWidget(widgetNumber)->GetRendererBackgroundColorGradient();
}

bool QmitkCustomMultiWidget::GetBackgroundColorGradientFlag(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Background color gradient flag can not be retrieved for an unknown widget. Returning 'false'.";
    return false;
  }

  return GetRenderWindowWidget(widgetNumber)->GetBackgroundColorGradientFlag();
}

void QmitkCustomMultiWidget::SetDepartmentLogoPath(const char* path)
{
  /* old
  m_LogoRendering->SetLogoImagePath(path);
  mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow());
  m_LogoRendering->Update(renderer);
  RequestUpdate();
  */
  /* new
  QImage* qimage = new QImage(path);
  vtkSmartPointer<vtkQImageToImageSource> qImageToVtk;
  qImageToVtk = vtkSmartPointer<vtkQImageToImageSource>::New();

  qImageToVtk->SetQImage(qimage);
  qImageToVtk->Update();

  m_LogoRendering->SetLogoImage(qImageToVtk->GetOutput());
  mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow());
  m_LogoRendering->Update(renderer);
  RequestUpdate();
  */
}

void QmitkCustomMultiWidget::ShowDepartmentLogo(unsigned int widgetNumber, bool show)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Department logo can not be shown for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->ShowDepartmentLogo(show);
  RequestUpdate(widgetNumber);
}

void QmitkCustomMultiWidget::ShowAllDepartmentLogos(bool show)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ShowDepartmentLogo(i, show);
  }
}

void QmitkCustomMultiWidget::SetDecorationColor(unsigned int widgetNumber, const mitk::Color& color)
{
  // #TODO: change specific plane color of a render window widget (parameter: unsigned int planeNumber)?
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Decoration color can not be set for an unknown widget.";
    return;
  }
  GetRenderWindowWidget(widgetNumber)->SetDecorationColor(color);
}

mitk::Color QmitkCustomMultiWidget::GetDecorationColor(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Decoration color can not be retrieved for an unknown widget. Returning black color!";
    float black[3] = { 0.0f, 0.0f, 0.0f };
    return mitk::Color(black);
  }

  return GetRenderWindowWidget(widgetNumber)->GetDecorationColor();
}

void QmitkCustomMultiWidget::ShowColoredRectangle(unsigned int widgetNumber, bool show)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Colored rectangle can not be set for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->ShowColoredRectangle(show);
}

void QmitkCustomMultiWidget::ShowAllColoredRectangles(bool show)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ShowColoredRectangle(i, show);
  }
}

bool QmitkCustomMultiWidget::IsColoredRectangleVisible(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Colored rectangle visibility can not be retrieved for an unknown widget. Returning 'false'.";
    return false;
  }

  return GetRenderWindowWidget(widgetNumber)->IsColoredRectangleVisible();
}

void QmitkCustomMultiWidget::SetCornerAnnotationText(unsigned int widgetNumber, const std::string& cornerAnnotation)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Corner annotation text can not be retrieved for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->SetCornerAnnotationText(cornerAnnotation);
}

std::string QmitkCustomMultiWidget::GetCornerAnnotationText(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Corner annotation text can not be retrieved for an unknown widget.";
    return "";
  }

  return GetRenderWindowWidget(widgetNumber)->GetCornerAnnotationText();
}

void QmitkCustomMultiWidget::ShowCornerAnnotation(unsigned int widgetNumber, bool show)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Corner annotation can not be set for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->ShowCornerAnnotation(show);
}

void QmitkCustomMultiWidget::ShowAllCornerAnnotations(bool show)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ShowCornerAnnotation(i, show);
  }
}

bool QmitkCustomMultiWidget::IsCornerAnnotationVisible(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Corner annotation visibility can not be retrieved for an unknown widget. Returning 'false'.";
    return false;
  }

  return GetRenderWindowWidget(widgetNumber)->IsCornerAnnotationVisible();
}

void QmitkCustomMultiWidget::ActivateRenderWindowMenu(unsigned int widgetNumber, bool state)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Level window widget can not be shown for an unknown widget.";
    return;
  }

  GetRenderWindowWidget(widgetNumber)->ActivateMenuWidget(state, this);
}

void QmitkCustomMultiWidget::ActivateAllRenderWindowMenus(bool state)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ActivateRenderWindowMenu(i, state);
  }
}

bool QmitkCustomMultiWidget::IsRenderWindowMenuActivated(unsigned int widgetNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Render window menu activation status can not be retrieved for an unknown widget. Returning 'false'.";
    return false;
  }

  return GetRenderWindowWidget(widgetNumber)->IsRenderWindowMenuActivated();
}

void QmitkCustomMultiWidget::ShowGeometryPlanes(unsigned int widgetNumber, bool state)
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Geometry plane can not be shown for an unknown widget.";
    return;
  }

  return GetRenderWindowWidget(widgetNumber)->ShowGeometryPlanes(state);
}

void QmitkCustomMultiWidget::ShowAllGeometryPlanes(bool state)
{
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
    ShowGeometryPlanes(i, state);
  }
}

mitk::DataNode::Pointer QmitkCustomMultiWidget::GetGeometryPlane(unsigned int widgetNumber, unsigned int planeNumber) const
{
  if (widgetNumber > m_RenderWindowWidgets.size())
  {
    MITK_ERROR << "Geometry plane can not be received for an unknown widget.";
    return nullptr;
  }

  return GetRenderWindowWidget(widgetNumber)->GetGeometryPlane(planeNumber);
}



void QmitkCustomMultiWidget::wheelEvent(QWheelEvent *e)
{
  emit WheelMoved(e);
}

void QmitkCustomMultiWidget::mousePressEvent(QMouseEvent *e)
{
}

void QmitkCustomMultiWidget::moveEvent(QMoveEvent *e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the overlays as the StdMultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

void QmitkCustomMultiWidget::SetWidgetPlanesLocked(bool locked)
{
  GetRenderWindow()->GetSliceNavigationController()->SetSliceLocked(locked);
}

void QmitkCustomMultiWidget::SetWidgetPlanesRotationLocked(bool locked)
{
  GetRenderWindow()->GetSliceNavigationController()->SetSliceRotationLocked(locked);
}

void QmitkCustomMultiWidget::SetWidgetPlanesRotationLinked(bool link)
{
  emit WidgetPlanesRotationLinked(link);
}

void QmitkCustomMultiWidget::SetWidgetPlaneMode(int userMode)
{
  MITK_DEBUG << "Changing crosshair mode to " << userMode;

  emit WidgetNotifyNewCrossHairMode(userMode);
  // Convert user interface mode to actual mode
  {
    switch (userMode)
    {
    case 0:
      m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::MITK);
      break;
    case 1:
      m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::ROTATION);
      break;
    case 2:
      m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::ROTATIONLINKED);
      break;
    case 3:
      m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::SWIVEL);
      break;
    }
  }
}

void QmitkCustomMultiWidget::SetWidgetPlaneModeToSlicing(bool activate)
{
  if (activate)
  {
    SetWidgetPlaneMode(PLANE_MODE_SLICING);
  }
}

void QmitkCustomMultiWidget::SetWidgetPlaneModeToRotation(bool activate)
{
  if (activate)
  {
    SetWidgetPlaneMode(PLANE_MODE_ROTATION);
  }
}

void QmitkCustomMultiWidget::SetWidgetPlaneModeToSwivel(bool activate)
{
  if (activate)
  {
    SetWidgetPlaneMode(PLANE_MODE_SWIVEL);
  }
}

void QmitkCustomMultiWidget::ResetCrosshair()
{
  if (m_DataStorage.IsNotNull())
  {
    m_RenderingManager->InitializeViewsByBoundingObjects(m_DataStorage);
    // m_RenderingManager->InitializeViews( m_DataStorage->ComputeVisibleBoundingGeometry3D() );
    // reset interactor to normal slicing
    SetWidgetPlaneMode(PLANE_MODE_SLICING);
  }
}

mitk::MouseModeSwitcher* QmitkCustomMultiWidget::GetMouseModeSwitcher()
{
  return m_MouseModeSwitcher;
}

void QmitkCustomMultiWidget::InitializeGUI()
{
  m_CustomMultiWidgetLayout = new QGridLayout(this);
  m_CustomMultiWidgetLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_CustomMultiWidgetLayout);
}

void QmitkCustomMultiWidget::AddRenderWindowWidget()
{
  // #TODO: add QSplitter?
  // #TODO: include technique, to set the image to level-slide on using the render window manager

  // create the render window widget and connect signals / slots
  std::string UID = "UID";
  QmitkRenderWindowWidget* renderWindowWidget = new QmitkRenderWindowWidget(this, UID, m_DataStorage);

  // create connections
  connect(renderWindowWidget, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(renderWindowWidget, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));
  connect(this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), renderWindowWidget, SLOT(OnWidgetPlaneModeChanged(int)));

  // store the newly created render window widget with the UID
  m_RenderWindowWidgets.insert(std::pair<std::string, QmitkRenderWindowWidget*>(UID, renderWindowWidget));

  mitk::SliceNavigationController* sliceNavigationController = renderWindowWidget->GetSliceNavigationController();
  if (nullptr != sliceNavigationController)
  {
    m_TimeNavigationController->ConnectGeometryTimeEvent(sliceNavigationController, false);
    sliceNavigationController->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  }

  // #TODO: define the grid cell to add the new render window widget
  // add the newly created render window widget to this multi widget
  m_CustomMultiWidgetLayout->addWidget(renderWindowWidget);
}
