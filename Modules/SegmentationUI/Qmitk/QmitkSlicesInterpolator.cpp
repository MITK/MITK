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
#include "mitkToolManager.h"
#include "mitkUndoController.h"

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

const std::map<QAction *, mitk::SliceNavigationController *> QmitkSlicesInterpolator::createActionToSlicer(const QList<QmitkRenderWindow*>& windows)
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

// Check whether the given contours are coplanar
bool AreContoursCoplanar(mitk::SurfaceInterpolationController::ContourPositionInformation leftHandSide,
                      mitk::SurfaceInterpolationController::ContourPositionInformation rightHandSide)
{
  // Here we check two things:
  // 1. Whether the normals of both contours are at least parallel
  // 2. Whether both contours lie in the same plane

  // Check for coplanarity:
  // a. Span a vector between two points one from each contour
  // b. Calculate dot product for the vector and one of the normals
  // c. If the dot is zero the two vectors are orthogonal and the contours are coplanar

  double vec[3];
  vec[0] = leftHandSide.ContourPoint[0] - rightHandSide.ContourPoint[0];
  vec[1] = leftHandSide.ContourPoint[1] - rightHandSide.ContourPoint[1];
  vec[2] = leftHandSide.ContourPoint[2] - rightHandSide.ContourPoint[2];
  double n[3];
  n[0] = rightHandSide.ContourNormal[0];
  n[1] = rightHandSide.ContourNormal[1];
  n[2] = rightHandSide.ContourNormal[2];
  double dot = vtkMath::Dot(n, vec);

  double n2[3];
  n2[0] = leftHandSide.ContourNormal[0];
  n2[1] = leftHandSide.ContourNormal[1];
  n2[2] = leftHandSide.ContourNormal[2];

  // The normals of both contours have to be parallel but not of the same orientation
  double lengthLHS = leftHandSide.ContourNormal.GetNorm();
  double lengthRHS = rightHandSide.ContourNormal.GetNorm();
  double dot2 = vtkMath::Dot(n, n2);
  bool contoursParallel = mitk::Equal(fabs(lengthLHS * lengthRHS), fabs(dot2), 0.001);

  if (mitk::Equal(dot, 0.0, 0.001) && contoursParallel)
    return true;
  else
    return false;
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


template <unsigned int VImageDimension = 3>
std::vector<mitk::Label::PixelType> GetPixelValuesPresentInImage(mitk::LabelSetImage* labelSetImage)
{
  std::vector<mitk::Label::PixelType> pixelsPresent;
  mitk::ImagePixelReadAccessor<mitk::LabelSet::PixelType, VImageDimension> readAccessor(labelSetImage);

  std::size_t numberOfPixels = 1;
  for (size_t dim = 0; dim < VImageDimension; ++dim)
    numberOfPixels *= static_cast<std::size_t>(readAccessor.GetDimension(dim));

  auto src = readAccessor.GetData();
  for (std::size_t i = 0; i < numberOfPixels; ++i)
  {
    mitk::Label::PixelType pixelVal = *(src + i);
    if ( (std::find(pixelsPresent.begin(), pixelsPresent.end(), pixelVal) == pixelsPresent.end()) && (pixelVal != mitk::LabelSetImage::UnlabeledValue) )
      pixelsPresent.push_back(pixelVal);
  }
  return pixelsPresent;
}


template <unsigned int VImageDimension = 3>
ModifyLabelActionTrigerred ModifyLabelProcessing(mitk::LabelSetImage* labelSetImage,
                          mitk::SurfaceInterpolationController::Pointer surfaceInterpolator,
                          unsigned int timePoint)
{
  auto currentLayerID = labelSetImage->GetActiveLayer();
  auto numTimeSteps = labelSetImage->GetTimeSteps();

  ModifyLabelActionTrigerred actionTriggered = ModifyLabelActionTrigerred::Null;
  auto* currentContourList = surfaceInterpolator->GetContours(timePoint, currentLayerID);

  while (nullptr == currentContourList)
  {
    surfaceInterpolator->OnAddLayer();
    currentContourList = surfaceInterpolator->GetContours(timePoint, currentLayerID);
  }

  mitk::LabelSetImage::Pointer labelSetImage2 = labelSetImage->Clone();

  mitk::ImagePixelReadAccessor<mitk::LabelSet::PixelType, VImageDimension> readAccessor(labelSetImage2.GetPointer());

  for (auto& contour : *currentContourList)
  {
    mitk::Label::PixelType contourPixelValue;

    itk::Index<3> itkIndex;
    labelSetImage2->GetGeometry()->WorldToIndex(contour.ContourPoint, itkIndex);
    if (VImageDimension == 4)
    {
      itk::Index<VImageDimension> time3DIndex;
      for (size_t i = 0; i < itkIndex.size(); ++i)
        time3DIndex[i] = itkIndex[i];
      time3DIndex[3] = timePoint;
      contourPixelValue = readAccessor.GetPixelByIndexSafe(time3DIndex);
    }
    else if (VImageDimension == 3)
    {
      itk::Index<VImageDimension> geomIndex;
      for (size_t i = 0; i < itkIndex.size(); ++i)
        geomIndex[i] = itkIndex[i];
      contourPixelValue = readAccessor.GetPixelByIndexSafe(geomIndex);
    }

    if (contour.LabelValue != contourPixelValue)
    {
      if (contourPixelValue == 0)   //  Erase label
      {
        for (size_t t = 0; t < numTimeSteps; ++t)
          surfaceInterpolator->RemoveContours(contour.LabelValue, t, currentLayerID);
        actionTriggered = ModifyLabelActionTrigerred::Erase;
      }
      else
      {
        contour.LabelValue = contourPixelValue;
        actionTriggered = ModifyLabelActionTrigerred::Merge;
      }
    }
  }
  return actionTriggered;
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
    m_PreviousActiveLabelValue(0),
    m_CurrentActiveLabelValue(0),
    m_PreviousLayerIndex(0),
    m_CurrentLayerIndex(0),
    m_FirstRun(true)
{
  m_GroupBoxEnableExclusiveInterpolationMode = new QGroupBox("Interpolation", this);

  QVBoxLayout *vboxLayout = new QVBoxLayout(m_GroupBoxEnableExclusiveInterpolationMode);

  m_EdgeDetector = mitk::FeatureBasedEdgeDetectionFilter::New();
  m_PointScorer = mitk::PointCloudScoringFilter::New();

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

  // T28261
  // m_BtnSuggestPlane = new QPushButton("Suggest a plane", m_GroupBoxEnableExclusiveInterpolationMode);
  // vboxLayout->addWidget(m_BtnSuggestPlane);

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

  m_3DContourNode = mitk::DataNode::New();
  m_3DContourNode->SetProperty("color", mitk::ColorProperty::New(0.0, 0.0, 0.0));
  m_3DContourNode->SetProperty("hidden object", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty("name", mitk::StringProperty::New("Drawn Contours"));
  m_3DContourNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  m_3DContourNode->SetProperty("material.wireframeLineWidth", mitk::FloatProperty::New(2.0f));
  m_3DContourNode->SetProperty("3DContourContainer", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_3DContourNode->SetVisibility(false);

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
  m_TimePoints.insert(slicer, slicer->GetSelectedTimePoint());

  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer deleteCommand =
    itk::MemberCommand<QmitkSlicesInterpolator>::New();
  deleteCommand->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted);
  m_ControllerToDeleteObserverTag[slicer] = slicer->AddObserver(itk::DeleteEvent(), deleteCommand);

  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer timeChangedCommand =
    itk::MemberCommand<QmitkSlicesInterpolator>::New();
  timeChangedCommand->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnTimeChanged);
  m_ControllerToTimeObserverTag[slicer] = slicer->AddObserver(mitk::SliceNavigationController::TimeGeometryEvent(nullptr, 0), timeChangedCommand);

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

    // connect to the slice navigation controller. after each change, call the interpolator
    for (auto* window : windows)
    {
      this->InitializeWindow(window);
    }

    m_ActionToSlicer = createActionToSlicer(windows);
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
  for (auto* slicer : m_ControllerToTimeObserverTag.keys())
  {
    slicer->RemoveObserver(m_ControllerToDeleteObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToTimeObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToSliceObserverTag.take(slicer));
  }

  this->ClearSegmentationObservers();

  m_ActionToSlicer.clear();
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
    if (m_DataStorage->Exists(m_3DContourNode))
      m_DataStorage->Remove(m_3DContourNode);
    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
      m_DataStorage->Remove(m_InterpolatedSurfaceNode);
  }

  // remove observer
  m_Interpolator->RemoveObserver(InterpolationAbortedObserverTag);
  m_Interpolator->RemoveObserver(InterpolationInfoChangedObserverTag);
  m_SurfaceInterpolator->RemoveObserver(SurfaceInterpolationInfoChangedObserverTag);

  m_SurfaceInterpolator->UnsetSelectedImage();

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

  // T28261
  // m_BtnSuggestPlane->setVisible(show);

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
  this->ClearSegmentationObservers();

  if (m_ToolManager->GetWorkingData(0) != nullptr)
  {
    m_Segmentation = dynamic_cast<mitk::Image *>(m_ToolManager->GetWorkingData(0)->GetData());
    auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
    m_BtnReinit3DInterpolation->setEnabled(true);
    try {
      if (m_SegmentationObserverTags.find(labelSetImage) == m_SegmentationObserverTags.end())
      {
        auto command2 = itk::MemberCommand<QmitkSlicesInterpolator>::New();
        command2->SetCallbackFunction(this, &QmitkSlicesInterpolator::OnModifyLabelChanged);
        auto workingImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
        m_SegmentationObserverTags[workingImage] = workingImage->AddObserver(itk::ModifiedEvent(), command2);
      }
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << "Error casting node data to LabelSetImage\n";
    }
  }
  else
  {
    // If no workingdata is set, remove the interpolation feedback
    this->GetDataStorage()->Remove(m_FeedbackNode);
    m_FeedbackNode->SetData(nullptr);
    this->GetDataStorage()->Remove(m_3DContourNode);
    m_3DContourNode->SetData(nullptr);
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
  // Check if we really have a GeometryTimeEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent *>(&e))
    return;

  mitk::SliceNavigationController *slicer = dynamic_cast<mitk::SliceNavigationController *>(sender);
  Q_ASSERT(slicer);

  const auto timePoint = slicer->GetSelectedTimePoint();
  m_TimePoints[slicer] = timePoint;

  if (m_Watcher.isRunning())
    m_Watcher.waitForFinished();

  if (timePoint != m_SurfaceInterpolator->GetCurrentTimePoint())
  {
    m_SurfaceInterpolator->SetCurrentTimePoint(timePoint);
    if (m_3DInterpolationEnabled)
    {
      m_3DContourNode->SetData(nullptr);
      m_InterpolatedSurfaceNode->SetData(nullptr);
    }
    m_SurfaceInterpolator->Modified();
  }

  if (m_LastSNC == slicer)
  {
    slicer->SendSlice(); // will trigger a new interpolation
  }
}

void QmitkSlicesInterpolator::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  // Check whether we really have a GeometrySliceEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent *>(&e))
    return;

  mitk::SliceNavigationController *slicer = dynamic_cast<mitk::SliceNavigationController *>(sender);

  if(m_2DInterpolationEnabled)
  {
    this->On2DInterpolationEnabled(m_2DInterpolationEnabled);
  }
  if (TranslateAndInterpolateChangedSlice(e, slicer))
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject &e,
                                                                  mitk::SliceNavigationController *slicer)
{
  if (!m_2DInterpolationEnabled)
    return false;

  try
  {
    const mitk::SliceNavigationController::GeometrySliceEvent &event =
      dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent &>(e);

    mitk::TimeGeometry *tsg = event.GetTimeGeometry();
    if (tsg && m_TimePoints.contains(slicer) && tsg->IsValidTimePoint(m_TimePoints[slicer]))
    {
      mitk::SlicedGeometry3D *slicedGeometry =
        dynamic_cast<mitk::SlicedGeometry3D *>(tsg->GetGeometryForTimePoint(m_TimePoints[slicer]).GetPointer());

      if (slicedGeometry)
      {
        m_LastSNC = slicer;
        mitk::PlaneGeometry *plane =
          dynamic_cast<mitk::PlaneGeometry *>(slicedGeometry->GetPlaneGeometry(event.GetPos()));
        if (plane)
        {
          Interpolate(plane, m_TimePoints[slicer], slicer);
        }
        return true;
      }
    }
  }
  catch (const std::bad_cast &)
  {
    return false; // so what
  }

  return false;
}

void QmitkSlicesInterpolator::OnLayerChanged()
{
  auto* workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode != nullptr)
  {
    m_3DContourNode->SetData(nullptr);
    this->Show3DInterpolationResult(false);
  }

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

void QmitkSlicesInterpolator::Interpolate(mitk::PlaneGeometry *plane,
                                          mitk::TimePointType timePoint,
                                          mitk::SliceNavigationController *slicer)
{
  if (m_ToolManager)
  {
    mitk::DataNode *node = m_ToolManager->GetWorkingData(0);
    if (node)
    {
      m_Segmentation = dynamic_cast<mitk::Image *>(node->GetData());

      if (m_Segmentation)
      {
        if (!m_Segmentation->GetTimeGeometry()->IsValidTimePoint(timePoint))
        {
          MITK_WARN << "Cannot interpolate segmentation. Passed time point is not within the time bounds of WorkingImage. Time point: " << timePoint;
          return;
        }
        const auto timeStep = m_Segmentation->GetTimeGeometry()->TimePointToTimeStep(timePoint);

        int clickedSliceDimension = -1;
        int clickedSliceIndex = -1;

        // calculate real slice position, i.e. slice of the image
        mitk::SegTool2D::DetermineAffectedImageSlice(m_Segmentation, plane, clickedSliceDimension, clickedSliceIndex);

        mitk::Image::Pointer interpolation =
          m_Interpolator->Interpolate(clickedSliceDimension, clickedSliceIndex, plane, timeStep);
        m_FeedbackNode->SetData(interpolation);

        //  maybe just have a variable that stores the active label color.
        if (m_ToolManager)
        {
          auto* workingNode = m_ToolManager->GetWorkingData(0);
          if (workingNode != nullptr)
          {
            auto* activeLabel = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData())->GetActiveLabelSet()->GetActiveLabel();
            if (nullptr != activeLabel)
            {
              auto activeColor = activeLabel->GetColor();
              m_FeedbackNode->SetProperty("color", mitk::ColorProperty::New(activeColor));
            }
          }
        }

        m_LastSNC = slicer;
        m_LastSliceIndex = clickedSliceIndex;
      }
    }
  }
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationFinished()
{
  mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);

  mitk::PlaneGeometry::Pointer slicingPlane = mitk::PlaneGeometry::New();
  mitk::Vector3D slicingPlaneNormalVector;
  FillVector3D(slicingPlaneNormalVector,0.0,1.0,0.0);
  mitk::Point3D origin;
  FillVector3D(origin, 0.0, 0.0, 0.0);
  slicingPlane->InitializePlane(origin, slicingPlaneNormalVector);

  if (interpolatedSurface.IsNotNull() && workingNode)
  {
    m_BtnApply3D->setEnabled(true);

    // T28261
    // m_BtnSuggestPlane->setEnabled(true);

    m_InterpolatedSurfaceNode->SetData(interpolatedSurface);

    m_3DContourNode->SetData(m_SurfaceInterpolator->GetContoursAsSurface());

    this->Show3DInterpolationResult(true);

    if (!m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      m_DataStorage->Add(m_InterpolatedSurfaceNode);
    }
  }
  else if (interpolatedSurface.IsNull())
  {
    m_BtnApply3D->setEnabled(false);

    // T28261
    // m_BtnSuggestPlane->setEnabled(false);

    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      this->Show3DInterpolationResult(false);
    }
  }

  m_BtnReinit3DInterpolation->setEnabled(true);

  for (auto* slicer : m_ControllerToTimeObserverTag.keys())
  {
    slicer->GetRenderer()->RequestUpdate();
  }
  m_SurfaceInterpolator->ReinitializeInterpolation();
}

void QmitkSlicesInterpolator::OnAcceptInterpolationClicked()
{
  auto* workingNode = m_ToolManager->GetWorkingData(0);
  auto* planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
  auto* interpolatedPreview = dynamic_cast<mitk::Image*>(m_FeedbackNode->GetData());
  if (nullptr == workingNode || nullptr == interpolatedPreview)
    return;

  auto* segmentationImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == segmentationImage)
    return;

  const auto timePoint = m_LastSNC->GetSelectedTimePoint();
  if (!segmentationImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
  {
    MITK_WARN << "Cannot accept interpolation. Time point selected by SliceNavigationController is not within the time bounds of segmentation. Time point: " << timePoint;
    return;
  }
  const auto timeStep = segmentationImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  auto interpolatedSlice = mitk::SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, segmentationImage, timeStep)->Clone();
  auto labelSet = segmentationImage->GetActiveLabelSet();
  auto activeValue = labelSet->GetActiveLabel()->GetValue();
  mitk::TransferLabelContentAtTimeStep(
    interpolatedPreview,
    interpolatedSlice,
    labelSet,
    timeStep,
    0,
    mitk::LabelSetImage::UnlabeledValue,
    false,
    { {0, mitk::LabelSetImage::UnlabeledValue}, {1, activeValue} }
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
    mitk::Image::Pointer segmentation3D = m_Segmentation;
    unsigned int timeStep = 0;
    const auto timePoint = slicer->GetSelectedTimePoint();

    if (4 == m_Segmentation->GetDimension())
    {
      const auto* geometry = m_Segmentation->GetTimeGeometry();

      if (!geometry->IsValidTimePoint(timePoint))
      {
        MITK_WARN << "Cannot accept all interpolations. Time point selected by passed SliceNavigationController is not within the time bounds of segmentation. Time point: " << timePoint;
        return;
      }

      mitk::Image::Pointer activeLabelImage;
      try
      {
        auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_Segmentation);
        activeLabelImage = labelSetImage->CreateLabelMask(labelSetImage->GetActiveLabelSet()->GetActiveLabel()->GetValue(), true, 0);
      }
      catch (const std::exception& e)
      {
        MITK_ERROR << e.what() << " | NO LABELSETIMAGE IN WORKING NODE\n";
      }

      m_Interpolator->SetSegmentationVolume(activeLabelImage);

      timeStep = geometry->TimePointToTimeStep(timePoint);

      auto timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(m_Segmentation);
      timeSelector->SetTimeNr(timeStep);
      timeSelector->Update();

      segmentation3D = timeSelector->GetOutput();
    }

    // Create an empty diff image for the undo operation
    auto diffImage = mitk::Image::New();
    diffImage->Initialize(segmentation3D);

    // Create scope for ImageWriteAccessor so that the accessor is destroyed right after use
    {
      mitk::ImageWriteAccessor accessor(diffImage);

      // Set all pixels to zero
      auto pixelType = mitk::MakeScalarPixelType<mitk::Tool::DefaultSegmentationDataType>();

      // For legacy purpose support former pixel type of segmentations (before multilabel)
      if (itk::IOComponentEnum::UCHAR == m_Segmentation->GetImageDescriptor()->GetChannelDescriptor().GetPixelType().GetComponentType())
        pixelType = mitk::MakeScalarPixelType<unsigned char>();

      memset(accessor.GetData(), 0, pixelType.GetSize() * diffImage->GetDimension(0) * diffImage->GetDimension(1) * diffImage->GetDimension(2));
    }

    // Since we need to shift the plane it must be clone so that the original plane isn't altered
    auto slicedGeometry = m_Segmentation->GetSlicedGeometry();
    auto planeGeometry = slicer->GetCurrentPlaneGeometry()->Clone();
    int sliceDimension = -1;
    int sliceIndex = -1;

    mitk::SegTool2D::DetermineAffectedImageSlice(m_Segmentation, planeGeometry, sliceDimension, sliceIndex);

    const auto numSlices = m_Segmentation->GetDimension(sliceDimension);
    mitk::ProgressBar::GetInstance()->AddStepsToDo(numSlices);

    std::atomic_uint totalChangedSlices;

    // Reuse interpolation algorithm instance for each slice to cache boundary calculations
    auto algorithm = mitk::ShapeBasedInterpolationAlgorithm::New();

    // Distribute slice interpolations to multiple threads
    const auto numThreads = std::min(std::thread::hardware_concurrency(), numSlices);
    // const auto numThreads = 1;
    std::vector<std::vector<unsigned int>> sliceIndices(numThreads);

    for (std::remove_const_t<decltype(numSlices)> sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
      sliceIndices[sliceIndex % numThreads].push_back(sliceIndex);

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

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

    const mitk::Label::PixelType newDestinationLabel = dynamic_cast<mitk::LabelSetImage *>(m_Segmentation)->GetActiveLabelSet()->GetActiveLabel()->GetValue();

    //  Do and Undo Operations
    if (totalChangedSlices > 0)
    {
      // Create do/undo operations
      auto* doOp = new mitk::ApplyDiffImageOperation(mitk::OpTEST, m_Segmentation, diffImage, timeStep);

      auto* undoOp = new mitk::ApplyDiffImageOperation(mitk::OpTEST, m_Segmentation, diffImage, timeStep);
      undoOp->SetFactor(-1.0);

      auto comment = "Confirm all interpolations (" + std::to_string(totalChangedSlices) + ")";
      auto* undoStackItem = new mitk::OperationEvent(mitk::DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, comment);

      mitk::OperationEvent::IncCurrGroupEventId();
      mitk::OperationEvent::IncCurrObjectEventId();
      mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
      mitk::DiffImageApplier::GetInstanceForUndo()->SetDestinationLabel(newDestinationLabel);
      // Apply the changes to the original image
      mitk::DiffImageApplier::GetInstanceForUndo()->ExecuteOperation(doOp);
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
  for (auto it = m_ActionToSlicer.begin(); it != m_ActionToSlicer.end(); ++it)
    orientationPopup.addAction(it->first);

  connect(&orientationPopup, SIGNAL(triggered(QAction *)), this, SLOT(OnAcceptAllPopupActivated(QAction *)));
  orientationPopup.exec(QCursor::pos());
}

void QmitkSlicesInterpolator::OnAccept3DInterpolationClicked()
{
  auto referenceImage = GetData<mitk::Image>(m_ToolManager->GetReferenceData(0));

  auto* segmentationDataNode = m_ToolManager->GetWorkingData(0);

  auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(segmentationDataNode->GetData());
  auto activeLabelColor = labelSetImage->GetActiveLabelSet()->GetActiveLabel()->GetColor();
  std::string activeLabelName = labelSetImage->GetActiveLabelSet()->GetActiveLabel()->GetName();

  auto segmentation = GetData<mitk::Image>(segmentationDataNode);

  if (referenceImage.IsNull() || segmentation.IsNull())
    return;

  const auto* segmentationGeometry = segmentation->GetTimeGeometry();
  const auto timePoint = m_LastSNC->GetSelectedTimePoint();

  if (!referenceImage->GetTimeGeometry()->IsValidTimePoint(timePoint) ||
      !segmentationGeometry->IsValidTimePoint(timePoint))
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
  surfaceToImageFilter->SetUShortBinaryPixelType(itk::IOComponentEnum::USHORT == segmentation->GetPixelType().GetComponentType());
  surfaceToImageFilter->SetInput(interpolatedSurface);
  surfaceToImageFilter->Update();

  mitk::Image::Pointer interpolatedSegmentation = surfaceToImageFilter->GetOutput();
  auto timeStep = segmentationGeometry->TimePointToTimeStep(timePoint);
  const mitk::Label::PixelType newDestinationLabel = labelSetImage->GetActiveLabelSet()->GetActiveLabel()->GetValue();

  TransferLabelContentAtTimeStep(
    interpolatedSegmentation,
    labelSetImage,
    labelSetImage->GetActiveLabelSet(),
    timeStep,
    0,
    0,
    false,
    {{1, newDestinationLabel}},
    mitk::MultiLabelSegmentation::MergeStyle::Merge,
    mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  // m_CmbInterpolation->setCurrentIndex(0);
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
  // possibility to seralize this information as properties.

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
    std::vector<const mitk::PlaneGeometry*> contourPlanes;
    std::vector<mitk::Surface::Pointer> contourList;
    if (m_ToolManager->GetWorkingData(0) != nullptr)
    {
      try
      {
        auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
        auto activeLayerID = labelSetImage->GetActiveLayer();
        const auto timePoint = m_LastSNC->GetSelectedTimePoint();
        if (!labelSetImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
        {
          MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
          return;
        }

        //  Adding layer, label and timeStep information for the contourNodes.
        for (auto it = contourNodes->Begin(); it != contourNodes->End(); ++it)
        {
          auto contourNode = it->Value();
          auto layerID = dynamic_cast<mitk::UIntProperty *>(contourNode->GetProperty("layerID"))->GetValue();
          auto labelID = dynamic_cast<mitk::UShortProperty *>(contourNode->GetProperty("labelID"))->GetValue();
          auto timeStep = dynamic_cast<mitk::IntProperty *>(contourNode->GetProperty("timeStep"))->GetValue();

          auto px = dynamic_cast<mitk::DoubleProperty *>(contourNode->GetProperty("px"))->GetValue();
          auto py = dynamic_cast<mitk::DoubleProperty *>(contourNode->GetProperty("py"))->GetValue();
          auto pz = dynamic_cast<mitk::DoubleProperty *>(contourNode->GetProperty("pz"))->GetValue();

          // auto layerImage = labelSetImage->GetLayerImage(layerID);

          auto planeGeometry = dynamic_cast<mitk::PlanarFigure *>(contourNode->GetData())->GetPlaneGeometry();
          labelSetImage->SetActiveLayer(layerID);
          auto sliceImage = ExtractSliceFromImage(labelSetImage, planeGeometry, timeStep);
          labelSetImage->SetActiveLayer(activeLayerID);
          mitk::ImageToContourFilter::Pointer contourExtractor = mitk::ImageToContourFilter::New();
          contourExtractor->SetInput(sliceImage);
          contourExtractor->SetContourValue(labelID);
          contourExtractor->Update();
          mitk::Surface::Pointer contour = contourExtractor->GetOutput();

          if (contour->GetVtkPolyData()->GetNumberOfPoints() == 0)
            continue;

          vtkSmartPointer<vtkIntArray> intArray = vtkSmartPointer<vtkIntArray>::New();
          intArray->InsertNextValue(labelID);
          intArray->InsertNextValue(layerID);
          intArray->InsertNextValue(timeStep);
          contour->GetVtkPolyData()->GetFieldData()->AddArray(intArray);
          vtkSmartPointer<vtkDoubleArray> doubleArray = vtkSmartPointer<vtkDoubleArray>::New();
          doubleArray->InsertNextValue(px);
          doubleArray->InsertNextValue(py);
          doubleArray->InsertNextValue(pz);
          contour->GetVtkPolyData()->GetFieldData()->AddArray(doubleArray);
          contour->DisconnectPipeline();
          contourList.push_back(contour);
          contourPlanes.push_back(planeGeometry);
        }
        labelSetImage->SetActiveLayer(activeLayerID);
        // size_t activeLayer = labelSetImage->GetActiveLayer();
        for (size_t l = 0; l < labelSetImage->GetNumberOfLayers(); ++l)
        {
          this->OnAddLabelSetConnection(l);
        }
        // labelSetImage->SetActiveLayer(activeLayer);
        m_SurfaceInterpolator->CompleteReinitialization(contourList, contourPlanes);
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
    auto iter = m_ActionToSlicer.find(action);
    if (iter != m_ActionToSlicer.end())
    {
      mitk::SliceNavigationController *slicer = iter->second;
      AcceptAllInterpolations(slicer);
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

    // additional error message on std::cerr
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
      auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
      if (nullptr == labelSetImage)
      {
        MITK_ERROR << "NO LABELSETIMAGE IN WORKING NODE\n";
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        return;
      }

      auto* activeLabel = labelSetImage->GetActiveLabelSet()->GetActiveLabel();
      auto* segmentation = dynamic_cast<mitk::Image*>(workingNode->GetData());
      if (nullptr != activeLabel && nullptr != segmentation)
      {
        auto activeLabelImage = labelSetImage->CreateLabelMask(activeLabel->GetValue(), true, 0);
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
  m_SurfaceInterpolator->Interpolate();
}

void QmitkSlicesInterpolator::StartUpdateInterpolationTimer()
{
  m_Timer->start(500);
}

void QmitkSlicesInterpolator::StopUpdateInterpolationTimer()
{
  if(m_ToolManager)
  {
    auto* workingNode = m_ToolManager->GetWorkingData(0);
    auto activeColor = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData())->GetActiveLabelSet()->GetActiveLabel()->GetColor();
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(activeColor));
    m_3DContourNode->SetProperty("color", mitk::ColorProperty::New(activeColor));
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

void QmitkSlicesInterpolator::PrepareInputsFor3DInterpolation()
{
  if (m_DataStorage.IsNotNull() && m_ToolManager && m_3DInterpolationEnabled)
  {
    auto *workingNode = m_ToolManager->GetWorkingData(0);
    if (workingNode != nullptr)
    {
      int ret = QMessageBox::Yes;

      if (m_SurfaceInterpolator->EstimatePortionOfNeededMemory() > 0.5)
      {
        QMessageBox msgBox;
        msgBox.setText("Due to short handed system memory the 3D interpolation may be very slow!");
        msgBox.setInformativeText("Are you sure you want to activate the 3D interpolation?");
        msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
        ret = msgBox.exec();
      }

      auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
      auto activeLabel = labelSetImage->GetActiveLabelSet()->GetActiveLabel()->GetValue();

      m_SurfaceInterpolator->AddActiveLabelContoursForInterpolation(activeLabel);

      if (m_Watcher.isRunning())
        m_Watcher.waitForFinished();

      if (ret == QMessageBox::Yes)
      {
        //  Maybe set the segmentation node here
        m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
        m_Watcher.setFuture(m_Future);
      }
      else
      {
        m_CmbInterpolation->setCurrentIndex(0);
      }
    }
    else
    {
      QWidget::setEnabled(false);
      m_ChkShowPositionNodes->setEnabled(m_3DInterpolationEnabled);
    }

  }
  if (!m_3DInterpolationEnabled)
  {
    this->Show3DInterpolationResult(false);
    m_BtnApply3D->setEnabled(m_3DInterpolationEnabled);
    // T28261
    // m_BtnSuggestPlane->setEnabled(m_3DInterpolationEnabled);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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
  if (m_Watcher.isRunning())
    m_Watcher.waitForFinished();

  if (m_3DInterpolationEnabled)
  {
    m_3DContourNode->SetData(nullptr);
    m_InterpolatedSurfaceNode->SetData(nullptr);
    auto* workingNode = m_ToolManager->GetWorkingData(0);

    if (workingNode == nullptr)
      return;

    auto* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
    auto* label = labelSetImage->GetActiveLabelSet()->GetActiveLabel();

    if (label == nullptr)
      return;

    m_SurfaceInterpolator->AddActiveLabelContoursForInterpolation(label->GetValue());
    m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
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

    try{
        auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
        for (size_t layerID = 0; layerID < labelSetImage->GetNumberOfLayers(); ++layerID)
        {
          this->OnAddLabelSetConnection(layerID);
        }
    }
    catch (std::exception &e)
    {
      MITK_ERROR << e.what() << "\n";
    }

    if (workingNode)
    {
      QWidget::setEnabled(true);

      const auto timePoint = m_LastSNC->GetSelectedTimePoint();
      // In case the time is not valid use 0 to access the time geometry of the working node
      unsigned int time_position = 0;
      if (!workingNode->GetData()->GetTimeGeometry()->IsValidTimePoint(timePoint))
      {
        MITK_WARN << "Cannot accept interpolation. Time point selected by SliceNavigationController is not within the time bounds of WorkingImage. Time point: " << timePoint;
        return;
      }

      //  Sets up the surface interpolator to accept
      time_position = workingNode->GetData()->GetTimeGeometry()->TimePointToTimeStep(timePoint);

      mitk::Vector3D spacing = workingNode->GetData()->GetGeometry(time_position)->GetSpacing();
      double minSpacing = 100;
      double maxSpacing = 0;
      for (int i = 0; i < 3; i++)
      {
        if (spacing[i] < minSpacing)
        {
          minSpacing = spacing[i];
        }
        if (spacing[i] > maxSpacing)
        {
          maxSpacing = spacing[i];
        }
      }

      m_SurfaceInterpolator->SetMaxSpacing(maxSpacing);
      m_SurfaceInterpolator->SetMinSpacing(minSpacing);
      m_SurfaceInterpolator->SetDistanceImageVolume(50000);

      mitk::Image::Pointer segmentationImage;

      segmentationImage = dynamic_cast<mitk::Image *>(workingNode->GetData());
      m_SurfaceInterpolator->SetCurrentInterpolationSession(segmentationImage);
      m_SurfaceInterpolator->SetCurrentTimePoint(timePoint);
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

  if (m_3DContourNode.IsNotNull())
  {
    auto allRenderWindows = mitk::BaseRenderer::GetAll3DRenderWindows();
    for (auto mapit = allRenderWindows.begin(); mapit != allRenderWindows.end(); ++mapit)
    {
      m_3DContourNode->SetVisibility(status, mapit->second);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::OnActiveLabelChanged(mitk::Label::PixelType)
{
  m_3DContourNode->SetData(nullptr);
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
    m_Segmentation = dynamic_cast<mitk::Image *>(m_ToolManager->GetWorkingData(0)->GetData());

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
    m_ControllerToTimeObserverTag.remove(slicer);
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
      node == m_3DContourNode ||
      node == m_FeedbackNode ||
      node == m_InterpolatedSurfaceNode)
  {
    WaitForFutures();
  }
}

void QmitkSlicesInterpolator::OnAddLabelSetConnection(unsigned int layerID)
{
  if (m_ToolManager->GetWorkingData(0) != nullptr)
  {
    try
    {
      auto workingImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
      auto labelSet = workingImage->GetLabelSet(layerID);
      labelSet->RemoveLabelEvent += mitk::MessageDelegate1<QmitkSlicesInterpolator, mitk::Label::PixelType>(
        this, &QmitkSlicesInterpolator::OnRemoveLabel);
      labelSet->ActiveLabelEvent += mitk::MessageDelegate1<QmitkSlicesInterpolator,mitk::Label::PixelType>(
            this, &QmitkSlicesInterpolator::OnActiveLabelChanged);
      workingImage->AfterChangeLayerEvent += mitk::MessageDelegate<QmitkSlicesInterpolator>(
        this, &QmitkSlicesInterpolator::OnLayerChanged);
      m_SurfaceInterpolator->AddLabelSetConnection(layerID);
    }
    catch(const std::exception& e)
    {
      MITK_ERROR << e.what() << '\n';
    }
  }
}



void QmitkSlicesInterpolator::OnAddLabelSetConnection()
{
  if (m_ToolManager->GetWorkingData(0) != nullptr)
  {
    try
    {
      auto workingImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
      workingImage->GetActiveLabelSet()->RemoveLabelEvent += mitk::MessageDelegate1<QmitkSlicesInterpolator, mitk::Label::PixelType>(
        this, &QmitkSlicesInterpolator::OnRemoveLabel);
      workingImage->GetActiveLabelSet()->ActiveLabelEvent += mitk::MessageDelegate1<QmitkSlicesInterpolator,mitk::Label::PixelType>(
            this, &QmitkSlicesInterpolator::OnActiveLabelChanged);
      workingImage->AfterChangeLayerEvent += mitk::MessageDelegate<QmitkSlicesInterpolator>(
        this, &QmitkSlicesInterpolator::OnLayerChanged);
      m_SurfaceInterpolator->AddLabelSetConnection();
    }
    catch(const std::exception& e)
    {
      MITK_ERROR << e.what() << '\n';
    }
  }
}

void QmitkSlicesInterpolator::OnRemoveLabelSetConnection(mitk::LabelSetImage* labelSetImage, unsigned int layerID)
{
  size_t previousLayerID = labelSetImage->GetActiveLayer();
  labelSetImage->SetActiveLayer(layerID);
  labelSetImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate1<QmitkSlicesInterpolator, mitk::Label::PixelType>(
        this, &QmitkSlicesInterpolator::OnRemoveLabel);
  labelSetImage->GetActiveLabelSet()->ActiveLabelEvent -= mitk::MessageDelegate1<QmitkSlicesInterpolator,mitk::Label::PixelType>(
            this, &QmitkSlicesInterpolator::OnActiveLabelChanged);
  labelSetImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSlicesInterpolator>(
        this, &QmitkSlicesInterpolator::OnLayerChanged);
  m_SurfaceInterpolator->RemoveLabelSetConnection(labelSetImage, layerID);
  labelSetImage->SetActiveLayer(previousLayerID);
}

void QmitkSlicesInterpolator::OnRemoveLabelSetConnection()
{
  if (m_ToolManager->GetWorkingData(0) != nullptr)
  {
    try
    {
      auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_ToolManager->GetWorkingData(0)->GetData());
      workingImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate1<QmitkSlicesInterpolator, mitk::Label::PixelType>(
        this, &QmitkSlicesInterpolator::OnRemoveLabel);
      workingImage->GetActiveLabelSet()->ActiveLabelEvent -= mitk::MessageDelegate1<QmitkSlicesInterpolator,mitk::Label::PixelType>(
            this, &QmitkSlicesInterpolator::OnActiveLabelChanged);
      workingImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSlicesInterpolator>(
        this, &QmitkSlicesInterpolator::OnLayerChanged);
    }
    catch(const std::exception& e)
    {
      MITK_ERROR << e.what() << '\n';
    }
  }
}

void QmitkSlicesInterpolator::OnRemoveLabel(mitk::Label::PixelType /*removedLabelValue*/)
{
  if (m_ToolManager->GetWorkingData(0) != nullptr)
  {
    try
    {
      auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
      auto currentLayerID = labelSetImage->GetActiveLayer();
      auto numTimeSteps = labelSetImage->GetTimeGeometry()->CountTimeSteps();
      for (size_t t = 0; t < numTimeSteps; ++t)
      {
        m_SurfaceInterpolator->RemoveContours(m_PreviousActiveLabelValue,t,currentLayerID);
      }
    }
    catch(const std::exception& e)
    {
      MITK_ERROR << "Bad cast error for labelSetImage";
    }
  }
}

void QmitkSlicesInterpolator::OnModifyLabelChanged(const itk::Object *caller,
                                                      const itk::EventObject & /*event*/)
{
  auto *tempImage = dynamic_cast<mitk::LabelSetImage *>(const_cast<itk::Object *>(caller) ) ;
  if( tempImage == nullptr)
  {
    MITK_ERROR << "Unable to cast caller to LabelSetImage.";
    return;
  }

  ModifyLabelActionTrigerred actionTriggered = ModifyLabelActionTrigerred::Null;
  if(m_ToolManager->GetWorkingData(0) != nullptr)
  {
    auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());
    if (labelSetImage == tempImage)
    {
      const auto timePoint = m_LastSNC->GetSelectedTimePoint();
      if (!labelSetImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
      {
        MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
        return;
      }
      auto timeStep = labelSetImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

      auto numLayersInCurrentSegmentation = m_SurfaceInterpolator->GetNumberOfLayersInCurrentSegmentation();
      //  This handles the add layer or remove layer operation.
      if (labelSetImage->GetNumberOfLayers() != numLayersInCurrentSegmentation)
      {
        bool addLayer = (labelSetImage->GetNumberOfLayers() == (numLayersInCurrentSegmentation +1) );
        bool removeLayer = (labelSetImage->GetNumberOfLayers() == (numLayersInCurrentSegmentation - 1) );

        m_SurfaceInterpolator->SetNumberOfLayersInCurrentSegmentation(labelSetImage->GetNumberOfLayers());

        if (addLayer)
        {
          m_SurfaceInterpolator->OnAddLayer();
          this->OnAddLabelSetConnection();
        }
        if (removeLayer)
        {
          m_SurfaceInterpolator->OnRemoveLayer();
        }
        return;
      }

      //  Get the pixels present in the image.
      //  This portion of the code deals with the merge and erase labels operations.
      auto imageDimension = labelSetImage->GetDimension();
      if (imageDimension == 4)
      {
        actionTriggered = ModifyLabelProcessing<4>(labelSetImage, m_SurfaceInterpolator, timeStep);
      }
      else
      {
        actionTriggered = ModifyLabelProcessing<3>(labelSetImage, m_SurfaceInterpolator, timeStep);
      }

      if (actionTriggered == ModifyLabelActionTrigerred::Erase)
      {
        m_InterpolatedSurfaceNode->SetData(nullptr);
      }

      auto currentLayerID = labelSetImage->GetActiveLayer();
      if (actionTriggered == ModifyLabelActionTrigerred::Merge)
      {
        this->MergeContours(timeStep, currentLayerID);
        m_SurfaceInterpolator->Modified();
      }
    }
  }
}

void QmitkSlicesInterpolator::MergeContours(unsigned int timeStep,
                                            unsigned int layerID)
{
  auto* contours = m_SurfaceInterpolator->GetContours(timeStep, layerID);

  if (nullptr == contours)
    return;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  for (size_t i = 0; i < contours->size(); ++i)
  {
    for (size_t j = i+1; j < contours->size(); ++j)
    {
      //  And Labels are the same and Layers are the same.
      bool areContoursCoplanar = AreContoursCoplanar((*contours)[i], (*contours)[j]);

      if ( areContoursCoplanar  && ((*contours)[i].LabelValue == (*contours)[j].LabelValue) )
      {
        //  Update the contour by re-extracting the slice from the corresponding plane.
        mitk::Image::Pointer slice = ExtractSliceFromImage(m_Segmentation, (*contours)[i].Plane, timeStep);
        mitk::ImageToContourFilter::Pointer contourExtractor = mitk::ImageToContourFilter::New();
        contourExtractor->SetInput(slice);
        contourExtractor->SetContourValue((*contours)[i].LabelValue);
        contourExtractor->Update();
        mitk::Surface::Pointer contour = contourExtractor->GetOutput();
        (*contours)[i].Contour = contour;

        //  Update the interior point of the contour
        (*contours)[i].ContourPoint = m_SurfaceInterpolator->ComputeInteriorPointOfContour((*contours)[i],dynamic_cast<mitk::LabelSetImage *>(m_Segmentation));

        //  Setting the contour polygon data to an empty vtkPolyData,
        //  as source label is empty after merge operation.
        (*contours)[j].Contour->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New());
      }
    }
  }

  auto segmentationNode = m_SurfaceInterpolator->GetSegmentationImageNode();

  if (segmentationNode == nullptr)
  {
    MITK_ERROR << "segmentation Image Node not found\n";
  }

  auto isContourPlaneGeometry = mitk::NodePredicateProperty::New("isContourPlaneGeometry", mitk::BoolProperty::New(true));

  mitk::DataStorage::SetOfObjects::ConstPointer contourNodes =
    m_DataStorage->GetDerivations(segmentationNode, isContourPlaneGeometry);

  //  Remove empty contour nodes.
  auto isContourEmpty = [] (const mitk::SurfaceInterpolationController::ContourPositionInformation& contour)
  {
    return (contour.Contour->GetVtkPolyData()->GetNumberOfPoints() == 0);
  };

  auto it = std::remove_if((*contours).begin(), (*contours).end(), isContourEmpty);
  (*contours).erase(it, (*contours).end());
}

void QmitkSlicesInterpolator::ClearSegmentationObservers()
{
  auto dataIter = m_SegmentationObserverTags.begin();
  while (dataIter != m_SegmentationObserverTags.end())
  {
    auto labelSetImage = (*dataIter).first;
    labelSetImage->RemoveObserver((*dataIter).second);
    for (size_t layerID = 0; layerID < labelSetImage->GetNumberOfLayers(); ++layerID)
    {
      this->OnRemoveLabelSetConnection(labelSetImage, layerID);
    }
    ++dataIter;
  }
  m_SegmentationObserverTags.clear();
}
