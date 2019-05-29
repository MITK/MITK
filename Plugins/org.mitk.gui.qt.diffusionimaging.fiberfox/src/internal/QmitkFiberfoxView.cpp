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

// Qmitk
#include "QmitkFiberfoxView.h"

// MITK
#include <mitkImage.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkDataStorage.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorImage.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageGenerator.h>
#include <mitkNodePredicateDataType.h>
#include <itkScalableAffineTransform.h>
#include <mitkLevelWindowProperty.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateIsDWI.h>
#include <boost/property_tree/ptree.hpp>
#define RAPIDXML_NO_EXCEPTIONS
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include "usModuleRegistry.h"
#include <mitkChiSquareNoiseModel.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <QScrollBar>
#include <itkInvertIntensityImageFilter.h>
#include <QDialogButtonBox>
#include <itkAdcImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <mitkITKImageImport.h>

#include "mitkNodePredicateDataType.h"
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>

QmitkFiberfoxWorker::QmitkFiberfoxWorker(QmitkFiberfoxView* view)
  : m_View(view)
{

}

void QmitkFiberfoxWorker::run()
{
  try{
    m_View->m_TractsToDwiFilter->Update();
  }
  catch( ... )
  {

  }
  m_View->m_Thread.quit();
}

const std::string QmitkFiberfoxView::VIEW_ID = "org.mitk.views.fiberfoxview";

QmitkFiberfoxView::QmitkFiberfoxView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_SelectedImageNode( nullptr )
  , m_Worker(this)
  , m_ThreadIsRunning(false)
{
  m_Worker.moveToThread(&m_Thread);
  connect(&m_Thread, SIGNAL(started()), this, SLOT(BeforeThread()));
  connect(&m_Thread, SIGNAL(started()), &m_Worker, SLOT(run()));
  connect(&m_Thread, SIGNAL(finished()), this, SLOT(AfterThread()));
  //    connect(&m_Thread, SIGNAL(terminated()), this, SLOT(AfterThread()));
  m_SimulationTimer = new QTimer(this);
}

void QmitkFiberfoxView::KillThread()
{
  MITK_INFO << "Aborting DWI simulation.";
  m_TractsToDwiFilter->SetAbortGenerateData(true);
  m_Controls->m_AbortSimulationButton->setEnabled(false);
  m_Controls->m_AbortSimulationButton->setText("Aborting simulation ...");
}

void QmitkFiberfoxView::BeforeThread()
{
  m_SimulationTime = QTime::currentTime();
  m_SimulationTimer->start(100);
  m_Controls->m_AbortSimulationButton->setVisible(true);
  m_Controls->m_GenerateImageButton->setVisible(false);
  m_Controls->m_SimulationStatusText->setVisible(true);
  m_ThreadIsRunning = true;
}

void QmitkFiberfoxView::AfterThread()
{
  UpdateSimulationStatus();
  m_SimulationTimer->stop();
  m_Controls->m_AbortSimulationButton->setVisible(false);
  m_Controls->m_AbortSimulationButton->setEnabled(true);
  m_Controls->m_AbortSimulationButton->setText("Abort simulation");
  m_Controls->m_GenerateImageButton->setVisible(true);
  m_ThreadIsRunning = false;

  QString statusText;
  FiberfoxParameters parameters;
  mitk::Image::Pointer mitkImage = mitk::Image::New();

  statusText = QString(m_TractsToDwiFilter->GetStatusText().c_str());
  if (m_TractsToDwiFilter->GetAbortGenerateData())
  {
    MITK_INFO << "Simulation aborted.";
    return;
  }

  parameters = m_TractsToDwiFilter->GetParameters();

  mitkImage = mitk::GrabItkImageMemory( m_TractsToDwiFilter->GetOutput() );
  if (parameters.m_SignalGen.GetNumWeightedVolumes() > 0)
  {
    mitk::DiffusionPropertyHelper::SetGradientContainer(mitkImage, parameters.m_SignalGen.GetItkGradientContainer());
    mitk::DiffusionPropertyHelper::SetReferenceBValue(mitkImage, parameters.m_SignalGen.GetBvalue());
    mitk::DiffusionPropertyHelper::InitializeImage( mitkImage );
  }
  parameters.m_Misc.m_ResultNode->SetData( mitkImage );

  GetDataStorage()->Add(parameters.m_Misc.m_ResultNode, parameters.m_Misc.m_ParentNode);

  if (m_Controls->m_VolumeFractionsBox->isChecked())
  {
    if (m_TractsToDwiFilter->GetTickImage().IsNotNull())
    {
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::Float2DImageType::Pointer itkImage = m_TractsToDwiFilter->GetTickImage();
      mitkImage = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData( mitkImage );
      node->SetName("Tick Image");
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
    }

    if (m_TractsToDwiFilter->GetRfImage().IsNotNull())
    {
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::Float2DImageType::Pointer itkImage = m_TractsToDwiFilter->GetRfImage();
      mitkImage = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData( mitkImage );
      node->SetName("RF Image");
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
    }

    if (m_TractsToDwiFilter->GetPhaseImage().IsNotNull())
    {
      mitk::Image::Pointer phaseImage = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkPhase = m_TractsToDwiFilter->GetPhaseImage();
      phaseImage = mitk::GrabItkImageMemory( itkPhase.GetPointer() );
      mitk::DataNode::Pointer phaseNode = mitk::DataNode::New();
      phaseNode->SetData( phaseImage );
      phaseNode->SetName("Phase Image");
      GetDataStorage()->Add(phaseNode, parameters.m_Misc.m_ResultNode);
    }

    if (m_TractsToDwiFilter->GetKspaceImage().IsNotNull())
    {
      mitk::Image::Pointer image = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkImage = m_TractsToDwiFilter->GetKspaceImage();
      image = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData( image );
      node->SetName("k-Space");
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
    }

    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(m_TractsToDwiFilter->GetCoilPointset());
      node->SetName("Coil Positions");
      node->SetProperty("pointsize", mitk::FloatProperty::New(parameters.m_SignalGen.m_ImageSpacing[0]/4));
      node->SetProperty("color", mitk::ColorProperty::New(0, 1, 0));
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
    }

    int c = 1;
    std::vector< itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer > output_real = m_TractsToDwiFilter->GetOutputImagesReal();
    for (auto real : output_real)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(real.GetPointer());
      image->SetVolume(real->GetBufferPointer());

      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData( image );
      node->SetName("Coil-"+QString::number(c).toStdString()+"-real");
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
      ++c;
    }

    c = 1;
    std::vector< itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer > output_imag = m_TractsToDwiFilter->GetOutputImagesImag();
    for (auto imag : output_imag)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(imag.GetPointer());
      image->SetVolume(imag->GetBufferPointer());

      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData( image );
      node->SetName("Coil-"+QString::number(c).toStdString()+"-imag");
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
      ++c;
    }

    std::vector< itk::TractsToDWIImageFilter< short >::ItkDoubleImgType::Pointer > volumeFractions = m_TractsToDwiFilter->GetVolumeFractions();
    for (unsigned int k=0; k<volumeFractions.size(); k++)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(volumeFractions.at(k).GetPointer());
      image->SetVolume(volumeFractions.at(k)->GetBufferPointer());

      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData( image );
      node->SetName("CompartmentVolume-"+QString::number(k).toStdString());
      GetDataStorage()->Add(node, parameters.m_Misc.m_ResultNode);
    }
  }
  m_TractsToDwiFilter = nullptr;

  if (parameters.m_Misc.m_AfterSimulationMessage.size()>0)
    QMessageBox::information( nullptr, "Warning", parameters.m_Misc.m_AfterSimulationMessage.c_str());

  mitk::BaseData::Pointer basedata = parameters.m_Misc.m_ResultNode->GetData();
  if (basedata.IsNotNull())
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(
          basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  if (!parameters.m_Misc.m_OutputPath.empty())
  {
    try{
      QString outputFileName(parameters.m_Misc.m_OutputPath.c_str());
      outputFileName += parameters.m_Misc.m_ResultNode->GetName().c_str();
      outputFileName.replace(QString("."), QString("_"));
      SaveParameters(outputFileName+".ffp");
      outputFileName += ".dwi";
      QString status("Saving output image to ");
      status += outputFileName;
      m_Controls->m_SimulationStatusText->append(status);
      mitk::IOUtil::Save(mitkImage, outputFileName.toStdString());
      m_Controls->m_SimulationStatusText->append("File saved successfully.");

    }
    catch (itk::ExceptionObject &e)
    {
      QString status("Exception during DWI writing: ");
      status += e.GetDescription();
      m_Controls->m_SimulationStatusText->append(status);
    }
    catch (...)
    {
      m_Controls->m_SimulationStatusText->append("Unknown exception during DWI writing!");
    }
  }
  parameters.m_SignalGen.m_FrequencyMap = nullptr;
}

void QmitkFiberfoxView::UpdateSimulationStatus()
{
  QString statusText = QString(m_TractsToDwiFilter->GetStatusText().c_str());

  if (QString::compare(m_SimulationStatusText,statusText)!=0)
  {
    m_Controls->m_SimulationStatusText->clear();
    m_Controls->m_SimulationStatusText->setText(statusText);
    QScrollBar *vScrollBar = m_Controls->m_SimulationStatusText->verticalScrollBar();
    vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
  }
}

// Destructor
QmitkFiberfoxView::~QmitkFiberfoxView()
{
  delete m_SimulationTimer;
}

void QmitkFiberfoxView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberfoxViewControls;
    m_Controls->setupUi( parent );

    m_Controls->m_StickWidget1->setVisible(true);
    m_Controls->m_StickWidget2->setVisible(false);
    m_Controls->m_ZeppelinWidget1->setVisible(false);
    m_Controls->m_ZeppelinWidget2->setVisible(false);
    m_Controls->m_TensorWidget1->setVisible(false);
    m_Controls->m_TensorWidget2->setVisible(false);

    m_Controls->m_BallWidget1->setVisible(true);
    m_Controls->m_BallWidget2->setVisible(false);
    m_Controls->m_BallWidget2->SetT1(4658);
    m_Controls->m_BallWidget2->SetT2(2200);
    m_Controls->m_AstrosticksWidget1->setVisible(false);
    m_Controls->m_AstrosticksWidget2->setVisible(false);
    m_Controls->m_AstrosticksWidget2->SetT1(4658);
    m_Controls->m_AstrosticksWidget2->SetT2(2200);
    m_Controls->m_DotWidget1->setVisible(false);
    m_Controls->m_DotWidget2->setVisible(false);
    m_Controls->m_DotWidget2->SetT1(4658);
    m_Controls->m_DotWidget2->SetT2(2200);

    m_Controls->m_PrototypeWidget1->setVisible(false);
    m_Controls->m_PrototypeWidget2->setVisible(false);
    m_Controls->m_PrototypeWidget3->setVisible(false);
    m_Controls->m_PrototypeWidget4->setVisible(false);

    m_Controls->m_PrototypeWidget3->SetMinFa(0.0);
    m_Controls->m_PrototypeWidget3->SetMaxFa(0.15);
    m_Controls->m_PrototypeWidget4->SetMinFa(0.0);
    m_Controls->m_PrototypeWidget4->SetMaxFa(0.15);
    m_Controls->m_PrototypeWidget3->SetMinAdc(0.0);
    m_Controls->m_PrototypeWidget3->SetMaxAdc(0.001);
    m_Controls->m_PrototypeWidget4->SetMinAdc(0.003);
    m_Controls->m_PrototypeWidget4->SetMaxAdc(0.004);

    m_Controls->m_Comp2FractionFrame->setVisible(false);
    m_Controls->m_Comp4FractionFrame->setVisible(false);
    m_Controls->m_DiffusionPropsMessage->setVisible(false);
    m_Controls->m_GeometryMessage->setVisible(false);
    m_Controls->m_AdvancedSignalOptionsFrame->setVisible(false);
    m_Controls->m_NoiseFrame->setVisible(false);
    m_Controls->m_ZeroRinging->setVisible(false);
    m_Controls->m_GhostFrame->setVisible(false);
    m_Controls->m_DistortionsFrame->setVisible(false);
    m_Controls->m_EddyFrame->setVisible(false);
    m_Controls->m_SpikeFrame->setVisible(false);
    m_Controls->m_AliasingFrame->setVisible(false);
    m_Controls->m_MotionArtifactFrame->setVisible(false);
    m_Controls->m_DriftFrame->setVisible(false);
    m_ParameterFile = QDir::currentPath()+"/param.ffp";

    m_Controls->m_AbortSimulationButton->setVisible(false);
    m_Controls->m_SimulationStatusText->setVisible(false);

    m_Controls->m_FrequencyMapBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_Comp1VolumeFraction->SetDataStorage(this->GetDataStorage());
    m_Controls->m_Comp2VolumeFraction->SetDataStorage(this->GetDataStorage());
    m_Controls->m_Comp3VolumeFraction->SetDataStorage(this->GetDataStorage());
    m_Controls->m_Comp4VolumeFraction->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MaskComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TemplateComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_FiberBundleComboBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::FiberBundle>::Pointer isFiberBundle = mitk::TNodePredicateDataType<mitk::FiberBundle>::New();
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateIsDWI::Pointer isDwi = mitk::NodePredicateIsDWI::New( );
    mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
    mitk::NodePredicateDataType::Pointer isOdf = mitk::NodePredicateDataType::New("Odfmage");
    mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
    isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isOdf);
    mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
    mitk::NodePredicateAnd::Pointer isNonDiffMitkImage = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateAnd::Pointer isBinaryMitkImage = mitk::NodePredicateAnd::New( isNonDiffMitkImage, isBinaryPredicate );

    m_Controls->m_FrequencyMapBox->SetPredicate(isNonDiffMitkImage);
    m_Controls->m_Comp1VolumeFraction->SetPredicate(isNonDiffMitkImage);
    m_Controls->m_Comp1VolumeFraction->SetZeroEntryText("--");
    m_Controls->m_Comp2VolumeFraction->SetPredicate(isNonDiffMitkImage);
    m_Controls->m_Comp2VolumeFraction->SetZeroEntryText("--");
    m_Controls->m_Comp3VolumeFraction->SetPredicate(isNonDiffMitkImage);
    m_Controls->m_Comp3VolumeFraction->SetZeroEntryText("--");
    m_Controls->m_Comp4VolumeFraction->SetPredicate(isNonDiffMitkImage);
    m_Controls->m_Comp4VolumeFraction->SetZeroEntryText("--");
    m_Controls->m_MaskComboBox->SetPredicate(isBinaryMitkImage);
    m_Controls->m_MaskComboBox->SetZeroEntryText("--");
    m_Controls->m_TemplateComboBox->SetPredicate(isMitkImage);
    m_Controls->m_TemplateComboBox->SetZeroEntryText("--");
    m_Controls->m_FiberBundleComboBox->SetPredicate(isFiberBundle);
    m_Controls->m_FiberBundleComboBox->SetZeroEntryText("--");

    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(7);
    m_Controls->m_SimulationStatusText->setFont(font);

    connect( m_SimulationTimer, SIGNAL(timeout()), this, SLOT(UpdateSimulationStatus()) );
    connect((QObject*) m_Controls->m_AbortSimulationButton, SIGNAL(clicked()), (QObject*) this, SLOT(KillThread()));
    connect((QObject*) m_Controls->m_GenerateImageButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateImage()));
    connect((QObject*) m_Controls->m_AddNoise, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddNoise(int)));
    connect((QObject*) m_Controls->m_AddGhosts, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddGhosts(int)));
    connect((QObject*) m_Controls->m_AddDistortions, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddDistortions(int)));
    connect((QObject*) m_Controls->m_AddEddy, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddEddy(int)));
    connect((QObject*) m_Controls->m_AddSpikes, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddSpikes(int)));
    connect((QObject*) m_Controls->m_AddAliasing, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddAliasing(int)));
    connect((QObject*) m_Controls->m_AddMotion, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddMotion(int)));
    connect((QObject*) m_Controls->m_AddDrift, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddDrift(int)));
    connect((QObject*) m_Controls->m_AddGibbsRinging, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddRinging(int)));

    connect((QObject*) m_Controls->m_Compartment1Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp1ModelFrameVisibility(int)));
    connect((QObject*) m_Controls->m_Compartment2Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp2ModelFrameVisibility(int)));
    connect((QObject*) m_Controls->m_Compartment3Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp3ModelFrameVisibility(int)));
    connect((QObject*) m_Controls->m_Compartment4Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp4ModelFrameVisibility(int)));

    connect((QObject*) m_Controls->m_AdvancedOptionsBox_2, SIGNAL( stateChanged(int)), (QObject*) this, SLOT(ShowAdvancedOptions(int)));
    connect((QObject*) m_Controls->m_UseBvalsBvecsBox, SIGNAL( stateChanged(int)), (QObject*) this, SLOT(OnBvalsBvecsCheck(int)));

    connect((QObject*) m_Controls->m_SaveParametersButton, SIGNAL(clicked()), (QObject*) this, SLOT(SaveParameters()));
    connect((QObject*) m_Controls->m_LoadParametersButton, SIGNAL(clicked()), (QObject*) this, SLOT(LoadParameters()));
    connect((QObject*) m_Controls->m_OutputPathButton, SIGNAL(clicked()), (QObject*) this, SLOT(SetOutputPath()));
    connect((QObject*) m_Controls->m_LoadBvalsButton, SIGNAL(clicked()), (QObject*) this, SLOT(SetBvalsEdit()));
    connect((QObject*) m_Controls->m_LoadBvecsButton, SIGNAL(clicked()), (QObject*) this, SLOT(SetBvecsEdit()));

    connect((QObject*) m_Controls->m_MaskComboBox, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(OnMaskSelected(int)));
    connect((QObject*) m_Controls->m_TemplateComboBox, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(OnTemplateSelected(int)));
    connect((QObject*) m_Controls->m_FiberBundleComboBox, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(OnFibSelected(int)));
    connect((QObject*) m_Controls->m_LineReadoutTimeBox, SIGNAL( valueChanged(double)), (QObject*) this, SLOT(OnTlineChanged()));
    connect((QObject*) m_Controls->m_SizeX, SIGNAL( valueChanged(int)), (QObject*) this, SLOT(OnTlineChanged()));
  }
  OnTlineChanged();
  UpdateGui();
}

void QmitkFiberfoxView::OnTlineChanged()
{
  double num_pix_line = 0;
  if (m_Controls->m_TemplateComboBox->GetSelectedNode().IsNotNull())   // use geometry of selected image
  {
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData());
    num_pix_line = img->GetDimension(0);
  }
  else if (m_Controls->m_MaskComboBox->GetSelectedNode().IsNotNull())   // use geometry of mask image
  {
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_MaskComboBox->GetSelectedNode()->GetData());
    num_pix_line = img->GetDimension(0);
  }
  else
  {
     num_pix_line = m_Controls->m_SizeX->value();
  }

  double value = static_cast<double>(m_Controls->m_LineReadoutTimeBox->value())/1000.0;  // line readout time in seconds
  double dweel_pp = value/num_pix_line; // pixel readout time in seconds
  double bw = 1/dweel_pp;
  double bw_pp = bw/num_pix_line;
  std::string tt = "Bandwidth:\n" + boost::lexical_cast<std::string>(itk::Math::Round<int, double>(bw)) + "Hz\n" + boost::lexical_cast<std::string>(itk::Math::Round<int, double>(bw_pp)) + "Hz/Px";
  m_Controls->m_LineReadoutTimeBox->setToolTip(tt.c_str());
}

void QmitkFiberfoxView::OnMaskSelected(int )
{
  UpdateGui();
}

void QmitkFiberfoxView::OnTemplateSelected(int )
{
  UpdateGui();
}

void QmitkFiberfoxView::OnFibSelected(int )
{
  UpdateGui();
}

void QmitkFiberfoxView::OnBvalsBvecsCheck(int )
{
  UpdateGui();
}

void QmitkFiberfoxView::UpdateParametersFromGui()
{
  m_Parameters.ClearSignalParameters();
  m_Parameters.m_Misc.m_CheckAdvancedSignalOptionsBox = m_Controls->m_AdvancedOptionsBox_2->isChecked();
  m_Parameters.m_Misc.m_OutputAdditionalImages = m_Controls->m_VolumeFractionsBox->isChecked();

  std::string outputPath = m_Controls->m_SavePathEdit->text().toStdString();
  if (outputPath.compare("-")!=0)
  {
    m_Parameters.m_Misc.m_OutputPath = outputPath;
    m_Parameters.m_Misc.m_OutputPath += "/";
  }
  else {
    m_Parameters.m_Misc.m_OutputPath = "";
  }

  if (m_Controls->m_MaskComboBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image::Pointer mitkMaskImage = dynamic_cast<mitk::Image*>(m_Controls->m_MaskComboBox->GetSelectedNode()->GetData());
    mitk::CastToItkImage<ItkUcharImgType>(mitkMaskImage, m_Parameters.m_SignalGen.m_MaskImage);
    itk::ImageDuplicator<ItkUcharImgType>::Pointer duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
    duplicator->SetInputImage(m_Parameters.m_SignalGen.m_MaskImage);
    duplicator->Update();
    m_Parameters.m_SignalGen.m_MaskImage = duplicator->GetOutput();
  }

  if (m_Controls->m_TemplateComboBox->GetSelectedNode().IsNotNull() && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( m_Controls->m_TemplateComboBox->GetSelectedNode()))  // use parameters of selected DWI
  {
    mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData());

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(dwi, itkVectorImagePointer);

    m_Parameters.m_SignalGen.m_ImageRegion = itkVectorImagePointer->GetLargestPossibleRegion();
    m_Parameters.m_SignalGen.m_ImageSpacing = itkVectorImagePointer->GetSpacing();
    m_Parameters.m_SignalGen.m_ImageOrigin = itkVectorImagePointer->GetOrigin();
    m_Parameters.m_SignalGen.m_ImageDirection = itkVectorImagePointer->GetDirection();
    m_Parameters.SetBvalue(mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi));
    m_Parameters.SetGradienDirections(mitk::DiffusionPropertyHelper::GetOriginalGradientContainer(dwi));
  }
  else if (m_Controls->m_TemplateComboBox->GetSelectedNode().IsNotNull())   // use geometry of selected image
  {
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData());
    itk::Image< float, 3 >::Pointer itkImg = itk::Image< float, 3 >::New();
    CastToItkImage< itk::Image< float, 3 > >(img, itkImg);

    m_Parameters.m_SignalGen.m_ImageRegion = itkImg->GetLargestPossibleRegion();
    m_Parameters.m_SignalGen.m_ImageSpacing = itkImg->GetSpacing();
    m_Parameters.m_SignalGen.m_ImageOrigin = itkImg->GetOrigin();
    m_Parameters.m_SignalGen.m_ImageDirection = itkImg->GetDirection();

    if (m_Controls->m_UseBvalsBvecsBox->isChecked())
    {
      double bval;
      m_Parameters.SetGradienDirections( mitk::gradients::ReadBvalsBvecs(m_Controls->m_LoadBvalsEdit->text().toStdString(), m_Controls->m_LoadBvecsEdit->text().toStdString(), bval) );
      m_Parameters.SetBvalue(bval);
    }
    else
    {
      m_Parameters.SetNumWeightedVolumes(m_Controls->m_NumGradientsBox->value());
      m_Parameters.SetBvalue(m_Controls->m_BvalueBox->value());
      m_Parameters.GenerateGradientHalfShell();
    }
  }
  else if (m_Parameters.m_SignalGen.m_MaskImage.IsNotNull())   // use geometry of mask image
  {
    ItkUcharImgType::Pointer itkImg = m_Parameters.m_SignalGen.m_MaskImage;
    m_Parameters.m_SignalGen.m_ImageRegion = itkImg->GetLargestPossibleRegion();
    m_Parameters.m_SignalGen.m_ImageSpacing = itkImg->GetSpacing();
    m_Parameters.m_SignalGen.m_ImageOrigin = itkImg->GetOrigin();
    m_Parameters.m_SignalGen.m_ImageDirection = itkImg->GetDirection();

    if (m_Controls->m_UseBvalsBvecsBox->isChecked())
    {
      double bval;
      m_Parameters.SetGradienDirections( mitk::gradients::ReadBvalsBvecs(m_Controls->m_LoadBvalsEdit->text().toStdString(), m_Controls->m_LoadBvecsEdit->text().toStdString(), bval) );
      m_Parameters.SetBvalue(bval);
    }
    else
    {
      m_Parameters.SetNumWeightedVolumes(m_Controls->m_NumGradientsBox->value());
      m_Parameters.SetBvalue(m_Controls->m_BvalueBox->value());
      m_Parameters.GenerateGradientHalfShell();
    }
  }
  else    // use GUI parameters
  {
    m_Parameters.m_SignalGen.m_ImageRegion.SetSize(0, m_Controls->m_SizeX->value());
    m_Parameters.m_SignalGen.m_ImageRegion.SetSize(1, m_Controls->m_SizeY->value());
    m_Parameters.m_SignalGen.m_ImageRegion.SetSize(2, m_Controls->m_SizeZ->value());
    m_Parameters.m_SignalGen.m_ImageSpacing[0] = m_Controls->m_SpacingX->value();
    m_Parameters.m_SignalGen.m_ImageSpacing[1] = m_Controls->m_SpacingY->value();
    m_Parameters.m_SignalGen.m_ImageSpacing[2] = m_Controls->m_SpacingZ->value();
    m_Parameters.m_SignalGen.m_ImageOrigin[0] = m_Parameters.m_SignalGen.m_ImageSpacing[0]/2;
    m_Parameters.m_SignalGen.m_ImageOrigin[1] = m_Parameters.m_SignalGen.m_ImageSpacing[1]/2;
    m_Parameters.m_SignalGen.m_ImageOrigin[2] = m_Parameters.m_SignalGen.m_ImageSpacing[2]/2;
    m_Parameters.m_SignalGen.m_ImageDirection.SetIdentity();

    if (m_Controls->m_UseBvalsBvecsBox->isChecked())
    {
      double bval;
      m_Parameters.SetGradienDirections( mitk::gradients::ReadBvalsBvecs(m_Controls->m_LoadBvalsEdit->text().toStdString(), m_Controls->m_LoadBvecsEdit->text().toStdString(), bval) );
      m_Parameters.SetBvalue(bval);
    }
    else
    {
      m_Parameters.SetNumWeightedVolumes(m_Controls->m_NumGradientsBox->value());
      m_Parameters.SetBvalue(m_Controls->m_BvalueBox->value());
      m_Parameters.GenerateGradientHalfShell();
    }
  }

  // signal relaxation
  m_Parameters.m_SignalGen.m_DoSimulateRelaxation = false;
  if (m_Controls->m_RelaxationBox->isChecked())
  {
    m_Parameters.m_SignalGen.m_DoSimulateRelaxation = true;
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Relaxation", BoolProperty::New(true));
    m_Parameters.m_Misc.m_ArtifactModelString += "_RELAX";
  }
  m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = m_Parameters.m_SignalGen.m_DoSimulateRelaxation;

  // N/2 ghosts
  m_Parameters.m_Misc.m_DoAddGhosts = m_Controls->m_AddGhosts->isChecked();
  m_Parameters.m_SignalGen.m_KspaceLineOffset = m_Controls->m_kOffsetBox->value();
  if (m_Controls->m_AddGhosts->isChecked())
  {
    m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    m_Parameters.m_Misc.m_ArtifactModelString += "_GHOST";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Ghost", DoubleProperty::New(m_Parameters.m_SignalGen.m_KspaceLineOffset));
  }

  // Aliasing
  m_Parameters.m_Misc.m_DoAddAliasing = m_Controls->m_AddAliasing->isChecked();
  m_Parameters.m_SignalGen.m_CroppingFactor = (100-m_Controls->m_WrapBox->value())/100;
  if (m_Controls->m_AddAliasing->isChecked())
  {
    m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    m_Parameters.m_Misc.m_ArtifactModelString += "_ALIASING";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Aliasing", DoubleProperty::New(m_Controls->m_WrapBox->value()));
  }

  // Spikes
  m_Parameters.m_Misc.m_DoAddSpikes = m_Controls->m_AddSpikes->isChecked();
  m_Parameters.m_SignalGen.m_Spikes = m_Controls->m_SpikeNumBox->value();
  m_Parameters.m_SignalGen.m_SpikeAmplitude = m_Controls->m_SpikeScaleBox->value();
  if (m_Controls->m_AddSpikes->isChecked())
  {
    m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    m_Parameters.m_Misc.m_ArtifactModelString += "_SPIKES";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Spikes.Number", IntProperty::New(m_Parameters.m_SignalGen.m_Spikes));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Spikes.Amplitude", DoubleProperty::New(m_Parameters.m_SignalGen.m_SpikeAmplitude));
  }

  // Drift
  m_Parameters.m_SignalGen.m_DoAddDrift = m_Controls->m_AddDrift->isChecked();
  m_Parameters.m_SignalGen.m_Drift = static_cast<float>(m_Controls->m_DriftFactor->value())/100;
  if (m_Controls->m_AddDrift->isChecked())
  {
    m_Parameters.m_Misc.m_ArtifactModelString += "_DRIFT";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Drift", FloatProperty::New(m_Parameters.m_SignalGen.m_Drift));
  }

  // gibbs ringing
  m_Parameters.m_SignalGen.m_DoAddGibbsRinging = m_Controls->m_AddGibbsRinging->isChecked();
  m_Parameters.m_SignalGen.m_ZeroRinging = m_Controls->m_ZeroRinging->value();
  if (m_Controls->m_AddGibbsRinging->isChecked())
  {
    m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Ringing", BoolProperty::New(true));
    m_Parameters.m_Misc.m_ArtifactModelString += "_RINGING";
  }

  // add distortions
  m_Parameters.m_Misc.m_DoAddDistortions = m_Controls->m_AddDistortions->isChecked();
  if (m_Controls->m_AddDistortions->isChecked() && m_Controls->m_FrequencyMapBox->GetSelectedNode().IsNotNull())
  {
    mitk::DataNode::Pointer fMapNode = m_Controls->m_FrequencyMapBox->GetSelectedNode();
    mitk::Image* img = dynamic_cast<mitk::Image*>(fMapNode->GetData());
    ItkFloatImgType::Pointer itkImg = ItkFloatImgType::New();
    CastToItkImage< ItkFloatImgType >(img, itkImg);

    if (m_Controls->m_TemplateComboBox->GetSelectedNode().IsNull())   // use geometry of frequency map
    {
      m_Parameters.m_SignalGen.m_ImageRegion = itkImg->GetLargestPossibleRegion();
      m_Parameters.m_SignalGen.m_ImageSpacing = itkImg->GetSpacing();
      m_Parameters.m_SignalGen.m_ImageOrigin = itkImg->GetOrigin();
      m_Parameters.m_SignalGen.m_ImageDirection = itkImg->GetDirection();
    }

    m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    itk::ImageDuplicator<ItkFloatImgType>::Pointer duplicator = itk::ImageDuplicator<ItkFloatImgType>::New();
    duplicator->SetInputImage(itkImg);
    duplicator->Update();
    m_Parameters.m_SignalGen.m_FrequencyMap = duplicator->GetOutput();
    m_Parameters.m_Misc.m_ArtifactModelString += "_DISTORTED";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Distortions", BoolProperty::New(true));
  }

  m_Parameters.m_SignalGen.m_EddyStrength = m_Controls->m_EddyGradientStrength->value();
  m_Parameters.m_Misc.m_DoAddEddyCurrents = m_Controls->m_AddEddy->isChecked();
  if (m_Controls->m_AddEddy->isChecked())
  {
    m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    m_Parameters.m_Misc.m_ArtifactModelString += "_EDDY";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Eddy-strength", DoubleProperty::New(m_Parameters.m_SignalGen.m_EddyStrength));
  }

  // Motion
  m_Parameters.m_SignalGen.m_DoAddMotion = false;
  m_Parameters.m_SignalGen.m_DoRandomizeMotion = m_Controls->m_RandomMotion->isChecked();
  m_Parameters.m_SignalGen.m_Translation[0] = m_Controls->m_MaxTranslationBoxX->value();
  m_Parameters.m_SignalGen.m_Translation[1] = m_Controls->m_MaxTranslationBoxY->value();
  m_Parameters.m_SignalGen.m_Translation[2] = m_Controls->m_MaxTranslationBoxZ->value();
  m_Parameters.m_SignalGen.m_Rotation[0] = m_Controls->m_MaxRotationBoxX->value();
  m_Parameters.m_SignalGen.m_Rotation[1] = m_Controls->m_MaxRotationBoxY->value();
  m_Parameters.m_SignalGen.m_Rotation[2] = m_Controls->m_MaxRotationBoxZ->value();
  m_Parameters.m_SignalGen.m_MotionVolumes.clear();
  m_Parameters.m_Misc.m_MotionVolumesBox = m_Controls->m_MotionVolumesBox->text().toStdString();

  if ( m_Controls->m_AddMotion->isChecked())
  {
    m_Parameters.m_SignalGen.m_DoAddMotion = true;
    m_Parameters.m_Misc.m_ArtifactModelString += "_MOTION";
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Random", BoolProperty::New(m_Parameters.m_SignalGen.m_DoRandomizeMotion));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Translation-x", DoubleProperty::New(m_Parameters.m_SignalGen.m_Translation[0]));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Translation-y", DoubleProperty::New(m_Parameters.m_SignalGen.m_Translation[1]));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Translation-z", DoubleProperty::New(m_Parameters.m_SignalGen.m_Translation[2]));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Rotation-x", DoubleProperty::New(m_Parameters.m_SignalGen.m_Rotation[0]));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Rotation-y", DoubleProperty::New(m_Parameters.m_SignalGen.m_Rotation[1]));
    m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Rotation-z", DoubleProperty::New(m_Parameters.m_SignalGen.m_Rotation[2]));

    if ( m_Parameters.m_Misc.m_MotionVolumesBox == "random" )
    {
      for ( size_t i=0; i < m_Parameters.m_SignalGen.GetNumVolumes(); ++i )
      {
        m_Parameters.m_SignalGen.m_MotionVolumes.push_back( bool( rand()%2 ) );
      }
      MITK_DEBUG << "QmitkFiberfoxView.cpp: Case m_Misc.m_MotionVolumesBox == \"random\".";
    }

    else if ( ! m_Parameters.m_Misc.m_MotionVolumesBox.empty() )
    {
      std::stringstream stream( m_Parameters.m_Misc.m_MotionVolumesBox );
      std::vector<int> numbers;
      int number = std::numeric_limits<int>::max();
      while( stream >> number )
      {
        if( number < std::numeric_limits<int>::max() )
        {
          numbers.push_back( number );
        }
      }

      // If a list of negative numbers is given:
      if(    *(std::min_element( numbers.begin(), numbers.end() )) < 0
             && *(std::max_element( numbers.begin(), numbers.end() )) <= 0 ) // cave: -0 == +0
      {
        for ( size_t i=0; i < m_Parameters.m_SignalGen.GetNumVolumes(); ++i )
        {
          m_Parameters.m_SignalGen.m_MotionVolumes.push_back( true );
        }
        // set all true except those given.
        for( auto iter = std::begin( numbers ); iter != std::end( numbers ); ++iter  )
        {
          if ( -(*iter) < (int)m_Parameters.m_SignalGen.GetNumVolumes() && -(*iter) >= 0 )
          {
            m_Parameters.m_SignalGen.m_MotionVolumes.at( -(*iter) ) = false;
          }
        }
        MITK_DEBUG << "QmitkFiberfoxView.cpp: Case list of negative numbers.";
      }

      // If a list of positive numbers is given:
      else if(    *(std::min_element( numbers.begin(), numbers.end() )) >= 0
                  && *(std::max_element( numbers.begin(), numbers.end() )) >= 0 )
      {
        for ( size_t i=0; i < m_Parameters.m_SignalGen.GetNumVolumes(); ++i )
        {
          m_Parameters.m_SignalGen.m_MotionVolumes.push_back( false );
        }
        // set all false except those given.
        for( auto iter = std::begin( numbers ); iter != std::end( numbers ); ++iter )
        {
          if ( *iter < (int)m_Parameters.m_SignalGen.GetNumVolumes() && *iter >= 0 )
          {
            m_Parameters.m_SignalGen.m_MotionVolumes.at( *iter ) = true;
          }
        }
        MITK_DEBUG << "QmitkFiberfoxView.cpp: Case list of positive numbers.";
      }

      else
      {
        MITK_ERROR << "QmitkFiberfoxView.cpp: Inconsistent list of numbers in m_MotionVolumesBox.";
      }
    }

    else
    {
      m_Parameters.m_Misc.m_MotionVolumesBox = ""; // set empty.
      m_Controls->m_MotionVolumesBox->setText("");
      for (unsigned int i=0; i<m_Parameters.m_SignalGen.GetNumVolumes(); i++)
      {
        m_Parameters.m_SignalGen.m_MotionVolumes.push_back(i);
      }
    }
  }

  // other imaging parameters
  m_Parameters.m_SignalGen.m_AcquisitionType = (SignalGenerationParameters::AcquisitionType)m_Controls->m_AcquisitionTypeBox->currentIndex();
  m_Parameters.m_SignalGen.m_CoilSensitivityProfile = (SignalGenerationParameters::CoilSensitivityProfile)m_Controls->m_CoilSensBox->currentIndex();
  m_Parameters.m_SignalGen.m_NumberOfCoils = m_Controls->m_NumCoilsBox->value();
  m_Parameters.m_SignalGen.m_PartialFourier = m_Controls->m_PartialFourier->value();
  m_Parameters.m_SignalGen.m_ReversePhase = m_Controls->m_ReversePhaseBox->isChecked();
  m_Parameters.m_SignalGen.m_tLine = m_Controls->m_LineReadoutTimeBox->value();
  m_Parameters.m_SignalGen.m_tInhom = m_Controls->m_T2starBox->value();
  m_Parameters.m_SignalGen.m_EchoTrainLength = m_Controls->m_EtlBox->value();
  m_Parameters.m_SignalGen.m_tEcho = m_Controls->m_TEbox->value();
  m_Parameters.m_SignalGen.m_tRep = m_Controls->m_TRbox->value();
  m_Parameters.m_SignalGen.m_tInv = m_Controls->m_TIbox->value();
  m_Parameters.m_SignalGen.m_DoDisablePartialVolume = m_Controls->m_EnforcePureFiberVoxelsBox->isChecked();
  m_Parameters.m_SignalGen.m_AxonRadius = m_Controls->m_FiberRadius->value();
  m_Parameters.m_SignalGen.m_SignalScale = m_Controls->m_SignalScaleBox->value();

//  double voxelVolume = m_Parameters.m_SignalGen.m_ImageSpacing[0]
//      * m_Parameters.m_SignalGen.m_ImageSpacing[1]
//      * m_Parameters.m_SignalGen.m_ImageSpacing[2];
//  if ( m_Parameters.m_SignalGen.m_SignalScale*voxelVolume > itk::NumericTraits<short>::max()*0.75 )
//  {
//    m_Parameters.m_SignalGen.m_SignalScale = itk::NumericTraits<short>::max()*0.75/voxelVolume;
//    m_Controls->m_SignalScaleBox->setValue(m_Parameters.m_SignalGen.m_SignalScale);
//    QMessageBox::information( nullptr, "Warning",
//                              "Maximum signal exceeding data type limits. Automatically adjusted to "
//                              + QString::number(m_Parameters.m_SignalGen.m_SignalScale)
//                              + " to obtain a maximum  signal of 75% of the data type maximum."
//                                " Relaxation and other effects that affect the signal intensities are not accounted for.");
//  }

  // Noise
  m_Parameters.m_Misc.m_DoAddNoise = m_Controls->m_AddNoise->isChecked();
  m_Parameters.m_SignalGen.m_NoiseVariance = m_Controls->m_NoiseLevel->value();
  if (m_Controls->m_AddNoise->isChecked())
  {
    switch (m_Controls->m_NoiseDistributionBox->currentIndex())
    {
    case 0:
    {
      if (m_Parameters.m_SignalGen.m_NoiseVariance>0)
      {
        m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
        m_Parameters.m_Misc.m_ArtifactModelString += "_COMPLEX-GAUSSIAN-";
        m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Complex Gaussian"));
      }
      break;
    }
    case 1:
    {
      if (m_Parameters.m_SignalGen.m_NoiseVariance>0)
      {
        m_Parameters.m_NoiseModel = std::make_shared< mitk::RicianNoiseModel<ScalarType> >();
        m_Parameters.m_Misc.m_ArtifactModelString += "_RICIAN-";
        m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Rician"));
        m_Parameters.m_NoiseModel->SetNoiseVariance(m_Parameters.m_SignalGen.m_NoiseVariance);
      }
      break;
    }
    case 2:
    {
      if (m_Parameters.m_SignalGen.m_NoiseVariance>0)
      {
        m_Parameters.m_NoiseModel = std::make_shared< mitk::ChiSquareNoiseModel<ScalarType> >();
        m_Parameters.m_Misc.m_ArtifactModelString += "_CHISQUARED-";
        m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Chi-squared"));
        m_Parameters.m_NoiseModel->SetNoiseVariance(m_Parameters.m_SignalGen.m_NoiseVariance);
      }
      break;
    }
    default:
    {
      if (m_Parameters.m_SignalGen.m_NoiseVariance>0)
      {
        m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
        m_Parameters.m_Misc.m_ArtifactModelString += "_COMPLEX-GAUSSIAN-";
        m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Complex Gaussian"));
      }
      break;
    }
    }

    if (m_Parameters.m_SignalGen.m_NoiseVariance>0)
    {
      m_Parameters.m_Misc.m_ArtifactModelString += QString::number(m_Parameters.m_SignalGen.m_NoiseVariance).toStdString();
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Variance", DoubleProperty::New(m_Parameters.m_SignalGen.m_NoiseVariance));
    }
  }

  // signal models
  {
    // compartment 1
    switch (m_Controls->m_Compartment1Box->currentIndex())
    {
    case 0:
    {
      mitk::StickModel<ScalarType>* model = new mitk::StickModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity(m_Controls->m_StickWidget1->GetD());
      model->SetT2(m_Controls->m_StickWidget1->GetT2());
      model->SetT1(m_Controls->m_StickWidget1->GetT1());
      model->m_CompartmentId = 1;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Stick";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Stick") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D", DoubleProperty::New(m_Controls->m_StickWidget1->GetD()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 1:
    {
      mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity1(m_Controls->m_ZeppelinWidget1->GetD1());
      model->SetDiffusivity2(m_Controls->m_ZeppelinWidget1->GetD2());
      model->SetDiffusivity3(m_Controls->m_ZeppelinWidget1->GetD2());
      model->SetT2(m_Controls->m_ZeppelinWidget1->GetT2());
      model->SetT1(m_Controls->m_ZeppelinWidget1->GetT1());
      model->m_CompartmentId = 1;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Zeppelin";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Zeppelin") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D1", DoubleProperty::New(m_Controls->m_ZeppelinWidget1->GetD1()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D2", DoubleProperty::New(m_Controls->m_ZeppelinWidget1->GetD2()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 2:
    {
      mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity1(m_Controls->m_TensorWidget1->GetD1());
      model->SetDiffusivity2(m_Controls->m_TensorWidget1->GetD2());
      model->SetDiffusivity3(m_Controls->m_TensorWidget1->GetD3());
      model->SetT2(m_Controls->m_TensorWidget1->GetT2());
      model->SetT1(m_Controls->m_TensorWidget1->GetT1());
      model->m_CompartmentId = 1;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Tensor";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Tensor") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D1", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD1()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D2", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD2()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D3", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD3()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 3:
    {
      mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
      m_Parameters.m_SignalGen.m_DoSimulateRelaxation = false;
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetMaxNumKernels(m_Controls->m_PrototypeWidget1->GetNumberOfSamples());
      model->SetFaRange(m_Controls->m_PrototypeWidget1->GetMinFa(), m_Controls->m_PrototypeWidget1->GetMaxFa());
      model->SetAdcRange(m_Controls->m_PrototypeWidget1->GetMinAdc(), m_Controls->m_PrototypeWidget1->GetMaxAdc());
      model->m_CompartmentId = 1;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Prototype";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Prototype") );
      break;
    }
    }
    if (m_Controls->m_Comp1VolumeFraction->GetSelectedNode().IsNotNull())
    {
      mitk::DataNode::Pointer volumeNode = m_Controls->m_Comp1VolumeFraction->GetSelectedNode();
      ItkDoubleImgType::Pointer comp1VolumeImage = ItkDoubleImgType::New();
      mitk::Image* img = dynamic_cast<mitk::Image*>(volumeNode->GetData());
      CastToItkImage< ItkDoubleImgType >(img, comp1VolumeImage);
      m_Parameters.m_FiberModelList.back()->SetVolumeFractionImage(comp1VolumeImage);
    }

    // compartment 2
    switch (m_Controls->m_Compartment2Box->currentIndex())
    {
    case 0:
      break;
    case 1:
    {
      mitk::StickModel<ScalarType>* model = new mitk::StickModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity(m_Controls->m_StickWidget2->GetD());
      model->SetT2(m_Controls->m_StickWidget2->GetT2());
      model->SetT1(m_Controls->m_StickWidget2->GetT1());
      model->m_CompartmentId = 2;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Stick";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Stick") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D", DoubleProperty::New(m_Controls->m_StickWidget2->GetD()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 2:
    {
      mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity1(m_Controls->m_ZeppelinWidget2->GetD1());
      model->SetDiffusivity2(m_Controls->m_ZeppelinWidget2->GetD2());
      model->SetDiffusivity3(m_Controls->m_ZeppelinWidget2->GetD2());
      model->SetT2(m_Controls->m_ZeppelinWidget2->GetT2());
      model->SetT1(m_Controls->m_ZeppelinWidget2->GetT1());
      model->m_CompartmentId = 2;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Zeppelin";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Zeppelin") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D1", DoubleProperty::New(m_Controls->m_ZeppelinWidget2->GetD1()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D2", DoubleProperty::New(m_Controls->m_ZeppelinWidget2->GetD2()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 3:
    {
      mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity1(m_Controls->m_TensorWidget2->GetD1());
      model->SetDiffusivity2(m_Controls->m_TensorWidget2->GetD2());
      model->SetDiffusivity3(m_Controls->m_TensorWidget2->GetD3());
      model->SetT2(m_Controls->m_TensorWidget2->GetT2());
      model->SetT1(m_Controls->m_TensorWidget2->GetT1());
      model->m_CompartmentId = 2;
      m_Parameters.m_FiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Tensor";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Tensor") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D1", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD1()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D2", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD2()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D3", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD3()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    }
    if (m_Controls->m_Comp2VolumeFraction->GetSelectedNode().IsNotNull() && m_Parameters.m_FiberModelList.size()==2)
    {
      mitk::DataNode::Pointer volumeNode = m_Controls->m_Comp2VolumeFraction->GetSelectedNode();
      ItkDoubleImgType::Pointer comp1VolumeImage = ItkDoubleImgType::New();
      mitk::Image* img = dynamic_cast<mitk::Image*>(volumeNode->GetData());
      CastToItkImage< ItkDoubleImgType >(img, comp1VolumeImage);
      m_Parameters.m_FiberModelList.back()->SetVolumeFractionImage(comp1VolumeImage);
    }

    // compartment 3
    switch (m_Controls->m_Compartment3Box->currentIndex())
    {
    case 0:
    {
      mitk::BallModel<ScalarType>* model = new mitk::BallModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity(m_Controls->m_BallWidget1->GetD());
      model->SetT2(m_Controls->m_BallWidget1->GetT2());
      model->SetT1(m_Controls->m_BallWidget1->GetT1());
      model->m_CompartmentId = 3;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Ball";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Ball") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.D", DoubleProperty::New(m_Controls->m_BallWidget1->GetD()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 1:
    {
      mitk::AstroStickModel<ScalarType>* model = new mitk::AstroStickModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity(m_Controls->m_AstrosticksWidget1->GetD());
      model->SetT2(m_Controls->m_AstrosticksWidget1->GetT2());
      model->SetT1(m_Controls->m_AstrosticksWidget1->GetT1());
      model->SetRandomizeSticks(m_Controls->m_AstrosticksWidget1->GetRandomizeSticks());
      model->m_CompartmentId = 3;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Astrosticks";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Astrosticks") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.D", DoubleProperty::New(m_Controls->m_AstrosticksWidget1->GetD()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(model->GetT2()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.RandomSticks", BoolProperty::New(m_Controls->m_AstrosticksWidget1->GetRandomizeSticks()) );
      break;
    }
    case 2:
    {
      mitk::DotModel<ScalarType>* model = new mitk::DotModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetT2(m_Controls->m_DotWidget1->GetT2());
      model->SetT1(m_Controls->m_DotWidget1->GetT1());
      model->m_CompartmentId = 3;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Dot";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Dot") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 3:
    {
      mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
      m_Parameters.m_SignalGen.m_DoSimulateRelaxation = false;
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetMaxNumKernels(m_Controls->m_PrototypeWidget3->GetNumberOfSamples());
      model->SetFaRange(m_Controls->m_PrototypeWidget3->GetMinFa(), m_Controls->m_PrototypeWidget3->GetMaxFa());
      model->SetAdcRange(m_Controls->m_PrototypeWidget3->GetMinAdc(), m_Controls->m_PrototypeWidget3->GetMaxAdc());
      model->m_CompartmentId = 3;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Prototype";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Prototype") );
      break;
    }
    }
    if (m_Controls->m_Comp3VolumeFraction->GetSelectedNode().IsNotNull())
    {
      mitk::DataNode::Pointer volumeNode = m_Controls->m_Comp3VolumeFraction->GetSelectedNode();
      ItkDoubleImgType::Pointer comp1VolumeImage = ItkDoubleImgType::New();
      mitk::Image* img = dynamic_cast<mitk::Image*>(volumeNode->GetData());
      CastToItkImage< ItkDoubleImgType >(img, comp1VolumeImage);
      m_Parameters.m_NonFiberModelList.back()->SetVolumeFractionImage(comp1VolumeImage);
    }

    switch (m_Controls->m_Compartment4Box->currentIndex())
    {
    case 0:
      break;
    case 1:
    {
      mitk::BallModel<ScalarType>* model = new mitk::BallModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity(m_Controls->m_BallWidget2->GetD());
      model->SetT2(m_Controls->m_BallWidget2->GetT2());
      model->SetT1(m_Controls->m_BallWidget2->GetT1());
      model->m_CompartmentId = 4;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Ball";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Ball") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.D", DoubleProperty::New(m_Controls->m_BallWidget2->GetD()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 2:
    {
      mitk::AstroStickModel<ScalarType>* model = new mitk::AstroStickModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetBvalue(m_Parameters.m_SignalGen.GetBvalue());
      model->SetDiffusivity(m_Controls->m_AstrosticksWidget2->GetD());
      model->SetT2(m_Controls->m_AstrosticksWidget2->GetT2());
      model->SetT1(m_Controls->m_AstrosticksWidget2->GetT1());
      model->SetRandomizeSticks(m_Controls->m_AstrosticksWidget2->GetRandomizeSticks());
      model->m_CompartmentId = 4;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Astrosticks";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Astrosticks") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.D", DoubleProperty::New(m_Controls->m_AstrosticksWidget2->GetD()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(model->GetT2()) );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.RandomSticks", BoolProperty::New(m_Controls->m_AstrosticksWidget2->GetRandomizeSticks()) );
      break;
    }
    case 3:
    {
      mitk::DotModel<ScalarType>* model = new mitk::DotModel<ScalarType>();
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetT2(m_Controls->m_DotWidget2->GetT2());
      model->SetT1(m_Controls->m_DotWidget2->GetT1());
      model->m_CompartmentId = 4;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Dot";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Dot") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(model->GetT2()) );
      break;
    }
    case 4:
    {
      mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
      m_Parameters.m_SignalGen.m_DoSimulateRelaxation = false;
      model->SetGradientList(m_Parameters.m_SignalGen.GetGradientDirections());
      model->SetMaxNumKernels(m_Controls->m_PrototypeWidget4->GetNumberOfSamples());
      model->SetFaRange(m_Controls->m_PrototypeWidget4->GetMinFa(), m_Controls->m_PrototypeWidget4->GetMaxFa());
      model->SetAdcRange(m_Controls->m_PrototypeWidget4->GetMinAdc(), m_Controls->m_PrototypeWidget4->GetMaxAdc());
      model->m_CompartmentId = 4;
      m_Parameters.m_NonFiberModelList.push_back(model);
      m_Parameters.m_Misc.m_SignalModelString += "Prototype";
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
      m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Prototype") );
      break;
    }
    }
    if (m_Controls->m_Comp4VolumeFraction->GetSelectedNode().IsNotNull() && m_Parameters.m_NonFiberModelList.size()==2)
    {
      mitk::DataNode::Pointer volumeNode = m_Controls->m_Comp4VolumeFraction->GetSelectedNode();
      ItkDoubleImgType::Pointer compVolumeImage = ItkDoubleImgType::New();
      mitk::Image* img = dynamic_cast<mitk::Image*>(volumeNode->GetData());
      CastToItkImage< ItkDoubleImgType >(img, compVolumeImage);
      m_Parameters.m_NonFiberModelList.back()->SetVolumeFractionImage(compVolumeImage);
    }
  }

  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.SignalScale", IntProperty::New(m_Parameters.m_SignalGen.m_SignalScale));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.FiberRadius", IntProperty::New(m_Parameters.m_SignalGen.m_AxonRadius));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Tinhom", DoubleProperty::New(m_Parameters.m_SignalGen.m_tInhom));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.echoTrainLength", IntProperty::New(m_Parameters.m_SignalGen.m_EchoTrainLength));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Tline", DoubleProperty::New(m_Parameters.m_SignalGen.m_tLine));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.TE", DoubleProperty::New(m_Parameters.m_SignalGen.m_tEcho));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.b-value", DoubleProperty::New(m_Parameters.m_SignalGen.GetBvalue()));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.NoPartialVolume", BoolProperty::New(m_Parameters.m_SignalGen.m_DoDisablePartialVolume));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Relaxation", BoolProperty::New(m_Parameters.m_SignalGen.m_DoSimulateRelaxation));
  m_Parameters.m_Misc.m_ResultNode->AddProperty("binary", BoolProperty::New(false));
}

void QmitkFiberfoxView::SaveParameters(QString filename)
{
  UpdateParametersFromGui();
  std::vector< int > bVals = m_Parameters.m_SignalGen.GetBvalues();
  std::cout << "b-values: ";
  for (auto v : bVals)
    std::cout << v << " ";
  std::cout << std::endl;
  bool ok = true;
  bool first = true;
  bool dosampling = false;
  mitk::Image::Pointer diffImg = nullptr;
  itk::Image< itk::DiffusionTensor3D< double >, 3 >::Pointer tensorImage = nullptr;
  const int shOrder = 2;
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,shOrder,ODF_SAMPLING_SIZE> QballFilterType;
  QballFilterType::CoefficientImageType::Pointer itkFeatureImage = nullptr;
  ItkDoubleImgType::Pointer adcImage = nullptr;

  for (unsigned int i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
  {
    mitk::RawShModel<>* model = nullptr;
    if (i<m_Parameters.m_FiberModelList.size())
    {
      model = dynamic_cast<  mitk::RawShModel<>* >(m_Parameters.m_FiberModelList.at(i));
    }
    else
    {
      model = dynamic_cast<  mitk::RawShModel<>* >(m_Parameters.m_NonFiberModelList.at(i-m_Parameters.m_FiberModelList.size()));
    }

    if ( model!=nullptr && model->GetNumberOfKernels() <= 0 )
    {
      if (first==true)
      {
        if ( QMessageBox::question(nullptr, "Prototype signal sampling",
                                   "Do you want to sample prototype signals from the selected diffusion-weighted imag and save them?",
                                   QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
          dosampling = true;

        first = false;
        if ( dosampling && (m_Controls->m_TemplateComboBox->GetSelectedNode().IsNull()
                            || !mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(
                              dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData()) ) ) )
        {
          QMessageBox::information(nullptr, "Parameter file not saved", "No diffusion-weighted image selected to sample signal from.");
          return;
        }
        else if (dosampling)
        {
          diffImg = dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData());

          typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, double > TensorReconstructionImageFilterType;
          TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
          ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
          mitk::CastToItkImage(diffImg, itkVectorImagePointer);
          filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
          filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg),
                                   itkVectorImagePointer );

          filter->Update();
          tensorImage = filter->GetOutput();

          QballFilterType::Pointer qballfilter = QballFilterType::New();

          qballfilter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
          qballfilter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg),
                                        itkVectorImagePointer );
          qballfilter->SetLambda(0.006);
          qballfilter->SetNormalizationMethod(QballFilterType::QBAR_RAW_SIGNAL);
          qballfilter->Update();
          itkFeatureImage = qballfilter->GetCoefficientImage();

          itk::AdcImageFilter< short, double >::Pointer adcFilter = itk::AdcImageFilter< short, double >::New();
          adcFilter->SetInput( itkVectorImagePointer );
          adcFilter->SetGradientDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg));
          adcFilter->SetB_value(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
          adcFilter->Update();
          adcImage = adcFilter->GetOutput();
        }
      }


      typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, double > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
      ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
      mitk::CastToItkImage(diffImg, itkVectorImagePointer);
      filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
      filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg), itkVectorImagePointer );

      filter->Update();
      tensorImage = filter->GetOutput();

      QballFilterType::Pointer qballfilter = QballFilterType::New();
      qballfilter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
      qballfilter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg),
                                    itkVectorImagePointer );
      qballfilter->SetLambda(0.006);
      qballfilter->SetNormalizationMethod(QballFilterType::QBAR_RAW_SIGNAL);
      qballfilter->Update();
      itkFeatureImage = qballfilter->GetCoefficientImage();

      itk::AdcImageFilter< short, double >::Pointer adcFilter = itk::AdcImageFilter< short, double >::New();
      adcFilter->SetInput( itkVectorImagePointer );
      adcFilter->SetGradientDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg));
      adcFilter->SetB_value(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
      adcFilter->Update();
      adcImage = adcFilter->GetOutput();

      if (dosampling && diffImg.IsNotNull())
      {
        ok = model->SampleKernels(diffImg, m_Parameters.m_SignalGen.m_MaskImage, tensorImage, itkFeatureImage, adcImage);
        if (!ok)
        {
          QMessageBox::information( nullptr, "Parameter file not saved", "No valid prototype signals could be sampled.");
          return;
        }
      }
    }
  }

  m_Parameters.SaveParameters(filename.toStdString());
  m_ParameterFile = filename;
}

void QmitkFiberfoxView::SaveParameters()
{
  QString filename = QFileDialog::getSaveFileName(
        0,
        tr("Save Parameters"),
        m_ParameterFile,
        tr("Fiberfox Parameters (*.ffp)") );

  SaveParameters(filename);
}

void QmitkFiberfoxView::LoadParameters()
{
  QString filename = QFileDialog::getOpenFileName(0, tr("Load Parameters"), QString(itksys::SystemTools::GetFilenamePath(m_ParameterFile.toStdString()).c_str()), tr("Fiberfox Parameters (*.ffp)") );
  if(filename.isEmpty() || filename.isNull())
    return;

  m_ParameterFile = filename;
  m_Parameters.LoadParameters(filename.toStdString());

  if (m_Parameters.m_MissingTags.size()>0)
  {
    QString missing("Parameter file might be corrupted. The following parameters could not be read: ");
    missing += QString(m_Parameters.m_MissingTags.c_str());
    missing += "\nDefault values have been assigned to the missing parameters.";
    QMessageBox::information( nullptr, "Warning!", missing);
  }

  // image generation parameters
  m_Controls->m_SizeX->setValue(m_Parameters.m_SignalGen.m_ImageRegion.GetSize(0));
  m_Controls->m_SizeY->setValue(m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1));
  m_Controls->m_SizeZ->setValue(m_Parameters.m_SignalGen.m_ImageRegion.GetSize(2));
  m_Controls->m_SpacingX->setValue(m_Parameters.m_SignalGen.m_ImageSpacing[0]);
  m_Controls->m_SpacingY->setValue(m_Parameters.m_SignalGen.m_ImageSpacing[1]);
  m_Controls->m_SpacingZ->setValue(m_Parameters.m_SignalGen.m_ImageSpacing[2]);
  m_Controls->m_NumGradientsBox->setValue(m_Parameters.m_SignalGen.GetNumWeightedVolumes());
  m_Controls->m_BvalueBox->setValue(m_Parameters.m_SignalGen.GetBvalue());
  m_Controls->m_SignalScaleBox->setValue(m_Parameters.m_SignalGen.m_SignalScale);
  m_Controls->m_TEbox->setValue(m_Parameters.m_SignalGen.m_tEcho);
  m_Controls->m_LineReadoutTimeBox->setValue(m_Parameters.m_SignalGen.m_tLine);
  m_Controls->m_T2starBox->setValue(m_Parameters.m_SignalGen.m_tInhom);
  m_Controls->m_EtlBox->setValue(m_Parameters.m_SignalGen.m_EchoTrainLength);
  m_Controls->m_FiberRadius->setValue(m_Parameters.m_SignalGen.m_AxonRadius);
  m_Controls->m_RelaxationBox->setChecked(m_Parameters.m_SignalGen.m_DoSimulateRelaxation);
  m_Controls->m_EnforcePureFiberVoxelsBox->setChecked(m_Parameters.m_SignalGen.m_DoDisablePartialVolume);
  m_Controls->m_ReversePhaseBox->setChecked(m_Parameters.m_SignalGen.m_ReversePhase);
  m_Controls->m_PartialFourier->setValue(m_Parameters.m_SignalGen.m_PartialFourier);
  m_Controls->m_TRbox->setValue(m_Parameters.m_SignalGen.m_tRep);
  m_Controls->m_TIbox->setValue(m_Parameters.m_SignalGen.m_tInv);
  m_Controls->m_NumCoilsBox->setValue(m_Parameters.m_SignalGen.m_NumberOfCoils);
  m_Controls->m_CoilSensBox->setCurrentIndex(m_Parameters.m_SignalGen.m_CoilSensitivityProfile);
  m_Controls->m_AcquisitionTypeBox->setCurrentIndex(m_Parameters.m_SignalGen.m_AcquisitionType);

  if (!m_Parameters.m_Misc.m_BvalsFile.empty())
  {
    m_Controls->m_UseBvalsBvecsBox->setChecked(true);
    m_Controls->m_LoadBvalsEdit->setText(QString(m_Parameters.m_Misc.m_BvalsFile.c_str()));
  }
  else
    m_Controls->m_LoadBvalsEdit->setText("-");

  if (!m_Parameters.m_Misc.m_BvecsFile.empty())
  {
    m_Controls->m_UseBvalsBvecsBox->setChecked(true);
    m_Controls->m_LoadBvecsEdit->setText(QString(m_Parameters.m_Misc.m_BvecsFile.c_str()));
  }
  else
    m_Controls->m_LoadBvecsEdit->setText("-");

  if (m_Parameters.m_NoiseModel!=nullptr)
  {
    m_Controls->m_AddNoise->setChecked(m_Parameters.m_Misc.m_DoAddNoise);
    if (dynamic_cast<mitk::RicianNoiseModel<double>*>(m_Parameters.m_NoiseModel.get()))
    {
      m_Controls->m_NoiseDistributionBox->setCurrentIndex(0);
    }
    else if (dynamic_cast<mitk::ChiSquareNoiseModel<double>*>(m_Parameters.m_NoiseModel.get()))
    {
      m_Controls->m_NoiseDistributionBox->setCurrentIndex(1);
    }
    m_Controls->m_NoiseLevel->setValue(m_Parameters.m_NoiseModel->GetNoiseVariance());
  }
  else
  {
    m_Controls->m_AddNoise->setChecked(m_Parameters.m_Misc.m_DoAddNoise);
    m_Controls->m_NoiseLevel->setValue(m_Parameters.m_SignalGen.m_NoiseVariance);
  }

  m_Controls->m_VolumeFractionsBox->setChecked(m_Parameters.m_Misc.m_OutputAdditionalImages);
  m_Controls->m_AdvancedOptionsBox_2->setChecked(m_Parameters.m_Misc.m_CheckAdvancedSignalOptionsBox);
  m_Controls->m_AddGhosts->setChecked(m_Parameters.m_Misc.m_DoAddGhosts);
  m_Controls->m_AddAliasing->setChecked(m_Parameters.m_Misc.m_DoAddAliasing);
  m_Controls->m_AddDistortions->setChecked(m_Parameters.m_Misc.m_DoAddDistortions);
  m_Controls->m_AddSpikes->setChecked(m_Parameters.m_Misc.m_DoAddSpikes);
  m_Controls->m_AddEddy->setChecked(m_Parameters.m_Misc.m_DoAddEddyCurrents);
  m_Controls->m_AddDrift->setChecked(m_Parameters.m_SignalGen.m_DoAddDrift);

  m_Controls->m_kOffsetBox->setValue(m_Parameters.m_SignalGen.m_KspaceLineOffset);
  m_Controls->m_WrapBox->setValue(100*(1-m_Parameters.m_SignalGen.m_CroppingFactor));
  m_Controls->m_DriftFactor->setValue(100*m_Parameters.m_SignalGen.m_Drift);
  m_Controls->m_SpikeNumBox->setValue(m_Parameters.m_SignalGen.m_Spikes);
  m_Controls->m_SpikeScaleBox->setValue(m_Parameters.m_SignalGen.m_SpikeAmplitude);
  m_Controls->m_EddyGradientStrength->setValue(m_Parameters.m_SignalGen.m_EddyStrength);
  m_Controls->m_AddGibbsRinging->setChecked(m_Parameters.m_SignalGen.m_DoAddGibbsRinging);
  m_Controls->m_ZeroRinging->setValue(m_Parameters.m_SignalGen.m_ZeroRinging);
  m_Controls->m_AddMotion->setChecked(m_Parameters.m_SignalGen.m_DoAddMotion);
  m_Controls->m_RandomMotion->setChecked(m_Parameters.m_SignalGen.m_DoRandomizeMotion);
  m_Controls->m_MotionVolumesBox->setText(QString(m_Parameters.m_Misc.m_MotionVolumesBox.c_str()));

  m_Controls->m_MaxTranslationBoxX->setValue(m_Parameters.m_SignalGen.m_Translation[0]);
  m_Controls->m_MaxTranslationBoxY->setValue(m_Parameters.m_SignalGen.m_Translation[1]);
  m_Controls->m_MaxTranslationBoxZ->setValue(m_Parameters.m_SignalGen.m_Translation[2]);
  m_Controls->m_MaxRotationBoxX->setValue(m_Parameters.m_SignalGen.m_Rotation[0]);
  m_Controls->m_MaxRotationBoxY->setValue(m_Parameters.m_SignalGen.m_Rotation[1]);
  m_Controls->m_MaxRotationBoxZ->setValue(m_Parameters.m_SignalGen.m_Rotation[2]);

  m_Controls->m_Compartment1Box->setCurrentIndex(0);
  m_Controls->m_Compartment2Box->setCurrentIndex(0);
  m_Controls->m_Compartment3Box->setCurrentIndex(0);
  m_Controls->m_Compartment4Box->setCurrentIndex(0);

  for (unsigned int i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
  {
    mitk::DiffusionSignalModel<ScalarType>* signalModel = nullptr;
    if (i<m_Parameters.m_FiberModelList.size())
      signalModel = m_Parameters.m_FiberModelList.at(i);
    else
      signalModel = m_Parameters.m_NonFiberModelList.at(i-m_Parameters.m_FiberModelList.size());

    mitk::DataNode::Pointer compVolNode;
    if ( signalModel->GetVolumeFractionImage().IsNotNull() )
    {
      compVolNode = mitk::DataNode::New();
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(signalModel->GetVolumeFractionImage().GetPointer());
      image->SetVolume(signalModel->GetVolumeFractionImage()->GetBufferPointer());

      compVolNode->SetData( image );
      compVolNode->SetName("Compartment volume "+QString::number(signalModel->m_CompartmentId).toStdString());
      GetDataStorage()->Add(compVolNode);
    }

    switch (signalModel->m_CompartmentId)
    {
    case 1:
    {
      if (compVolNode.IsNotNull())
        m_Controls->m_Comp1VolumeFraction->SetSelectedNode(compVolNode);
      if (dynamic_cast<mitk::StickModel<>*>(signalModel))
      {
        mitk::StickModel<>* model = dynamic_cast<mitk::StickModel<>*>(signalModel);
        m_Controls->m_StickWidget1->SetT2(model->GetT2());
        m_Controls->m_StickWidget1->SetT1(model->GetT1());
        m_Controls->m_StickWidget1->SetD(model->GetDiffusivity());
        m_Controls->m_Compartment1Box->setCurrentIndex(0);
        break;
      }
      else if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
      {
        mitk::TensorModel<>* model = dynamic_cast<mitk::TensorModel<>*>(signalModel);
        m_Controls->m_TensorWidget1->SetT2(model->GetT2());
        m_Controls->m_TensorWidget1->SetT1(model->GetT1());
        m_Controls->m_TensorWidget1->SetD1(model->GetDiffusivity1());
        m_Controls->m_TensorWidget1->SetD2(model->GetDiffusivity2());
        m_Controls->m_TensorWidget1->SetD3(model->GetDiffusivity3());
        m_Controls->m_Compartment1Box->setCurrentIndex(2);
        break;
      }
      else if (dynamic_cast<mitk::RawShModel<>*>(signalModel))
      {
        mitk::RawShModel<>* model = dynamic_cast<mitk::RawShModel<>*>(signalModel);
        m_Controls->m_PrototypeWidget1->SetNumberOfSamples(model->GetMaxNumKernels());
        m_Controls->m_PrototypeWidget1->SetMinFa(model->GetFaRange().first);
        m_Controls->m_PrototypeWidget1->SetMaxFa(model->GetFaRange().second);
        m_Controls->m_PrototypeWidget1->SetMinAdc(model->GetAdcRange().first);
        m_Controls->m_PrototypeWidget1->SetMaxAdc(model->GetAdcRange().second);
        m_Controls->m_Compartment1Box->setCurrentIndex(3);
        break;
      }
      break;
    }
    case 2:
    {
      if (compVolNode.IsNotNull())
        m_Controls->m_Comp2VolumeFraction->SetSelectedNode(compVolNode);
      if (dynamic_cast<mitk::StickModel<>*>(signalModel))
      {
        mitk::StickModel<>* model = dynamic_cast<mitk::StickModel<>*>(signalModel);
        m_Controls->m_StickWidget2->SetT2(model->GetT2());
        m_Controls->m_StickWidget2->SetT1(model->GetT1());
        m_Controls->m_StickWidget2->SetD(model->GetDiffusivity());
        m_Controls->m_Compartment2Box->setCurrentIndex(1);
        break;
      }
      else if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
      {
        mitk::TensorModel<>* model = dynamic_cast<mitk::TensorModel<>*>(signalModel);
        m_Controls->m_TensorWidget2->SetT2(model->GetT2());
        m_Controls->m_TensorWidget2->SetT1(model->GetT1());
        m_Controls->m_TensorWidget2->SetD1(model->GetDiffusivity1());
        m_Controls->m_TensorWidget2->SetD2(model->GetDiffusivity2());
        m_Controls->m_TensorWidget2->SetD3(model->GetDiffusivity3());
        m_Controls->m_Compartment2Box->setCurrentIndex(3);
        break;
      }
      break;
    }
    case 3:
    {
      if (compVolNode.IsNotNull())
        m_Controls->m_Comp3VolumeFraction->SetSelectedNode(compVolNode);
      if (dynamic_cast<mitk::BallModel<>*>(signalModel))
      {
        mitk::BallModel<>* model = dynamic_cast<mitk::BallModel<>*>(signalModel);
        m_Controls->m_BallWidget1->SetT2(model->GetT2());
        m_Controls->m_BallWidget1->SetT1(model->GetT1());
        m_Controls->m_BallWidget1->SetD(model->GetDiffusivity());
        m_Controls->m_Compartment3Box->setCurrentIndex(0);
        break;
      }
      else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
      {
        mitk::AstroStickModel<>* model = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);
        m_Controls->m_AstrosticksWidget1->SetT2(model->GetT2());
        m_Controls->m_AstrosticksWidget1->SetT1(model->GetT1());
        m_Controls->m_AstrosticksWidget1->SetD(model->GetDiffusivity());
        m_Controls->m_AstrosticksWidget1->SetRandomizeSticks(model->GetRandomizeSticks());
        m_Controls->m_Compartment3Box->setCurrentIndex(1);
        break;
      }
      else if (dynamic_cast<mitk::DotModel<>*>(signalModel))
      {
        mitk::DotModel<>* model = dynamic_cast<mitk::DotModel<>*>(signalModel);
        m_Controls->m_DotWidget1->SetT2(model->GetT2());
        m_Controls->m_DotWidget1->SetT1(model->GetT1());
        m_Controls->m_Compartment3Box->setCurrentIndex(2);
        break;
      }
      else if (dynamic_cast<mitk::RawShModel<>*>(signalModel))
      {
        mitk::RawShModel<>* model = dynamic_cast<mitk::RawShModel<>*>(signalModel);
        m_Controls->m_PrototypeWidget3->SetNumberOfSamples(model->GetMaxNumKernels());
        m_Controls->m_PrototypeWidget3->SetMinFa(model->GetFaRange().first);
        m_Controls->m_PrototypeWidget3->SetMaxFa(model->GetFaRange().second);
        m_Controls->m_PrototypeWidget3->SetMinAdc(model->GetAdcRange().first);
        m_Controls->m_PrototypeWidget3->SetMaxAdc(model->GetAdcRange().second);
        m_Controls->m_Compartment3Box->setCurrentIndex(3);
        break;
      }
      break;
    }
    case 4:
    {
      if (compVolNode.IsNotNull())
        m_Controls->m_Comp4VolumeFraction->SetSelectedNode(compVolNode);
      if (dynamic_cast<mitk::BallModel<>*>(signalModel))
      {
        mitk::BallModel<>* model = dynamic_cast<mitk::BallModel<>*>(signalModel);
        m_Controls->m_BallWidget2->SetT2(model->GetT2());
        m_Controls->m_BallWidget2->SetT1(model->GetT1());
        m_Controls->m_BallWidget2->SetD(model->GetDiffusivity());
        m_Controls->m_Compartment4Box->setCurrentIndex(1);
        break;
      }
      else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
      {
        mitk::AstroStickModel<>* model = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);
        m_Controls->m_AstrosticksWidget2->SetT2(model->GetT2());
        m_Controls->m_AstrosticksWidget2->SetT1(model->GetT1());
        m_Controls->m_AstrosticksWidget2->SetD(model->GetDiffusivity());
        m_Controls->m_AstrosticksWidget2->SetRandomizeSticks(model->GetRandomizeSticks());
        m_Controls->m_Compartment4Box->setCurrentIndex(2);
        break;
      }
      else if (dynamic_cast<mitk::DotModel<>*>(signalModel))
      {
        mitk::DotModel<>* model = dynamic_cast<mitk::DotModel<>*>(signalModel);
        m_Controls->m_DotWidget2->SetT2(model->GetT2());
        m_Controls->m_DotWidget2->SetT1(model->GetT1());
        m_Controls->m_Compartment4Box->setCurrentIndex(3);
        break;
      }
      else if (dynamic_cast<mitk::RawShModel<>*>(signalModel))
      {
        mitk::RawShModel<>* model = dynamic_cast<mitk::RawShModel<>*>(signalModel);
        m_Controls->m_PrototypeWidget4->SetNumberOfSamples(model->GetMaxNumKernels());
        m_Controls->m_PrototypeWidget4->SetMinFa(model->GetFaRange().first);
        m_Controls->m_PrototypeWidget4->SetMaxFa(model->GetFaRange().second);
        m_Controls->m_PrototypeWidget4->SetMinAdc(model->GetAdcRange().first);
        m_Controls->m_PrototypeWidget4->SetMaxAdc(model->GetAdcRange().second);
        m_Controls->m_Compartment4Box->setCurrentIndex(4);
        break;
      }
      break;
    }
    }
  }

  if ( m_Parameters.m_SignalGen.m_MaskImage )
  {
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk(m_Parameters.m_SignalGen.m_MaskImage.GetPointer());
    image->SetVolume(m_Parameters.m_SignalGen.m_MaskImage->GetBufferPointer());

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName("Tissue mask");
    GetDataStorage()->Add(node);
    m_Controls->m_MaskComboBox->SetSelectedNode(node);
  }

  if ( m_Parameters.m_SignalGen.m_FrequencyMap )
  {
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk(m_Parameters.m_SignalGen.m_FrequencyMap.GetPointer());
    image->SetVolume(m_Parameters.m_SignalGen.m_FrequencyMap->GetBufferPointer());

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName("Frequency map");
    GetDataStorage()->Add(node);
    m_Controls->m_FrequencyMapBox->SetSelectedNode(node);
  }
}

void QmitkFiberfoxView::ShowAdvancedOptions(int state)
{
  if (state)
  {
    m_Controls->m_AdvancedSignalOptionsFrame->setVisible(true);
    m_Controls->m_AdvancedOptionsBox_2->setChecked(true);
  }
  else
  {
    m_Controls->m_AdvancedSignalOptionsFrame->setVisible(false);
    m_Controls->m_AdvancedOptionsBox_2->setChecked(false);
  }
}

void QmitkFiberfoxView::Comp1ModelFrameVisibility(int index)
{
  m_Controls->m_StickWidget1->setVisible(false);
  m_Controls->m_ZeppelinWidget1->setVisible(false);
  m_Controls->m_TensorWidget1->setVisible(false);
  m_Controls->m_PrototypeWidget1->setVisible(false);

  switch (index)
  {
  case 0:
    m_Controls->m_StickWidget1->setVisible(true);
    break;
  case 1:
    m_Controls->m_ZeppelinWidget1->setVisible(true);
    break;
  case 2:
    m_Controls->m_TensorWidget1->setVisible(true);
    break;
  case 3:
    m_Controls->m_PrototypeWidget1->setVisible(true);
    break;
  }
}

void QmitkFiberfoxView::Comp2ModelFrameVisibility(int index)
{
  m_Controls->m_StickWidget2->setVisible(false);
  m_Controls->m_ZeppelinWidget2->setVisible(false);
  m_Controls->m_TensorWidget2->setVisible(false);
  m_Controls->m_Comp2FractionFrame->setVisible(false);

  switch (index)
  {
  case 0:
    break;
  case 1:
    m_Controls->m_StickWidget2->setVisible(true);
    m_Controls->m_Comp2FractionFrame->setVisible(true);
    break;
  case 2:
    m_Controls->m_ZeppelinWidget2->setVisible(true);
    m_Controls->m_Comp2FractionFrame->setVisible(true);
    break;
  case 3:
    m_Controls->m_TensorWidget2->setVisible(true);
    m_Controls->m_Comp2FractionFrame->setVisible(true);
    break;
  }
}

void QmitkFiberfoxView::Comp3ModelFrameVisibility(int index)
{
  m_Controls->m_BallWidget1->setVisible(false);
  m_Controls->m_AstrosticksWidget1->setVisible(false);
  m_Controls->m_DotWidget1->setVisible(false);
  m_Controls->m_PrototypeWidget3->setVisible(false);

  switch (index)
  {
  case 0:
    m_Controls->m_BallWidget1->setVisible(true);
    break;
  case 1:
    m_Controls->m_AstrosticksWidget1->setVisible(true);
    break;
  case 2:
    m_Controls->m_DotWidget1->setVisible(true);
    break;
  case 3:
    m_Controls->m_PrototypeWidget3->setVisible(true);
    break;
  }
}

void QmitkFiberfoxView::Comp4ModelFrameVisibility(int index)
{
  m_Controls->m_BallWidget2->setVisible(false);
  m_Controls->m_AstrosticksWidget2->setVisible(false);
  m_Controls->m_DotWidget2->setVisible(false);
  m_Controls->m_PrototypeWidget4->setVisible(false);
  m_Controls->m_Comp4FractionFrame->setVisible(false);

  switch (index)
  {
  case 0:
    break;
  case 1:
    m_Controls->m_BallWidget2->setVisible(true);
    m_Controls->m_Comp4FractionFrame->setVisible(true);
    break;
  case 2:
    m_Controls->m_AstrosticksWidget2->setVisible(true);
    m_Controls->m_Comp4FractionFrame->setVisible(true);
    break;
  case 3:
    m_Controls->m_DotWidget2->setVisible(true);
    m_Controls->m_Comp4FractionFrame->setVisible(true);
    break;
  case 4:
    m_Controls->m_PrototypeWidget4->setVisible(true);
    m_Controls->m_Comp4FractionFrame->setVisible(true);
    break;
  }
}

void QmitkFiberfoxView::OnAddMotion(int value)
{
  if (value>0)
    m_Controls->m_MotionArtifactFrame->setVisible(true);
  else
    m_Controls->m_MotionArtifactFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddDrift(int value)
{
  if (value>0)
    m_Controls->m_DriftFrame->setVisible(true);
  else
    m_Controls->m_DriftFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddAliasing(int value)
{
  if (value>0)
    m_Controls->m_AliasingFrame->setVisible(true);
  else
    m_Controls->m_AliasingFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddSpikes(int value)
{
  if (value>0)
    m_Controls->m_SpikeFrame->setVisible(true);
  else
    m_Controls->m_SpikeFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddEddy(int value)
{
  if (value>0)
    m_Controls->m_EddyFrame->setVisible(true);
  else
    m_Controls->m_EddyFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddDistortions(int value)
{
  if (value>0)
    m_Controls->m_DistortionsFrame->setVisible(true);
  else
    m_Controls->m_DistortionsFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddGhosts(int value)
{
  if (value>0)
    m_Controls->m_GhostFrame->setVisible(true);
  else
    m_Controls->m_GhostFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddNoise(int value)
{
  if (value>0)
    m_Controls->m_NoiseFrame->setVisible(true);
  else
    m_Controls->m_NoiseFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddRinging(int value)
{
  if (value>0)
    m_Controls->m_ZeroRinging->setVisible(true);
  else
    m_Controls->m_ZeroRinging->setVisible(false);
}

QmitkFiberfoxView::GradientListType QmitkFiberfoxView::GenerateHalfShell(int NPoints)
{
  NPoints *= 2;
  GradientListType pointshell;

  int numB0 = NPoints/20;
  if (numB0==0)
    numB0=1;
  GradientType g;
  g.Fill(0.0);
  for (int i=0; i<numB0; i++)
    pointshell.push_back(g);

  if (NPoints==0)
    return pointshell;

  vnl_vector<double> theta; theta.set_size(NPoints);
  vnl_vector<double> phi; phi.set_size(NPoints);
  double C = sqrt(4*itk::Math::pi);
  phi(0) = 0.0;
  phi(NPoints-1) = 0.0;
  for(int i=0; i<NPoints; i++)
  {
    theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - itk::Math::pi / 2.0;
    if( i>0 && i<NPoints-1)
    {
      phi(i) = (phi(i-1) + C /
                sqrt(NPoints*(1-(-1.0+2.0*i/(NPoints-1.0))*(-1.0+2.0*i/(NPoints-1.0)))));
      // % (2*DIST_POINTSHELL_PI);
    }
  }

  for(int i=0; i<NPoints; i++)
  {
    g[2] = sin(theta(i));
    if (g[2]<0)
      continue;
    g[0] = cos(theta(i)) * cos(phi(i));
    g[1] = cos(theta(i)) * sin(phi(i));
    pointshell.push_back(g);
  }

  return pointshell;
}

template<int ndirs>
std::vector<itk::Vector<double,3> > QmitkFiberfoxView::MakeGradientList()
{
  std::vector<itk::Vector<double,3> > retval;
  vnl_matrix_fixed<double, 3, ndirs>* U =
      itk::PointShell<ndirs, vnl_matrix_fixed<double, 3, ndirs> >::DistributePointShell();


  // Add 0 vector for B0
  int numB0 = ndirs/10;
  if (numB0==0)
    numB0=1;
  itk::Vector<double,3> v;
  v.Fill(0.0);
  for (int i=0; i<numB0; i++)
  {
    retval.push_back(v);
  }

  for(int i=0; i<ndirs;i++)
  {
    itk::Vector<double,3> v;
    v[0] = U->get(0,i); v[1] = U->get(1,i); v[2] = U->get(2,i);
    retval.push_back(v);
  }

  return retval;
}

void QmitkFiberfoxView::GenerateImage()
{
  if (m_Controls->m_FiberBundleComboBox->GetSelectedNode().IsNull() && !mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( m_Controls->m_TemplateComboBox->GetSelectedNode()))
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateGradientImage<unsigned int>(
          m_Controls->m_SizeX->value(),
          m_Controls->m_SizeY->value(),
          m_Controls->m_SizeZ->value(),
          m_Controls->m_SpacingX->value(),
          m_Controls->m_SpacingY->value(),
          m_Controls->m_SpacingZ->value());
    mitk::Point3D origin;
    origin[0] = m_Controls->m_SpacingX->value()/2;
    origin[1] = m_Controls->m_SpacingY->value()/2;
    origin[2] = m_Controls->m_SpacingZ->value()/2;
    image->SetOrigin(origin);

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName("Dummy");
    GetDataStorage()->Add(node);
    m_SelectedImageNode = node;

    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
      mitk::RenderingManager::GetInstance()->InitializeViews( basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    UpdateGui();
    QMessageBox::information(nullptr, "Template image generated", "You have selected no fiber bundle or diffusion-weighted image, which can be used to simulate a new diffusion-weighted image. A template image with the specified geometry has been generated that can be used to draw artificial fibers (see view 'Fiber Generator').");
  }
  else if (m_Controls->m_FiberBundleComboBox->GetSelectedNode().IsNotNull())
    SimulateImageFromFibers(m_Controls->m_FiberBundleComboBox->GetSelectedNode());
  else if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( m_Controls->m_TemplateComboBox->GetSelectedNode()) )
    SimulateForExistingDwi(m_Controls->m_TemplateComboBox->GetSelectedNode());
  else
    QMessageBox::information(nullptr, "No image generated", "You have selected no fiber bundle or diffusion-weighted image, which can be used to simulate a new diffusion-weighted image.");
}

void QmitkFiberfoxView::SetFocus()
{
}

void QmitkFiberfoxView::SimulateForExistingDwi(mitk::DataNode* imageNode)
{
  bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(imageNode->GetData())) );
  if ( !isDiffusionImage )
  {
    return;
  }

  UpdateParametersFromGui();

  mitk::Image::Pointer diffImg = dynamic_cast<mitk::Image*>(imageNode->GetData());
  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(diffImg, itkVectorImagePointer);

  m_TractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
  m_Parameters.m_Misc.m_ParentNode = imageNode;
  m_Parameters.m_SignalGen.m_SignalScale = 1;

  m_Parameters.m_Misc.m_ResultNode->SetName("b"+QString::number(m_Parameters.m_SignalGen.GetBvalue()).toStdString()
                                            +"_"+m_Parameters.m_Misc.m_SignalModelString
                                            +m_Parameters.m_Misc.m_ArtifactModelString);

  m_Parameters.ApplyDirectionMatrix();
  m_TractsToDwiFilter->SetParameters(m_Parameters);
  m_TractsToDwiFilter->SetInputImage(itkVectorImagePointer);
  m_Thread.start(QThread::LowestPriority);
}

void QmitkFiberfoxView::SimulateImageFromFibers(mitk::DataNode* fiberNode)
{
  mitk::FiberBundle::Pointer fiberBundle = dynamic_cast<mitk::FiberBundle*>(fiberNode->GetData());
  if (fiberBundle->GetNumFibers()<=0) { return; }

  UpdateParametersFromGui();

  m_TractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
  m_Parameters.m_Misc.m_ParentNode = fiberNode;

  m_Parameters.m_Misc.m_ResultNode->SetName("b"+QString::number(m_Parameters.m_SignalGen.GetBvalue()).toStdString()
                                            +"_"+m_Parameters.m_Misc.m_SignalModelString
                                            +m_Parameters.m_Misc.m_ArtifactModelString);

  if ( m_Controls->m_TemplateComboBox->GetSelectedNode().IsNotNull()
       && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image*>
                                                                   (m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData()) ) )
  {
    bool first = true;
    bool ok = true;
    mitk::Image::Pointer diffImg = dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData());
    itk::Image< itk::DiffusionTensor3D< double >, 3 >::Pointer tensorImage = nullptr;
    const int shOrder = 2;
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,shOrder,ODF_SAMPLING_SIZE> QballFilterType;
    QballFilterType::CoefficientImageType::Pointer itkFeatureImage = nullptr;
    ItkDoubleImgType::Pointer adcImage = nullptr;

    for (unsigned int i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
    {
      mitk::RawShModel<>* model = nullptr;
      if (i<m_Parameters.m_FiberModelList.size())
        model = dynamic_cast<  mitk::RawShModel<>* >(m_Parameters.m_FiberModelList.at(i));
      else
        model = dynamic_cast<  mitk::RawShModel<>* >(m_Parameters.m_NonFiberModelList.at(i-m_Parameters.m_FiberModelList.size()));

      if (model!=0 && model->GetNumberOfKernels()<=0)
      {
        if (first==true)
        {
          ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
          mitk::CastToItkImage(diffImg, itkVectorImagePointer);

          typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, double > TensorReconstructionImageFilterType;
          TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
          filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
          filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg),
                                   itkVectorImagePointer );
          filter->Update();
          tensorImage = filter->GetOutput();

          QballFilterType::Pointer qballfilter = QballFilterType::New();
          qballfilter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg),
                                        itkVectorImagePointer );
          qballfilter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
          qballfilter->SetLambda(0.006);
          qballfilter->SetNormalizationMethod(QballFilterType::QBAR_RAW_SIGNAL);
          qballfilter->Update();
          itkFeatureImage = qballfilter->GetCoefficientImage();

          itk::AdcImageFilter< short, double >::Pointer adcFilter = itk::AdcImageFilter< short, double >::New();
          adcFilter->SetInput( itkVectorImagePointer );
          adcFilter->SetGradientDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(diffImg));
          adcFilter->SetB_value(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
          adcFilter->Update();
          adcImage = adcFilter->GetOutput();
        }
        ok = model->SampleKernels(diffImg, m_Parameters.m_SignalGen.m_MaskImage, tensorImage, itkFeatureImage, adcImage);
        if (!ok)
          break;
      }
    }

    if (!ok)
    {
      QMessageBox::information( nullptr, "Simulation cancelled", "No valid prototype signals could be sampled.");
      return;
    }
  }
  else if ( m_Controls->m_Compartment1Box->currentIndex()==3
            || m_Controls->m_Compartment3Box->currentIndex()==3
            || m_Controls->m_Compartment4Box->currentIndex()==4 )
  {
    QMessageBox::information( nullptr, "Simulation cancelled",
                              "Prototype signal but no diffusion-weighted image selected to sample signal from.");
    return;
  }

  m_Parameters.ApplyDirectionMatrix();
  m_TractsToDwiFilter->SetParameters(m_Parameters);
  m_TractsToDwiFilter->SetFiberBundle(fiberBundle);
  m_Thread.start(QThread::LowestPriority);
}

void QmitkFiberfoxView::SetBvalsEdit()
{
  // SELECT FOLDER DIALOG
  std::string filename;
  filename = QFileDialog::getOpenFileName(nullptr, "Select bvals file", QString(filename.c_str())).toStdString();

  if (filename.empty())
    m_Controls->m_LoadBvalsEdit->setText("-");
  else
    m_Controls->m_LoadBvalsEdit->setText(QString(filename.c_str()));
}

void QmitkFiberfoxView::SetBvecsEdit()
{
  // SELECT FOLDER DIALOG
  std::string filename;
  filename = QFileDialog::getOpenFileName(nullptr, "Select bvecs file", QString(filename.c_str())).toStdString();

  if (filename.empty())
    m_Controls->m_LoadBvecsEdit->setText("-");
  else
    m_Controls->m_LoadBvecsEdit->setText(QString(filename.c_str()));
}

void QmitkFiberfoxView::SetOutputPath()
{
  // SELECT FOLDER DIALOG
  std::string outputPath;
  outputPath = QFileDialog::getExistingDirectory(nullptr, "Save images to...", QString(outputPath.c_str())).toStdString();

  if (outputPath.empty())
    m_Controls->m_SavePathEdit->setText("-");
  else
  {
    outputPath += "/";
    m_Controls->m_SavePathEdit->setText(QString(outputPath.c_str()));
  }
}

void QmitkFiberfoxView::UpdateGui()
{
  OnTlineChanged();
  m_Controls->m_GeometryFrame->setEnabled(true);
  m_Controls->m_GeometryMessage->setVisible(false);
  m_Controls->m_DiffusionPropsMessage->setVisible(false);

  m_Controls->m_LoadGradientsFrame->setVisible(false);
  m_Controls->m_GenerateGradientsFrame->setVisible(false);
  if (m_Controls->m_UseBvalsBvecsBox->isChecked())
    m_Controls->m_LoadGradientsFrame->setVisible(true);
  else
    m_Controls->m_GenerateGradientsFrame->setVisible(true);

  // Signal generation gui
  if (m_Controls->m_MaskComboBox->GetSelectedNode().IsNotNull() || m_Controls->m_TemplateComboBox->GetSelectedNode().IsNotNull())
  {
    m_Controls->m_GeometryMessage->setVisible(true);
    m_Controls->m_GeometryFrame->setEnabled(false);
  }

  if ( m_Controls->m_TemplateComboBox->GetSelectedNode().IsNotNull()
       && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(
         dynamic_cast<mitk::Image*>(m_Controls->m_TemplateComboBox->GetSelectedNode()->GetData()) ) )
  {
    m_Controls->m_DiffusionPropsMessage->setVisible(true);
    m_Controls->m_GeometryMessage->setVisible(true);
    m_Controls->m_GeometryFrame->setEnabled(false);

    m_Controls->m_LoadGradientsFrame->setVisible(false);
    m_Controls->m_GenerateGradientsFrame->setVisible(false);
  }
}
