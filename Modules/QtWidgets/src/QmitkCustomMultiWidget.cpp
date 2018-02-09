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

#include "QmitkCustomMultiWidget.h"

#include <QList>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>

#include <mitkUIDGenerator.h>

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

QmitkCustomMultiWidget::QmitkCustomMultiWidget(QWidget* parent,
                                               Qt::WindowFlags f/* = 0*/,
                                               mitk::RenderingManager* renderingManager/* = nullptr*/,
                                               mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/,
                                               const QString& multiWidgetName/* = "custommulti"*/)
  : QWidget(parent, f)
  , m_CustomMultiWidgetLayout(nullptr)
  , m_RenderingManager(renderingManager)
  , m_RenderingMode(renderingMode)
  , m_MultiWidgetName(multiWidgetName)
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
}

QmitkCustomMultiWidget::~QmitkCustomMultiWidget()
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    m_TimeNavigationController->Disconnect(renderWindowWidget.second->GetRenderWindow()->GetSliceNavigationController());
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

QmitkCustomMultiWidget::RenderWindowWidgetMap QmitkCustomMultiWidget::GetRenderWindowWidgets() const
{
  return m_RenderWindowWidgets;
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetRenderWindowWidget(const QString& widgetID) const
{
  RenderWindowWidgetMap::const_iterator it = m_RenderWindowWidgets.find(widgetID);
  if (it == m_RenderWindowWidgets.end())
  {
    return nullptr;
  }

  return it->second;
}

QmitkRenderWindow* QmitkCustomMultiWidget::GetRenderWindow(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->GetRenderWindow();
  }

  return nullptr;
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetActiveRenderWindowWidget() const
{
  //return m_ActiveRenderWindowWidget;
  return m_RenderWindowWidgets.begin()->second;
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

void QmitkCustomMultiWidget::RequestUpdate(const QString& widgetID)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->RequestUpdate();
  }
}

void QmitkCustomMultiWidget::RequestUpdateAll()
{
  // #TODO: Update only render windows that show the same image?
  // #TODO: Update only type specific render windows (2D / 3D)?
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->RequestUpdate();
  }
}

void QmitkCustomMultiWidget::ForceImmediateUpdate(const QString& widgetID)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ForceImmediateUpdate();
  }
}

void QmitkCustomMultiWidget::ForceImmediateUpdateAll()
{
  // #TODO: Update only render windows that show the same image?
  // #TODO: Update only type specific render windows (2D / 3D)?
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ForceImmediateUpdate();
  }
}

mitk::MouseModeSwitcher* QmitkCustomMultiWidget::GetMouseModeSwitcher()
{
  return m_MouseModeSwitcher;
}

const mitk::Point3D QmitkCustomMultiWidget::GetCrossPosition(const QString& widgetID) const
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

//////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidget::ShowLevelWindowWidget(const QString& widgetID, bool show)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ShowLevelWindowWidget(show);
    return;
  }

  MITK_ERROR << "Level window widget can not be shown for an unknown widget.";
}

void QmitkCustomMultiWidget::ShowAllLevelWindowWidgets(bool show)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ShowLevelWindowWidget(show);
  }
}

void QmitkCustomMultiWidget::SetBackgroundColorGradient(const mitk::Color& upper, const mitk::Color& lower, const QString& widgetID)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->SetBackgroundColorGradient(upper, lower);
    return;
  }

  MITK_ERROR << "Background color gradient can not be set for an unknown widget.";
}

void QmitkCustomMultiWidget::SetAllBackgroundColorGradients(const mitk::Color& upper, const mitk::Color& lower)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->SetBackgroundColorGradient(upper, lower);
  }
}

void QmitkCustomMultiWidget::FillAllBackgroundColorGradientsWithBlack()
{
  float black[3] = { 0.0f, 0.0f, 0.0f };
  SetAllBackgroundColorGradients(black, black);
}

void QmitkCustomMultiWidget::ShowBackgroundColorGradient(const QString& widgetID, bool show)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ShowBackgroundColorGradient(show);
    return;
  }

  MITK_ERROR << "Background color gradient can not be shown for an unknown widget.";
}

void QmitkCustomMultiWidget::ShowAllBackgroundColorGradients(bool show)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ShowBackgroundColorGradient(show);
  }
}

std::pair<mitk::Color, mitk::Color> QmitkCustomMultiWidget::GetBackgroundColorGradient(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->GetRendererBackgroundColorGradient();
  }

  MITK_ERROR << "Background color gradient can not be retrieved for an unknown widget. Returning black color pair.";
  float black[3] = { 0.0f, 0.0f, 0.0f };
  return std::make_pair(mitk::Color(black), mitk::Color(black));
}

bool QmitkCustomMultiWidget::GetBackgroundColorGradientFlag(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->GetBackgroundColorGradientFlag();
  }

  MITK_ERROR << "Background color gradient flag can not be retrieved for an unknown widget. Returning 'false'.";
  return false;
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

void QmitkCustomMultiWidget::ShowDepartmentLogo(const QString& widgetID, bool show)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    GetRenderWindowWidget(widgetID)->ShowDepartmentLogo(show);
    RequestUpdate(widgetID);
  }

  MITK_ERROR << "Department logo can not be shown for an unknown widget.";
  return;
}

void QmitkCustomMultiWidget::ShowAllDepartmentLogos(bool show)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ShowDepartmentLogo(show);
  }
}

void QmitkCustomMultiWidget::SetDecorationColor(const QString& widgetID, const mitk::Color& color)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->SetDecorationColor(color);
  }

  MITK_ERROR << "Decoration color can not be set for an unknown widget.";
}

mitk::Color QmitkCustomMultiWidget::GetDecorationColor(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->GetDecorationColor();
  }

  MITK_ERROR << "Decoration color can not be retrieved for an unknown widget. Returning black color!";
  float black[3] = { 0.0f, 0.0f, 0.0f };
  return mitk::Color(black);
}

void QmitkCustomMultiWidget::ShowColoredRectangle(const QString& widgetID, bool show)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ShowColoredRectangle(show);
  }

  MITK_ERROR << "Colored rectangle can not be set for an unknown widget.";
}

void QmitkCustomMultiWidget::ShowAllColoredRectangles(bool show)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ShowColoredRectangle(show);
  }
}

bool QmitkCustomMultiWidget::IsColoredRectangleVisible(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->IsColoredRectangleVisible();
  }

  MITK_ERROR << "Colored rectangle visibility can not be retrieved for an unknown widget. Returning 'false'.";
  return false;
}

void QmitkCustomMultiWidget::SetCornerAnnotationText(const QString& widgetID, const std::string& cornerAnnotation)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->SetCornerAnnotationText(cornerAnnotation);
  }

  MITK_ERROR << "Corner annotation text can not be retrieved for an unknown widget.";
}

std::string QmitkCustomMultiWidget::GetCornerAnnotationText(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->GetCornerAnnotationText();
  }

  MITK_ERROR << "Corner annotation text can not be retrieved for an unknown widget.";
  return "";
}

void QmitkCustomMultiWidget::ShowCornerAnnotation(const QString& widgetID, bool show)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ShowCornerAnnotation(show);
  }

  MITK_ERROR << "Corner annotation can not be set for an unknown widget.";
}

void QmitkCustomMultiWidget::ShowAllCornerAnnotations(bool show)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ShowCornerAnnotation(show);
  }
}

bool QmitkCustomMultiWidget::IsCornerAnnotationVisible(const QString& widgetID) const
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->IsCornerAnnotationVisible();
  }

  MITK_ERROR << "Corner annotation visibility can not be retrieved for an unknown widget. Returning 'false'.";
  return false;
}

void QmitkCustomMultiWidget::HandleCrosshairPositionEvent()
{
  /*
  if (!m_PendingCrosshairPositionEvent)
  {
    m_PendingCrosshairPositionEvent = true;
    QTimer::singleShot(0, this, SLOT(HandleCrosshairPositionEventDelayed()));
  }
  */
}

void QmitkCustomMultiWidget::HandleCrosshairPositionEventDelayed()
{
  /*
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
  */
}

void QmitkCustomMultiWidget::Fit()
{
  // #TODO: what is this function's purpose?

  /*
  vtkSmartPointer<vtkRenderer> vtkrenderer;
  size_t numberOfRenderWindowWidgets = m_RenderWindowWidgets.size();
  for (size_t i = 0; i < numberOfRenderWindowWidgets; ++i)
  {
  vtkRenderer* renderer = GetRenderWindow(widgetID)->GetRenderer()->GetVtkRenderer();
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
  */
}

void QmitkCustomMultiWidget::EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D &p)
{
  // #TODO: what is this function's purpose?

  /*
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
  */
}

void QmitkCustomMultiWidget::MoveCrossToPosition(const QString& widgetID, const mitk::Point3D& newPosition)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    GetRenderWindowWidget(widgetID)->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
    GetRenderWindowWidget(widgetID)->RequestUpdate();
  }

  MITK_ERROR << "Geometry plane can not be shown for an unknown widget.";

}

void QmitkCustomMultiWidget::ResetCrosshair()
{
  // #TODO: new concept: we do not want to initialize all views;
  //        we do not want to rely on the geometry planes
  /*
  if (m_DataStorage.IsNotNull())
  {
    m_RenderingManager->InitializeViewsByBoundingObjects(m_DataStorage);
    // m_RenderingManager->InitializeViews( m_DataStorage->ComputeVisibleBoundingGeometry3D() );
    // reset interactor to normal slicing
    SetWidgetPlaneMode(PLANE_MODE_SLICING);
  }
  */
}

//////////////////////////////////////////////////////////////////////////
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidget::wheelEvent(QWheelEvent* e)
{
  emit WheelMoved(e);
}

void QmitkCustomMultiWidget::mousePressEvent(QMouseEvent* e)
{
}

void QmitkCustomMultiWidget::moveEvent(QMoveEvent* e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the overlays as the StdMultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}


//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidget::InitializeGUI()
{
  m_CustomMultiWidgetLayout = new QGridLayout(this);
  m_CustomMultiWidgetLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_CustomMultiWidgetLayout);
}

void QmitkCustomMultiWidget::InitializeWidget()
{
  // #TODO: some things have to be handled globally (hold for all render window (widgets)
  // analyse those things and design a controlling mechanism

  // necessary here? mouse mode is valid for all render windows (and also used in editor)
  m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();
  m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::MITK);

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

void QmitkCustomMultiWidget::AddRenderWindowWidget()
{
  // #TODO: add QSplitter?
  // #TODO: include technique, to set the image to level-slide on using the render window manager

  // create the render window widget and connect signals / slots
  mitk::UIDGenerator generator;
  std::string renderWindowUID = generator.GetUID();
  QString UID = m_MultiWidgetName + "_" + QString::fromStdString(renderWindowUID);
  QmitkRenderWindowWidget* renderWindowWidget = new QmitkRenderWindowWidget(this, UID, m_DataStorage);

  // create connections
  connect(renderWindowWidget, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(renderWindowWidget, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));

  // store the newly created render window widget with the UID
  m_RenderWindowWidgets.insert(std::pair<QString, QmitkRenderWindowWidget*>(UID, renderWindowWidget));

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

mitk::DataNode::Pointer QmitkCustomMultiWidget::GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes)
{
  // #TODO: see T24173
  return nodes->front();
}
