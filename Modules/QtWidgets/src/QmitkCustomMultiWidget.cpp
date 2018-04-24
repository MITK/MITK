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

// mitk core
#include <mitkUIDGenerator.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkCameraController.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>
#include <mitkNodePredicateBase.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkStatusBar.h>
#include <mitkVtkLayerController.h>
#include <vtkSmartPointer.h>

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
  , m_PendingCrosshairPositionEvent(false)
  , m_CrosshairNavigationEnabled(false)
  , m_DisplayActionEventBroadcast(nullptr)
  , m_StdDisplayActionEventHandler(nullptr)
  , m_DataStorage(nullptr)
{
  // create widget manually
  // create and set layout
  InitializeGUI();
  InitializeWidget();
  InitializeDisplayActionEventHandling();
  resize(QSize(364, 477).expandedTo(minimumSizeHint()));
}

QmitkCustomMultiWidget::~QmitkCustomMultiWidget()
{
  // nothing here
}

void QmitkCustomMultiWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (dataStorage == m_DataStorage)
  {
    return;
  }

  m_DataStorage = dataStorage;
  // set new data storage for the render window widgets
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->SetDataStorage(m_DataStorage);
  }
}

void QmitkCustomMultiWidget::InitializeRenderWindowWidgets()
{
  // create render window (widgets) initially
  AddRenderWindowWidget(0, 0, "2015-01-14 - CT");
  /*
  AddRenderWindowWidget(0, 1, "2015-08-19 - CT");
  AddRenderWindowWidget(0, 2, "2016-06-29 - CT");
  AddRenderWindowWidget(1, 0, "2015-01-14 - MR");
  AddRenderWindowWidget(1, 1, "2015-08-19 - MR");
  AddRenderWindowWidget(1, 2, "2016-06-29 - MR");
  */
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

void QmitkCustomMultiWidget::AddRenderWindowWidget(int row, int column, const std::string& cornerAnnotation/* = ""*/)
{
  // create the render window widget and connect signals / slots
  mitk::UIDGenerator generator;
  std::string renderWindowUID = generator.GetUID();
  QString UID = m_MultiWidgetName + "_" + QString::fromStdString(renderWindowUID);
  QmitkRenderWindowWidget* renderWindowWidget = new QmitkRenderWindowWidget(this, UID, m_DataStorage);
  renderWindowWidget->SetCornerAnnotationText(cornerAnnotation);

  // create connections
  connect(renderWindowWidget, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(renderWindowWidget, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));

  // store the newly created render window widget with the UID
  m_RenderWindowWidgets.insert(std::pair<QString, QmitkRenderWindowWidget*>(UID, renderWindowWidget));

  // add the newly created render window widget to this multi widget
  m_CustomMultiWidgetLayout->addWidget(renderWindowWidget, row, column);
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
  if (!m_RenderWindowWidgets.empty())
  {
    return m_RenderWindowWidgets.begin()->second;
  }
  else
  {
    return nullptr;
  }
}

QmitkRenderWindowWidget* QmitkCustomMultiWidget::GetLastRenderWindowWidget() const
{
  if (!m_RenderWindowWidgets.empty())
  {
    return m_RenderWindowWidgets.rbegin()->second;
  }
  else
  {
    return nullptr;
  }
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

void QmitkCustomMultiWidget::MoveCrossToPosition(const QString& widgetID, const mitk::Point3D& newPosition)
{
  QmitkRenderWindowWidget* renderWindowWidget = GetRenderWindowWidget(widgetID);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
    renderWindowWidget->RequestUpdate();
    return;
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

  // it is necessary to readjust the position of the overlays as the MultiWidget has moved
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
  //m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();
  //m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::MITK);
}

void QmitkCustomMultiWidget::InitializeDisplayActionEventHandling()
{
  m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
  m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
  m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigPACS.xml");

  m_StdDisplayActionEventHandler = std::make_unique<mitk::StdDisplayActionEventHandler>();
  m_StdDisplayActionEventHandler->SetObservableBroadcast(m_DisplayActionEventBroadcast);
  m_StdDisplayActionEventHandler->InitStdActions();
}

mitk::DataNode::Pointer QmitkCustomMultiWidget::GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes)
{
  // #TODO: see T24173
  return nodes->front();
}
