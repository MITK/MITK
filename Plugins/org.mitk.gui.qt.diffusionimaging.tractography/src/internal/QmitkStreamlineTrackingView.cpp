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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

// Qmitk
#include "QmitkStreamlineTrackingView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

// MITK
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkImageToItk.h>
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDimension.h>
#include <mitkOdfImage.h>
#include <mitkSliceNavigationController.h>

// VTK
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyLine.h>
#include <vtkCellData.h>

#include <itkTensorImageToOdfImageFilter.h>
#include <omp.h>
#include <mitkLexicalCast.h>

const std::string QmitkStreamlineTrackingView::VIEW_ID = "org.mitk.views.streamlinetracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkStreamlineTrackingWorker::QmitkStreamlineTrackingWorker(QmitkStreamlineTrackingView* view)
    : m_View(view)
{
}

void QmitkStreamlineTrackingWorker::run()
{
    m_View->m_Tracker->Update();
    m_View->m_TrackingThread.quit();
}

QmitkStreamlineTrackingView::QmitkStreamlineTrackingView()
  : m_TrackingWorker(this)
  , m_Controls(nullptr)
  , m_FirstTensorProbRun(true)
  , m_FirstInteractiveRun(true)
  , m_TrackingHandler(nullptr)
  , m_ThreadIsRunning(false)
  , m_DeleteTrackingHandler(false)
  , m_Visible(false)
  , m_LastPrior("")
  , m_TrackingPriorHandler(nullptr)
{
  m_TrackingWorker.moveToThread(&m_TrackingThread);
  connect(&m_TrackingThread, SIGNAL(started()), this, SLOT(BeforeThread()));
  connect(&m_TrackingThread, SIGNAL(started()), &m_TrackingWorker, SLOT(run()));
  connect(&m_TrackingThread, SIGNAL(finished()), this, SLOT(AfterThread()));
  m_TrackingTimer = new QTimer(this);
}

// Destructor
QmitkStreamlineTrackingView::~QmitkStreamlineTrackingView()
{
  if (m_Tracker.IsNull())
    return;

  m_Tracker->SetStopTracking(true);
  m_TrackingThread.wait();
}

void QmitkStreamlineTrackingView::CreateQtPartControl( QWidget *parent )
{
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkStreamlineTrackingViewControls;
    m_Controls->setupUi( parent );
    m_Controls->m_FaImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_SeedImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MaskImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TargetImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_PriorImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_StopImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ForestBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ExclusionImageBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::PeakImage>::Pointer isPeakImagePredicate = mitk::TNodePredicateDataType<mitk::PeakImage>::New();
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::TNodePredicateDataType<mitk::TractographyForest>::Pointer isTractographyForest = mitk::TNodePredicateDataType<mitk::TractographyForest>::New();

    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer isNotBinaryPredicate = mitk::NodePredicateNot::New( isBinaryPredicate );
    mitk::NodePredicateAnd::Pointer isNotABinaryImagePredicate = mitk::NodePredicateAnd::New( isImagePredicate, isNotBinaryPredicate );
    mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);

    m_Controls->m_ForestBox->SetPredicate(isTractographyForest);
    m_Controls->m_FaImageBox->SetPredicate( mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, dimensionPredicate) );
    m_Controls->m_FaImageBox->SetZeroEntryText("--");
    m_Controls->m_SeedImageBox->SetPredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_SeedImageBox->SetZeroEntryText("--");
    m_Controls->m_MaskImageBox->SetPredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_MaskImageBox->SetZeroEntryText("--");
    m_Controls->m_StopImageBox->SetPredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_StopImageBox->SetZeroEntryText("--");
    m_Controls->m_TargetImageBox->SetPredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_TargetImageBox->SetZeroEntryText("--");
    m_Controls->m_PriorImageBox->SetPredicate( isPeakImagePredicate );
    m_Controls->m_PriorImageBox->SetZeroEntryText("--");
    m_Controls->m_ExclusionImageBox->SetPredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_ExclusionImageBox->SetZeroEntryText("--");

    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()) );
    connect( m_Controls->commandLinkButton_2, SIGNAL(clicked()), this, SLOT(StopTractography()) );
    connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
    connect( m_Controls->m_InteractiveBox, SIGNAL(stateChanged(int)), this, SLOT(ToggleInteractive()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()) );
    connect( m_Controls->m_FaImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DeleteTrackingHandler()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DeleteTrackingHandler()) );
    connect( m_Controls->m_OutputProbMap, SIGNAL(stateChanged(int)), this, SLOT(OutputStyleSwitched()) );

    connect( m_Controls->m_SeedImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_StopImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_TargetImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_PriorImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ExclusionImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MaskImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FaImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ForestBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ForestSwitched()) );
    connect( m_Controls->m_ForestBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SeedsPerVoxelBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_NumFibersBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ScalarThresholdBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_OdfCutoffBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_StepSizeBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SamplingDistanceBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_AngularThresholdBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MinTractLengthBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_fBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_gBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_NumSamplesBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SeedRadiusBox, SIGNAL(editingFinished()), this, SLOT(InteractiveSeedChanged()) );
    connect( m_Controls->m_NumSeedsBox, SIGNAL(editingFinished()), this, SLOT(InteractiveSeedChanged()) );
    connect( m_Controls->m_OutputProbMap, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SharpenOdfsBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_InterpolationBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MaskInterpolationBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FlipXBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FlipYBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FlipZBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FrontalSamplesBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_StopVotesBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_LoopCheckBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_TrialsPerSeedBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_EpConstraintsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );

    m_Controls->m_SeedsPerVoxelBox->editingFinished();
    m_Controls->m_NumFibersBox->editingFinished();
    m_Controls->m_ScalarThresholdBox->editingFinished();
    m_Controls->m_OdfCutoffBox->editingFinished();
    m_Controls->m_StepSizeBox->editingFinished();
    m_Controls->m_SamplingDistanceBox->editingFinished();
    m_Controls->m_AngularThresholdBox->editingFinished();
    m_Controls->m_MinTractLengthBox->editingFinished();
    m_Controls->m_fBox->editingFinished();
    m_Controls->m_gBox->editingFinished();
    m_Controls->m_NumSamplesBox->editingFinished();
    m_Controls->m_SeedRadiusBox->editingFinished();
    m_Controls->m_NumSeedsBox->editingFinished();
    m_Controls->m_LoopCheckBox->editingFinished();
    m_Controls->m_TrialsPerSeedBox->editingFinished();

    StartStopTrackingGui(false);
  }

  UpdateGui();
}

void QmitkStreamlineTrackingView::StopTractography()
{
  if (m_Tracker.IsNull())
    return;

  m_Tracker->SetStopTracking(true);
}

void QmitkStreamlineTrackingView::TimerUpdate()
{
  if (m_Tracker.IsNull())
    return;

  QString status_text(m_Tracker->GetStatusText().c_str());
  m_Controls->m_StatusTextBox->setText(status_text);
}

void QmitkStreamlineTrackingView::BeforeThread()
{
  m_TrackingTimer->start(1000);
}

void QmitkStreamlineTrackingView::AfterThread()
{
  m_TrackingTimer->stop();
  if (!m_Tracker->GetUseOutputProbabilityMap())
  {
    vtkSmartPointer<vtkPolyData> fiberBundle = m_Tracker->GetFiberPolyData();

    if (!m_Controls->m_InteractiveBox->isChecked() && fiberBundle->GetNumberOfLines() == 0)
    {
      QMessageBox warnBox;
      warnBox.setWindowTitle("Warning");
      warnBox.setText("No fiberbundle was generated!");
      warnBox.setDetailedText("No fibers were generated using the chosen parameters. Typical reasons are:\n\n- Cutoff too high. Some images feature very low FA/GFA/peak size. Try to lower this parameter.\n- Angular threshold too strict. Try to increase this parameter.\n- A small step sizes also means many steps to go wrong. Especially in the case of probabilistic tractography. Try to adjust the angular threshold.");
      warnBox.setIcon(QMessageBox::Warning);
      warnBox.exec();

      if (m_InteractivePointSetNode.IsNotNull())
        m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
      StartStopTrackingGui(false);
      if (m_DeleteTrackingHandler)
        DeleteTrackingHandler();
      UpdateGui();

      return;
    }

    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(fiberBundle);
    fib->SetReferenceGeometry(dynamic_cast<mitk::Image*>(m_ParentNode->GetData())->GetGeometry());
    if (m_Controls->m_ResampleFibersBox->isChecked() && fiberBundle->GetNumberOfLines()>0)
      fib->Compress(m_Controls->m_FiberErrorBox->value());
    fib->ColorFibersByOrientation();
    m_Tracker->SetDicomProperties(fib);

    if (m_Controls->m_InteractiveBox->isChecked())
    {
      if (m_InteractiveNode.IsNull())
      {
        m_InteractiveNode = mitk::DataNode::New();
        QString name("Interactive");
        m_InteractiveNode->SetName(name.toStdString());
        GetDataStorage()->Add(m_InteractiveNode);
      }
      m_InteractiveNode->SetData(fib);
      m_InteractiveNode->SetFloatProperty("Fiber2DSliceThickness", m_Tracker->GetMinVoxelSize()/2);

      if (auto renderWindowPart = this->GetRenderWindowPart())
          renderWindowPart->RequestUpdate();
    }
    else
    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(fib);
      QString name("FiberBundle_");
      name += m_ParentNode->GetName().c_str();
      name += "_Streamline";
      node->SetName(name.toStdString());
      node->SetFloatProperty("Fiber2DSliceThickness", m_Tracker->GetMinVoxelSize()/2);
      GetDataStorage()->Add(node, m_ParentNode);
    }
  }
  else
  {
    TrackerType::ItkDoubleImgType::Pointer outImg = m_Tracker->GetOutputProbabilityMap();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    if (m_Controls->m_InteractiveBox->isChecked())
    {
      if (m_InteractiveNode.IsNull())
      {
        m_InteractiveNode = mitk::DataNode::New();
        QString name("Interactive");
        m_InteractiveNode->SetName(name.toStdString());
        GetDataStorage()->Add(m_InteractiveNode);
      }
      m_InteractiveNode->SetData(img);

      mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
      lut->SetType(mitk::LookupTable::JET_TRANSPARENT);
      mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
      lut_prop->SetLookupTable(lut);
      m_InteractiveNode->SetProperty("LookupTable", lut_prop);
      m_InteractiveNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
      m_InteractiveNode->SetFloatProperty("Fiber2DSliceThickness", m_Tracker->GetMinVoxelSize()/2);

      if (auto renderWindowPart = this->GetRenderWindowPart())
          renderWindowPart->RequestUpdate();
    }
    else
    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(img);
      QString name("ProbabilityMap_");
      name += m_ParentNode->GetName().c_str();
      node->SetName(name.toStdString());

      mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
      lut->SetType(mitk::LookupTable::JET_TRANSPARENT);
      mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
      lut_prop->SetLookupTable(lut);
      node->SetProperty("LookupTable", lut_prop);
      node->SetProperty("opacity", mitk::FloatProperty::New(0.5));

      GetDataStorage()->Add(node, m_ParentNode);
    }
  }
  if (m_InteractivePointSetNode.IsNotNull())
    m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
  StartStopTrackingGui(false);
  if (m_DeleteTrackingHandler)
    DeleteTrackingHandler();
  UpdateGui();
}

void QmitkStreamlineTrackingView::InteractiveSeedChanged(bool posChanged)
{
  if(!CheckAndStoreLastParams(sender()) && !posChanged)
    return;
  if (m_ThreadIsRunning || !m_Visible)
    return;

  if (!posChanged && (!m_Controls->m_InteractiveBox->isChecked() || !m_Controls->m_ParamUpdateBox->isChecked()) )
    return;

  std::srand(std::time(0));
  m_SeedPoints.clear();

  itk::Point<float> world_pos = this->GetRenderWindowPart()->GetSelectedPosition();

  m_SeedPoints.push_back(world_pos);
  float radius = m_Controls->m_SeedRadiusBox->value();
  int num = m_Controls->m_NumSeedsBox->value();

  mitk::PointSet::Pointer pointset = mitk::PointSet::New();
  pointset->InsertPoint(0, world_pos);
  m_InteractivePointSetNode->SetProperty("pointsize", mitk::FloatProperty::New(radius*2));
  m_InteractivePointSetNode->SetProperty("point 2D size", mitk::FloatProperty::New(radius*2));
  m_InteractivePointSetNode->SetData(pointset);

  for (int i=1; i<num; i++)
  {
    itk::Vector<float> p;
    p[0] = rand()%1000-500;
    p[1] = rand()%1000-500;
    p[2] = rand()%1000-500;
    p.Normalize();
    p *= radius;
    m_SeedPoints.push_back(world_pos+p);
  }
  m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,0,0));
  DoFiberTracking();
}

bool QmitkStreamlineTrackingView::CheckAndStoreLastParams(QObject* obj)
{
  if (obj!=nullptr)
  {
    std::string new_val = "";
    if(qobject_cast<QDoubleSpinBox*>(obj)!=nullptr)
      new_val = boost::lexical_cast<std::string>(qobject_cast<QDoubleSpinBox*>(obj)->value());
    else if (qobject_cast<QSpinBox*>(obj)!=nullptr)
      new_val = boost::lexical_cast<std::string>(qobject_cast<QSpinBox*>(obj)->value());

    if (m_LastTractoParams.find(obj->objectName())==m_LastTractoParams.end())
    {
      m_LastTractoParams[obj->objectName()] = new_val;
      return false;
    }
    else if (m_LastTractoParams.at(obj->objectName()) != new_val)
    {
      m_LastTractoParams[obj->objectName()] = new_val;
      return true;
    }
    else if (m_LastTractoParams.at(obj->objectName()) == new_val)
      return false;
  }
  return true;
}

void QmitkStreamlineTrackingView::OnParameterChanged()
{
  UpdateGui();

  if(!CheckAndStoreLastParams(sender()))
    return;

  if (m_Controls->m_InteractiveBox->isChecked() && m_Controls->m_ParamUpdateBox->isChecked())
    DoFiberTracking();
}

void QmitkStreamlineTrackingView::ToggleInteractive()
{
  UpdateGui();

  m_Controls->m_SeedsPerVoxelBox->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
  m_Controls->m_SeedsPerVoxelLabel->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
  m_Controls->m_SeedImageBox->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
  m_Controls->label_6->setEnabled(!m_Controls->m_InteractiveBox->isChecked());

  if ( m_Controls->m_InteractiveBox->isChecked() )
  {
    if (m_FirstInteractiveRun)
    {
      QMessageBox::information(nullptr, "Information", "Place and move a spherical seed region anywhere in the image by left-clicking and dragging. If the seed region is colored red, tracking is in progress. If the seed region is colored white, tracking is finished.\nPlacing the seed region for the first time in a newly selected dataset might cause a short delay, since the tracker needs to be initialized.");
      m_FirstInteractiveRun = false;
    }

    QApplication::setOverrideCursor(Qt::PointingHandCursor);
    QApplication::processEvents();

    m_InteractivePointSetNode = mitk::DataNode::New();
    m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
    m_InteractivePointSetNode->SetName("InteractiveSeedRegion");
    mitk::PointSetShapeProperty::Pointer shape_prop = mitk::PointSetShapeProperty::New();
    shape_prop->SetValue(mitk::PointSetShapeProperty::PointSetShape::CIRCLE);
    m_InteractivePointSetNode->SetProperty("Pointset.2D.shape", shape_prop);
    GetDataStorage()->Add(m_InteractivePointSetNode);


    m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
    connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
  else
  {
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();

    m_InteractiveNode = nullptr;
    m_InteractivePointSetNode = nullptr;

    m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
    disconnect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
}

void QmitkStreamlineTrackingView::Activated()
{

}

void QmitkStreamlineTrackingView::Deactivated()
{

}

void QmitkStreamlineTrackingView::Visible()
{
  m_Visible = true;
}

void QmitkStreamlineTrackingView::Hidden()
{
  m_Visible = false;
  m_Controls->m_InteractiveBox->setChecked(false);
  ToggleInteractive();
}

void QmitkStreamlineTrackingView::OnSliceChanged()
{
  InteractiveSeedChanged(true);
}

void QmitkStreamlineTrackingView::SetFocus()
{
}

void QmitkStreamlineTrackingView::DeleteTrackingHandler()
{
  if (!m_ThreadIsRunning && m_TrackingHandler != nullptr)
  {
    delete m_TrackingHandler;
    m_TrackingHandler = nullptr;
    m_DeleteTrackingHandler = false;
    m_LastPrior = "";
    if (m_TrackingPriorHandler != nullptr)
      delete m_TrackingPriorHandler;
  }
  else if (m_ThreadIsRunning)
  {
    m_DeleteTrackingHandler = true;
  }
}

void QmitkStreamlineTrackingView::ForestSwitched()
{
  DeleteTrackingHandler();
}

void QmitkStreamlineTrackingView::OutputStyleSwitched()
{
  if (m_InteractiveNode.IsNotNull())
    GetDataStorage()->Remove(m_InteractiveNode);
  m_InteractiveNode = nullptr;
}

void QmitkStreamlineTrackingView::OnSelectionChanged( berry::IWorkbenchPart::Pointer , const QList<mitk::DataNode::Pointer>& nodes )
{
  std::vector< mitk::DataNode::Pointer > last_nodes = m_InputImageNodes;
  m_InputImageNodes.clear();
  m_InputImages.clear();
  m_AdditionalInputImages.clear();
  bool retrack = false;

  for( auto node : nodes )
  {

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      if( dynamic_cast<mitk::TensorImage*>(node->GetData()) )
      {
        m_InputImageNodes.push_back(node);
        m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
        retrack = true;
      }
      else if ( dynamic_cast<mitk::OdfImage*>(node->GetData()) )
      {
        m_InputImageNodes.push_back(node);
        m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
        retrack = true;
      }
      else if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(node->GetData())) )
      {
        m_InputImageNodes.push_back(node);
        m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
        retrack = true;
      }
      else
      {
        mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
        if (img!=nullptr)
        {
          int dim = img->GetDimension();
          unsigned int* dimensions = img->GetDimensions();
          if (dim==4 && dimensions[3]%3==0)
          {
            m_InputImageNodes.push_back(node);
            m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
            retrack = true;
          }
          else if (dim==3)
          {
            m_AdditionalInputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
          }
        }
      }
    }
  }

  // sometimes the OnSelectionChanged event is sent twice and actually no selection has changed for the first event. We need to catch that.
  if (last_nodes.size() == m_InputImageNodes.size())
  {
    bool same_nodes = true;
    for (unsigned int i=0; i<m_InputImageNodes.size(); i++)
      if (last_nodes.at(i)!=m_InputImageNodes.at(i))
      {
        same_nodes = false;
        break;
      }
    if (same_nodes)
      return;
  }

  DeleteTrackingHandler();
  UpdateGui();
  if (retrack)
    OnParameterChanged();
}

void QmitkStreamlineTrackingView::UpdateGui()
{
  m_Controls->m_TensorImageLabel->setText("<font color='red'>select in data-manager</font>");

  m_Controls->m_fBox->setEnabled(false);
  m_Controls->m_fLabel->setEnabled(false);
  m_Controls->m_gBox->setEnabled(false);
  m_Controls->m_gLabel->setEnabled(false);
  m_Controls->m_FaImageBox->setEnabled(true);
  m_Controls->mFaImageLabel->setEnabled(true);
  m_Controls->m_OdfCutoffBox->setEnabled(false);
  m_Controls->m_OdfCutoffLabel->setEnabled(false);
  m_Controls->m_SharpenOdfsBox->setEnabled(false);
  m_Controls->m_ForestBox->setVisible(false);
  m_Controls->m_ForestLabel->setVisible(false);
  m_Controls->commandLinkButton->setEnabled(false);
  m_Controls->m_TrialsPerSeedBox->setEnabled(false);
  m_Controls->m_TrialsPerSeedLabel->setEnabled(false);
  m_Controls->m_TargetImageBox->setEnabled(false);
  m_Controls->m_TargetImageLabel->setEnabled(false);

  if (m_Controls->m_InteractiveBox->isChecked())
  {
    m_Controls->m_InteractiveSeedingFrame->setVisible(true);
    m_Controls->m_StaticSeedingFrame->setVisible(false);
    m_Controls->commandLinkButton_2->setVisible(false);
    m_Controls->commandLinkButton->setVisible(false);
  }
  else
  {
    m_Controls->m_InteractiveSeedingFrame->setVisible(false);
    m_Controls->m_StaticSeedingFrame->setVisible(true);
    m_Controls->commandLinkButton_2->setVisible(m_ThreadIsRunning);
    m_Controls->commandLinkButton->setVisible(!m_ThreadIsRunning);
  }

  if (m_Controls->m_EpConstraintsBox->currentIndex()>0)
  {
    m_Controls->m_TargetImageBox->setEnabled(true);
    m_Controls->m_TargetImageLabel->setEnabled(true);
  }

  // trials per seed are only important for probabilistic tractography
  if (m_Controls->m_ModeBox->currentIndex()==1)
  {
    m_Controls->m_TrialsPerSeedBox->setEnabled(true);
    m_Controls->m_TrialsPerSeedLabel->setEnabled(true);
  }

  if(!m_InputImageNodes.empty())
  {
    if (m_InputImageNodes.size()>1)
      m_Controls->m_TensorImageLabel->setText( ( std::to_string(m_InputImageNodes.size()) + " images selected").c_str() );
    else
      m_Controls->m_TensorImageLabel->setText(m_InputImageNodes.at(0)->GetName().c_str());
    m_Controls->commandLinkButton->setEnabled(!m_Controls->m_InteractiveBox->isChecked() && !m_ThreadIsRunning);

    m_Controls->m_ScalarThresholdBox->setEnabled(true);
    m_Controls->m_FaThresholdLabel->setEnabled(true);

    if ( dynamic_cast<mitk::TensorImage*>(m_InputImageNodes.at(0)->GetData()) )
    {
      m_Controls->m_fBox->setEnabled(true);
      m_Controls->m_fLabel->setEnabled(true);
      m_Controls->m_gBox->setEnabled(true);
      m_Controls->m_gLabel->setEnabled(true);
    }
    else if ( dynamic_cast<mitk::OdfImage*>(m_InputImageNodes.at(0)->GetData()) )
    {
      m_Controls->m_OdfCutoffBox->setEnabled(true);
      m_Controls->m_OdfCutoffLabel->setEnabled(true);
      m_Controls->m_SharpenOdfsBox->setEnabled(true);
    }
    else if (  mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(m_InputImageNodes.at(0)->GetData())) )
    {
      m_Controls->m_ForestBox->setVisible(true);
      m_Controls->m_ForestLabel->setVisible(true);
      m_Controls->m_ScalarThresholdBox->setEnabled(false);
      m_Controls->m_FaThresholdLabel->setEnabled(false);
    }
  }
}

void QmitkStreamlineTrackingView::StartStopTrackingGui(bool start)
{
  m_ThreadIsRunning = start;

  if (!m_Controls->m_InteractiveBox->isChecked())
  {
    m_Controls->commandLinkButton_2->setVisible(start);
    m_Controls->commandLinkButton->setVisible(!start);
    m_Controls->m_InteractiveBox->setEnabled(!start);
    m_Controls->m_StatusTextBox->setVisible(start);
  }
}

void QmitkStreamlineTrackingView::DoFiberTracking()
{
  if (m_InputImages.empty())
  {
    QMessageBox::information(nullptr, "Information", "Please select an input image in the datamaneger (tensor, ODF, peak or dMRI image)!");
    return;
  }
  if (m_ThreadIsRunning || !m_Visible)
    return;
  if (m_Controls->m_InteractiveBox->isChecked() && m_SeedPoints.empty())
    return;
  StartStopTrackingGui(true);

  m_Tracker = TrackerType::New();

  if( dynamic_cast<mitk::TensorImage*>(m_InputImageNodes.at(0)->GetData()) )
  {
    if (m_Controls->m_ModeBox->currentIndex()==1)
    {
      if (m_InputImages.size()>1)
      {
        QMessageBox::information(nullptr, "Information", "Probabilistic tensor tractography is only implemented for single-tensor mode!");
        StartStopTrackingGui(false);
        return;
      }

      if (m_TrackingHandler==nullptr)
      {
        m_TrackingHandler = new mitk::TrackingHandlerOdf();
        mitk::TensorImage::ItkTensorImageType::Pointer itkImg = mitk::TensorImage::ItkTensorImageType::New();
        mitk::CastToItkImage(m_InputImages.at(0), itkImg);

        typedef itk::TensorImageToOdfImageFilter< float, float > FilterType;
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput( itkImg );
        filter->Update();
        dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfImage(filter->GetOutput());

        if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
        {
          ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
          mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);

          dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetGfaImage(itkImg);
        }
      }

      dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetGfaThreshold(m_Controls->m_ScalarThresholdBox->value());
      dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfThreshold(0);
      dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetSharpenOdfs(true);
      dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetIsOdfFromTensor(true);
    }
    else
    {
      if (m_TrackingHandler==nullptr)
      {
        m_TrackingHandler = new mitk::TrackingHandlerTensor();
        for (int i=0; i<(int)m_InputImages.size(); i++)
        {
          typedef mitk::ImageToItk< mitk::TrackingHandlerTensor::ItkTensorImageType > CasterType;
          CasterType::Pointer caster = CasterType::New();
          caster->SetInput(m_InputImages.at(i));
          caster->Update();
          mitk::TrackingHandlerTensor::ItkTensorImageType::ConstPointer itkImg = caster->GetOutput();

          dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->AddTensorImage(itkImg);
        }

        if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
        {
          ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
          mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);

          dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->SetFaImage(itkImg);
        }
      }

      dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->SetFaThreshold(m_Controls->m_ScalarThresholdBox->value());
      dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->SetF((float)m_Controls->m_fBox->value());
      dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->SetG((float)m_Controls->m_gBox->value());
    }
  }
  else if ( dynamic_cast<mitk::OdfImage*>(m_InputImageNodes.at(0)->GetData()) )
  {
    if (m_TrackingHandler==nullptr)
    {
      m_TrackingHandler = new mitk::TrackingHandlerOdf();
      mitk::TrackingHandlerOdf::ItkOdfImageType::Pointer itkImg = mitk::TrackingHandlerOdf::ItkOdfImageType::New();
      mitk::CastToItkImage(m_InputImages.at(0), itkImg);
      dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfImage(itkImg);

      if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
      {
        ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);
        dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetGfaImage(itkImg);
      }
    }

    dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetGfaThreshold(m_Controls->m_ScalarThresholdBox->value());
    dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfThreshold(m_Controls->m_OdfCutoffBox->value());
    dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetSharpenOdfs(m_Controls->m_SharpenOdfsBox->isChecked());
  }
  else if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())) )
  {
    if ( m_Controls->m_ForestBox->GetSelectedNode().IsNull() )
    {
      QMessageBox::information(nullptr, "Information", "Not random forest for machine learning based tractography (raw dMRI tractography) selected. Did you accidentally select the raw diffusion-weighted image in the datamanager?");
      StartStopTrackingGui(false);
      return;
    }

    if (m_TrackingHandler==nullptr)
    {
      mitk::TractographyForest::Pointer forest = dynamic_cast<mitk::TractographyForest*>(m_Controls->m_ForestBox->GetSelectedNode()->GetData());
      mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData());

      std::vector< std::vector< ItkFloatImageType::Pointer > > additionalFeatureImages;
      additionalFeatureImages.push_back(std::vector< ItkFloatImageType::Pointer >());
      for (auto img : m_AdditionalInputImages)
      {
        ItkFloatImageType::Pointer itkimg = ItkFloatImageType::New();
        mitk::CastToItkImage(img, itkimg);
        additionalFeatureImages.at(0).push_back(itkimg);
      }

      bool forest_valid = false;
      if (forest->GetNumFeatures()>=100)
      {
        int num_previous_directions = (forest->GetNumFeatures() - (100 + additionalFeatureImages.at(0).size()))/3;
        m_TrackingHandler = new mitk::TrackingHandlerRandomForest<6, 100>();
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->AddDwi(dwi);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->SetAdditionalFeatureImages(additionalFeatureImages);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->SetForest(forest);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->SetNumPreviousDirections(num_previous_directions);
        forest_valid = dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->IsForestValid();
      }
      else
      {
        int num_previous_directions = (forest->GetNumFeatures() - (28 + additionalFeatureImages.at(0).size()))/3;
        m_TrackingHandler = new mitk::TrackingHandlerRandomForest<6, 28>();
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->AddDwi(dwi);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->SetAdditionalFeatureImages(additionalFeatureImages);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->SetForest(forest);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->SetNumPreviousDirections(num_previous_directions);
        forest_valid = dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->IsForestValid();
      }

      if (!forest_valid)
      {
        QMessageBox::information(nullptr, "Information", "Random forest is invalid. The forest signatue does not match the parameters of TrackingHandlerRandomForest.");
        StartStopTrackingGui(false);
        return;
      }
    }
  }
  else
  {
    if (m_Controls->m_ModeBox->currentIndex()==1)
    {
      QMessageBox::information(nullptr, "Information", "Probabilstic tractography is not implemented for peak images.");
      StartStopTrackingGui(false);
      return;
    }
    try {

      if (m_TrackingHandler==nullptr)
      {
        typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(m_InputImages.at(0));
        caster->SetCopyMemFlag(true);
        caster->Update();
        mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();
        m_TrackingHandler = new mitk::TrackingHandlerPeaks();
        dynamic_cast<mitk::TrackingHandlerPeaks*>(m_TrackingHandler)->SetPeakImage(itkImg);
      }

      dynamic_cast<mitk::TrackingHandlerPeaks*>(m_TrackingHandler)->SetPeakThreshold(m_Controls->m_ScalarThresholdBox->value());
    }
    catch(...)
    {
      QMessageBox::information(nullptr, "Error", "Peak tracker could not be initialized. Is your input image in the correct format (4D float image, peaks in the 4th dimension)?");
      StartStopTrackingGui(false);
      return;
    }
  }

  m_TrackingHandler->SetFlipX(m_Controls->m_FlipXBox->isChecked());
  m_TrackingHandler->SetFlipY(m_Controls->m_FlipYBox->isChecked());
  m_TrackingHandler->SetFlipZ(m_Controls->m_FlipZBox->isChecked());
  m_TrackingHandler->SetInterpolate(m_Controls->m_InterpolationBox->isChecked());
  switch (m_Controls->m_ModeBox->currentIndex())
  {
  case 0:
    m_TrackingHandler->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);
    break;
  case 1:
    m_TrackingHandler->SetMode(mitk::TrackingDataHandler::MODE::PROBABILISTIC);
    break;
  default:
    m_TrackingHandler->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);
  }

  if (m_Controls->m_InteractiveBox->isChecked())
  {
    m_Tracker->SetSeedPoints(m_SeedPoints);
  }
  else if (m_Controls->m_SeedImageBox->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_SeedImageBox->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetSeedImage(mask);
  }

  if (m_Controls->m_MaskImageBox->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_MaskImageBox->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetMaskImage(mask);
  }

  if (m_Controls->m_StopImageBox->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_StopImageBox->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetStoppingRegions(mask);
  }

  if (m_Controls->m_TargetImageBox->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_TargetImageBox->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetTargetRegions(mask);
  }

  if (m_Controls->m_PriorImageBox->GetSelectedNode().IsNotNull())
  {
    if (m_LastPrior!=m_Controls->m_PriorImageBox->GetSelectedNode()->GetUID() || m_TrackingPriorHandler==nullptr)
    {
      typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(dynamic_cast<mitk::PeakImage*>(m_Controls->m_PriorImageBox->GetSelectedNode()->GetData()));
      caster->SetCopyMemFlag(true);
      caster->Update();
      mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();
      m_TrackingPriorHandler = new mitk::TrackingHandlerPeaks();
      dynamic_cast<mitk::TrackingHandlerPeaks*>(m_TrackingPriorHandler)->SetPeakImage(itkImg);
      dynamic_cast<mitk::TrackingHandlerPeaks*>(m_TrackingPriorHandler)->SetPeakThreshold(0.0);
      m_LastPrior = m_Controls->m_PriorImageBox->GetSelectedNode()->GetUID();
    }

    m_TrackingPriorHandler->SetInterpolate(m_Controls->m_InterpolationBox->isChecked());
    m_TrackingPriorHandler->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);

    m_Tracker->SetTrackingPriorHandler(m_TrackingPriorHandler);
    m_Tracker->SetTrackingPriorWeight(m_Controls->m_PriorWeightBox->value());
    m_Tracker->SetTrackingPriorAsMask(m_Controls->m_PriorAsMaskBox->isChecked());
    m_Tracker->SetIntroduceDirectionsFromPrior(m_Controls->m_NewDirectionsFromPriorBox->isChecked());
  }
  else if (m_Controls->m_PriorImageBox->GetSelectedNode().IsNull())
    m_Tracker->SetTrackingPriorHandler(nullptr);

  if (m_Controls->m_ExclusionImageBox->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_ExclusionImageBox->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetExclusionRegions(mask);
  }

  // Endpoint constraints
  switch (m_Controls->m_EpConstraintsBox->currentIndex())
  {
  case 0:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::NONE);
    m_Tracker->SetTargetRegions(nullptr);
    break;
  case 1:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET);
    break;
  case 2:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET_LABELDIFF);
    break;
  case 3:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_SEED_AND_TARGET);
    break;
  case 4:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::MIN_ONE_EP_IN_TARGET);
    break;
  case 5:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::ONE_EP_IN_TARGET);
    break;
  case 6:
    m_Tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::NO_EP_IN_TARGET);
    break;
  }

  if (m_Tracker->GetEndpointConstraint()!=itk::StreamlineTrackingFilter::EndpointConstraints::NONE && m_Controls->m_TargetImageBox->GetSelectedNode().IsNull())
  {
    QMessageBox::information(nullptr, "Error", "Endpoint constraints are used but no target image is set!");
    StartStopTrackingGui(false);
    return;
  }
  else if (m_Tracker->GetEndpointConstraint()==itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_SEED_AND_TARGET
           && (m_Controls->m_SeedImageBox->GetSelectedNode().IsNull()|| m_Controls->m_TargetImageBox->GetSelectedNode().IsNull()) )
  {
    QMessageBox::information(nullptr, "Error", "Endpoint constraint EPS_IN_SEED_AND_TARGET is used but no target or no seed image is set!");
    StartStopTrackingGui(false);
    return;
  }

  m_Tracker->SetInterpolateMasks(m_Controls->m_MaskInterpolationBox->isChecked());
  m_Tracker->SetVerbose(!m_Controls->m_InteractiveBox->isChecked());
  m_Tracker->SetSeedsPerVoxel(m_Controls->m_SeedsPerVoxelBox->value());
  m_Tracker->SetStepSize(m_Controls->m_StepSizeBox->value());
  m_Tracker->SetSamplingDistance(m_Controls->m_SamplingDistanceBox->value());
  m_Tracker->SetUseStopVotes(m_Controls->m_StopVotesBox->isChecked());
  m_Tracker->SetOnlyForwardSamples(m_Controls->m_FrontalSamplesBox->isChecked());
  m_Tracker->SetTrialsPerSeed(m_Controls->m_TrialsPerSeedBox->value());
  m_Tracker->SetMaxNumTracts(m_Controls->m_NumFibersBox->value());
  m_Tracker->SetNumberOfSamples(m_Controls->m_NumSamplesBox->value());
  m_Tracker->SetTrackingHandler(m_TrackingHandler);
  m_Tracker->SetLoopCheck(m_Controls->m_LoopCheckBox->value());
  m_Tracker->SetAngularThreshold(m_Controls->m_AngularThresholdBox->value());
  m_Tracker->SetMinTractLength(m_Controls->m_MinTractLengthBox->value());
  m_Tracker->SetUseOutputProbabilityMap(m_Controls->m_OutputProbMap->isChecked());

  m_ParentNode = m_InputImageNodes.at(0);
  m_TrackingThread.start(QThread::LowestPriority);
}
