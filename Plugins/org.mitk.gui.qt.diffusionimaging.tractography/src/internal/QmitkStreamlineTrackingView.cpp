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
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDimension.h>
#include <mitkQBallImage.h>
#include <mitkSliceNavigationController.h>

// VTK
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyLine.h>
#include <vtkCellData.h>

#include <itkTensorImageToQBallImageFilter.h>
#include <omp.h>


const std::string QmitkStreamlineTrackingView::VIEW_ID = "org.mitk.views.streamlinetracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkStreamlineTrackingView::QmitkStreamlineTrackingView()
  : m_Controls(nullptr)
{
}

// Destructor
QmitkStreamlineTrackingView::~QmitkStreamlineTrackingView()
{

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
    m_Controls->m_StopImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TissueImageBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();

    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer isNotBinaryPredicate = mitk::NodePredicateNot::New( isBinaryPredicate );
    mitk::NodePredicateAnd::Pointer isNotABinaryImagePredicate = mitk::NodePredicateAnd::New( isImagePredicate, isNotBinaryPredicate );
    mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);

    m_Controls->m_FaImageBox->SetPredicate( mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, dimensionPredicate) );
    m_Controls->m_FaImageBox->SetZeroEntryText("--");
    m_Controls->m_SeedImageBox->SetPredicate( mitk::NodePredicateAnd::New(isBinaryPredicate, dimensionPredicate) );
    m_Controls->m_SeedImageBox->SetZeroEntryText("--");
    m_Controls->m_MaskImageBox->SetPredicate( mitk::NodePredicateAnd::New(isBinaryPredicate, dimensionPredicate) );
    m_Controls->m_MaskImageBox->SetZeroEntryText("--");
    m_Controls->m_StopImageBox->SetPredicate( mitk::NodePredicateAnd::New(isBinaryPredicate, dimensionPredicate) );
    m_Controls->m_StopImageBox->SetZeroEntryText("--");
    m_Controls->m_TissueImageBox->SetPredicate( mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, dimensionPredicate) );
    m_Controls->m_TissueImageBox->SetZeroEntryText("--");

    connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
    connect( m_Controls->m_InteractiveBox, SIGNAL(stateChanged(int)), this, SLOT(ToggleInteractive()) );
    connect( m_Controls->m_TissueImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()) );

    m_FirstTensorProbRun = true;
  }

  UpdateGui();
}

void QmitkStreamlineTrackingView::ToggleInteractive()
{
  UpdateGui();

  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  if ( m_Controls->m_InteractiveBox->isChecked() )
  {
    m_InteractiveNode = mitk::DataNode::New();

    QString name("InteractiveFib");
    m_InteractiveNode->SetName(name.toStdString());
    GetDataStorage()->Add(m_InteractiveNode);

    m_InteractivePointSetNode = mitk::DataNode::New();
    m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
    m_InteractivePointSetNode->SetName("InteractiveSeedRegion");
    mitk::PointSetShapeProperty::Pointer shape_prop = mitk::PointSetShapeProperty::New();
    shape_prop->SetValue(mitk::PointSetShapeProperty::PointSetShape::CIRCLE);
    m_InteractivePointSetNode->SetProperty("Pointset.2D.shape", shape_prop);
    GetDataStorage()->Add(m_InteractivePointSetNode);

    if (renderWindow)
    {
      {
        mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("axial"))->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkStreamlineTrackingView>::Pointer command = itk::ReceptorMemberCommand<QmitkStreamlineTrackingView>::New();
        command->SetCallbackFunction( this, &QmitkStreamlineTrackingView::OnSliceChanged );
        m_SliceObserverTag1 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0), command );
      }

      {
        mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("sagittal"))->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkStreamlineTrackingView>::Pointer command = itk::ReceptorMemberCommand<QmitkStreamlineTrackingView>::New();
        command->SetCallbackFunction( this, &QmitkStreamlineTrackingView::OnSliceChanged );
        m_SliceObserverTag2 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0), command );
      }

      {
        mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("coronal"))->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkStreamlineTrackingView>::Pointer command = itk::ReceptorMemberCommand<QmitkStreamlineTrackingView>::New();
        command->SetCallbackFunction( this, &QmitkStreamlineTrackingView::OnSliceChanged );
        m_SliceObserverTag3 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0), command );
      }
    }
  }
  else
  {
    m_InteractiveNode = nullptr;
    m_InteractivePointSetNode = nullptr;

    mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("axial"))->GetSliceNavigationController();
    slicer->RemoveObserver(m_SliceObserverTag1);
    slicer = renderWindow->GetQmitkRenderWindow(QString("sagittal"))->GetSliceNavigationController();
    slicer->RemoveObserver(m_SliceObserverTag2);
    slicer = renderWindow->GetQmitkRenderWindow(QString("coronal"))->GetSliceNavigationController();
    slicer->RemoveObserver(m_SliceObserverTag3);
  }
}

void QmitkStreamlineTrackingView::OnSliceChanged(const itk::EventObject& /*e*/)
{
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

  DoFiberTracking();
}

void QmitkStreamlineTrackingView::SetFocus()
{
}

void QmitkStreamlineTrackingView::OnSelectionChanged( berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes )
{
  m_InputImageNodes.clear();
  m_InputImages.clear();

  for( auto node : nodes )
  {

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      if( dynamic_cast<mitk::TensorImage*>(node->GetData()) )
      {
        m_InputImageNodes.push_back(node);
        m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
      }
      else if ( dynamic_cast<mitk::QBallImage*>(node->GetData()) )
      {
        m_InputImageNodes.push_back(node);
        m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
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
          }
        }
      }
    }
  }

  UpdateGui();
}

void QmitkStreamlineTrackingView::UpdateGui()
{
  m_Controls->m_TensorImageLabel->setText("<font color='red'>mandatory</font>");

  m_Controls->m_fBox->setVisible(false);
  m_Controls->m_fLabel->setVisible(false);
  m_Controls->m_gBox->setVisible(false);
  m_Controls->m_gLabel->setVisible(false);
  m_Controls->m_FaImageBox->setVisible(false);
  m_Controls->mFaImageLabel->setVisible(false);
  m_Controls->m_NormalizeODFsBox->setVisible(false);

  if (m_Controls->m_TissueImageBox->GetSelectedNode().IsNotNull())
    m_Controls->m_SeedGmBox->setVisible(true);
  else
    m_Controls->m_SeedGmBox->setVisible(false);

  if(!m_InputImageNodes.empty())
  {
    if (m_InputImageNodes.size()>1)
      m_Controls->m_TensorImageLabel->setText(m_InputImageNodes.size()+" images selected");
    else
      m_Controls->m_TensorImageLabel->setText(m_InputImageNodes.at(0)->GetName().c_str());
    m_Controls->m_InputData->setTitle("Input Data");
    m_Controls->commandLinkButton->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
    m_Controls->m_InteractiveBox->setEnabled(true);

    if ( dynamic_cast<mitk::TensorImage*>(m_InputImageNodes.at(0)->GetData()) )
    {
      m_Controls->m_fBox->setVisible(true);
      m_Controls->m_fLabel->setVisible(true);
      m_Controls->m_gBox->setVisible(true);
      m_Controls->m_gLabel->setVisible(true);
      m_Controls->mFaImageLabel->setVisible(true);
      m_Controls->m_FaImageBox->setVisible(true);

      if (m_Controls->m_ModeBox->currentIndex()==1)
        m_Controls->m_NormalizeODFsBox->setVisible(true);
    }
    else if ( dynamic_cast<mitk::QBallImage*>(m_InputImageNodes.at(0)->GetData()) )
    {
      m_Controls->mFaImageLabel->setVisible(true);
      m_Controls->m_FaImageBox->setVisible(true);
      m_Controls->m_NormalizeODFsBox->setVisible(true);
    }
    else
    {

    }
  }
  else
  {
    m_Controls->m_InputData->setTitle("Please Select Input Data");
    m_Controls->commandLinkButton->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
    m_Controls->m_InteractiveBox->setEnabled(false);
  }

}

void QmitkStreamlineTrackingView::DoFiberTracking()
{
  if (m_InputImages.empty())
    return;

  mitk::TrackingDataHandler* trackingHandler;

  if( dynamic_cast<mitk::TensorImage*>(m_InputImageNodes.at(0)->GetData()) )
  {
    typedef itk::Image< itk::DiffusionTensor3D<float>, 3> TensorImageType;
    typedef mitk::ImageToItk<TensorImageType> CasterType;

    if (m_Controls->m_ModeBox->currentIndex()==1)
    {
      if (m_InputImages.size()>1)
      {
        QMessageBox::information(nullptr, "Information", "Probabilistic tensor tractography is only implemented for single-tensor mode!");
        return;
      }

      if (m_FirstTensorProbRun)
      {
        QMessageBox::information(nullptr, "Information", "Internally calculating ODF from tensor image and performing probabilistic ODF tractography. Please keep the state of the ODF normalization box (see advanced parameters) in mind. TEND parameters are ignored.");
        m_FirstTensorProbRun = false;
      }

      TensorImageType::Pointer itkImg = TensorImageType::New();
      mitk::CastToItkImage(m_InputImages.at(0), itkImg);

      typedef itk::TensorImageToQBallImageFilter< float, float > FilterType;
      FilterType::Pointer filter = FilterType::New();
      filter->SetInput( itkImg );
      filter->Update();

      typedef mitk::ImageToItk< mitk::TrackingHandlerOdf::ItkOdfImageType > CasterType;
      trackingHandler = new mitk::TrackingHandlerOdf();
      dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetOdfImage(filter->GetOutput());
      dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetGfaThreshold(m_Controls->m_ScalarThresholdBox->value());
      dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetMinMaxNormalize(m_Controls->m_NormalizeODFsBox->isChecked());
      dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetOdfPower(1);

      if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
      {
        ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);

        dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetGfaImage(itkImg);
      }
    }
    else
    {
      trackingHandler = new mitk::TrackingHandlerTensor();
      for (int i=0; i<(int)m_InputImages.size(); i++)
      {
        TensorImageType::Pointer itkImg = TensorImageType::New();
        mitk::CastToItkImage(m_InputImages.at(i), itkImg);

        dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->AddTensorImage(itkImg);
      }

      if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
      {
        ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);

        dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetFaImage(itkImg);
      }

      dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetFaThreshold(m_Controls->m_ScalarThresholdBox->value());
      dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetF((float)m_Controls->m_fBox->value());
      dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetG((float)m_Controls->m_gBox->value());
    }
  }
  else if ( dynamic_cast<mitk::QBallImage*>(m_InputImageNodes.at(0)->GetData()) )
  {
    typedef mitk::ImageToItk< mitk::TrackingHandlerOdf::ItkOdfImageType > CasterType;
    trackingHandler = new mitk::TrackingHandlerOdf();
    mitk::TrackingHandlerOdf::ItkOdfImageType::Pointer itkImg = mitk::TrackingHandlerOdf::ItkOdfImageType::New();
    mitk::CastToItkImage(m_InputImages.at(0), itkImg);
    dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetOdfImage(itkImg);
    dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetGfaThreshold(m_Controls->m_ScalarThresholdBox->value());
    dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetMinMaxNormalize(m_Controls->m_NormalizeODFsBox->isChecked());

    if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
    {
      ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
      mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);

      dynamic_cast<mitk::TrackingHandlerOdf*>(trackingHandler)->SetGfaImage(itkImg);
    }
  }
  else
  {
    if (m_Controls->m_ModeBox->currentIndex()==1)
    {
      QMessageBox::information(nullptr, "Information", "Probabilstic tractography is only implementedfor ODF images.");
      return;
    }
    try {
      typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(m_InputImages.at(0));
      caster->Update();
      mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();
      trackingHandler = new mitk::TrackingHandlerPeaks();
      dynamic_cast<mitk::TrackingHandlerPeaks*>(trackingHandler)->SetPeakImage(itkImg);
      dynamic_cast<mitk::TrackingHandlerPeaks*>(trackingHandler)->SetPeakThreshold(m_Controls->m_ScalarThresholdBox->value());
    }
    catch(...)
    {
      return;
    }
  }

  trackingHandler->SetFlipX(m_Controls->m_FlipXBox->isChecked());
  trackingHandler->SetFlipY(m_Controls->m_FlipYBox->isChecked());
  trackingHandler->SetFlipZ(m_Controls->m_FlipZBox->isChecked());
  trackingHandler->SetInterpolate(m_Controls->m_InterpolationBox->isChecked());
  switch (m_Controls->m_ModeBox->currentIndex())
  {
  case 0:
    trackingHandler->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);
    break;
  case 1:
    trackingHandler->SetMode(mitk::TrackingDataHandler::MODE::PROBABILISTIC);
    break;
  default:
    trackingHandler->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);
  }

  typedef itk::StreamlineTrackingFilter TrackerType;
  TrackerType::Pointer tracker = TrackerType::New();

  if (m_Controls->m_InteractiveBox->isChecked())
  {
    tracker->SetSeedPoints(m_SeedPoints);
  }
  else if (m_Controls->m_SeedImageBox->GetSelectedNode().IsNotNull())
  {
    ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_SeedImageBox->GetSelectedNode()->GetData()), mask);
    tracker->SetSeedImage(mask);
  }

  if (m_Controls->m_MaskImageBox->GetSelectedNode().IsNotNull())
  {
    ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_MaskImageBox->GetSelectedNode()->GetData()), mask);
    tracker->SetMaskImage(mask);
  }

  if (m_Controls->m_StopImageBox->GetSelectedNode().IsNotNull())
  {
    ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_StopImageBox->GetSelectedNode()->GetData()), mask);
    tracker->SetStoppingRegions(mask);
  }

  if (m_Controls->m_TissueImageBox->GetSelectedNode().IsNotNull())
  {
    ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_TissueImageBox->GetSelectedNode()->GetData()), mask);
    tracker->SetTissueImage(mask);
    tracker->SetSeedOnlyGm(m_Controls->m_SeedGmBox->isChecked());
  }

  tracker->SetSeedsPerVoxel(m_Controls->m_SeedsPerVoxelBox->value());
  tracker->SetStepSize(m_Controls->m_StepSizeBox->value());
  //tracker->SetSamplingDistance(0.7);
  tracker->SetUseStopVotes(false);
  tracker->SetOnlyForwardSamples(false);
  tracker->SetAposterioriCurvCheck(false);
  tracker->SetMaxNumTracts(m_Controls->m_NumFibersBox->value());
  tracker->SetNumberOfSamples(m_Controls->m_NumSamplesBox->value());
  tracker->SetTrackingHandler(trackingHandler);
  tracker->SetAngularThreshold(m_Controls->m_AngularThresholdBox->value());
  tracker->SetMinTractLength(m_Controls->m_MinTractLengthBox->value());
  tracker->SetUseOutputProbabilityMap(m_Controls->m_OutputProbMap->isChecked() && !m_Controls->m_InteractiveBox->isChecked());
  tracker->Update();

  delete trackingHandler;

  if (m_Controls->m_InteractiveBox->isChecked())
  {
    vtkSmartPointer<vtkPolyData> fiberBundle = tracker->GetFiberPolyData();
    if (m_InteractiveNode.IsNull())
    {
      m_InteractiveNode = mitk::DataNode::New();
      QString name("InteractiveFib");
      m_InteractiveNode->SetName(name.toStdString());
      GetDataStorage()->Add(m_InteractiveNode);
    }

    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(fiberBundle);
    fib->SetReferenceGeometry(dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())->GetGeometry());
    if (m_Controls->m_ResampleFibersBox->isChecked())
      fib->Compress(m_Controls->m_FiberErrorBox->value());

    m_InteractiveNode->SetData(fib);
  }
  else if (!tracker->GetUseOutputProbabilityMap())
  {
    vtkSmartPointer<vtkPolyData> fiberBundle = tracker->GetFiberPolyData();
    if (fiberBundle->GetNumberOfLines() == 0)
    {
      QMessageBox warnBox;
      warnBox.setWindowTitle("Warning");
      warnBox.setText("No fiberbundle was generated!");
      warnBox.setDetailedText("No fibers were generated using the parameters: \n\n" + m_Controls->m_FaThresholdLabel->text() + "\n" + m_Controls->m_AngularThresholdLabel->text() + "\n" + m_Controls->m_fLabel->text() + "\n" + m_Controls->m_gLabel->text() + "\n" + m_Controls->m_StepsizeLabel->text() + "\n" + m_Controls->m_MinTractLengthLabel->text() + "\n" + m_Controls->m_SeedsPerVoxelLabel->text() + "\n\nPlease check your parametersettings.");
      warnBox.setIcon(QMessageBox::Warning);
      warnBox.exec();
      return;
    }
    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(fiberBundle);
    fib->SetReferenceGeometry(dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())->GetGeometry());
    if (m_Controls->m_ResampleFibersBox->isChecked())
      fib->Compress(m_Controls->m_FiberErrorBox->value());
    fib->ColorFibersByOrientation();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(fib);
    QString name("FiberBundle_");
    name += m_InputImageNodes.at(0)->GetName().c_str();
    name += "_Streamline";
    node->SetName(name.toStdString());
    GetDataStorage()->Add(node, m_InputImageNodes.at(0));
  }
  else
  {
    TrackerType::ItkDoubleImgType::Pointer outImg = tracker->GetOutputProbabilityMap();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(img);
    QString name("ProbabilityMap_");
    name += m_InputImageNodes.at(0)->GetName().c_str();
    node->SetName(name.toStdString());

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType(mitk::LookupTable::JET_TRANSPARENT);
    mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
    lut_prop->SetLookupTable(lut);
    node->SetProperty("LookupTable", lut_prop);
    node->SetProperty("opacity", mitk::FloatProperty::New(0.5));

    GetDataStorage()->Add(node, m_InputImageNodes.at(0));
  }
}
