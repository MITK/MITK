/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSlicesInterpolator.h"
#include "QmitkRenderWindow.h"
#include "QmitkRenderWindowWidget.h"

#include "mitkApplyDiffImageOperation.h"
#include "mitkColorProperty.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDiffImageApplier.h"
#include "mitkInteractionConst.h"
#include "mitkLevelWindowProperty.h"
#include "mitkOperationEvent.h"
#include "mitkProgressBar.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkSegTool2D.h"
#include "mitkSliceNavigationController.h"
#include "mitkSurfaceToImageFilter.h"
#include <mitkTimeNavigationController.h>
#include "mitkToolManager.h"
#include "mitkUndoController.h"

#include <mitkNodePredicateProperty.h>


#include <mitkExtractSliceFilter.h>
#include <mitkPlanarCircle.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageWriteAccessor.h>
#include <mitkPlaneProposer.h>
#include <mitkUnstructuredGridClusteringFilter.h>
#include <mitkVtkImageOverwrite.h>
#include <mitkShapeBasedInterpolationAlgorithm.h>
#include <itkCommand.h>

#include <mitkImageToContourFilter.h>
#include <mitkImagePixelReadAccessor.h>

//  Includes for the merge operation
#include "mitkImageToContourFilter.h"
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageConverter.h>

#include <QCheckBox>
#include <QCursor>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkPolyVertex.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <array>
#include <atomic>
#include <thread>
#include <vector>

namespace
{
  template <typename T = mitk::BaseData>
  itk::SmartPointer<T> GetData(const mitk::DataNode* dataNode)
  {
    return nullptr != dataNode
      ? dynamic_cast<T*>(dataNode->GetData())
      : nullptr;
  }
}

float SURFACE_COLOR_RGB[3] = {0.49f, 1.0f, 0.16f};

const QmitkSlicesInterpolator::ActionToSliceDimensionMapType QmitkSlicesInterpolator::CreateActionToSlicer(const QList<QmitkRenderWindow*>& windows)
{
  std::map<QAction *, mitk::SliceNavigationController *> actionToSliceDimension;
  for (auto* window : windows)
  {
    std::string windowName;
    auto renderWindowWidget = dynamic_cast<QmitkRenderWindowWidget*>(window->parentWidget());
    if (renderWindowWidget)
    {
      windowName = renderWindowWidget->GetCornerAnnotationText();
    }
    else
    {
      windowName = window->GetRenderer()->GetName();
    }
    auto slicer = window->GetSliceNavigationController();
    actionToSliceDimension[new QAction(QString::fromStdString(windowName), nullptr)] = slicer;
  }

  return actionToSliceDimension;
}

mitk::Image::Pointer ExtractSliceFromImage(mitk::Image* image,
                                          const mitk::PlaneGeometry * contourPlane,
                                          unsigned int timeStep)
{
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  // set to false to extract a slice
  reslice->SetOverwriteMode(false);
  reslice->Modified();

  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput(image);
  extractor->SetTimeStep(timeStep);
  extractor->SetWorldGeometry(contourPlane);
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry(image->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));
  extractor->Update();
  mitk::Image::Pointer slice = extractor->GetOutput();
  return slice;
}

QmitkSlicesInterpolator::QmitkSlicesInterpolator(QWidget *parent, const char * /*name*/)
  : QWidget(parent),
    m_Interpolator(mitk::SegmentationInterpolationController::New()),
    m_SurfaceInterpolator(mitk::SurfaceInterpolationController::GetInstance()),
    m_ToolManager(nullptr),
    m_Initialized(false),
    m_LastSNC(nullptr),
    m_LastSliceIndex(0),
    m_2DInterpolationEnabled(false),
    m_3DInterpolationEnabled(false),
    m_CurrentActiveLabelValue(0),
    m_FirstRun(true)
{
  m_GroupBoxEnableExclusiveInterpolationMode = new QGroupBox("Interpolation", this);

  QVBoxLayout *vboxLayout = new QVBoxLayout(m_GroupBoxEnableExclusiveInterpolationMode);

  m_CmbInterpolation = new QComboBox(m_GroupBoxEnableExclusiveInterpolationMode);
  m_CmbInterpolation->addItem("Disabled");
  m_CmbInterpolation->addItem("2-Dimensional");
  m_CmbInterpolation->addItem("3-Dimensional");
  vboxLayout->addWidget(m_CmbInterpolation);

  m_BtnApply2D = new QPushButton("Confirm for single slice", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApply2D);

  m_BtnApplyForAllSlices2D = new QPushButton("Confirm for all slices", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApplyForAllSlices2D);

  m_BtnApply3D = new QPushButton("Confirm", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApply3D);

  m_BtnReinit3DInterpolation = new QPushButton("Reinit Interpolation", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnReinit3DInterpolation);

  m_ChkShowPositionNodes = new QCheckBox("Show Position Nodes", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_ChkShowPositionNodes);

  this->HideAllInterpolationControls();

  connect(m_CmbInterpolation, SIGNAL(currentIndexChanged(int)), this, SLOT(OnInterpolationMethodChanged(int)));
  connect(m_BtnApply2D, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_BtnApplyForAllSlices2D, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()));
  connect(m_BtnApply3D, SIGNAL(clicked()), this, SLOT(OnAccept3DInterpolationClicked()));


  connect(m_BtnReinit3DInterpolation, SIGNAL(clicked()), this, SLOT(OnReinit3DInterpolation()));
  connect(m_ChkShowPositionNodes, SIGNAL(toggled(bool)), this, SLOT(OnShowMarkers(bool)));
  connect(m_ChkShowPositionNodes, SIGNAL(toggled(bool)), this, SIGNAL(SignalShowMarkerNodes(bool)));

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(m_GroupBoxEnableExclusiveInterpolationMode);
  this->setLayout(layout);

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command =
    itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnInterpolationInfoChanged);
  InterpolationInfoChangedObserverTag = m_Interpolator->AddObserver(itk::ModifiedEvent(), command);

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command2 =
    itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command2->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged);
  SurfaceInterpolationInfoChangedObserverTag = m_SurfaceInterpolator->AddObserver(itk::ModifiedEvent(), command2);

  auto command3 = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command3->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnInterpolationAborted);
  InterpolationAbortedObserverTag = m_Interpolator->AddObserver(itk::AbortEvent(), command3);

  // feedback node and its visualization properties
  m_FeedbackNode = mitk::DataNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties(m_FeedbackNode);

  m_FeedbackNode->SetProperty("binary", mitk::BoolProperty::New(true));
  m_FeedbackNode->SetProperty("outline binary", mitk::BoolProperty::New(true));
  m_FeedbackNode->SetProperty("color", mitk::ColorProperty::New(255.0, 255.0, 0.0));
  m_FeedbackNode->SetProperty("texture interpolation", mitk::BoolProperty::New(false));
  m_FeedbackNode->SetProperty("layer", mitk::IntProperty::New(20));
  m_FeedbackNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0, 1)));
  m_FeedbackNode->SetProperty("name", mitk::StringProperty::New("Interpolation feedback"));
  m_FeedbackNode->SetProperty("opacity", mitk::FloatProperty::New(0.8));
  m_FeedbackNode->SetProperty("helper object", mitk::BoolProperty::New(true));

  m_InterpolatedSurfaceNode = mitk::DataNode::New();
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(SURFACE_COLOR_RGB));
  m_InterpolatedSurfaceNode->SetProperty("name", mitk::StringProperty::New("Surface Interpolation feedback"));
  m_InterpolatedSurfaceNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
  m_InterpolatedSurfaceNode->SetProperty("line width", mitk::FloatProperty::New(4.0f));
  m_InterpolatedSurfaceNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_InterpolatedSurfaceNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_InterpolatedSurfaceNode->SetVisibility(false);

  QWidget::setContentsMargins(0, 0, 0, 0);
  if (QWidget::layout() != nullptr)
  {
    QWidget::layout()->setContentsMargins(0, 0, 0, 0);
  }


  // For running 3D Interpolation in background
  // create a QFuture and a QFutureWatcher

  connect(&m_Watcher, SIGNAL(started()), this, SLOT(StartUpdateInterpolationTimer()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnSurfaceInterpolationFinished()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(StopUpdateInterpolationTimer()));
  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(ChangeSurfaceColor()));
}

void QmitkSlicesInterpolator::SetDataStorage(mitk::DataStorage::Pointer storage)
{
  if (m_DataStorage == storage)
  {
    return;
  }

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkSlicesInterpolator, const mitk::DataNode*>(this, &QmitkSlicesInterpolator::NodeRemoved)
    );
  }

  m_DataStorage = storage;
  m_SurfaceInterpolator->SetDataStorage(storage);

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkSlicesInterpolator, const mitk::DataNode*>(this, &QmitkSlicesInterpolator::NodeRemoved)
    );
  }
}

void QmitkSlicesInterpolator::SetActiveLabelValue(mitk::MultiLabelSegmentation::LabelValueType labelValue)
{
  bool changedValue = labelValue != this->m_CurrentActiveLabelValue;

  this->m_CurrentActiveLabelValue = labelValue;

  if (changedValue) this->OnActiveLabelChanged(labelValue);
};


mitk::DataStorage *QmitkSlicesInterpolator::GetDataStorage()
{
  if (m_DataStorage.IsNotNull())
  {
    return m_DataStorage;
  }
  else
  {
    return nullptr;
  }
}

void QmitkSlicesInterpolator::InitializeWindow(QmitkRenderWindow* window)
{
  auto slicer = window->GetSliceNavigationController();

  if (slicer == nullptr)
  {
    MITK_WARN << "Tried setting up interpolation for a render window that does not have a slice navigation controller set";
    return;
  }

  // Has to be initialized
  m_LastSNC = slicer;

  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer deleteCommand =
    itk::MemberCommand<QmitkSlicesInterpolator>::New();
  deleteCommand->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted);
  m_ControllerToDeleteObserverTag[slicer] = slicer->AddObserver(itk::DeleteEvent(), deleteCommand);

  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer sliceChangedCommand =
    itk::MemberCommand<QmitkSlicesInterpolator>::New();
  sliceChangedCommand->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnSliceChanged);
  m_ControllerToSliceObserverTag[slicer] = slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0), sliceChangedCommand);
}

void QmitkSlicesInterpolator::Initialize(mitk::ToolManager *toolManager,
                                         const QList<QmitkRenderWindow*>& windows)
{
  Q_ASSERT(!windows.empty());

  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  m_ToolManager = toolManager;

  if (m_ToolManager)
  {
    // set enabled only if a segmentation is selected
    mitk::DataNode *node = m_ToolManager->GetWorkingData(0);
    QWidget::setEnabled(node != nullptr);

    // react whenever the set of selected segmentation changes
    m_ToolManager->WorkingDataChanged +=
      mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified);
    m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>(
      this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified);

    auto* timeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
    itk::MemberCommand<QmitkSlicesInterpolator>::Pointer timeChangedCommand =
      itk::MemberCommand<QmitkSlicesInterpolator>::New();
    timeChangedCommand->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnTimeChanged);
    m_ControllerToTimeObserverTag =
      timeNavigationController->AddObserver(mitk::TimeNavigationController::TimeEvent(0), timeChangedCommand);

    m_TimePoint = timeNavigationController->GetSelectedTimePoint();

    // connect to the slice navigation controller. after each change, call the interpolator
    for (auto* window : windows)
    {
      this->InitializeWindow(window);
    }

    m_ActionToSlicerMap = CreateActionToSlicer(windows);
  }

  m_Initialized = true;
}

void QmitkSlicesInterpolator::Uninitialize()
{
  if (m_ToolManager.IsNotNull())
  {
    m_ToolManager->WorkingDataChanged -=
      mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified);
    m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>(
      this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified);
  }

  auto* timeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  timeNavigationController->RemoveObserver(m_ControllerToTimeObserverTag);

for (auto* slicer : m_ControllerToSliceObserverTag.keys())
  {
    slicer->RemoveObserver(m_ControllerToDeleteObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToSliceObserverTag.take(slicer));
  }

  m_ActionToSlicerMap.clear();
  m_ToolManager = nullptr;

  m_Initialized = false;
}

QmitkSlicesInterpolator::~QmitkSlicesInterpolator()
{
  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  WaitForFutures();

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkSlicesInterpolator, const mitk::DataNode*>(this, &QmitkSlicesInterpolator::NodeRemoved)
    );

    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
      m_DataStorage->Remove(m_InterpolatedSurfaceNode);
  }

  // remove observer
  m_Interpolator->RemoveObserver(InterpolationAbortedObserverTag);
  m_Interpolator->RemoveObserver(InterpolationInfoChangedObserverTag);
  m_SurfaceInterpolator->RemoveObserver(SurfaceInterpolationInfoChangedObserverTag);

  m_SurfaceInterpolator->SetCurrentInterpolationSession(nullptr);

  delete m_Timer;
}

/**
External enableization...
*/
void QmitkSlicesInterpolator::setEnabled(bool enable)
{
  QWidget::setEnabled(enable);

  // Set the gui elements of the different interpolation modi enabled
  if (enable)
  {
    if (m_2DInterpolationEnabled)
    {
      this->Show2DInterpolationControls(true);
      m_Interpolator->Activate2DInterpolation(true);
    }
    else if (m_3DInterpolationEnabled)
    {
      this->Show3DInterpolationControls(true);
      this->Show3DInterpolationResult(true);
    }
  }
  // Set all gui elements of the interpolation disabled
  else
  {
    this->HideAllInterpolationControls();
    this->Show3DInterpolationResult(false);
  }
}

void QmitkSlicesInterpolator::On2DInterpolationEnabled(bool status)
{
  OnInterpolationActivated(status);
  m_Interpolator->Activate2DInterpolation(status);
}

void QmitkSlicesInterpolator::On3DInterpolationEnabled(bool status)
{
  On3DInterpolationActivated(status);
}

void QmitkSlicesInterpolator::OnInterpolationDisabled(bool status)
{
  if (status)
  {
    OnInterpolationActivated(!status);
    On3DInterpolationActivated(!status);
    this->Show3DInterpolationResult(false);
  }
}

void QmitkSlicesInterpolator::HideAllInterpolationControls()
{
  this->Show2DInterpolationControls(false);
  this->Show3DInterpolationControls(false);
}

void QmitkSlicesInterpolator::Show2DInterpolationControls(bool show)
{
  m_BtnApply2D->setVisible(show);
  m_BtnApplyForAllSlices2D->setVisible(show);
}

void QmitkSlicesInterpolator::Show3DInterpolationControls(bool show)
{
  m_BtnApply3D->setVisible(show);

  m_ChkShowPositionNodes->setVisible(show);
  m_BtnReinit3DInterpolation->setVisible(show);
}

void QmitkSlicesInterpolator::OnInterpolationMethodChanged(int index)
{
  switch (index)
  {
    case 0: // Disabled
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle("Interpolation");
      this->HideAllInterpolationControls();
      this->OnInterpolationActivated(false);
      this->On3DInterpolationActivated(false);
      this->Show3DInterpolationResult(false);
      m_Interpolator->Activate2DInterpolation(false);
      break;

    case 1: // 2D
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle("Interpolation (Enabled)");
      this->HideAllInterpolationControls();
      this->Show2DInterpolationControls(true);
      this->OnInterpolationActivated(true);
      this->On3DInterpolationActivated(false);
      this->Show3DInterpolationResult(false);
      m_Interpolator->Activate2DInterpolation(true);
      break;

    case 2: // 3D
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle("Interpolation (Enabled)");
      this->HideAllInterpolationControls();
      this->Show3DInterpolationControls(true);
      this->OnInterpolationActivated(false);
      this->On3DInterpolationActivated(true);
      m_Interpolator->Activate2DInterpolation(false);
      break;

    default:
      MITK_ERROR << "Unknown interpolation method!";
      m_CmbInterpolation->setCurrentIndex(0);
      break;
  }
}

void QmitkSlicesInterpolator::OnShowMarkers(bool state)
{
  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers =
    m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End();
       ++it)
  {
    it->Value()->SetProperty("helper object", mitk::BoolProperty::New(!state));
  }
}

void QmitkSlicesInterpolator::OnToolManagerWorkingDataModified()
{
  if (m_ToolManager->GetWorkingData(0) != nullptr)
  {
    m_Segmentation = dynamic_cast<mitk::MultiLabelSegmentation *>(m_ToolManager->GetWorkingData(0)->GetData());
    m_BtnReinit3DInterpolation->setEnabled(true);
  }
  else
  {
    // If no workingdata is set, remove the interpolation feedback
    this->GetDataStorage()->Remove(m_FeedbackNode);
    m_FeedbackNode->SetData(nullptr);
    this->GetDataStorage()->Remove(m_InterpolatedSurfaceNode);
    m_InterpolatedSurfaceNode->SetData(nullptr);
    m_BtnReinit3DInterpolation->setEnabled(false);
    m_CmbInterpolation->setCurrentIndex(0);
    return;
  }
  // Updating the current selected segmentation for the 3D interpolation
  this->SetCurrentContourListID();

  if (m_2DInterpolationEnabled)
  {
    OnInterpolationActivated(true); // re-initialize if needed
  }
}

void QmitkSlicesInterpolator::OnToolManagerReferenceDataModified()
{
}

void QmitkSlicesInterpolator::OnTimeChanged(itk::Object *sender, const itk::EventObject &e)
{
  if (!dynamic_cast<const mitk::TimeNavigationController::TimeEvent*>(&e))
  {
    return;
  }

  const auto* timeNavigationController = dynamic_cast<mitk::TimeNavigationController*>(sender);
  if (nullptr == timeNavigationController)
  {
    return;
  }

  bool timeChanged = m_TimePoint != timeNavigationController->GetSelectedTimePoint();
  m_TimePoint = timeNavigationController->GetSelectedTimePoint();

  if (m_Watcher.isRunning())
    m_Watcher.waitForFinished();

  if (timeChanged)
  {
    if (m_3DInterpolationEnabled)
    {
      m_InterpolatedSurfaceNode->SetData(nullptr);
    }
    m_SurfaceInterpolator->Modified();
  }

  if (nullptr == m_LastSNC)
  {
    return;
  }

  if (TranslateAndInterpolateChangedSlice(m_LastSNC->GetCreatedWorldGeometry()))
  {
    m_LastSNC->GetRenderer()->RequestUpdate();
  }
}

void QmitkSlicesInterpolator::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent*>(&e))
  {
    return;
  }

  auto sliceNavigationController = dynamic_cast<mitk::SliceNavigationController*>(sender);
  if (nullptr == sliceNavigationController)
  {
    return;
  }

  if(m_2DInterpolationEnabled)
  {
    this->On2DInterpolationEnabled(m_2DInterpolationEnabled);
  }  

  if (TranslateAndInterpolateChangedSlice(e, sliceNavigationController))
  {
    sliceNavigationController->GetRenderer()->RequestUpdate();
  }
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e,
  mitk::SliceNavigationController* sliceNavigationController)
{
  const mitk::SliceNavigationController::GeometrySliceEvent* event =
    dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent*>(&e);

  mitk::TimeGeometry* timeGeometry = event->GetTimeGeometry();
  m_LastSNC = sliceNavigationController;

  return this->TranslateAndInterpolateChangedSlice(timeGeometry);
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const mitk::TimeGeometry* timeGeometry)
{
  if (!m_2DInterpolationEnabled)
  {
    return false;
  }

  if (nullptr == timeGeometry)
  {
    return false;
  }

  if (!timeGeometry->IsValidTimePoint(m_TimePoint))
  {
    return false;
  }

  mitk::SlicedGeometry3D* slicedGeometry =
    dynamic_cast<mitk::SlicedGeometry3D*>(timeGeometry->GetGeometryForTimePoint(m_TimePoint).GetPointer());
  if (nullptr == slicedGeometry)
  {
    return false;
  }

  mitk::PlaneGeometry* plane = dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry->GetPlaneGeometry(m_LastSNC->GetStepper()->GetPos()));
  if (nullptr == plane)
  {
    return false;
  }

  this->Interpolate(plane);
  return true;
}

void QmitkSlicesInterpolator::Interpolate(mitk::PlaneGeometry *plane)
{
  if (nullptr == m_ToolManager)
  {
    return;
  }

  mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
  if (nullptr == node)
  {
    return;
  }

  m_Segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
  if (nullptr == m_Segmentation)
  {
    return;
  }

  if (!m_Segmentation->GetTimeGeometry()->IsValidTimePoint(m_TimePoint))
  {
    MITK_WARN << "Cannot interpolate WorkingImage. Passed time point is not within the time bounds of WorkingImage. "
                 "Time point: "
              << m_TimePoint;
    return;
  }

  const auto timeStep = m_Segmentation->GetTimeGeometry()->TimePointToTimeStep(m_TimePoint);

  int clickedSliceDimension = -1;
  int clickedSliceIndex = -1;

  // calculate real slice position, i.e. slice of the image
  mitk::SegTool2D::DetermineAffectedImageSlice(m_Segmentation->GetGroupImage(m_Segmentation->GetActiveLayer()), plane, clickedSliceDimension, clickedSliceIndex);

  mitk::Image::Pointer interpolation =
    m_Interpolator->Interpolate(clickedSliceDimension, clickedSliceIndex, plane, timeStep);
  m_FeedbackNode->SetData(interpolation);

  //  maybe just have a variable that stores the active label color.
  if (m_ToolManager)
  {
    auto* workingNode = m_ToolManager->GetWorkingData(0);
    if (workingNode != nullptr)
    {
      auto* activeLabel = dynamic_cast<mitk::MultiLabelSegmentation*>(workingNode->GetData())->GetActiveLabel();
      if (nullptr != activeLabel)
      {
        auto activeColor = activeLabel->GetColor();
        m_FeedbackNode->SetProperty("color", mitk::ColorProperty::New(activeColor));
      }
    }
  }

  m_LastSliceIndex = clickedSliceIndex;
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationFinished()
{
  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode && workingNode->GetData())
  {
    const auto segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(workingNode->GetData());

    if (segmentation == nullptr)
    {
      MITK_ERROR << "Run3DInterpolation triggered with no MultiLabelSegmentation as working data.";
      return;
    }
    mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult(segmentation, m_CurrentActiveLabelValue, segmentation->GetTimeGeometry()->TimePointToTimeStep(m_TimePoint));

    if (interpolatedSurface.IsNotNull())
    {
      m_BtnApply3D->setEnabled(true);;

      m_InterpolatedSurfaceNode->SetData(interpolatedSurface);
      this->Show3DInterpolationResult(true);

      if (!m_DataStorage->Exists(m_InterpolatedSurfaceNode))
      {
        m_DataStorage->Add(m_InterpolatedSurfaceNode);
      }
    }
    else
    {
      m_BtnApply3D->setEnabled(false);

      if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
      {
        this->Show3DInterpolationResult(false);
      }
    }
  }

  m_BtnReinit3DInterpolation->setEnabled(true);

  for (auto* slicer : m_ControllerToSliceObserverTag.keys())
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

void QmitkSlicesInterpolator::OnAcceptInterpolationClicked()
{
  auto* workingNode = m_ToolManager->GetWorkingData(0);
  auto* planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
  auto* interpolatedPreview = dynamic_cast<mitk::Image*>(m_FeedbackNode->GetData());
  if (nullptr == workingNode || nullptr == interpolatedPreview)
    return;

  auto* segmentationImage = dynamic_cast<mitk::MultiLabelSegmentation*>(workingNode->GetData());
  if (nullptr == segmentationImage)
    return;

  if (!segmentationImage->GetTimeGeometry()->IsValidTimePoint(m_TimePoint))
  {
    MITK_WARN << "Cannot accept interpolation. Time point selected by SliceNavigationController is not within the "
      "time bounds of segmentation. Time point: "
      << m_TimePoint;
    return;
  }

  if (!segmentationImage->ExistLabel(m_CurrentActiveLabelValue))
  {
    MITK_ERROR << "OnAcceptInterpolationClicked triggered with no valid label selected. Currently selected invalid label: " << m_CurrentActiveLabelValue;
    return;
  }

  const auto timeStep = segmentationImage->GetTimeGeometry()->TimePointToTimeStep(m_TimePoint);

  auto interpolatedSlice = mitk::SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, segmentationImage->GetGroupImage(segmentationImage->GetGroupIndexOfLabel(m_CurrentActiveLabelValue)), timeStep)->Clone();
  auto activeValue = segmentationImage->GetActiveLabel()->GetValue();
  mitk::TransferLabelContentAtTimeStep(
    interpolatedPreview,
    interpolatedSlice,
    segmentationImage->GetConstLabelsByValue(segmentationImage->GetLabelValuesByGroup(segmentationImage->GetActiveLayer())),
    timeStep,
    0,
    mitk::MultiLabelSegmentation::UNLABELED_VALUE,
    false,
    { {0, mitk::MultiLabelSegmentation::UNLABELED_VALUE}, {1, activeValue} }
  );

  mitk::SegTool2D::WriteBackSegmentationResult(workingNode, planeGeometry, interpolatedSlice, timeStep);
  m_FeedbackNode->SetData(nullptr);
}

void QmitkSlicesInterpolator::AcceptAllInterpolations(mitk::SliceNavigationController *slicer)
{
  /*
   * What exactly is done here:
   * 1. We create an empty diff image for the current segmentation
   * 2. All interpolated slices are written into the diff image
   * 3. Then the diffimage is applied to the original segmentation
   */
  if (m_Segmentation)
  {
    if (!m_Segmentation->ExistLabel(m_CurrentActiveLabelValue))
    {
      MITK_ERROR << "AcceptAllInterpolations triggered with no valid label selected. Currently selected invalid label: " << m_CurrentActiveLabelValue;
      return;
    }
    if (!m_Segmentation->GetTimeGeometry()->IsValidTimePoint(m_TimePoint))
    {
      MITK_WARN << "Cannot accept all interpolations. Time point selected by passed SliceNavigationController is not "
        "within the time bounds of segmentation. Time point: "
        << m_TimePoint;
      return;
    }

    mitk::Image::Pointer activeLabelImage;
    try
    {
      activeLabelImage = mitk::CreateLabelMask(m_Segmentation, m_CurrentActiveLabelValue);
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << e.what() << " | NO LABELSETIMAGE IN WORKING NODE\n";
    }
    m_Interpolator->SetSegmentationVolume(activeLabelImage);

    const auto relevantGroupImage = m_Segmentation->GetGroupImage(m_Segmentation->GetGroupIndexOfLabel(m_CurrentActiveLabelValue));
    const auto segmentation3D = mitk::SelectImageByTimePoint(relevantGroupImage, m_TimePoint);

    // Create an empty diff image for the undo operation
    auto diffImage = mitk::Image::New();
    diffImage->Initialize(segmentation3D);

    // Create scope for ImageWriteAccessor so that the accessor is destroyed right after use
    {
      mitk::ImageWriteAccessor accessor(diffImage);

      // Set all pixels to zero
      auto pixelType = mitk::MakeScalarPixelType<mitk::Tool::DefaultSegmentationDataType>();

      memset(accessor.GetData(), 0, pixelType.GetSize() * diffImage->GetDimension(0) * diffImage->GetDimension(1) * diffImage->GetDimension(2));
    }

    // Since we need to shift the plane it must be clone so that the original plane isn't altered
    auto slicedGeometry = m_Segmentation->GetSlicedGeometry();
    auto planeGeometry = slicer->GetCurrentPlaneGeometry()->Clone();
    int sliceDimension = -1;
    int sliceIndex = -1;

    mitk::SegTool2D::DetermineAffectedImageSlice(segmentation3D, planeGeometry, sliceDimension, sliceIndex);

    if (sliceIndex == -1 || sliceDimension == -1)
    {
      MITK_ERROR << "Unable to determine affected image slice. Render windows seems to be not initialized on image geometry.";
      return;
    }

    const auto numSlices = m_Segmentation->GetDimensions()[sliceDimension];
    mitk::ProgressBar::GetInstance()->AddStepsToDo(numSlices);

    std::atomic_uint totalChangedSlices;

    // Reuse interpolation algorithm instance for each slice to cache boundary calculations
    auto algorithm = mitk::ShapeBasedInterpolationAlgorithm::New();

    // Distribute slice interpolations to multiple threads
    const auto numThreads = std::min(std::thread::hardware_concurrency(), numSlices);
    std::vector<std::vector<unsigned int>> sliceIndices(numThreads);

    for (std::remove_const_t<decltype(numSlices)> sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
      sliceIndices[sliceIndex % numThreads].push_back(sliceIndex);

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    auto timeStep = m_Segmentation->GetTimeGeometry()->TimePointToTimeStep(m_TimePoint);

    // This lambda will be executed by the threads
    auto interpolate = [=, &interpolator = m_Interpolator, &totalChangedSlices](unsigned int threadIndex)
    {
      auto clonedPlaneGeometry = planeGeometry->Clone();
      auto origin = clonedPlaneGeometry->GetOrigin();

      //  Go through the sliced indices
      for (auto sliceIndex : sliceIndices[threadIndex])
      {
        slicedGeometry->WorldToIndex(origin, origin);
        origin[sliceDimension] = sliceIndex;
        slicedGeometry->IndexToWorld(origin, origin);
        clonedPlaneGeometry->SetOrigin(origin);

        auto interpolation = interpolator->Interpolate(sliceDimension, sliceIndex, clonedPlaneGeometry, timeStep, algorithm);

        if (interpolation.IsNotNull())
        {
          // Setting up the reslicing pipeline which allows us to write the interpolation results back into the image volume
          auto reslicer = vtkSmartPointer<mitkVtkImageOverwrite>::New();

          // Set overwrite mode to true to write back to the image volume
          reslicer->SetInputSlice(interpolation->GetSliceData()->GetVtkImageAccessor(interpolation)->GetVtkImageData());
          reslicer->SetOverwriteMode(true);
          reslicer->Modified();

          auto diffSliceWriter = mitk::ExtractSliceFilter::New(reslicer);

          diffSliceWriter->SetInput(diffImage);
          diffSliceWriter->SetTimeStep(0);
          diffSliceWriter->SetWorldGeometry(clonedPlaneGeometry);
          diffSliceWriter->SetVtkOutputRequest(true);
          diffSliceWriter->SetResliceTransformByGeometry(diffImage->GetTimeGeometry()->GetGeometryForTimeStep(0));
          diffSliceWriter->Modified();
          diffSliceWriter->Update();

          ++totalChangedSlices;
        }

        mitk::ProgressBar::GetInstance()->Progress();
      }
    };
    m_Interpolator->EnableSliceImageCache();

    //  Do the interpolation here.
    for (size_t threadIndex = 0; threadIndex < numThreads; ++threadIndex)
    {
      interpolate(threadIndex);
    }

    m_Interpolator->DisableSliceImageCache();

    //  Do and Undo Operations
    if (totalChangedSlices > 0)
    {
      /////////////////////////////////////////////////////////////
      //Commented out code that does not work due to changes in #536.
      //This break functionality, but code has to be completly to be rworked
      //anyways will be done directly after #536 in #81
      //choose to just defunc code for now to avoid making the code review
      //of #536 even more complex

      //// Create do/undo operations
      //auto* doOp = new mitk::ApplyDiffImageOperation(mitk::OpTEST, m_Segmentation, diffImage, timeStep);

      //auto* undoOp = new mitk::ApplyDiffImageOperation(mitk::OpTEST, m_Segmentation, diffImage, timeStep);
      //undoOp->SetFactor(-1.0);

      //auto comment = "Confirm all interpolations (" + std::to_string(totalChangedSlices) + ")";
      //auto* undoStackItem = new mitk::OperationEvent(mitk::DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, comment);

      //mitk::OperationEvent::IncCurrGroupEventId();
      //mitk::OperationEvent::IncCurrObjectEventId();
      //mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
      //const auto newDestinationLabel = dynamic_cast<mitk::MultiLabelSegmentation*>(m_Segmentation)->GetActiveLabel()->GetValue();
      //mitk::DiffImageApplier::GetInstanceForUndo()->SetDestinationLabel(newDestinationLabel);
      //// Apply the changes to the original image
      //mitk::DiffImageApplier::GetInstanceForUndo()->ExecuteOperation(doOp);

      /////////////////////////////////////////////////////////////
    }
    m_FeedbackNode->SetData(nullptr);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::FinishInterpolation(mitk::SliceNavigationController *slicer)
{
  // this redirect is for calling from outside
  if (slicer == nullptr)
    OnAcceptAllInterpolationsClicked();
  else
    AcceptAllInterpolations(slicer);
}

void QmitkSlicesInterpolator::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  for (auto it = m_ActionToSlicerMap.begin(); it != m_ActionToSlicerMap.end(); ++it)
  {
    orientationPopup.addAction(it->first);
  }

  connect(&orientationPopup, SIGNAL(triggered(QAction *)), this, SLOT(OnAcceptAllPopupActivated(QAction *)));
  orientationPopup.exec(QCursor::pos());
}

void QmitkSlicesInterpolator::OnAccept3DInterpolationClicked()
{
  auto referenceImage = GetData<mitk::Image>(m_ToolManager->GetReferenceData(0));

  auto* segmentationDataNode = m_ToolManager->GetWorkingData(0);

  auto segmentation = GetData<mitk::MultiLabelSegmentation>(segmentationDataNode);
  auto activeLabelColor = segmentation->GetActiveLabel()->GetColor();
  std::string activeLabelName = segmentation->GetActiveLabel()->GetName();

  if (referenceImage.IsNull() || segmentation.IsNull())
    return;

  const auto* segmentationGeometry = segmentation->GetTimeGeometry();

  if (!referenceImage->GetTimeGeometry()->IsValidTimePoint(m_TimePoint) ||
      !segmentationGeometry->IsValidTimePoint(m_TimePoint))
  {
    MITK_WARN << "Cannot accept interpolation. Current time point is not within the time bounds of the patient image and segmentation.";
    return;
  }

  auto interpolatedSurface = GetData<mitk::Surface>(m_InterpolatedSurfaceNode);

  if (interpolatedSurface.IsNull())
    return;

  auto surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

  surfaceToImageFilter->SetImage(referenceImage);
  surfaceToImageFilter->SetMakeOutputBinary(true);
  surfaceToImageFilter->SetUShortBinaryPixelType(true);
  surfaceToImageFilter->SetInput(interpolatedSurface);
  surfaceToImageFilter->Update();

  mitk::Image::Pointer interpolatedSegmentation = surfaceToImageFilter->GetOutput();
  auto timeStep = segmentationGeometry->TimePointToTimeStep(m_TimePoint);
  const mitk::Label::PixelType newDestinationLabel = segmentation->GetActiveLabel()->GetValue();

  TransferLabelContentAtTimeStep(
    interpolatedSegmentation,
    segmentation->GetGroupImage(segmentation->GetActiveLayer()),
    segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(segmentation->GetActiveLayer())),
    timeStep,
    0,
    0,
    false,
    {{1, newDestinationLabel}},
    mitk::MultiLabelSegmentation::MergeStyle::Merge,
    mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  this->Show3DInterpolationResult(false);

  std::string name = segmentationDataNode->GetName() + " 3D-interpolation - " + activeLabelName;
  mitk::TimeBounds timeBounds;

  if (1 < interpolatedSurface->GetTimeSteps())
  {
    name += "_t" + std::to_string(timeStep);

    auto* polyData = vtkPolyData::New();
    polyData->DeepCopy(interpolatedSurface->GetVtkPolyData(timeStep));

    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(polyData);

    interpolatedSurface = surface;
    timeBounds = segmentationGeometry->GetTimeBounds(timeStep);
  }
  else
  {
    timeBounds = segmentationGeometry->GetTimeBounds(0);
  }

  auto* surfaceGeometry = static_cast<mitk::ProportionalTimeGeometry*>(interpolatedSurface->GetTimeGeometry());
  surfaceGeometry->SetFirstTimePoint(timeBounds[0]);
  surfaceGeometry->SetStepDuration(timeBounds[1] - timeBounds[0]);

  // Typical file formats for surfaces do not save any time-related information. As a workaround at least for MITK scene files, we have the
  // possibility to serialize this information as properties.

  interpolatedSurface->SetProperty("ProportionalTimeGeometry.FirstTimePoint", mitk::FloatProperty::New(surfaceGeometry->GetFirstTimePoint()));
  interpolatedSurface->SetProperty("ProportionalTimeGeometry.StepDuration", mitk::FloatProperty::New(surfaceGeometry->GetStepDuration()));

  auto interpolatedSurfaceDataNode = mitk::DataNode::New();

  interpolatedSurfaceDataNode->SetData(interpolatedSurface);
  interpolatedSurfaceDataNode->SetName(name);
  interpolatedSurfaceDataNode->SetOpacity(0.7f);

  interpolatedSurfaceDataNode->SetColor(activeLabelColor);
  m_DataStorage->Add(interpolatedSurfaceDataNode, segmentationDataNode);
}

void QmitkSlicesInterpolator::OnReinit3DInterpolation()
{
  //  Step 1. Load from the isContourPlaneGeometry nodes the contourNodes.
  mitk::NodePredicateProperty::Pointer pred =
    mitk::NodePredicateProperty::New("isContourPlaneGeometry", mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer contourNodes =
    m_DataStorage->GetDerivations(m_ToolManager->GetWorkingData(0), pred);

  if (contourNodes->Size() != 0)
  {
    if (m_ToolManager->GetWorkingData(0) != nullptr)
    {
      try
      {
        auto labelSetImage = GetData<mitk::MultiLabelSegmentation>(m_ToolManager->GetWorkingData(0));
        if (!labelSetImage->GetTimeGeometry()->IsValidTimePoint(m_TimePoint))
        {
          MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
          return;
        }

        mitk::SurfaceInterpolationController::CPIVector newCPIs;
        //  Adding label and timeStep information for the contourNodes.
        for (auto it = contourNodes->Begin(); it != contourNodes->End(); ++it)
        {
          auto contourNode = it->Value();
          auto labelID = dynamic_cast<mitk::UShortProperty *>(contourNode->GetProperty("labelID"))->GetValue();
          auto timeStep = dynamic_cast<mitk::IntProperty *>(contourNode->GetProperty("timeStep"))->GetValue();

          auto planeGeometry = dynamic_cast<mitk::PlanarFigure *>(contourNode->GetData())->GetPlaneGeometry();
          auto groupID = labelSetImage->GetGroupIndexOfLabel(labelID);
          auto sliceImage = ExtractSliceFromImage(labelSetImage->GetGroupImage(groupID), planeGeometry, timeStep);
          mitk::ImageToContourFilter::Pointer contourExtractor = mitk::ImageToContourFilter::New();
          contourExtractor->SetInput(sliceImage);
          contourExtractor->SetContourValue(labelID);
          contourExtractor->Update();
          mitk::Surface::Pointer contour = contourExtractor->GetOutput();

          if (contour->GetVtkPolyData()->GetNumberOfPoints() == 0)
            continue;

          contour->DisconnectPipeline();
          newCPIs.emplace_back(contour, planeGeometry->Clone(),labelID,timeStep);
        }
        m_SurfaceInterpolator->CompleteReinitialization(newCPIs);
      }
      catch(const std::exception& e)
      {
        MITK_ERROR << "Exception thrown casting toolmanager working data to labelsetImage";
      }
    }
  }
  else
  {
    m_BtnApply3D->setEnabled(false);
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Reinitialize surface interpolation");
    errorInfo.setIcon(QMessageBox::Information);
    errorInfo.setText("No contours available for the selected segmentation!");
    errorInfo.exec();
  }
}

void QmitkSlicesInterpolator::OnAcceptAllPopupActivated(QAction *action)
{
  try
  {
    auto iter = m_ActionToSlicerMap.find(action);
    if (iter != m_ActionToSlicerMap.end())
    {
      mitk::SliceNavigationController *slicer = iter->second;
      this->AcceptAllInterpolations(slicer);
    }
  }
  catch (...)
  {
    /* Showing message box with possible memory error */
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Interpolation Process");
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText("An error occurred during interpolation. Possible cause: Not enough memory!");
    errorInfo.exec();

    std::cerr << "Ill construction in " __FILE__ " l. " << __LINE__ << std::endl;
  }
}

void QmitkSlicesInterpolator::OnInterpolationActivated(bool on)
{
  m_2DInterpolationEnabled = on;

  try
  {
    if (m_DataStorage.IsNotNull())
    {
      if (on && !m_DataStorage->Exists(m_FeedbackNode))
      {
        m_DataStorage->Add(m_FeedbackNode);
      }
    }
  }
  catch (...)
  {
    // don't care (double add/remove)
  }

  if (m_ToolManager)
  {
    mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
    mitk::DataNode *referenceNode = m_ToolManager->GetReferenceData(0);
    QWidget::setEnabled(workingNode != nullptr);

    m_BtnApply2D->setEnabled(on);
    m_FeedbackNode->SetVisibility(on);

    if (!on)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return;
    }

    if (workingNode)
    {
      auto labelSetImage = GetData<mitk::MultiLabelSegmentation>(workingNode);
      if (nullptr == labelSetImage)
      {
        MITK_ERROR << "NO LABELSETIMAGE IN WORKING NODE\n";
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        return;
      }

      const auto* activeLabel = labelSetImage->GetActiveLabel();
      if (nullptr != activeLabel)
      {
        auto activeLabelImage = mitk::CreateLabelMask(labelSetImage, activeLabel->GetValue());
        m_Interpolator->SetSegmentationVolume(activeLabelImage);

        if (referenceNode)
        {
          mitk::Image *referenceImage = dynamic_cast<mitk::Image *>(referenceNode->GetData());
          m_Interpolator->SetReferenceVolume(referenceImage); // may be nullptr
        }
      }
    }
  }
  this->UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::Run3DInterpolation()
{
  auto workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode == nullptr)
  {
    MITK_ERROR << "Run3DInterpolation triggered with no working data set.";
    return;
  }

  const auto segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(workingNode->GetData());

  if (segmentation == nullptr)
  {
    MITK_ERROR << "Run3DInterpolation triggered with no MultiLabelSegmentation as working data.";
    return;
  }

  if (!segmentation->ExistLabel(m_CurrentActiveLabelValue))
  {
    MITK_ERROR << "Run3DInterpolation triggered with no valid label selected. Currently selected invalid label: "<<m_CurrentActiveLabelValue;
    return;
  }

  m_SurfaceInterpolator->Interpolate(segmentation,m_CurrentActiveLabelValue,segmentation->GetTimeGeometry()->TimePointToTimeStep(m_TimePoint));
}

void QmitkSlicesInterpolator::StartUpdateInterpolationTimer()
{
  m_Timer->start(500);
}

void QmitkSlicesInterpolator::StopUpdateInterpolationTimer()
{
  if(m_ToolManager)
  {
    const auto* workingNode = m_ToolManager->GetWorkingData(0);
    const auto activeColor = dynamic_cast<mitk::MultiLabelSegmentation*>(workingNode->GetData())->GetActiveLabel()->GetColor();
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(activeColor));
  }

  m_Timer->stop();
}

void QmitkSlicesInterpolator::ChangeSurfaceColor()
{
  float currentColor[3];
  m_InterpolatedSurfaceNode->GetColor(currentColor);

    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(SURFACE_COLOR_RGB));
  m_InterpolatedSurfaceNode->Update();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSlicesInterpolator::On3DInterpolationActivated(bool on)
{
  m_3DInterpolationEnabled = on;
  try
  {
    // this->PrepareInputsFor3DInterpolation();
    m_SurfaceInterpolator->Modified();
  }
  catch (...)
  {
    MITK_ERROR << "Error with 3D surface interpolation!";
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::EnableInterpolation(bool on)
{
  // only to be called from the outside world
  // just a redirection to OnInterpolationActivated
  OnInterpolationActivated(on);
}

void QmitkSlicesInterpolator::Enable3DInterpolation(bool on)
{
  // only to be called from the outside world
  // just a redirection to OnInterpolationActivated
  this->On3DInterpolationActivated(on);
}

void QmitkSlicesInterpolator::UpdateVisibleSuggestion()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::OnInterpolationInfoChanged(const itk::EventObject & /*e*/)
{
  // something (e.g. undo) changed the interpolation info, we should refresh our display
  this->UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::OnInterpolationAborted(const itk::EventObject& /*e*/)
{
  m_CmbInterpolation->setCurrentIndex(0);
  m_FeedbackNode->SetData(nullptr);
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged(const itk::EventObject & /*e*/)
{
  auto workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode == nullptr)
  {
    MITK_DEBUG << "OnSurfaceInterpolationInfoChanged triggered with no working data set.";
    return;
  }

  const auto segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(workingNode->GetData());

  if (segmentation == nullptr)
  {
    MITK_DEBUG << "OnSurfaceInterpolationInfoChanged triggered with no MultiLabelSegmentation as working data.";
    return;
  }

  if (!segmentation->ExistLabel(m_CurrentActiveLabelValue))
  {
    MITK_DEBUG << "OnSurfaceInterpolationInfoChanged triggered with no valid label selected. Currently selected invalid label: " << m_CurrentActiveLabelValue;
    return;
  }

  if (m_Watcher.isRunning())
    m_Watcher.waitForFinished();

  if (m_3DInterpolationEnabled)
  {
    m_InterpolatedSurfaceNode->SetData(nullptr);
    m_Future = QtConcurrent::run(&QmitkSlicesInterpolator::Run3DInterpolation, this);
    m_Watcher.setFuture(m_Future);
  }
}

void QmitkSlicesInterpolator::SetCurrentContourListID()
{
  // New ContourList = hide current interpolation
  Show3DInterpolationResult(false);

  if (m_DataStorage.IsNotNull() && m_ToolManager && m_LastSNC)
  {
    mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);

    if (workingNode)
    {
      QWidget::setEnabled(true);

      if (!workingNode->GetData()->GetTimeGeometry()->IsValidTimePoint(m_TimePoint))
      {
        MITK_WARN << "Cannot accept interpolation. Time point selected by SliceNavigationController is not within the time bounds of WorkingImage. Time point: " << m_TimePoint;
        return;
      }

      m_SurfaceInterpolator->SetDistanceImageVolume(50000);

      auto segmentationImage = dynamic_cast<mitk::MultiLabelSegmentation *>(workingNode->GetData());
      m_SurfaceInterpolator->SetCurrentInterpolationSession(segmentationImage);
    }
    else
    {
      QWidget::setEnabled(false);
    }
  }
}

void QmitkSlicesInterpolator::Show3DInterpolationResult(bool status)
{
  if (m_InterpolatedSurfaceNode.IsNotNull())
    m_InterpolatedSurfaceNode->SetVisibility(status);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::OnActiveLabelChanged(mitk::Label::PixelType)
{
  m_FeedbackNode->SetData(nullptr);
  m_InterpolatedSurfaceNode->SetData(nullptr);

  if (m_Watcher.isRunning())
    m_Watcher.waitForFinished();

  if (m_3DInterpolationEnabled)
  {
    m_SurfaceInterpolator->Modified();
  }

  if (m_2DInterpolationEnabled)
  {
    m_FeedbackNode->SetData(nullptr);
    this->OnInterpolationActivated(true);

    m_LastSNC->SendSlice();
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::CheckSupportedImageDimension()
{
  if (m_ToolManager->GetWorkingData(0))
  {
    m_Segmentation = dynamic_cast<mitk::MultiLabelSegmentation *>(m_ToolManager->GetWorkingData(0)->GetData());

    if (m_3DInterpolationEnabled && m_Segmentation && ((m_Segmentation->GetDimension() != 3) || (m_Segmentation->GetDimension() != 4)) )
    {
      QMessageBox info;
      info.setWindowTitle("3D Interpolation Process");
      info.setIcon(QMessageBox::Information);
      info.setText("3D Interpolation is only supported for 3D/4D images at the moment!");
      info.exec();
      m_CmbInterpolation->setCurrentIndex(0);
    }
  }
}

void QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted(const itk::Object *sender,
                                                                 const itk::EventObject & /*e*/)
{
  // Don't know how to avoid const_cast here?!
  mitk::SliceNavigationController *slicer =
    dynamic_cast<mitk::SliceNavigationController *>(const_cast<itk::Object *>(sender));
  if (slicer)
  {
    m_ControllerToSliceObserverTag.remove(slicer);
    m_ControllerToDeleteObserverTag.remove(slicer);
  }
}

void QmitkSlicesInterpolator::WaitForFutures()
{
  if (m_Watcher.isRunning())
  {
    m_Watcher.waitForFinished();
  }

  if (m_PlaneWatcher.isRunning())
  {
    m_PlaneWatcher.waitForFinished();
  }
}

void QmitkSlicesInterpolator::NodeRemoved(const mitk::DataNode* node)
{
  if ((m_ToolManager && m_ToolManager->GetWorkingData(0) == node) ||
      node == m_FeedbackNode ||
      node == m_InterpolatedSurfaceNode)
  {
    WaitForFutures();
  }
}
