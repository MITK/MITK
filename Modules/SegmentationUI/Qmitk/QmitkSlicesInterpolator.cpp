/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkSlicesInterpolator.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"

#include "mitkApplyDiffImageOperation.h"
#include "mitkColorProperty.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDiffImageApplier.h"
#include <mitkExtractSliceFilter.h>
#include <mitkImageTimeSelector.h>
#include "mitkInteractionConst.h"
#include "mitkLevelWindowProperty.h"
#include "mitkOperationEvent.h"
#include "mitkOverwriteSliceImageFilter.h"
#include <mitkPlaneProposer.h>
#include "mitkProgressBar.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkSegTool2D.h"
#include "mitkSliceNavigationController.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkToolManager.h"
#include "mitkUndoController.h"
#include <mitkUnstructuredGridClusteringFilter.h>
#include <mitkVtkImageOverwrite.h>
#include <mitkImageVtkAccessor.h>
#include <ThemeUtils.h>
#include <mitkSurfaceUtils.h>

#include <itkCommand.h>

#include <QCheckBox>
#include <QPushButton>
#include <QMenu>
#include <QCursor>
#include <QVBoxLayout>
#include <QMessageBox>

#include <vtkPolyVertex.h>
#include <vtkSMPTools.h>
#include <vtkUnstructuredGrid.h>


const QString QmitkSlicesInterpolator::TR_INTERPOLATION = QGroupBox::tr("Interpolation");
const QString QmitkSlicesInterpolator::TR_DISABLED = QGroupBox::tr("Disabled");
const QString QmitkSlicesInterpolator::TR_2D = QGroupBox::tr("2-Dimensional");
const QString QmitkSlicesInterpolator::TR_3D = QGroupBox::tr("3-Dimensional");
const QString QmitkSlicesInterpolator::TR_3D_MODIFICATION = QGroupBox::tr("3-Dimensional Modification");
const QString QmitkSlicesInterpolator::TR_CONFIRM_SINGLE = QPushButton::tr("Confirm for single slice");
const QString QmitkSlicesInterpolator::TR_CONFIRM_ALL = QPushButton::tr("Confirm for all slices");
const QString QmitkSlicesInterpolator::TR_CONFIRM = QPushButton::tr("Confirm");
const QString QmitkSlicesInterpolator::TR_SHOW_POSITION_NODE = QCheckBox::tr("Show Position Nodes");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_ENB = QGroupBox::tr("Interpolation (Enabled)");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_PROC = QMessageBox::tr("Interpolation Process");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_ERROR = QMessageBox::tr("An error occurred during interpolation. Possible cause: Not enough memory!");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_SLOW = QMessageBox::tr("Due to short handed system memory the 3D interpolation may be very slow!");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_SURE = QMessageBox::tr("Are you sure you want to activate the 3D interpolation?");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_3D_PROC = QMessageBox::tr("3D Interpolation Process");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_3D_SUPPORT = QMessageBox::tr("3D Interpolation is only supported for 3D images at the moment!");
const QString QmitkSlicesInterpolator::TR_INTERPOLATION_REINIT = QMessageBox::tr("Reinit Interpolation");

//#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

float SURFACE_COLOR_RGB [3] = {0.49f, 1.0f, 0.16f};

const std::map<QAction*, mitk::SliceNavigationController*> QmitkSlicesInterpolator::createActionToSliceDimension()
{
  std::map<QAction*, mitk::SliceNavigationController*> actionToSliceDimension;
  foreach(mitk::SliceNavigationController* slicer, m_ControllerToDeleteObserverTag.keys())
  {
    actionToSliceDimension[new QAction(qApp->translate("QmitkStdMultiWidget", slicer->GetViewDirectionAsString()),0)] = slicer;
  }

  return actionToSliceDimension;
}


QmitkSlicesInterpolator::QmitkSlicesInterpolator(QWidget* parent, const char*  /*name*/)
  :QWidget(parent),
//    ACTION_TO_SLICEDIMENSION( createActionToSliceDimension() ),
    m_Interpolator( mitk::SegmentationInterpolationController::New() ),
    m_SurfaceInterpolator(mitk::SurfaceInterpolationController::GetInstance()),
    m_ToolManager(NULL),
    m_Initialized(false),
    m_LastSNC(0),
    m_2DInterpolationEnabled(false),
    m_3DInterpolationEnabled(false),
    m_3DModificationEnabled(false),
    m_FirstRun(true),
    m_Activate3DInterpolation(false)
{
  m_GroupBoxEnableExclusiveInterpolationMode = new QGroupBox(TR_INTERPOLATION, this);



  QVBoxLayout* vboxLayout = new QVBoxLayout(m_GroupBoxEnableExclusiveInterpolationMode);

  m_EdgeDetector = mitk::FeatureBasedEdgeDetectionFilter::New();
  m_PointScorer = mitk::PointCloudScoringFilter::New();

  m_CmbInterpolation = new QComboBox(m_GroupBoxEnableExclusiveInterpolationMode);

  m_CmbInterpolation->addItem(TR_DISABLED);
  m_CmbInterpolation->addItem(TR_2D);
  m_CmbInterpolation->addItem(TR_3D);
  m_CmbInterpolation->addItem(TR_3D_MODIFICATION);

  vboxLayout->addWidget(m_CmbInterpolation);

  m_BtnApply2D = new QPushButton(TR_CONFIRM_SINGLE, m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApply2D);

  m_BtnApplyForAllSlices2D = new QPushButton(TR_CONFIRM_ALL, m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApplyForAllSlices2D);

  m_BtnApply3D = new QPushButton(TR_CONFIRM, m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApply3D);

  QHBoxLayout* mod3DLayout = new QHBoxLayout();
  vboxLayout->addLayout(mod3DLayout);

  m_BtnUnion = new QPushButton("", m_GroupBoxEnableExclusiveInterpolationMode);
  m_BtnUnion->setToolTip(tr("Union"));
  m_BtnUnion->setIcon(mitk::themeUtils::colorizeIcon(QIcon(":/SegmentationUI/union.png"), qApp->palette()));
  connect(m_BtnUnion, &QAbstractButton::pressed, this, &QmitkSlicesInterpolator::tempSegmentationUnion);
  mod3DLayout->addWidget(m_BtnUnion);

  m_BtnSubtraction = new QPushButton("", m_GroupBoxEnableExclusiveInterpolationMode);
  m_BtnSubtraction->setToolTip(tr("Subtraction"));
  m_BtnSubtraction->setIcon(mitk::themeUtils::colorizeIcon(QIcon(":/SegmentationUI/subtract.png"), qApp->palette()));
  connect(m_BtnSubtraction, &QAbstractButton::pressed, this, &QmitkSlicesInterpolator::tempSegmentationSubtraction);
  mod3DLayout->addWidget(m_BtnSubtraction);

  m_BtnIntersection = new QPushButton("", m_GroupBoxEnableExclusiveInterpolationMode);
  m_BtnIntersection->setToolTip(tr("Intersection"));
  m_BtnIntersection->setIcon(mitk::themeUtils::colorizeIcon(QIcon(":/SegmentationUI/intersection.png"), qApp->palette()));
  connect(m_BtnIntersection, &QAbstractButton::pressed, this, &QmitkSlicesInterpolator::tempSegmentationIntersection);
  mod3DLayout->addWidget(m_BtnIntersection);
  
  m_BtnDifference = new QPushButton("", m_GroupBoxEnableExclusiveInterpolationMode);
  m_BtnDifference->setToolTip(tr("Difference"));
  m_BtnDifference->setIcon(mitk::themeUtils::colorizeIcon(QIcon(":/SegmentationUI/difference.png"), qApp->palette()));
  connect(m_BtnDifference, &QAbstractButton::pressed, this, &QmitkSlicesInterpolator::tempSegmentationDifference);
  mod3DLayout->addWidget(m_BtnDifference);

  m_TempSegClear = new QPushButton(tr("Clear"), m_GroupBoxEnableExclusiveInterpolationMode);
  connect(m_TempSegClear, &QAbstractButton::pressed, this, &QmitkSlicesInterpolator::tempSegmentationClear);
  vboxLayout->addWidget(m_TempSegClear);

  m_BtnSuggestPlane = new QPushButton(tr("Suggest a plane"), m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnSuggestPlane);

  m_BtnReinit3DInterpolation = new QPushButton(TR_INTERPOLATION_REINIT, m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnReinit3DInterpolation);

  m_ChkShowPositionNodes = new QCheckBox(TR_SHOW_POSITION_NODE, m_GroupBoxEnableExclusiveInterpolationMode);

  vboxLayout->addWidget(m_ChkShowPositionNodes);


  this->HideAllInterpolationControls();

  connect(m_CmbInterpolation, SIGNAL(currentIndexChanged(int)), this, SLOT(OnInterpolationMethodChanged(int)));
  connect(m_BtnApply2D, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_BtnApplyForAllSlices2D, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()));
  connect(m_BtnApply3D, SIGNAL(clicked()), this, SLOT(OnAccept3DInterpolationClicked()));

  connect(m_BtnSuggestPlane, SIGNAL(clicked()), this, SLOT(OnSuggestPlaneClicked()));

  connect(m_BtnReinit3DInterpolation, SIGNAL(clicked()), this, SLOT(OnReinit3DInterpolation()));
  connect(m_ChkShowPositionNodes, SIGNAL(toggled(bool)), this, SLOT(OnShowMarkers(bool)));
  connect(m_ChkShowPositionNodes, SIGNAL(toggled(bool)), this, SIGNAL(SignalShowMarkerNodes(bool)));

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->addWidget(m_GroupBoxEnableExclusiveInterpolationMode);
  this->setLayout(layout);

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnInterpolationInfoChanged );
  InterpolationInfoChangedObserverTag = m_Interpolator->AddObserver( itk::ModifiedEvent(), command );

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command2->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged );
  SurfaceInterpolationInfoChangedObserverTag = m_SurfaceInterpolator->AddObserver( itk::ModifiedEvent(), command2 );

  // feedback node and its visualization properties
  m_FeedbackNode = mitk::DataNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( m_FeedbackNode );

  m_FeedbackNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
  m_FeedbackNode->SetProperty( "outline binary", mitk::BoolProperty::New(true) );
  m_FeedbackNode->SetProperty( "color", mitk::ColorProperty::New(255.0, 255.0, 0.0) );
  m_FeedbackNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(false) );
  m_FeedbackNode->SetProperty( "layer", mitk::IntProperty::New( 20 ) );
  m_FeedbackNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  m_FeedbackNode->SetProperty( "name", mitk::StringProperty::New("Interpolation feedback") );
  m_FeedbackNode->SetProperty( "opacity", mitk::FloatProperty::New(0.8) );
  m_FeedbackNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );

  m_InterpolatedSurfaceNode = mitk::DataNode::New();
  m_InterpolatedSurfaceNode->SetProperty( "color", mitk::ColorProperty::New(SURFACE_COLOR_RGB) );
  m_InterpolatedSurfaceNode->SetProperty( "name", mitk::StringProperty::New("Surface Interpolation feedback") );
  m_InterpolatedSurfaceNode->SetProperty( "opacity", mitk::FloatProperty::New(0.5) );
  m_InterpolatedSurfaceNode->SetProperty( "line width", mitk::FloatProperty::New(4.0f) );
  m_InterpolatedSurfaceNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
  m_InterpolatedSurfaceNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
  m_InterpolatedSurfaceNode->SetVisibility(false);

  m_3DContourNode = mitk::DataNode::New();
  m_3DContourNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 0.0, 0.0) );
  m_3DContourNode->SetProperty("hidden object", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty( "name", mitk::StringProperty::New("Drawn Contours") );
  m_3DContourNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  m_3DContourNode->SetProperty("material.wireframeLineWidth", mitk::FloatProperty::New(2.0f));
  m_3DContourNode->SetProperty("3DContourContainer", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

  QWidget::setContentsMargins(0, 0, 0, 0);
  if ( QWidget::layout() != NULL )
  {
    QWidget::layout()->setContentsMargins(0, 0, 0, 0);
  }

  //For running 3D Interpolation in background
  // create a QFuture and a QFutureWatcher

  connect(&m_Watcher, SIGNAL(started()), this, SLOT(StartUpdateInterpolationTimer()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnSurfaceInterpolationFinished()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(StopUpdateInterpolationTimer()));
  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(ChangeSurfaceColor()));

  m_TempSegmentation = mitk::Image::New();
  m_TempSegmentationNode = mitk::DataNode::New();
  m_TempSegmentationNode->SetData(m_TempSegmentation);
  m_TempSegmentationNode->SetBoolProperty("binary", true);
  m_TempSegmentationNode->SetBoolProperty("visible", true);
  m_TempSegmentationNode->SetBoolProperty("helper object", true);
  m_TempSegmentationNode->SetColor(1.f, 1.f, 0.f);
}

void QmitkSlicesInterpolator::tempSegmentationClear()
{
  size_t segSize = 1;
  for (unsigned int dim = 0; dim < m_TempSegmentation->GetDimension(); ++dim) {
    segSize *= m_TempSegmentation->GetDimension(dim);
  }

  {
    mitk::ImageRegionAccessor accessor(m_TempSegmentation);
    mitk::ImageAccessLock lock(&accessor, true);
    memset(accessor.getData(), 0, segSize * m_Segmentation->GetPixelType(0).GetSize());
  }

  m_TempSegmentation->Modified();

  m_SurfaceInterpolator->RemoveInterpolationSession(m_TempSegmentation);
  m_DataStorage->Remove(m_DataStorage->GetDerivations(m_TempSegmentationNode, nullptr, false));

  SetCurrentContourListID();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

template<typename T, T(*OP)(T, T)>
struct vtkSMPOperation {
  T* a;
  T* b;
  void operator()(vtkIdType begin, vtkIdType end) {
    for (int i = begin; i < end; i++) {
      a[i] = OP(a[i], b[i]);
    }
  }
};

template<typename T, T(*OP)(T,T)>
void applyTempSegmentationOperation(mitk::Surface::Pointer m_InterpolatedSurface, mitk::Image::Pointer m_OrigSegmentation, mitk::Image::Pointer refImage)
{
  if (m_InterpolatedSurface == nullptr) {
    return;
  }

  mitk::SurfaceToImageFilter::Pointer s2iFilter = mitk::SurfaceToImageFilter::New();
  s2iFilter->MakeOutputBinaryOn();
  if (m_OrigSegmentation->GetPixelType().GetComponentType() == itk::ImageIOBase::USHORT) {
    s2iFilter->SetUShortBinaryPixelType(true);
  }
  s2iFilter->SetInput(m_InterpolatedSurface);
  s2iFilter->SetImage(refImage);
  s2iFilter->Update();

  mitk::Image::Pointer interpolatedSeg = s2iFilter->GetOutput();

  mitk::ImageRegionAccessor tempAccessor(interpolatedSeg);
  mitk::ImageAccessLock tempLock(&tempAccessor, false);

  mitk::ImageRegionAccessor accessor(m_OrigSegmentation);
  mitk::ImageAccessLock lock(&accessor, true);
  
  size_t segSize = 1;
  for (unsigned int dim = 0; dim < m_OrigSegmentation->GetDimension(); ++dim) {
    segSize *= m_OrigSegmentation->GetDimension(dim);
  }

  T* segData = (T*)accessor.getData();
  T* tempSegData = (T*)tempAccessor.getData();
  vtkSMPOperation<T, OP> vtkParalelOp;
  vtkParalelOp.a = segData;
  vtkParalelOp.b = tempSegData;
  vtkSMPTools::For(0, segSize, vtkParalelOp);
  //for (size_t i = 0; i < segSize; i++) {
  //  segData[i] = OP(segData[i], tempSegData[i]);
  //}
  m_OrigSegmentation->Modified();
}

template<typename T>
T fUnion(T a, T b) { return a | b; }
void QmitkSlicesInterpolator::tempSegmentationUnion()
{
  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData());
  mitk::Image* ref = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData()[0]->GetData());
  int pixelSize = m_OrigSegmentation->GetPixelType().GetSize();
  if (pixelSize == 1) {
    applyTempSegmentationOperation<uchar, fUnion<uchar>>(surf, m_OrigSegmentation, ref);
  } else if (pixelSize == 2) {
    applyTempSegmentationOperation<ushort, fUnion<ushort>>(surf, m_OrigSegmentation, ref);
  }
  tempSegmentationClear();
  rebuildOrigSegmentationSurface();
}

template<typename T>
T fSubtract(T a, T b) { return a & (b == 0); }
void QmitkSlicesInterpolator::tempSegmentationSubtraction()
{
  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData());
  mitk::Image* ref = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData()[0]->GetData());
  int pixelSize = m_OrigSegmentation->GetPixelType().GetSize();
  if (pixelSize == 1) {
    applyTempSegmentationOperation<uchar, fSubtract<uchar>>(surf, m_OrigSegmentation, ref);
  } else if (pixelSize == 2) {
    applyTempSegmentationOperation<ushort, fSubtract<ushort>>(surf, m_OrigSegmentation, ref);
  }
  tempSegmentationClear();
  rebuildOrigSegmentationSurface();
}

template<typename T>
T fIntersect(T a, T b) { return (a > 0) && (b > 0); }
void QmitkSlicesInterpolator::tempSegmentationIntersection()
{
  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData());
  mitk::Image* ref = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData()[0]->GetData());
  int pixelSize = m_OrigSegmentation->GetPixelType().GetSize();
  if (pixelSize == 1) {
    applyTempSegmentationOperation<uchar, fIntersect<uchar>>(surf, m_OrigSegmentation, ref);
  } else if (pixelSize == 2) {
    applyTempSegmentationOperation<ushort, fIntersect<ushort>>(surf, m_OrigSegmentation, ref);
  }
  tempSegmentationClear();
  rebuildOrigSegmentationSurface();
}

template<typename T>
T fDifference(T a, T b) { return a != b; }
void QmitkSlicesInterpolator::tempSegmentationDifference()
{
  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData());
  mitk::Image* ref = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData()[0]->GetData());
  int pixelSize = m_OrigSegmentation->GetPixelType().GetSize();
  if (pixelSize == 1) {
    applyTempSegmentationOperation<uchar, fDifference<uchar>>(surf, m_OrigSegmentation, ref);
  } else if (pixelSize == 2) {
    applyTempSegmentationOperation<ushort, fDifference<ushort>>(surf, m_OrigSegmentation, ref);
  }
  tempSegmentationClear();
  rebuildOrigSegmentationSurface();
}

void QmitkSlicesInterpolator::rebuildOrigSegmentationSurface()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData()[0];
  auto childs = m_DataStorage->GetDerivations(workingNode);
  bool hasSurface = false;
  for (const auto& children : *childs) {
    mitk::Surface* surf = dynamic_cast<mitk::Surface*>(children->GetData());
    if (surf != nullptr) {
      hasSurface = true;
      break;
    }
  }
  if (!hasSurface) {
    return;
  }

  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(100);

  mitk::SurfaceCreator::Pointer surfaceCreator = mitk::SurfaceCreator::New();
  mitk::SurfaceCreator::SurfaceCreationArgs surfaceArgs;
  surfaceArgs.outputStorage = m_ToolManager->GetDataStorage();
  surfaceArgs.recreate = true;
  surfaceArgs.timestep = m_LastSNC->GetTime()->GetPos();
  surfaceCreator->setArgs(surfaceArgs);
  surfaceCreator->setInput(m_ToolManager->GetWorkingData()[0]);

  CommandProgressUpdate::Pointer progressCommand = CommandProgressUpdate::New();
  surfaceCreator->AddObserver(itk::ProgressEvent(), progressCommand);

  surfaceCreator->Update();
}

void QmitkSlicesInterpolator::CommandProgressUpdate::Execute(const itk::Object* caller, const itk::EventObject& event)
{
  const itk::ProcessObject* filter = static_cast<const itk::ProcessObject*>(caller);
  if (!itk::ProgressEvent().CheckEvent(&event)) {
    return;
  }

  int nextStep = (int)(filter->GetProgress() * m_NumberOfSteps);
  if (nextStep > m_LastStep) {
    mitk::ProgressBar::GetInstance()->Progress(nextStep - m_LastStep);
    m_LastStep = nextStep;
  }
}

void QmitkSlicesInterpolator::SetDataStorage( mitk::DataStorage::Pointer storage )
{
  if (m_DataStorage == storage)
  {
    return;
  }

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkSlicesInterpolator, const mitk::DataNode*>(this, &QmitkSlicesInterpolator::nodeRemoved)
    );
  }

  m_DataStorage = storage;
  m_SurfaceInterpolator->SetDataStorage(storage);

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkSlicesInterpolator, const mitk::DataNode*>(this, &QmitkSlicesInterpolator::nodeRemoved)
    );
  }
}

mitk::DataStorage* QmitkSlicesInterpolator::GetDataStorage()
{
  if ( m_DataStorage.IsNotNull() )
  {
    return m_DataStorage;
  }
  else
  {
    return NULL;
  }
}


void QmitkSlicesInterpolator::Initialize(mitk::ToolManager* toolManager, const QList<mitk::SliceNavigationController *> &controllers)
{
  Q_ASSERT(!controllers.empty());

  if (m_Initialized)
  {
    // remove old observers
    Uninitialize();
  }

  m_ToolManager = toolManager;

  if (m_ToolManager)
  {
    // set enabled only if a segmentation is selected
    mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
    QWidget::setEnabled( node != NULL );

    // react whenever the set of selected segmentation changes
    m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );
    m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified );

    // connect to the slice navigation controller. after each change, call the interpolator
    foreach(mitk::SliceNavigationController* slicer, controllers)
    {
      //Has to be initialized
      m_LastSNC = slicer;
      m_TimeStep.insert(slicer, slicer->GetTime()->GetPos());

      itk::MemberCommand<QmitkSlicesInterpolator>::Pointer deleteCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
      deleteCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted);
      m_ControllerToDeleteObserverTag.insert(slicer, slicer->AddObserver(itk::DeleteEvent(), deleteCommand));

      itk::MemberCommand<QmitkSlicesInterpolator>::Pointer timeChangedCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
      timeChangedCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnTimeChanged);
      m_ControllerToTimeObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::TimeGeometryEvent(NULL,0), timeChangedCommand));

      itk::MemberCommand<QmitkSlicesInterpolator>::Pointer sliceChangedCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
      sliceChangedCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSliceChanged);
      m_ControllerToSliceObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL,0), sliceChangedCommand));
    }
    ACTION_TO_SLICEDIMENSION = createActionToSliceDimension();

    // Initialization call
    OnToolManagerWorkingDataModified();
  }

  m_Initialized = true;
}

void QmitkSlicesInterpolator::Uninitialize()
{
  if (m_ToolManager.IsNotNull())
  {
    m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified);
    m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified);
  }

  foreach(mitk::SliceNavigationController* slicer, m_ControllerToSliceObserverTag.keys())
  {
    slicer->RemoveObserver(m_ControllerToDeleteObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToTimeObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToSliceObserverTag.take(slicer));
  }

  ACTION_TO_SLICEDIMENSION.clear();

  m_ToolManager = NULL;

  m_Initialized = false;

  m_LastSNC = nullptr;
}

QmitkSlicesInterpolator::~QmitkSlicesInterpolator()
{
  if (m_Initialized)
  {
    // remove old observers
    Uninitialize();
  }

  WaitForFutures();

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkSlicesInterpolator, const mitk::DataNode*>(this, &QmitkSlicesInterpolator::nodeRemoved)
    );
    if (m_DataStorage->Exists(m_3DContourNode))
      m_DataStorage->Remove(m_3DContourNode);
    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
      m_DataStorage->Remove(m_InterpolatedSurfaceNode);
  }

  // remove observer
  m_Interpolator->RemoveObserver( InterpolationInfoChangedObserverTag );
  m_SurfaceInterpolator->RemoveObserver( SurfaceInterpolationInfoChangedObserverTag );

  delete m_Timer;
}

/**
External enableization...
*/
void QmitkSlicesInterpolator::setEnabled( bool enable )
{
  if (m_ToolManager) {
    mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

    enable &= referenceNode && workingNode;
  }

  if (QWidget::isEnabled() == enable) {
    return;
  }

  QWidget::setEnabled(enable);

  //Set the gui elements of the different interpolation modi enabled
  this->HideAllInterpolationControls();
  if (enable) {
    if (m_2DInterpolationEnabled) {
      this->Show2DInterpolationControls(true);
      m_Interpolator->Activate2DInterpolation(true);
    } else if (m_3DInterpolationEnabled && !m_3DModificationEnabled) {
      this->Show3DInterpolationControls(true);
      this->Show3DInterpolationResult(true);
    } else if (m_3DModificationEnabled) {
      this->Show3DModificationControls(true);
      this->Show3DInterpolationResult(true);
    }
  } else {
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
  this->Show3DModificationControls(false);
}

void QmitkSlicesInterpolator::Show2DInterpolationControls(bool show)
{
  m_BtnApply2D->setVisible(show);
  m_BtnApplyForAllSlices2D->setVisible(show);
}

void QmitkSlicesInterpolator::Show3DInterpolationControls(bool show)
{
  m_BtnApply3D->setVisible(show);
  m_BtnSuggestPlane->setVisible(show);
  m_ChkShowPositionNodes->setVisible(show);
  m_BtnReinit3DInterpolation->setVisible(show);
}

void QmitkSlicesInterpolator::Show3DModificationControls(bool show)
{
  m_BtnUnion->setVisible(show);
  m_BtnSubtraction->setVisible(show);
  m_BtnIntersection->setVisible(show);
  m_BtnDifference->setVisible(show);
  m_TempSegClear->setVisible(show);
}

void QmitkSlicesInterpolator::OnInterpolationMethodChanged(int index)
{
  switch(index)
  {
    case 0: // Disabled
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle(TR_INTERPOLATION);
      m_Activate3DInterpolation = false;
      this->HideAllInterpolationControls();
      this->OnInterpolationActivated(false);
      this->On3DInterpolationActivated(false);
      this->On3DModificationActivated(false);
      this->Show3DInterpolationResult(false);
      m_Interpolator->Activate2DInterpolation(false);

      if (m_DataStorage->Exists(m_3DContourNode))
        m_DataStorage->Remove(m_3DContourNode);
      if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
        m_DataStorage->Remove(m_InterpolatedSurfaceNode);

      break;

    case 1: // 2D
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle(TR_INTERPOLATION_ENB);
      m_Activate3DInterpolation = false;
      this->HideAllInterpolationControls();
      this->Show2DInterpolationControls(true);
      this->On3DModificationActivated(false);
      this->OnInterpolationActivated(true);
      this->On3DInterpolationActivated(false);
      m_Interpolator->Activate2DInterpolation(true);
      break;

    case 2: // 3D
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle(TR_INTERPOLATION_ENB);
      m_Activate3DInterpolation = true;
      this->HideAllInterpolationControls();
      this->Show3DInterpolationControls(true);
      this->On3DModificationActivated(false);
      this->OnInterpolationActivated(false);
      this->On3DInterpolationActivated(true);
      m_Interpolator->Activate2DInterpolation(false);
      break;

    case 3: // 3D Modification
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle(TR_INTERPOLATION_ENB);
      m_Activate3DInterpolation = true;
      HideAllInterpolationControls();
      Show3DModificationControls(true);
      OnInterpolationActivated(false);
      On3DInterpolationActivated(false);
      On3DModificationActivated(true);
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
  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("isContourMarker"
                                                                                                                              , mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
  {
    it->Value()->SetProperty("helper object", mitk::BoolProperty::New(!state));
  }
}

void QmitkSlicesInterpolator::OnToolManagerWorkingDataModified()
{
  if (m_ToolManager->GetWorkingData(0) != 0)
  {
    m_Segmentation = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());
    m_BtnReinit3DInterpolation->setEnabled(true);
  }
  else
  {
    //If no workingdata is set, remove the interpolation feedback
    this->GetDataStorage()->Remove(m_FeedbackNode);
    m_FeedbackNode->SetData(NULL);
    this->GetDataStorage()->Remove(m_3DContourNode);
    m_3DContourNode->SetData(NULL);
    this->GetDataStorage()->Remove(m_InterpolatedSurfaceNode);
    m_InterpolatedSurfaceNode->SetData(NULL);
    m_BtnReinit3DInterpolation->setEnabled(false);
    return;
  }

  if (m_3DModificationEnabled) {
    tempSegmentationClear();
    if (m_ToolManager->GetWorkingData().size() > 0) {
      m_OrigSegmentation = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData()[0]->GetData());
      if (!m_DataStorage->Exists(m_TempSegmentationNode)) {
        m_DataStorage->Add(m_TempSegmentationNode, m_ToolManager->GetWorkingData()[0]);
      }
    }
  }

  //Updating the current selected segmentation for the 3D interpolation
  SetCurrentContourListID();

  if (m_2DInterpolationEnabled)
  {
    OnInterpolationActivated( true ); // re-initialize if needed
  }
  this->CheckSupportedImageDimension();
}

void QmitkSlicesInterpolator::OnToolManagerReferenceDataModified()
{
}

void QmitkSlicesInterpolator::OnTimeChanged(itk::Object* sender, const itk::EventObject& e)
{
  //Check if we really have a GeometryTimeEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent*>(&e))
    return;

  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(sender);
  Q_ASSERT(slicer);

  m_TimeStep[slicer] = slicer->GetTime()->GetPos();

  m_SurfaceInterpolator->SetCurrentTimeStep( slicer->GetTime()->GetPos() );

  if (m_LastSNC == slicer)
  {
    slicer->SendSlice();//will trigger a new interpolation
  }
}

void QmitkSlicesInterpolator::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  //Check whether we really have a GeometrySliceEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent*>(&e))
    return;

  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(sender);

  if (TranslateAndInterpolateChangedSlice(e, slicer))
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e, mitk::SliceNavigationController* slicer)
{
  if (!m_2DInterpolationEnabled) return false;

  try
  {
    const mitk::SliceNavigationController::GeometrySliceEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent&>(e);

    mitk::TimeGeometry* tsg = event.GetTimeGeometry();
    if (tsg && m_TimeStep.contains(slicer))
    {
      mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(tsg->GetGeometryForTimeStep(m_TimeStep[slicer]).GetPointer());
      if (slicedGeometry)
      {
        mitk::PlaneGeometry* plane = dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry->GetPlaneGeometry( event.GetPos() ));
        if (plane)
          Interpolate( plane, m_TimeStep[slicer], slicer );
        return true;
      }
    }
  }
  catch(std::bad_cast)
  {
    return false; // so what
  }

  return false;
}

void QmitkSlicesInterpolator::Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep, mitk::SliceNavigationController* slicer )
{
  if (m_ToolManager)
  {
    mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
    if (node)
    {
      m_Segmentation = dynamic_cast<mitk::Image*>(node->GetData());
      if (m_Segmentation)
      {
        int clickedSliceDimension(-1);
        int clickedSliceIndex(-1);

        // calculate real slice position, i.e. slice of the image and not slice of the TimeSlicedGeometry
        mitk::SegTool2D::DetermineAffectedImageSlice( m_Segmentation, plane, clickedSliceDimension, clickedSliceIndex );

        mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( clickedSliceDimension, clickedSliceIndex, plane, timeStep );
        if (interpolation) {
          m_FeedbackNode->SetData(interpolation);
          m_LastSNC = slicer;
        } else if (m_LastSNC == slicer) {
          m_FeedbackNode->SetData(nullptr);
        }
      }
    }
  }
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationFinished()
{
  mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if(interpolatedSurface.IsNotNull() && workingNode &&
     workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))))
  {
    m_BtnApply3D->setEnabled(true);
    m_BtnSuggestPlane->setEnabled(true);
    m_InterpolatedSurfaceNode->SetData(interpolatedSurface);
    m_3DContourNode->SetData(m_SurfaceInterpolator->GetContoursAsSurface());

    if (m_Activate3DInterpolation) {
      this->Show3DInterpolationResult(true);
    } else {
      this->Show3DInterpolationResult(false);
    }

    if( !m_DataStorage->Exists(m_InterpolatedSurfaceNode) )
    {
      m_DataStorage->Add(m_InterpolatedSurfaceNode);
    }
    if (!m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Add(m_3DContourNode, workingNode);
    }

  }
  else if (interpolatedSurface.IsNull())
  {
    m_BtnApply3D->setEnabled(false);
    m_BtnSuggestPlane->setEnabled(false);

    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      this->Show3DInterpolationResult(false);
    }
  }

  m_BtnReinit3DInterpolation->setEnabled(true);

  foreach (mitk::SliceNavigationController* slicer, m_ControllerToTimeObserverTag.keys())
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

void QmitkSlicesInterpolator::OnAcceptInterpolationClicked()
{
  if (m_Segmentation && m_FeedbackNode->GetData())
  {
    //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
    vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

    // Set slice as input
    mitk::Image::Pointer slice = dynamic_cast<mitk::Image*>(m_FeedbackNode->GetData());
    reslice->SetInputSlice(slice->GetSliceData()->GetVtkImageAccessor(slice)->getVtkImageData());
    //set overwrite mode to true to write back to the image volume
    reslice->SetOverwriteMode(true);
    reslice->Modified();

    mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
    extractor->SetInput( m_Segmentation );
    unsigned int timestep = m_LastSNC->GetTime()->GetPos();
    extractor->SetTimeStep( timestep );
    extractor->SetWorldGeometry( m_LastSNC->GetCurrentPlaneGeometry() );
    extractor->SetVtkOutputRequest(true);
    extractor->SetResliceTransformByGeometry( m_Segmentation->GetTimeGeometry()->GetGeometryForTimeStep( timestep ) );

    extractor->Modified();
    extractor->Update();

    //the image was modified within the pipeline, but not marked so
    m_Segmentation->Modified();
    //m_Segmentation->GetVtkImageData()->Modified();

    m_FeedbackNode->SetData(NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSlicesInterpolator::AcceptAllInterpolations(mitk::SliceNavigationController* slicer)
{
  /*
   * What exactly is done here:
   * 1. We create an empty diff image for the current segmentation
   * 2. All interpolated slices are written into the diff image
   * 3. Then the diffimage is applied to the original segmentation
   */
  if (m_Segmentation)
  {
    mitk::Image::Pointer image3D = m_Segmentation;
    unsigned int timeStep( slicer->GetTime()->GetPos() );
    if (m_Segmentation->GetDimension() == 4)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput( m_Segmentation );
      timeSelector->SetTimeNr( timeStep );
      timeSelector->Update();
      image3D = timeSelector->GetOutput();
    }
    // create a empty diff image for the undo operation
    mitk::Image::Pointer diffImage = mitk::Image::New();
    diffImage->Initialize( image3D );

    // Create scope for ImageWriteAccessor so that the accessor is destroyed
    // after the image is initialized. Otherwise later image access will lead to an error
    {
      mitk::ImageRegionAccessor imAccess(diffImage);
      mitk::ImageAccessLock lock(&imAccess);

      // Set all pixels to zero
      mitk::PixelType pixelType (mitk::MakeScalarPixelType<mitk::Tool::DefaultSegmentationDataType>());

      // For legacy purpose support former pixel type of segmentations (before multilabel)
      if (m_Segmentation->GetImageDescriptor()->GetChannelDescriptor().GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR)
      {
        pixelType = mitk::MakeScalarPixelType<unsigned char>();
      }

      memset( imAccess.getData(), 0, (pixelType.GetBpe() >> 3) * diffImage->GetDimension(0) * diffImage->GetDimension(1) * diffImage->GetDimension(2) );
    }


    // Since we need to shift the plane it must be clone so that the original plane isn't altered
    mitk::PlaneGeometry::Pointer reslicePlane = slicer->GetCurrentPlaneGeometry()->Clone();

    int sliceDimension(-1);
    int sliceIndex(-1);
    mitk::SegTool2D::DetermineAffectedImageSlice( m_Segmentation, reslicePlane, sliceDimension, sliceIndex );

    unsigned int zslices = m_Segmentation->GetDimension( sliceDimension );
    mitk::ProgressBar::GetInstance()->AddStepsToDo(zslices);

    mitk::Point3D origin = reslicePlane->GetOrigin();
    unsigned int totalChangedSlices(0);

    for (unsigned int sliceIndex = 0; sliceIndex < zslices; ++sliceIndex)
    {
      // Transforming the current origin of the reslice plane
      // so that it matches the one of the next slice
      m_Segmentation->GetSlicedGeometry()->WorldToIndex(origin, origin);
      origin[sliceDimension] = sliceIndex;
      m_Segmentation->GetSlicedGeometry()->IndexToWorld(origin, origin);
      reslicePlane->SetOrigin(origin);
      //Set the slice as 'input'
      mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( sliceDimension, sliceIndex, reslicePlane, timeStep );

      if (interpolation.IsNotNull()) // we don't check if interpolation is necessary/sensible - but m_Interpolator does
      {
        //Setting up the reslicing pipeline which allows us to write the interpolation results back into
        //the image volume
        vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

        //set overwrite mode to true to write back to the image volume
        reslice->SetInputSlice(interpolation->GetSliceData()->GetVtkImageAccessor(interpolation)->getVtkImageData());
        reslice->SetOverwriteMode(true);
        reslice->Modified();

        mitk::ExtractSliceFilter::Pointer diffslicewriter =  mitk::ExtractSliceFilter::New(reslice);
        diffslicewriter->SetInput( diffImage );
        diffslicewriter->SetTimeStep( 0 );
        diffslicewriter->SetWorldGeometry(reslicePlane);
        diffslicewriter->SetVtkOutputRequest(true);
        diffslicewriter->SetResliceTransformByGeometry( diffImage->GetTimeGeometry()->GetGeometryForTimeStep( 0 ) );

        diffslicewriter->Modified();
        diffslicewriter->Update();
        ++totalChangedSlices;
      }
      mitk::ProgressBar::GetInstance()->Progress();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    if (totalChangedSlices > 0)
    {
      // store undo stack items
      if ( true )
      {
        // create do/undo operations
        mitk::ApplyDiffImageOperation* doOp = new mitk::ApplyDiffImageOperation( mitk::OpTEST, m_Segmentation, diffImage, timeStep );
        mitk::ApplyDiffImageOperation* undoOp = new mitk::ApplyDiffImageOperation( mitk::OpTEST, m_Segmentation, diffImage, timeStep );
        undoOp->SetFactor( -1.0 );
        std::stringstream comment;
        comment << "Confirm all interpolations (" << totalChangedSlices << ")";
        mitk::OperationEvent* undoStackItem = new mitk::OperationEvent( mitk::DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, comment.str() );
        mitk::OperationEvent::IncCurrGroupEventId();
        mitk::OperationEvent::IncCurrObjectEventId();
        mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );

        // acutally apply the changes here to the original image
        mitk::DiffImageApplier::GetInstanceForUndo()->ExecuteOperation( doOp );
      }
    }

    m_FeedbackNode->SetData(NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSlicesInterpolator::FinishInterpolation(mitk::SliceNavigationController* slicer)
{
  //this redirect is for calling from outside

  if (slicer == NULL)
    OnAcceptAllInterpolationsClicked();
  else
    AcceptAllInterpolations( slicer );
}

void QmitkSlicesInterpolator::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  std::map<QAction*, mitk::SliceNavigationController*>::const_iterator it;
  for(it = ACTION_TO_SLICEDIMENSION.begin(); it != ACTION_TO_SLICEDIMENSION.end(); it++)
    orientationPopup.addAction(it->first);

  connect( &orientationPopup, SIGNAL(triggered(QAction*)), this, SLOT(OnAcceptAllPopupActivated(QAction*)) );

  orientationPopup.exec( QCursor::pos() );
}

void QmitkSlicesInterpolator::OnAccept3DInterpolationClicked()
{
  if (m_InterpolatedSurfaceNode.IsNotNull() && m_InterpolatedSurfaceNode->GetData())
  {
    mitk::DataNode* segmentationNode = m_ToolManager->GetWorkingData(0);
    mitk::Image* currSeg = dynamic_cast<mitk::Image*>(segmentationNode->GetData());

    mitk::SurfaceToImageFilter::Pointer s2iFilter = mitk::SurfaceToImageFilter::New();
    s2iFilter->MakeOutputBinaryOn();
    if (currSeg->GetPixelType().GetComponentType() == itk::ImageIOBase::USHORT)
      s2iFilter->SetUShortBinaryPixelType(true);
    s2iFilter->SetInput(dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData()));

    // check if ToolManager holds valid ReferenceData
    if (m_ToolManager->GetReferenceData(0) == NULL || m_ToolManager->GetWorkingData(0) == NULL)
    {
        return;
    }
    s2iFilter->SetImage(dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData()));
    s2iFilter->Update();

    mitk::Image::Pointer newSeg = s2iFilter->GetOutput();

    unsigned int timestep = m_LastSNC->GetTime()->GetPos();
    mitk::ImageRegionAccessor accessor(newSeg);
    mitk::ImageAccessLock lock(&accessor);
    const void* cPointer = accessor.getData(timestep);

    if (currSeg && cPointer)
    {
      currSeg->SetVolume( cPointer, timestep, 0 );
    }
    else
    {
      return;
    }

    m_CmbInterpolation->setCurrentIndex(0);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    mitk::DataNode::Pointer segSurface = mitk::DataNode::New();
    float rgb[3];
    segmentationNode->GetColor(rgb);
    segSurface->SetColor(rgb);
    segSurface->SetData(m_InterpolatedSurfaceNode->GetData());
    std::stringstream stream;
    stream << segmentationNode->GetName();
    stream << "_";
    stream << "3D-interpolation";
    segSurface->SetName(stream.str());
    segSurface->SetProperty( "opacity", mitk::FloatProperty::New(0.7) );
    segSurface->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(true));
    segSurface->SetProperty( "3DInterpolationResult", mitk::BoolProperty::New(true));
    segSurface->SetVisibility(false);

    // Remove current 3D model
    mitk::DataNode::Pointer previousModel = nullptr;
    mitk::Surface::Pointer previousSurface = nullptr;
    auto childs = m_DataStorage->GetDerivations(segmentationNode);
    for (const auto& children : *childs) {
      previousSurface = dynamic_cast<mitk::Surface*>(children->GetData());
      if (previousSurface != nullptr) {
        previousModel = children;
        break;
      }
    }

    if (previousModel) {
      m_DataStorage->Remove(previousModel);
    }

    // Add new 3D model
    m_DataStorage->Add(segSurface, segmentationNode);
    this->Show3DInterpolationResult(false);
    emit SignalCreated3dInterpolation();
  }
}

void::QmitkSlicesInterpolator::OnSuggestPlaneClicked()
{
  if (m_PlaneWatcher.isRunning())
    m_PlaneWatcher.waitForFinished();
  m_PlaneFuture = QtConcurrent::run(this, &QmitkSlicesInterpolator::RunPlaneSuggestion);
  m_PlaneWatcher.setFuture(m_PlaneFuture);
}

void::QmitkSlicesInterpolator::RunPlaneSuggestion()
{
  if(m_FirstRun)
    mitk::ProgressBar::GetInstance()->AddStepsToDo(7);
  else
    mitk::ProgressBar::GetInstance()->AddStepsToDo(3);

  m_EdgeDetector->SetSegmentationMask(m_Segmentation);
  m_EdgeDetector->SetInput(dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData()));
  m_EdgeDetector->Update();

  mitk::UnstructuredGrid::Pointer uGrid = mitk::UnstructuredGrid::New();
  uGrid->SetVtkUnstructuredGrid(m_EdgeDetector->GetOutput()->GetVtkUnstructuredGrid());

  mitk::ProgressBar::GetInstance()->Progress();

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData());

  vtkSmartPointer< vtkPolyData > vtkpoly = surface->GetVtkPolyData();
  vtkSmartPointer< vtkPoints> vtkpoints = vtkpoly->GetPoints();

  vtkSmartPointer<vtkUnstructuredGrid> vGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

  verts->GetPointIds()->SetNumberOfIds(vtkpoints->GetNumberOfPoints());
  for(int i=0; i<vtkpoints->GetNumberOfPoints(); i++)
  {
    verts->GetPointIds()->SetId(i,i);
  }

  vGrid->Allocate(1);
  vGrid->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  vGrid->SetPoints(vtkpoints);

  mitk::UnstructuredGrid::Pointer interpolationGrid = mitk::UnstructuredGrid::New();
  interpolationGrid->SetVtkUnstructuredGrid(vGrid);

  m_PointScorer->SetInput(0, uGrid);
  m_PointScorer->SetInput(1, interpolationGrid);
  m_PointScorer->Update();

  mitk::UnstructuredGrid::Pointer scoredGrid = mitk::UnstructuredGrid::New();
  scoredGrid = m_PointScorer->GetOutput();

  mitk::ProgressBar::GetInstance()->Progress();

  double spacing = mitk::SurfaceInterpolationController::GetInstance()->GetDistanceImageSpacing();
  mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
  clusterFilter->SetInput(scoredGrid);
  clusterFilter->SetMeshing(false);
  clusterFilter->SetMinPts(4);
  clusterFilter->Seteps(spacing);
  clusterFilter->Update();

  mitk::ProgressBar::GetInstance()->Progress();

  // Create plane suggestion
  mitk::BaseRenderer::Pointer br = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"));
  mitk::PlaneProposer planeProposer;
  std::vector <mitk::UnstructuredGrid::Pointer > grids = clusterFilter->GetAllClusters();

  planeProposer.SetUnstructuredGrids(grids);
  mitk::SliceNavigationController::Pointer snc = br->GetSliceNavigationController();
  planeProposer.SetSliceNavigationController(snc);
  planeProposer.SetUseDistances(true);
  try
  {
    planeProposer.CreatePlaneInfo();
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR<< e.what();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_FirstRun = false;
}

void QmitkSlicesInterpolator::OnReinit3DInterpolation()
{
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("3DContourContainer", mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer contourNodes = m_DataStorage->GetDerivations( m_ToolManager->GetWorkingData(0), pred);
  if (contourNodes->Size() != 0)
  {
    m_3DContourNode = contourNodes->at(0);
  }
  else
  {
    QMessageBox errorInfo;
    errorInfo.setWindowTitle(tr("Reinitialize surface interpolation"));
    errorInfo.setIcon(QMessageBox::Information);
    errorInfo.setText(tr("No contours available for the selected segmentation!"));
    errorInfo.exec();
  }
  mitk::Surface::Pointer contours = dynamic_cast<mitk::Surface*>(m_3DContourNode->GetData());
  if (contours)
    mitk::SurfaceInterpolationController::GetInstance()->ReinitializeInterpolation(contours);
  m_BtnReinit3DInterpolation->setEnabled(false);
}

void QmitkSlicesInterpolator::OnAcceptAllPopupActivated(QAction* action)
{
  try
  {
    std::map<QAction*, mitk::SliceNavigationController*>::const_iterator iter = ACTION_TO_SLICEDIMENSION.find( action );
    if (iter != ACTION_TO_SLICEDIMENSION.end())
    {
      mitk::SliceNavigationController* slicer = iter->second;
      AcceptAllInterpolations( slicer );
    }
  }
  catch(...)
  {
    /* Showing message box with possible memory error */
    QMessageBox errorInfo;
    errorInfo.setWindowTitle(TR_INTERPOLATION_PROC);
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText(TR_INTERPOLATION_ERROR);
    errorInfo.exec();

    //additional error message on std::cerr
    std::cerr << "Ill construction in " __FILE__ " l. " << __LINE__ << std::endl;
  }
}

void QmitkSlicesInterpolator::OnInterpolationActivated(bool on)
{
  m_2DInterpolationEnabled = on;

  try
  {
    if ( m_DataStorage.IsNotNull() )
    {
      if (on && !m_DataStorage->Exists(m_FeedbackNode))
      {
        m_DataStorage->Add( m_FeedbackNode );
      }
    }
  }
  catch(...)
  {
    // don't care (double add/remove)
  }

  if (m_ToolManager)
  {
    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
    mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
    QWidget::setEnabled( workingNode != NULL );

    m_BtnApply2D->setEnabled( on );
    m_FeedbackNode->SetVisibility( on );

    if (!on)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return;
    }

    if (workingNode)
    {
      mitk::Image* segmentation = dynamic_cast<mitk::Image*>(workingNode->GetData());
      if (segmentation)
      {
        m_Interpolator->SetSegmentationVolume( segmentation );

        if (referenceNode)
        {
          mitk::Image* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
          m_Interpolator->SetReferenceVolume( referenceImage ); // may be NULL
        }
      }
    }
  }

  UpdateVisibleSuggestion();
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
  m_Timer->stop();
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(SURFACE_COLOR_RGB));
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetRenderWindow());
}

void QmitkSlicesInterpolator::ChangeSurfaceColor()
{
  float currentColor[3];
  m_InterpolatedSurfaceNode->GetColor(currentColor);

  if( currentColor[2] == SURFACE_COLOR_RGB[2])
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(1.0f,1.0f,1.0f));
  }
  else
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(SURFACE_COLOR_RGB));
  }
  m_InterpolatedSurfaceNode->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetRenderWindow());
}

void QmitkSlicesInterpolator::On3DInterpolationActivated(bool on)
{
  m_3DInterpolationEnabled = on;

  this->CheckSupportedImageDimension();
  try
  {
    if ( m_DataStorage.IsNotNull() && m_ToolManager && m_3DInterpolationEnabled)
    {
      mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

      if (workingNode)
      {
        bool isInterpolationResult(false);
        workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);
        mitk::NodePredicateAnd::Pointer pred = mitk::NodePredicateAnd::New(mitk::NodePredicateProperty::New("3DInterpolationResult", mitk::BoolProperty::New(true)),
                                                                           mitk::NodePredicateDataType::New("Surface"));
        mitk::DataStorage::SetOfObjects::ConstPointer interpolationResults =
            m_DataStorage->GetDerivations(workingNode, pred);

        for (unsigned int i = 0; i < interpolationResults->Size(); ++i)
        {
          mitk::DataNode::Pointer currNode = interpolationResults->at(i);
          if (currNode.IsNotNull())
            m_DataStorage->Remove(currNode);
        }

        if ((workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")))) &&
            !isInterpolationResult && m_3DInterpolationEnabled)
        {
          int ret = QMessageBox::Yes;

          if (m_SurfaceInterpolator->EstimatePortionOfNeededMemory() > 0.5)
          {
            QMessageBox msgBox;
            msgBox.setText(TR_INTERPOLATION_SLOW);
            msgBox.setInformativeText(TR_INTERPOLATION_SURE);
            msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
            ret = msgBox.exec();
          }

          if (m_Watcher.isRunning())
            m_Watcher.waitForFinished();

          if (ret == QMessageBox::Yes)
          {
            m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
            m_Watcher.setFuture(m_Future);
          }
          else
          {
            m_CmbInterpolation->setCurrentIndex(0);
          }
        }
        else if (!m_3DInterpolationEnabled)
        {
          this->Show3DInterpolationResult(false);
          m_BtnApply3D->setEnabled(m_3DInterpolationEnabled);
          m_BtnSuggestPlane->setEnabled(m_3DInterpolationEnabled);
        }
      }
      else
      {
        QWidget::setEnabled( false );
        m_ChkShowPositionNodes->setEnabled(m_3DInterpolationEnabled);
      }
    }
    if (!m_3DInterpolationEnabled)
    {
       this->Show3DInterpolationResult(false);
       m_BtnApply3D->setEnabled(m_3DInterpolationEnabled);
       m_BtnSuggestPlane->setEnabled(m_3DInterpolationEnabled);
    }
  }
  catch(...)
  {
    MITK_ERROR<<"Error with 3D surface interpolation!";
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::On3DModificationActivated(bool on)
{
  if (m_Watcher.isRunning())
    m_Watcher.waitForFinished();
  m_3DModificationEnabled = on;

  // In reality only initialized m_Segmentation for convienience
  CheckSupportedImageDimension();

  if (m_3DModificationEnabled) {
    mitk::DataNode* segNode = m_ToolManager->GetWorkingData(0);
    m_TempSegmentationNode->SetName(segNode->GetName() + "_temp");

    m_OrigSegmentation = m_Segmentation;
    m_TempSegmentation->Initialize(m_Segmentation->GetPixelType(0), m_Segmentation->GetDimension(), m_Segmentation->GetDimensions());
    m_TempSegmentation->SetGeometry(m_Segmentation->GetGeometry());

    size_t segSize = 1;
    for (unsigned int dim = 0; dim < m_Segmentation->GetDimension(); ++dim) {
      segSize *= m_Segmentation->GetDimension(dim);
    }

    {
      mitk::ImageRegionAccessor accessor(m_TempSegmentation);
      mitk::ImageAccessLock lock(&accessor, true);
      memset(accessor.getData(), 0, segSize * m_Segmentation->GetPixelType(0).GetSize());
    }

    if (m_TempSegmentation->GetTimeGeometry()) {
      mitk::TimeGeometry::Pointer originalGeometry = m_Segmentation->GetTimeGeometry()->Clone();
      m_TempSegmentation->SetTimeGeometry(originalGeometry);
    }

    m_ToolManager->overwirteWorkingData = m_TempSegmentationNode;
    m_DataStorage->Add(m_TempSegmentationNode, segNode);

    SetCurrentContourListID();
    On3DInterpolationActivated(true);
  } else {
    On3DInterpolationActivated(false);
    
    if (m_ToolManager->overwirteWorkingData != nullptr) {
      m_ToolManager->overwirteWorkingData = nullptr;
      SetCurrentContourListID();
    }

    m_SurfaceInterpolator->RemoveInterpolationSession(m_TempSegmentation);
    m_DataStorage->Remove(m_DataStorage->GetDerivations(m_TempSegmentationNode, nullptr, false));
    m_DataStorage->Remove(m_TempSegmentationNode);
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
  On3DInterpolationActivated(on);
}

void QmitkSlicesInterpolator::UpdateVisibleSuggestion()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::OnInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  // something (e.g. undo) changed the interpolation info, we should refresh our display
  UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  if(m_3DInterpolationEnabled)
  {
    if (m_Watcher.isRunning())
      m_Watcher.waitForFinished();
    m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
    m_Watcher.setFuture(m_Future);
  }
}

void QmitkSlicesInterpolator:: SetCurrentContourListID()
{
  // New ContourList = hide current interpolation
  Show3DInterpolationResult(false);

  if ( m_DataStorage.IsNotNull() && m_ToolManager && m_LastSNC )
  {
    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

    if (workingNode)
    {
      bool isInterpolationResult(false);
      workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);

      if (!isInterpolationResult)
      {
        QmitkSlicesInterpolator::setEnabled( true );

        // In case the time is not valid use 0 to access the time geometry of the working node
        unsigned int time_position = 0;
        if (m_LastSNC->GetTime() != NULL)
        {
          time_position = m_TimeStep[m_LastSNC];
        }

        mitk::Vector3D spacing = workingNode->GetData()->GetGeometry( time_position )->GetSpacing();
        double minSpacing (100);
        double maxSpacing (0);
        for (int i =0; i < 3; i++)
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

        mitk::Image* segmentationImage = dynamic_cast<mitk::Image*>(workingNode->GetData());
        /*if (segmentationImage->GetDimension() == 3)
        {*/
          m_SurfaceInterpolator->SetCurrentInterpolationSession(segmentationImage);
          m_SurfaceInterpolator->SetCurrentTimeStep( time_position );
        //}
        /*else
          MITK_INFO<<"3D Interpolation is only supported for 3D images at the moment!";*/

        if (m_3DInterpolationEnabled)
        {
          if (m_Watcher.isRunning())
            m_Watcher.waitForFinished();
          m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
          m_Watcher.setFuture(m_Future);
        }
      }
    }
    else
    {
      QmitkSlicesInterpolator::setEnabled(false);
    }
  }
}

void QmitkSlicesInterpolator::Show3DInterpolationResult(bool status)
{
   if (m_InterpolatedSurfaceNode.IsNotNull())
      m_InterpolatedSurfaceNode->SetVisibility(status);

   if (m_3DContourNode.IsNotNull())
      m_3DContourNode->SetVisibility(status, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::CheckSupportedImageDimension()
{
  if (m_ToolManager->GetWorkingData(0))
    m_Segmentation = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());

  /*if (m_3DInterpolationEnabled && m_Segmentation && m_Segmentation->GetDimension() != 3)
  {
    QMessageBox info;
    info.setWindowTitle(TR_INTERPOLATION_3D_PROC);
    info.setIcon(QMessageBox::Information);
    info.setText(TR_INTERPOLATION_3D_SUPPORT);
    info.exec();
    m_CmbInterpolation->setCurrentIndex(0);
  }*/
}

void QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted(const itk::Object *sender, const itk::EventObject& /*e*/)
{
  //Don't know how to avoid const_cast here?!
  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(const_cast<itk::Object*>(sender));
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

void QmitkSlicesInterpolator::nodeRemoved(const mitk::DataNode* node)
{
  if ((m_ToolManager && m_ToolManager->GetWorkingData(0) == node) ||
      node == m_3DContourNode ||
      node == m_FeedbackNode ||
      node == m_InterpolatedSurfaceNode)
  {
    WaitForFutures();
  }
}
