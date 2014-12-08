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

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberfoxView.h"

// MITK
#include <mitkImage.h>
#include <mitkDiffusionImage.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkDataStorage.h>
#include <itkFibersFromPlanarFiguresFilter.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorImage.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkGlobalInteraction.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageGenerator.h>
#include <mitkNodePredicateDataType.h>
#include <itkScalableAffineTransform.h>
#include <mitkLevelWindowProperty.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <boost/property_tree/ptree.hpp>
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

#define _USE_MATH_DEFINES
#include <math.h>

QmitkFiberfoxWorker::QmitkFiberfoxWorker(QmitkFiberfoxView* view)
    : m_View(view)
{

}

void QmitkFiberfoxWorker::run()
{
    try{
        switch (m_FilterType)
        {
        case 0:
            m_View->m_TractsToDwiFilter->Update();
            break;
        case 1:
            m_View->m_ArtifactsToDwiFilter->Update();
            break;
        }
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
    , m_SelectedImage( NULL )
    , m_Worker(this)
    , m_ThreadIsRunning(false)
{
    m_Worker.moveToThread(&m_Thread);
    connect(&m_Thread, SIGNAL(started()), this, SLOT(BeforeThread()));
    connect(&m_Thread, SIGNAL(started()), &m_Worker, SLOT(run()));
    connect(&m_Thread, SIGNAL(finished()), this, SLOT(AfterThread()));
    connect(&m_Thread, SIGNAL(terminated()), this, SLOT(AfterThread()));
    m_SimulationTimer = new QTimer(this);
}

void QmitkFiberfoxView::KillThread()
{
    MITK_INFO << "Aborting DWI simulation.";
    switch (m_Worker.m_FilterType)
    {
    case 0:
        m_TractsToDwiFilter->SetAbortGenerateData(true);
        break;
    case 1:
        m_ArtifactsToDwiFilter->SetAbortGenerateData(true);
        break;
    }
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
    FiberfoxParameters<double> parameters;
    mitk::DiffusionImage<short>::Pointer mitkImage = mitk::DiffusionImage<short>::New();
    switch (m_Worker.m_FilterType)
    {
    case 0:
    {
        statusText = QString(m_TractsToDwiFilter->GetStatusText().c_str());
        if (m_TractsToDwiFilter->GetAbortGenerateData())
        {
            MITK_INFO << "Simulation aborted.";
            return;
        }

        parameters = m_TractsToDwiFilter->GetParameters();

        mitkImage->SetVectorImage( m_TractsToDwiFilter->GetOutput() );
        mitkImage->SetReferenceBValue(parameters.m_SignalGen.m_Bvalue);
        mitkImage->SetDirections(parameters.m_SignalGen.GetGradientDirections());
        mitkImage->InitializeFromVectorImage();
        parameters.m_Misc.m_ResultNode->SetData( mitkImage );

        parameters.m_Misc.m_ResultNode->SetName(parameters.m_Misc.m_ParentNode->GetName()
                                                +"_D"+QString::number(parameters.m_SignalGen.m_ImageRegion.GetSize(0)).toStdString()
                                                +"-"+QString::number(parameters.m_SignalGen.m_ImageRegion.GetSize(1)).toStdString()
                                                +"-"+QString::number(parameters.m_SignalGen.m_ImageRegion.GetSize(2)).toStdString()
                                                +"_S"+QString::number(parameters.m_SignalGen.m_ImageSpacing[0]).toStdString()
                +"-"+QString::number(parameters.m_SignalGen.m_ImageSpacing[1]).toStdString()
                +"-"+QString::number(parameters.m_SignalGen.m_ImageSpacing[2]).toStdString()
                +"_b"+QString::number(parameters.m_SignalGen.m_Bvalue).toStdString()
                +"_"+parameters.m_Misc.m_SignalModelString
                +parameters.m_Misc.m_ArtifactModelString);

        GetDataStorage()->Add(parameters.m_Misc.m_ResultNode, parameters.m_Misc.m_ParentNode);

        parameters.m_Misc.m_ResultNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New(m_TractsToDwiFilter->GetLevelWindow()) );

        if (m_Controls->m_VolumeFractionsBox->isChecked())
        {
            std::vector< itk::TractsToDWIImageFilter< short >::ItkDoubleImgType::Pointer > volumeFractions = m_TractsToDwiFilter->GetVolumeFractions();
            for (unsigned int k=0; k<volumeFractions.size(); k++)
            {
                mitk::Image::Pointer image = mitk::Image::New();
                image->InitializeByItk(volumeFractions.at(k).GetPointer());
                image->SetVolume(volumeFractions.at(k)->GetBufferPointer());

                mitk::DataNode::Pointer node = mitk::DataNode::New();
                node->SetData( image );
                node->SetName(parameters.m_Misc.m_ParentNode->GetName()+"_CompartmentVolume-"+QString::number(k).toStdString());
                GetDataStorage()->Add(node, parameters.m_Misc.m_ParentNode);
            }
        }
        m_TractsToDwiFilter = NULL;
        break;
    }
    case 1:
    {
        statusText = QString(m_ArtifactsToDwiFilter->GetStatusText().c_str());
        if (m_ArtifactsToDwiFilter->GetAbortGenerateData())
        {
            MITK_INFO << "Simulation aborted.";
            return;
        }

        parameters = m_ArtifactsToDwiFilter->GetParameters().CopyParameters<double>();

        mitk::DiffusionImage<short>::Pointer diffImg = dynamic_cast<mitk::DiffusionImage<short>*>(parameters.m_Misc.m_ParentNode->GetData());
        mitkImage = mitk::DiffusionImage<short>::New();
        mitkImage->SetVectorImage( m_ArtifactsToDwiFilter->GetOutput() );
        mitkImage->SetReferenceBValue(diffImg->GetReferenceBValue());
        mitkImage->SetDirections(diffImg->GetDirections());
        mitkImage->InitializeFromVectorImage();
        parameters.m_Misc.m_ResultNode->SetData( mitkImage );
        parameters.m_Misc.m_ResultNode->SetName(parameters.m_Misc.m_ParentNode->GetName()+parameters.m_Misc.m_ArtifactModelString);
        GetDataStorage()->Add(parameters.m_Misc.m_ResultNode, parameters.m_Misc.m_ParentNode);
        m_ArtifactsToDwiFilter = NULL;
        break;
    }
    }

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
            outputFileName += ".dwi";
            QString status("Saving output image to ");
            status += outputFileName;
            m_Controls->m_SimulationStatusText->append(status);
            mitk::IOUtil::SaveBaseData(mitkImage, outputFileName.toStdString());
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
    parameters.m_SignalGen.m_FrequencyMap = NULL;
}

void QmitkFiberfoxView::UpdateSimulationStatus()
{
    QString statusText;
    switch (m_Worker.m_FilterType)
    {
    case 0:
        statusText = QString(m_TractsToDwiFilter->GetStatusText().c_str());
        break;
    case 1:
        statusText = QString(m_ArtifactsToDwiFilter->GetStatusText().c_str());
        break;
    }

    if (QString::compare(m_SimulationStatusText,statusText)!=0)
    {
        m_Controls->m_SimulationStatusText->clear();
        statusText = "<pre>"+statusText+"</pre>";
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

        // commented out
        m_Controls->m_DiffusionDirectionBox->setVisible(false);
        m_Controls->label_3->setVisible(false);
        m_Controls->m_SeparationAngleBox->setVisible(false);
        m_Controls->label_4->setVisible(false);
        //

        m_Controls->m_StickWidget1->setVisible(true);
        m_Controls->m_StickWidget2->setVisible(false);
        m_Controls->m_ZeppelinWidget1->setVisible(false);
        m_Controls->m_ZeppelinWidget2->setVisible(false);
        m_Controls->m_TensorWidget1->setVisible(false);
        m_Controls->m_TensorWidget2->setVisible(false);

        m_Controls->m_BallWidget1->setVisible(true);
        m_Controls->m_BallWidget2->setVisible(false);
        m_Controls->m_AstrosticksWidget1->setVisible(false);
        m_Controls->m_AstrosticksWidget2->setVisible(false);
        m_Controls->m_DotWidget1->setVisible(false);
        m_Controls->m_DotWidget2->setVisible(false);

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

        m_Controls->m_Comp4FractionFrame->setVisible(false);
        m_Controls->m_DiffusionPropsMessage->setVisible(false);
        m_Controls->m_GeometryMessage->setVisible(false);
        m_Controls->m_AdvancedSignalOptionsFrame->setVisible(false);
        m_Controls->m_AdvancedFiberOptionsFrame->setVisible(false);
        m_Controls->m_VarianceBox->setVisible(false);
        m_Controls->m_NoiseFrame->setVisible(false);
        m_Controls->m_GhostFrame->setVisible(false);
        m_Controls->m_DistortionsFrame->setVisible(false);
        m_Controls->m_EddyFrame->setVisible(false);
        m_Controls->m_SpikeFrame->setVisible(false);
        m_Controls->m_AliasingFrame->setVisible(false);
        m_Controls->m_MotionArtifactFrame->setVisible(false);
        m_ParameterFile = QDir::currentPath()+"/param.ffp";

        m_Controls->m_AbortSimulationButton->setVisible(false);
        m_Controls->m_SimulationStatusText->setVisible(false);

        m_Controls->m_FrequencyMapBox->SetDataStorage(this->GetDataStorage());
        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
        mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
        mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
        mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
        isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        m_Controls->m_FrequencyMapBox->SetPredicate(finalPredicate);
        m_Controls->m_Comp4VolumeFraction->SetDataStorage(this->GetDataStorage());
        m_Controls->m_Comp4VolumeFraction->SetPredicate(finalPredicate);

        connect( m_SimulationTimer, SIGNAL(timeout()), this, SLOT(UpdateSimulationStatus()) );
        connect((QObject*) m_Controls->m_AbortSimulationButton, SIGNAL(clicked()), (QObject*) this, SLOT(KillThread()));
        connect((QObject*) m_Controls->m_GenerateImageButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateImage()));
        connect((QObject*) m_Controls->m_GenerateFibersButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateFibers()));
        connect((QObject*) m_Controls->m_CircleButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnDrawROI()));
        connect((QObject*) m_Controls->m_FlipButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnFlipButton()));
        connect((QObject*) m_Controls->m_JoinBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(JoinBundles()));
        connect((QObject*) m_Controls->m_VarianceBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnVarianceChanged(double)));
        connect((QObject*) m_Controls->m_DistributionBox, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(OnDistributionChanged(int)));
        connect((QObject*) m_Controls->m_FiberDensityBox, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(OnFiberDensityChanged(int)));
        connect((QObject*) m_Controls->m_FiberSamplingBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnFiberSamplingChanged(double)));
        connect((QObject*) m_Controls->m_TensionBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnTensionChanged(double)));
        connect((QObject*) m_Controls->m_ContinuityBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnContinuityChanged(double)));
        connect((QObject*) m_Controls->m_BiasBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnBiasChanged(double)));
        connect((QObject*) m_Controls->m_AddNoise, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddNoise(int)));
        connect((QObject*) m_Controls->m_AddGhosts, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddGhosts(int)));
        connect((QObject*) m_Controls->m_AddDistortions, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddDistortions(int)));
        connect((QObject*) m_Controls->m_AddEddy, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddEddy(int)));
        connect((QObject*) m_Controls->m_AddSpikes, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddSpikes(int)));
        connect((QObject*) m_Controls->m_AddAliasing, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddAliasing(int)));
        connect((QObject*) m_Controls->m_AddMotion, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddMotion(int)));

        connect((QObject*) m_Controls->m_ConstantRadiusBox, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnConstantRadius(int)));
        connect((QObject*) m_Controls->m_CopyBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(CopyBundles()));
        connect((QObject*) m_Controls->m_TransformBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(ApplyTransform()));
        connect((QObject*) m_Controls->m_AlignOnGrid, SIGNAL(clicked()), (QObject*) this, SLOT(AlignOnGrid()));

        connect((QObject*) m_Controls->m_Compartment1Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp1ModelFrameVisibility(int)));
        connect((QObject*) m_Controls->m_Compartment2Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp2ModelFrameVisibility(int)));
        connect((QObject*) m_Controls->m_Compartment3Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp3ModelFrameVisibility(int)));
        connect((QObject*) m_Controls->m_Compartment4Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp4ModelFrameVisibility(int)));

        connect((QObject*) m_Controls->m_AdvancedOptionsBox, SIGNAL( stateChanged(int)), (QObject*) this, SLOT(ShowAdvancedOptions(int)));
        connect((QObject*) m_Controls->m_AdvancedOptionsBox_2, SIGNAL( stateChanged(int)), (QObject*) this, SLOT(ShowAdvancedOptions(int)));

        connect((QObject*) m_Controls->m_SaveParametersButton, SIGNAL(clicked()), (QObject*) this, SLOT(SaveParameters()));
        connect((QObject*) m_Controls->m_LoadParametersButton, SIGNAL(clicked()), (QObject*) this, SLOT(LoadParameters()));
        connect((QObject*) m_Controls->m_OutputPathButton, SIGNAL(clicked()), (QObject*) this, SLOT(SetOutputPath()));

    }
}

template< class ScalarType >
FiberfoxParameters< ScalarType > QmitkFiberfoxView::UpdateImageParameters()
{
    FiberfoxParameters< ScalarType > parameters;
    parameters.m_Misc.m_OutputPath = "";
    parameters.m_Misc.m_CheckAdvancedFiberOptionsBox = m_Controls->m_AdvancedOptionsBox->isChecked();
    parameters.m_Misc.m_CheckAdvancedSignalOptionsBox = m_Controls->m_AdvancedOptionsBox_2->isChecked();
    parameters.m_Misc.m_CheckOutputVolumeFractionsBox = m_Controls->m_VolumeFractionsBox->isChecked();

    string outputPath = m_Controls->m_SavePathEdit->text().toStdString();
    if (outputPath.compare("-")!=0)
    {
        parameters.m_Misc.m_OutputPath = outputPath;
        parameters.m_Misc.m_OutputPath += "/";
    }

    if (m_MaskImageNode.IsNotNull())
    {
        mitk::Image::Pointer mitkMaskImage = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
        mitk::CastToItkImage<ItkUcharImgType>(mitkMaskImage, parameters.m_SignalGen.m_MaskImage);
        itk::ImageDuplicator<ItkUcharImgType>::Pointer duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
        duplicator->SetInputImage(parameters.m_SignalGen.m_MaskImage);
        duplicator->Update();
        parameters.m_SignalGen.m_MaskImage = duplicator->GetOutput();
    }

    if (m_SelectedDWI.IsNotNull())  // use parameters of selected DWI
    {
        mitk::DiffusionImage<short>::Pointer dwi = dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedDWI->GetData());
        parameters.m_SignalGen.m_ImageRegion = dwi->GetVectorImage()->GetLargestPossibleRegion();
        parameters.m_SignalGen.m_ImageSpacing = dwi->GetVectorImage()->GetSpacing();
        parameters.m_SignalGen.m_ImageOrigin = dwi->GetVectorImage()->GetOrigin();
        parameters.m_SignalGen.m_ImageDirection = dwi->GetVectorImage()->GetDirection();
        parameters.m_SignalGen.m_Bvalue = dwi->GetReferenceBValue();
        parameters.m_SignalGen.SetGradienDirections(dwi->GetDirections());
    }
    else if (m_SelectedImage.IsNotNull())   // use geometry of selected image
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_SelectedImage->GetData());
        itk::Image< float, 3 >::Pointer itkImg = itk::Image< float, 3 >::New();
        CastToItkImage< itk::Image< float, 3 > >(img, itkImg);

        parameters.m_SignalGen.m_ImageRegion = itkImg->GetLargestPossibleRegion();
        parameters.m_SignalGen.m_ImageSpacing = itkImg->GetSpacing();
        parameters.m_SignalGen.m_ImageOrigin = itkImg->GetOrigin();
        parameters.m_SignalGen.m_ImageDirection = itkImg->GetDirection();
        parameters.m_SignalGen.SetNumWeightedVolumes(m_Controls->m_NumGradientsBox->value());
        parameters.m_SignalGen.m_Bvalue = m_Controls->m_BvalueBox->value();
    }
    else    // use GUI parameters
    {
        parameters.m_SignalGen.m_ImageRegion.SetSize(0, m_Controls->m_SizeX->value());
        parameters.m_SignalGen.m_ImageRegion.SetSize(1, m_Controls->m_SizeY->value());
        parameters.m_SignalGen.m_ImageRegion.SetSize(2, m_Controls->m_SizeZ->value());
        parameters.m_SignalGen.m_ImageSpacing[0] = m_Controls->m_SpacingX->value();
        parameters.m_SignalGen.m_ImageSpacing[1] = m_Controls->m_SpacingY->value();
        parameters.m_SignalGen.m_ImageSpacing[2] = m_Controls->m_SpacingZ->value();
        parameters.m_SignalGen.m_ImageOrigin[0] = parameters.m_SignalGen.m_ImageSpacing[0]/2;
        parameters.m_SignalGen.m_ImageOrigin[1] = parameters.m_SignalGen.m_ImageSpacing[1]/2;
        parameters.m_SignalGen.m_ImageOrigin[2] = parameters.m_SignalGen.m_ImageSpacing[2]/2;
        parameters.m_SignalGen.m_ImageDirection.SetIdentity();
        parameters.m_SignalGen.SetNumWeightedVolumes(m_Controls->m_NumGradientsBox->value());
        parameters.m_SignalGen.m_Bvalue = m_Controls->m_BvalueBox->value();
        parameters.m_SignalGen.GenerateGradientHalfShell();
    }

    // signal relaxation
    parameters.m_SignalGen.m_DoSimulateRelaxation = m_Controls->m_RelaxationBox->isChecked();
    parameters.m_SignalGen.m_SimulateKspaceAcquisition = parameters.m_SignalGen.m_DoSimulateRelaxation;
    if (parameters.m_SignalGen.m_DoSimulateRelaxation && m_SelectedBundles.size()>0 )
        parameters.m_Misc.m_ArtifactModelString += "_RELAX";

    // N/2 ghosts
    parameters.m_Misc.m_CheckAddGhostsBox = m_Controls->m_AddGhosts->isChecked();
    if (m_Controls->m_AddGhosts->isChecked())
    {
        parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
        parameters.m_Misc.m_ArtifactModelString += "_GHOST";
        parameters.m_SignalGen.m_KspaceLineOffset = m_Controls->m_kOffsetBox->value();
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Ghost", DoubleProperty::New(parameters.m_SignalGen.m_KspaceLineOffset));
    }
    else
        parameters.m_SignalGen.m_KspaceLineOffset = 0;

    // Aliasing
    parameters.m_Misc.m_CheckAddAliasingBox = m_Controls->m_AddAliasing->isChecked();
    if (m_Controls->m_AddAliasing->isChecked())
    {
        parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
        parameters.m_Misc.m_ArtifactModelString += "_ALIASING";
        parameters.m_SignalGen.m_CroppingFactor = (100-m_Controls->m_WrapBox->value())/100;
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Aliasing", DoubleProperty::New(m_Controls->m_WrapBox->value()));
    }

    // Spikes
    parameters.m_Misc.m_CheckAddSpikesBox = m_Controls->m_AddSpikes->isChecked();
    if (m_Controls->m_AddSpikes->isChecked())
    {
        parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
        parameters.m_SignalGen.m_Spikes = m_Controls->m_SpikeNumBox->value();
        parameters.m_SignalGen.m_SpikeAmplitude = m_Controls->m_SpikeScaleBox->value();
        parameters.m_Misc.m_ArtifactModelString += "_SPIKES";
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Spikes.Number", IntProperty::New(parameters.m_SignalGen.m_Spikes));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Spikes.Amplitude", DoubleProperty::New(parameters.m_SignalGen.m_SpikeAmplitude));
    }

    // gibbs ringing
    parameters.m_SignalGen.m_DoAddGibbsRinging = m_Controls->m_AddGibbsRinging->isChecked();
    if (m_Controls->m_AddGibbsRinging->isChecked())
    {
        parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Ringing", BoolProperty::New(true));
        parameters.m_Misc.m_ArtifactModelString += "_RINGING";
    }

    // add distortions
    parameters.m_Misc.m_CheckAddDistortionsBox = m_Controls->m_AddDistortions->isChecked();
    if (m_Controls->m_AddDistortions->isChecked() && m_Controls->m_FrequencyMapBox->GetSelectedNode().IsNotNull())
    {
        mitk::DataNode::Pointer fMapNode = m_Controls->m_FrequencyMapBox->GetSelectedNode();
        mitk::Image* img = dynamic_cast<mitk::Image*>(fMapNode->GetData());
        ItkDoubleImgType::Pointer itkImg = ItkDoubleImgType::New();
        CastToItkImage< ItkDoubleImgType >(img, itkImg);

        if (m_SelectedImage.IsNull())   // use geometry of frequency map
        {
            parameters.m_SignalGen.m_ImageRegion = itkImg->GetLargestPossibleRegion();
            parameters.m_SignalGen.m_ImageSpacing = itkImg->GetSpacing();
            parameters.m_SignalGen.m_ImageOrigin = itkImg->GetOrigin();
            parameters.m_SignalGen.m_ImageDirection = itkImg->GetDirection();
        }

        if (parameters.m_SignalGen.m_ImageRegion.GetSize(0)==itkImg->GetLargestPossibleRegion().GetSize(0) &&
                parameters.m_SignalGen.m_ImageRegion.GetSize(1)==itkImg->GetLargestPossibleRegion().GetSize(1) &&
                parameters.m_SignalGen.m_ImageRegion.GetSize(2)==itkImg->GetLargestPossibleRegion().GetSize(2))
        {
            parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
            itk::ImageDuplicator<ItkDoubleImgType>::Pointer duplicator = itk::ImageDuplicator<ItkDoubleImgType>::New();
            duplicator->SetInputImage(itkImg);
            duplicator->Update();
            parameters.m_SignalGen.m_FrequencyMap = duplicator->GetOutput();
            parameters.m_Misc.m_ArtifactModelString += "_DISTORTED";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Distortions", BoolProperty::New(true));
        }
    }

    parameters.m_SignalGen.m_EddyStrength = 0;
    parameters.m_Misc.m_CheckAddEddyCurrentsBox = m_Controls->m_AddEddy->isChecked();
    if (m_Controls->m_AddEddy->isChecked())
    {
        parameters.m_SignalGen.m_EddyStrength = m_Controls->m_EddyGradientStrength->value();
        parameters.m_Misc.m_ArtifactModelString += "_EDDY";
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Eddy-strength", DoubleProperty::New(parameters.m_SignalGen.m_EddyStrength));
    }

    // Motion
    parameters.m_SignalGen.m_DoAddMotion = m_Controls->m_AddMotion->isChecked();
    parameters.m_SignalGen.m_DoRandomizeMotion = m_Controls->m_RandomMotion->isChecked();
    parameters.m_SignalGen.m_Translation[0] = m_Controls->m_MaxTranslationBoxX->value();
    parameters.m_SignalGen.m_Translation[1] = m_Controls->m_MaxTranslationBoxY->value();
    parameters.m_SignalGen.m_Translation[2] = m_Controls->m_MaxTranslationBoxZ->value();
    parameters.m_SignalGen.m_Rotation[0] = m_Controls->m_MaxRotationBoxX->value();
    parameters.m_SignalGen.m_Rotation[1] = m_Controls->m_MaxRotationBoxY->value();
    parameters.m_SignalGen.m_Rotation[2] = m_Controls->m_MaxRotationBoxZ->value();
    if ( m_Controls->m_AddMotion->isChecked() && m_SelectedBundles.size()>0 )
    {
        parameters.m_Misc.m_ArtifactModelString += "_MOTION";
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Random", BoolProperty::New(parameters.m_SignalGen.m_DoRandomizeMotion));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Translation-x", DoubleProperty::New(parameters.m_SignalGen.m_Translation[0]));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Translation-y", DoubleProperty::New(parameters.m_SignalGen.m_Translation[1]));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Translation-z", DoubleProperty::New(parameters.m_SignalGen.m_Translation[2]));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Rotation-x", DoubleProperty::New(parameters.m_SignalGen.m_Rotation[0]));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Rotation-y", DoubleProperty::New(parameters.m_SignalGen.m_Rotation[1]));
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Motion.Rotation-z", DoubleProperty::New(parameters.m_SignalGen.m_Rotation[2]));
    }

    // other imaging parameters
    parameters.m_SignalGen.m_tLine = m_Controls->m_LineReadoutTimeBox->value();
    parameters.m_SignalGen.m_tInhom = m_Controls->m_T2starBox->value();
    parameters.m_SignalGen.m_tEcho = m_Controls->m_TEbox->value();
    parameters.m_SignalGen.m_DoDisablePartialVolume = m_Controls->m_EnforcePureFiberVoxelsBox->isChecked();
    parameters.m_SignalGen.m_AxonRadius = m_Controls->m_FiberRadius->value();
    parameters.m_SignalGen.m_SignalScale = m_Controls->m_SignalScaleBox->value();

    // adjust echo time if needed
    if ( parameters.m_SignalGen.m_tEcho < parameters.m_SignalGen.m_ImageRegion.GetSize(1)*parameters.m_SignalGen.m_tLine )
    {
        this->m_Controls->m_TEbox->setValue( parameters.m_SignalGen.m_ImageRegion.GetSize(1)*parameters.m_SignalGen.m_tLine );
        parameters.m_SignalGen.m_tEcho = m_Controls->m_TEbox->value();
        QMessageBox::information( NULL, "Warning", "Echo time is too short! Time not sufficient to read slice. Automaticall adjusted to "+QString::number(parameters.m_SignalGen.m_tEcho)+" ms");
    }

    // Noise
    parameters.m_Misc.m_CheckAddNoiseBox = m_Controls->m_AddNoise->isChecked();
    if (m_Controls->m_AddNoise->isChecked())
    {
        double noiseVariance = m_Controls->m_NoiseLevel->value();
        {
            switch (m_Controls->m_NoiseDistributionBox->currentIndex())
            {
            case 0:
            {
                parameters.m_NoiseModel = new mitk::RicianNoiseModel<ScalarType>();
                parameters.m_Misc.m_ArtifactModelString += "_RICIAN-";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Rician"));
                break;
            }
            case 1:
            {
                parameters.m_NoiseModel = new mitk::ChiSquareNoiseModel<ScalarType>();
                parameters.m_Misc.m_ArtifactModelString += "_CHISQUARED-";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Chi-squared"));
                break;
            }
            default:
            {
                parameters.m_NoiseModel = new mitk::RicianNoiseModel<ScalarType>();
                parameters.m_Misc.m_ArtifactModelString += "_RICIAN-";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Distribution", StringProperty::New("Rician"));
            }
            }
        }
        parameters.m_NoiseModel->SetNoiseVariance(noiseVariance);
        parameters.m_Misc.m_ArtifactModelString += QString::number(noiseVariance).toStdString();
        parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Noise-Variance", DoubleProperty::New(noiseVariance));
    }

    // adjusting line readout time to the adapted image size needed for the DFT
    unsigned int y = parameters.m_SignalGen.m_ImageRegion.GetSize(1);
    y += y%2;
    if ( y>parameters.m_SignalGen.m_ImageRegion.GetSize(1) )
        parameters.m_SignalGen.m_tLine *= (double)parameters.m_SignalGen.m_ImageRegion.GetSize(1)/y;

    // signal models
    {
        // compartment 1
        switch (m_Controls->m_Compartment1Box->currentIndex())
        {
        case 0:
        {
            mitk::StickModel<ScalarType>* model = new mitk::StickModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity(m_Controls->m_StickWidget1->GetD());
            model->SetT2(m_Controls->m_StickWidget1->GetT2());
            model->m_CompartmentId = 1;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Stick";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Stick") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D", DoubleProperty::New(m_Controls->m_StickWidget1->GetD()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 1:
        {
            mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity1(m_Controls->m_ZeppelinWidget1->GetD1());
            model->SetDiffusivity2(m_Controls->m_ZeppelinWidget1->GetD2());
            model->SetDiffusivity3(m_Controls->m_ZeppelinWidget1->GetD2());
            model->SetT2(m_Controls->m_ZeppelinWidget1->GetT2());
            model->m_CompartmentId = 1;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Zeppelin";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Zeppelin") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D1", DoubleProperty::New(m_Controls->m_ZeppelinWidget1->GetD1()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D2", DoubleProperty::New(m_Controls->m_ZeppelinWidget1->GetD2()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 2:
        {
            mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity1(m_Controls->m_TensorWidget1->GetD1());
            model->SetDiffusivity2(m_Controls->m_TensorWidget1->GetD2());
            model->SetDiffusivity3(m_Controls->m_TensorWidget1->GetD3());
            model->SetT2(m_Controls->m_TensorWidget1->GetT2());
            model->m_CompartmentId = 1;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Tensor";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Tensor") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D1", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD1()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D2", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD2()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.D3", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD3()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 3:
        {
            mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
            parameters.m_SignalGen.m_DoSimulateRelaxation = false;
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetMaxNumKernels(m_Controls->m_PrototypeWidget1->GetNumberOfSamples());
            model->SetFaRange(m_Controls->m_PrototypeWidget1->GetMinFa(), m_Controls->m_PrototypeWidget1->GetMaxFa());
            model->SetAdcRange(m_Controls->m_PrototypeWidget1->GetMinAdc(), m_Controls->m_PrototypeWidget1->GetMaxAdc());
            model->m_CompartmentId = 1;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Prototype";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Prototype") );
            break;
        }
        }

        // compartment 2
        switch (m_Controls->m_Compartment2Box->currentIndex())
        {
        case 0:
            break;
        case 1:
        {
            mitk::StickModel<ScalarType>* model = new mitk::StickModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity(m_Controls->m_StickWidget2->GetD());
            model->SetT2(m_Controls->m_StickWidget2->GetT2());
            model->m_CompartmentId = 2;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Stick";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Stick") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D", DoubleProperty::New(m_Controls->m_StickWidget2->GetD()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 2:
        {
            mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity1(m_Controls->m_ZeppelinWidget2->GetD1());
            model->SetDiffusivity2(m_Controls->m_ZeppelinWidget2->GetD2());
            model->SetDiffusivity3(m_Controls->m_ZeppelinWidget2->GetD2());
            model->SetT2(m_Controls->m_ZeppelinWidget2->GetT2());
            model->m_CompartmentId = 2;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Zeppelin";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Zeppelin") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D1", DoubleProperty::New(m_Controls->m_ZeppelinWidget2->GetD1()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D2", DoubleProperty::New(m_Controls->m_ZeppelinWidget2->GetD2()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 3:
        {
            mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity1(m_Controls->m_TensorWidget2->GetD1());
            model->SetDiffusivity2(m_Controls->m_TensorWidget2->GetD2());
            model->SetDiffusivity3(m_Controls->m_TensorWidget2->GetD3());
            model->SetT2(m_Controls->m_TensorWidget2->GetT2());
            model->m_CompartmentId = 2;
            parameters.m_FiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Tensor";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Tensor") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D1", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD1()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D2", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD2()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.D3", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD3()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        }

        // compartment 3
        switch (m_Controls->m_Compartment3Box->currentIndex())
        {
        case 0:
        {
            mitk::BallModel<ScalarType>* model = new mitk::BallModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity(m_Controls->m_BallWidget1->GetD());
            model->SetT2(m_Controls->m_BallWidget1->GetT2());
            model->m_CompartmentId = 3;
            parameters.m_NonFiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Ball";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Ball") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.D", DoubleProperty::New(m_Controls->m_BallWidget1->GetD()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 1:
        {
            mitk::AstroStickModel<ScalarType>* model = new mitk::AstroStickModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
            model->SetDiffusivity(m_Controls->m_AstrosticksWidget1->GetD());
            model->SetT2(m_Controls->m_AstrosticksWidget1->GetT2());
            model->SetRandomizeSticks(m_Controls->m_AstrosticksWidget1->GetRandomizeSticks());
            model->m_CompartmentId = 3;
            parameters.m_NonFiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Astrosticks";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Astrosticks") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.D", DoubleProperty::New(m_Controls->m_AstrosticksWidget1->GetD()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(model->GetT2()) );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.RandomSticks", BoolProperty::New(m_Controls->m_AstrosticksWidget1->GetRandomizeSticks()) );
            break;
        }
        case 2:
        {
            mitk::DotModel<ScalarType>* model = new mitk::DotModel<ScalarType>();
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetT2(m_Controls->m_DotWidget1->GetT2());
            model->m_CompartmentId = 3;
            parameters.m_NonFiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Dot";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Dot") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(model->GetT2()) );
            break;
        }
        case 3:
        {
            mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
            parameters.m_SignalGen.m_DoSimulateRelaxation = false;
            model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
            model->SetMaxNumKernels(m_Controls->m_PrototypeWidget3->GetNumberOfSamples());
            model->SetFaRange(m_Controls->m_PrototypeWidget3->GetMinFa(), m_Controls->m_PrototypeWidget3->GetMaxFa());
            model->SetAdcRange(m_Controls->m_PrototypeWidget3->GetMinAdc(), m_Controls->m_PrototypeWidget3->GetMaxAdc());
            model->m_CompartmentId = 3;
            parameters.m_NonFiberModelList.push_back(model);
            parameters.m_Misc.m_SignalModelString += "Prototype";
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
            parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Prototype") );
            break;
        }
        }

        // compartment 4
        ItkDoubleImgType::Pointer comp4VolumeImage = NULL;
        ItkDoubleImgType::Pointer comp3VolumeImage = NULL;
        if (m_Controls->m_Compartment4Box->currentIndex()>0)
        {
            mitk::DataNode::Pointer volumeNode = m_Controls->m_Comp4VolumeFraction->GetSelectedNode();
            if (volumeNode.IsNull())
            {
                QMessageBox::information( NULL, "Information", "No volume fraction image selected! Second extra-axonal compartment has been disabled for this simultation.");
                MITK_WARN << "No volume fraction image selected! Second extra-axonal compartment has been disabled.";
            }
            else
            {
                MITK_INFO << "Rescaling volume fraction image...";
                comp4VolumeImage = ItkDoubleImgType::New();
                mitk::Image* img = dynamic_cast<mitk::Image*>(volumeNode->GetData());
                CastToItkImage< ItkDoubleImgType >(img, comp4VolumeImage);

                double max = itk::NumericTraits<double>::min();
                double min = itk::NumericTraits<double>::max();

                itk::ImageRegionIterator< ItkDoubleImgType > it(comp4VolumeImage, comp4VolumeImage->GetLargestPossibleRegion());
                while(!it.IsAtEnd())
                {
                    if (parameters.m_SignalGen.m_MaskImage.IsNotNull() && parameters.m_SignalGen.m_MaskImage->GetPixel(it.GetIndex())<=0)
                    {
                        it.Set(0.0);
                        ++it;
                        continue;
                    }

                    if (it.Get()>max)
                        max = it.Get();
                    if (it.Get()<min)
                        min = it.Get();
                    ++it;
                }
                itk::ShiftScaleImageFilter< ItkDoubleImgType, ItkDoubleImgType >::Pointer scaler = itk::ShiftScaleImageFilter< ItkDoubleImgType, ItkDoubleImgType >::New();
                scaler->SetInput(comp4VolumeImage);
                scaler->SetShift(-min);
                scaler->SetScale(1.0/(max-min));
                scaler->Update();
                comp4VolumeImage = scaler->GetOutput();

                //            itk::ImageFileWriter< ItkDoubleImgType >::Pointer wr = itk::ImageFileWriter< ItkDoubleImgType >::New();
                //            wr->SetInput(comp4VolumeImage);
                //            wr->SetFileName("/local/comp4.nrrd");
                //            wr->Update();

                //            if (max>1 || min<0) // are volume fractions between 0 and 1?
                //            {
                //                itk::RescaleIntensityImageFilter<ItkDoubleImgType,ItkDoubleImgType>::Pointer rescaler = itk::RescaleIntensityImageFilter<ItkDoubleImgType,ItkDoubleImgType>::New();
                //                rescaler->SetInput(0, comp4VolumeImage);
                //                rescaler->SetOutputMaximum(1);
                //                rescaler->SetOutputMinimum(0);
                //                rescaler->Update();
                //                comp4VolumeImage = rescaler->GetOutput();
                //            }

                itk::InvertIntensityImageFilter< ItkDoubleImgType, ItkDoubleImgType >::Pointer inverter = itk::InvertIntensityImageFilter< ItkDoubleImgType, ItkDoubleImgType >::New();
                inverter->SetMaximum(1.0);
                inverter->SetInput(comp4VolumeImage);
                inverter->Update();
                comp3VolumeImage = inverter->GetOutput();
            }
        }

        if (comp4VolumeImage.IsNotNull())
        {
            switch (m_Controls->m_Compartment4Box->currentIndex())
            {
            case 0:
                break;
            case 1:
            {
                mitk::BallModel<ScalarType>* model = new mitk::BallModel<ScalarType>();
                model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
                model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
                model->SetDiffusivity(m_Controls->m_BallWidget2->GetD());
                model->SetT2(m_Controls->m_BallWidget2->GetT2());
                model->SetVolumeFractionImage(comp4VolumeImage);
                model->m_CompartmentId = 4;
                parameters.m_NonFiberModelList.back()->SetVolumeFractionImage(comp3VolumeImage);
                parameters.m_NonFiberModelList.push_back(model);
                parameters.m_Misc.m_SignalModelString += "Ball";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Ball") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.D", DoubleProperty::New(m_Controls->m_BallWidget2->GetD()) );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(model->GetT2()) );
                break;
            }
            case 2:
            {
                mitk::AstroStickModel<ScalarType>* model = new mitk::AstroStickModel<ScalarType>();
                model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
                model->SetBvalue(parameters.m_SignalGen.m_Bvalue);
                model->SetDiffusivity(m_Controls->m_AstrosticksWidget2->GetD());
                model->SetT2(m_Controls->m_AstrosticksWidget2->GetT2());
                model->SetRandomizeSticks(m_Controls->m_AstrosticksWidget2->GetRandomizeSticks());
                parameters.m_NonFiberModelList.back()->SetVolumeFractionImage(comp3VolumeImage);
                model->SetVolumeFractionImage(comp4VolumeImage);
                model->m_CompartmentId = 4;
                parameters.m_NonFiberModelList.push_back(model);
                parameters.m_Misc.m_SignalModelString += "Astrosticks";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Astrosticks") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.D", DoubleProperty::New(m_Controls->m_AstrosticksWidget2->GetD()) );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(model->GetT2()) );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.RandomSticks", BoolProperty::New(m_Controls->m_AstrosticksWidget2->GetRandomizeSticks()) );
                break;
            }
            case 3:
            {
                mitk::DotModel<ScalarType>* model = new mitk::DotModel<ScalarType>();
                model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
                model->SetT2(m_Controls->m_DotWidget2->GetT2());
                model->SetVolumeFractionImage(comp4VolumeImage);
                model->m_CompartmentId = 4;
                parameters.m_NonFiberModelList.back()->SetVolumeFractionImage(comp3VolumeImage);
                parameters.m_NonFiberModelList.push_back(model);
                parameters.m_Misc.m_SignalModelString += "Dot";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Dot") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(model->GetT2()) );
                break;
            }
            case 4:
            {
                mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
                parameters.m_SignalGen.m_DoSimulateRelaxation = false;
                model->SetGradientList(parameters.m_SignalGen.GetGradientDirections());
                model->SetMaxNumKernels(m_Controls->m_PrototypeWidget4->GetNumberOfSamples());
                model->SetFaRange(m_Controls->m_PrototypeWidget4->GetMinFa(), m_Controls->m_PrototypeWidget4->GetMaxFa());
                model->SetAdcRange(m_Controls->m_PrototypeWidget4->GetMinAdc(), m_Controls->m_PrototypeWidget4->GetMaxAdc());
                model->SetVolumeFractionImage(comp4VolumeImage);
                model->m_CompartmentId = 4;
                parameters.m_NonFiberModelList.back()->SetVolumeFractionImage(comp3VolumeImage);
                parameters.m_NonFiberModelList.push_back(model);
                parameters.m_Misc.m_SignalModelString += "Prototype";
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
                parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Prototype") );
                break;
            }
            }
        }
    }

    parameters.m_SignalGen.m_FiberSeparationThreshold = m_Controls->m_SeparationAngleBox->value();
    switch (m_Controls->m_DiffusionDirectionBox->currentIndex())
    {
    case 0:
        parameters.m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS;
        break;
    case 1:
        parameters.m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::MAIN_FIBER_DIRECTIONS;
        break;
    case 2:
        parameters.m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::RANDOM_DIRECTIONS;
        parameters.m_SignalGen.m_DoAddMotion = false;
        parameters.m_SignalGen.m_DoAddGibbsRinging = false;
        parameters.m_SignalGen.m_KspaceLineOffset = 0.0;
        parameters.m_SignalGen.m_FrequencyMap = NULL;
        parameters.m_SignalGen.m_CroppingFactor = 1.0;
        parameters.m_SignalGen.m_EddyStrength = 0;
        break;
    default:
        parameters.m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS;
    }

    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.SignalScale", IntProperty::New(parameters.m_SignalGen.m_SignalScale));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.FiberRadius", IntProperty::New(parameters.m_SignalGen.m_AxonRadius));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Tinhom", DoubleProperty::New(parameters.m_SignalGen.m_tInhom));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Tline", DoubleProperty::New(parameters.m_SignalGen.m_tLine));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.TE", DoubleProperty::New(parameters.m_SignalGen.m_tEcho));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.b-value", DoubleProperty::New(parameters.m_SignalGen.m_Bvalue));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.NoPartialVolume", BoolProperty::New(parameters.m_SignalGen.m_DoDisablePartialVolume));
    parameters.m_Misc.m_ResultNode->AddProperty("Fiberfox.Relaxation", BoolProperty::New(parameters.m_SignalGen.m_DoSimulateRelaxation));
    parameters.m_Misc.m_ResultNode->AddProperty("binary", BoolProperty::New(false));

    parameters.m_Misc.m_CheckRealTimeFibersBox = m_Controls->m_RealTimeFibers->isChecked();
    parameters.m_Misc.m_CheckAdvancedFiberOptionsBox = m_Controls->m_AdvancedOptionsBox->isChecked();
    parameters.m_Misc.m_CheckIncludeFiducialsBox = m_Controls->m_IncludeFiducials->isChecked();
    parameters.m_Misc.m_CheckConstantRadiusBox = m_Controls->m_ConstantRadiusBox->isChecked();

    switch(m_Controls->m_DistributionBox->currentIndex())
    {
    case 0:
        parameters.m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
        break;
    case 1:
        parameters.m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_GAUSSIAN;
        break;
    default:
        parameters.m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
    }
    parameters.m_FiberGen.m_Variance = m_Controls->m_VarianceBox->value();
    parameters.m_FiberGen.m_Density = m_Controls->m_FiberDensityBox->value();
    parameters.m_FiberGen.m_Sampling = m_Controls->m_FiberSamplingBox->value();
    parameters.m_FiberGen.m_Tension = m_Controls->m_TensionBox->value();
    parameters.m_FiberGen.m_Continuity = m_Controls->m_ContinuityBox->value();
    parameters.m_FiberGen.m_Bias = m_Controls->m_BiasBox->value();
    parameters.m_FiberGen.m_Rotation[0] = m_Controls->m_XrotBox->value();
    parameters.m_FiberGen.m_Rotation[1] = m_Controls->m_YrotBox->value();
    parameters.m_FiberGen.m_Rotation[2] = m_Controls->m_ZrotBox->value();
    parameters.m_FiberGen.m_Translation[0] = m_Controls->m_XtransBox->value();
    parameters.m_FiberGen.m_Translation[1] = m_Controls->m_YtransBox->value();
    parameters.m_FiberGen.m_Translation[2] = m_Controls->m_ZtransBox->value();
    parameters.m_FiberGen.m_Scale[0] = m_Controls->m_XscaleBox->value();
    parameters.m_FiberGen.m_Scale[1] = m_Controls->m_YscaleBox->value();
    parameters.m_FiberGen.m_Scale[2] = m_Controls->m_ZscaleBox->value();

    return parameters;
}

void QmitkFiberfoxView::SaveParameters()
{
    FiberfoxParameters<> ffParamaters = UpdateImageParameters<double>();

    QString filename = QFileDialog::getSaveFileName(
                0,
                tr("Save Parameters"),
                m_ParameterFile,
                tr("Fiberfox Parameters (*.ffp)") );

    bool ok = true;
    bool first = true;
    bool dosampling = false;
    mitk::DiffusionImage<short>::Pointer diffImg;
    itk::Image< itk::DiffusionTensor3D< double >, 3 >::Pointer tensorImage = NULL;
    const int shOrder = 2;
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,shOrder,QBALL_ODFSIZE> QballFilterType;
    QballFilterType::CoefficientImageType::Pointer itkFeatureImage = NULL;
    ItkDoubleImgType::Pointer adcImage = NULL;

    for (unsigned int i=0; i<ffParamaters.m_FiberModelList.size()+ffParamaters.m_NonFiberModelList.size(); i++)
    {
        mitk::RawShModel<>* model = NULL;
        if (i<ffParamaters.m_FiberModelList.size())
            model = dynamic_cast<  mitk::RawShModel<>* >(ffParamaters.m_FiberModelList.at(i));
        else
            model = dynamic_cast<  mitk::RawShModel<>* >(ffParamaters.m_NonFiberModelList.at(i-ffParamaters.m_FiberModelList.size()));

        if (model!=0 && model->GetNumberOfKernels()<=0)
        {
            if (first==true)
            {
                if (QMessageBox::question(NULL, "Prototype signal sampling", "Do you want to sample prototype signals from the selected diffusion-weighted imag and save them?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
                    dosampling = true;

                first = false;
                if (dosampling && m_SelectedDWI.IsNull())
                {
                    QMessageBox::information(NULL, "Parameter file not saved", "No diffusion-weighted image selected to sample signal from.");
                    return;
                }
                else if (dosampling)
                {
                    diffImg = dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedDWI->GetData());

                    typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, double > TensorReconstructionImageFilterType;
                    TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
                    filter->SetGradientImage( diffImg->GetDirections(), diffImg->GetVectorImage() );
                    filter->SetBValue(diffImg->GetReferenceBValue());
                    filter->Update();
                    tensorImage = filter->GetOutput();

                    const int NumCoeffs = (shOrder*shOrder + shOrder + 2)/2 + shOrder;
                    QballFilterType::Pointer qballfilter = QballFilterType::New();
                    qballfilter->SetGradientImage( diffImg->GetDirections(), diffImg->GetVectorImage() );
                    qballfilter->SetBValue(diffImg->GetReferenceBValue());
                    qballfilter->SetLambda(0.006);
                    qballfilter->SetNormalizationMethod(QballFilterType::QBAR_RAW_SIGNAL);
                    qballfilter->Update();
                    itkFeatureImage = qballfilter->GetCoefficientImage();

                    itk::AdcImageFilter< short, double >::Pointer adcFilter = itk::AdcImageFilter< short, double >::New();
                    adcFilter->SetInput(diffImg->GetVectorImage());
                    adcFilter->SetGradientDirections(diffImg->GetDirections());
                    adcFilter->SetB_value(diffImg->GetReferenceBValue());
                    adcFilter->Update();
                    adcImage = adcFilter->GetOutput();
                }
            }

            if (dosampling && m_SelectedDWI.IsNotNull())
            {
                ok = model->SampleKernels(diffImg, ffParamaters.m_SignalGen.m_MaskImage, tensorImage, itkFeatureImage, adcImage);
                if (!ok)
                {
                    QMessageBox::information( NULL, "Parameter file not saved", "No valid prototype signals could be sampled.");
                    return;
                }
            }
        }
    }

    ffParamaters.SaveParameters(filename.toStdString());
    m_ParameterFile = filename;
}

void QmitkFiberfoxView::LoadParameters()
{
    QString filename = QFileDialog::getOpenFileName(0, tr("Load Parameters"), QString(itksys::SystemTools::GetFilenamePath(m_ParameterFile.toStdString()).c_str()), tr("Fiberfox Parameters (*.ffp)") );
    if(filename.isEmpty() || filename.isNull())
        return;

    m_ParameterFile = filename;

    FiberfoxParameters<> parameters;
    parameters.LoadParameters(filename.toStdString());

    m_Controls->m_RealTimeFibers->setChecked(parameters.m_Misc.m_CheckRealTimeFibersBox);
    m_Controls->m_AdvancedOptionsBox->setChecked(parameters.m_Misc.m_CheckAdvancedFiberOptionsBox);
    m_Controls->m_IncludeFiducials->setChecked(parameters.m_Misc.m_CheckIncludeFiducialsBox);
    m_Controls->m_ConstantRadiusBox->setChecked(parameters.m_Misc.m_CheckConstantRadiusBox);

    m_Controls->m_DistributionBox->setCurrentIndex(parameters.m_FiberGen.m_Distribution);
    m_Controls->m_VarianceBox->setValue(parameters.m_FiberGen.m_Variance);
    m_Controls->m_FiberDensityBox->setValue(parameters.m_FiberGen.m_Density);
    m_Controls->m_FiberSamplingBox->setValue(parameters.m_FiberGen.m_Sampling);
    m_Controls->m_TensionBox->setValue(parameters.m_FiberGen.m_Tension);
    m_Controls->m_ContinuityBox->setValue(parameters.m_FiberGen.m_Continuity);
    m_Controls->m_BiasBox->setValue(parameters.m_FiberGen.m_Bias);
    m_Controls->m_XrotBox->setValue(parameters.m_FiberGen.m_Rotation[0]);
    m_Controls->m_YrotBox->setValue(parameters.m_FiberGen.m_Rotation[1]);
    m_Controls->m_ZrotBox->setValue(parameters.m_FiberGen.m_Rotation[2]);
    m_Controls->m_XtransBox->setValue(parameters.m_FiberGen.m_Translation[0]);
    m_Controls->m_YtransBox->setValue(parameters.m_FiberGen.m_Translation[1]);
    m_Controls->m_ZtransBox->setValue(parameters.m_FiberGen.m_Translation[2]);
    m_Controls->m_XscaleBox->setValue(parameters.m_FiberGen.m_Scale[0]);
    m_Controls->m_YscaleBox->setValue(parameters.m_FiberGen.m_Scale[1]);
    m_Controls->m_ZscaleBox->setValue(parameters.m_FiberGen.m_Scale[2]);

    // image generation parameters
    m_Controls->m_SizeX->setValue(parameters.m_SignalGen.m_ImageRegion.GetSize(0));
    m_Controls->m_SizeY->setValue(parameters.m_SignalGen.m_ImageRegion.GetSize(1));
    m_Controls->m_SizeZ->setValue(parameters.m_SignalGen.m_ImageRegion.GetSize(2));
    m_Controls->m_SpacingX->setValue(parameters.m_SignalGen.m_ImageSpacing[0]);
    m_Controls->m_SpacingY->setValue(parameters.m_SignalGen.m_ImageSpacing[1]);
    m_Controls->m_SpacingZ->setValue(parameters.m_SignalGen.m_ImageSpacing[2]);
    m_Controls->m_NumGradientsBox->setValue(parameters.m_SignalGen.GetNumWeightedVolumes());
    m_Controls->m_BvalueBox->setValue(parameters.m_SignalGen.m_Bvalue);
    m_Controls->m_SignalScaleBox->setValue(parameters.m_SignalGen.m_SignalScale);
    m_Controls->m_TEbox->setValue(parameters.m_SignalGen.m_tEcho);
    m_Controls->m_LineReadoutTimeBox->setValue(parameters.m_SignalGen.m_tLine);
    m_Controls->m_T2starBox->setValue(parameters.m_SignalGen.m_tInhom);
    m_Controls->m_FiberRadius->setValue(parameters.m_SignalGen.m_AxonRadius);
    m_Controls->m_RelaxationBox->setChecked(parameters.m_SignalGen.m_DoSimulateRelaxation);
    m_Controls->m_EnforcePureFiberVoxelsBox->setChecked(parameters.m_SignalGen.m_DoDisablePartialVolume);

    if (parameters.m_NoiseModel!=NULL)
    {
        m_Controls->m_AddNoise->setChecked(parameters.m_Misc.m_CheckAddNoiseBox);
        if (dynamic_cast<mitk::RicianNoiseModel<double>*>(parameters.m_NoiseModel))
            m_Controls->m_NoiseDistributionBox->setCurrentIndex(0);
        else if (dynamic_cast<mitk::ChiSquareNoiseModel<double>*>(parameters.m_NoiseModel))
            m_Controls->m_NoiseDistributionBox->setCurrentIndex(1);
        m_Controls->m_NoiseLevel->setValue(parameters.m_NoiseModel->GetNoiseVariance());
    }
    else
        m_Controls->m_AddNoise->setChecked(false);

    m_Controls->m_VolumeFractionsBox->setChecked(parameters.m_Misc.m_CheckOutputVolumeFractionsBox);
    m_Controls->m_AdvancedOptionsBox_2->setChecked(parameters.m_Misc.m_CheckAdvancedSignalOptionsBox);
    m_Controls->m_AddGhosts->setChecked(parameters.m_Misc.m_CheckAddGhostsBox);
    m_Controls->m_AddAliasing->setChecked(parameters.m_Misc.m_CheckAddAliasingBox);
    m_Controls->m_AddDistortions->setChecked(parameters.m_Misc.m_CheckAddDistortionsBox);
    m_Controls->m_AddSpikes->setChecked(parameters.m_Misc.m_CheckAddSpikesBox);
    m_Controls->m_AddEddy->setChecked(parameters.m_Misc.m_CheckAddEddyCurrentsBox);

    m_Controls->m_kOffsetBox->setValue(parameters.m_SignalGen.m_KspaceLineOffset);
    m_Controls->m_WrapBox->setValue(100*(1-parameters.m_SignalGen.m_CroppingFactor));
    m_Controls->m_SpikeNumBox->setValue(parameters.m_SignalGen.m_Spikes);
    m_Controls->m_SpikeScaleBox->setValue(parameters.m_SignalGen.m_SpikeAmplitude);
    m_Controls->m_EddyGradientStrength->setValue(parameters.m_SignalGen.m_EddyStrength);
    m_Controls->m_AddGibbsRinging->setChecked(parameters.m_SignalGen.m_DoAddGibbsRinging);
    m_Controls->m_AddMotion->setChecked(parameters.m_SignalGen.m_DoAddMotion);
    m_Controls->m_RandomMotion->setChecked(parameters.m_SignalGen.m_DoRandomizeMotion);
    m_Controls->m_MaxTranslationBoxX->setValue(parameters.m_SignalGen.m_Translation[0]);
    m_Controls->m_MaxTranslationBoxY->setValue(parameters.m_SignalGen.m_Translation[1]);
    m_Controls->m_MaxTranslationBoxZ->setValue(parameters.m_SignalGen.m_Translation[2]);
    m_Controls->m_MaxRotationBoxX->setValue(parameters.m_SignalGen.m_Rotation[0]);
    m_Controls->m_MaxRotationBoxY->setValue(parameters.m_SignalGen.m_Rotation[1]);
    m_Controls->m_MaxRotationBoxZ->setValue(parameters.m_SignalGen.m_Rotation[2]);
    m_Controls->m_DiffusionDirectionBox->setCurrentIndex(parameters.m_SignalGen.m_DiffusionDirectionMode);
    m_Controls->m_SeparationAngleBox->setValue(parameters.m_SignalGen.m_FiberSeparationThreshold);

    m_Controls->m_Compartment1Box->setCurrentIndex(0);
    m_Controls->m_Compartment2Box->setCurrentIndex(0);
    m_Controls->m_Compartment3Box->setCurrentIndex(0);
    m_Controls->m_Compartment4Box->setCurrentIndex(0);

    for (unsigned int i=0; i<parameters.m_FiberModelList.size()+parameters.m_NonFiberModelList.size(); i++)
    {
        mitk::DiffusionSignalModel<ScalarType>* signalModel = NULL;
        if (i<parameters.m_FiberModelList.size())
            signalModel = parameters.m_FiberModelList.at(i);
        else
            signalModel = parameters.m_NonFiberModelList.at(i-parameters.m_FiberModelList.size());

        switch (signalModel->m_CompartmentId)
        {
        case 1:
        {
            if (dynamic_cast<mitk::StickModel<>*>(signalModel))
            {
                mitk::StickModel<>* model = dynamic_cast<mitk::StickModel<>*>(signalModel);
                m_Controls->m_StickWidget1->SetT2(model->GetT2());
                m_Controls->m_StickWidget1->SetD(model->GetDiffusivity());
                m_Controls->m_Compartment1Box->setCurrentIndex(0);
                break;
            }
            else if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
            {
                mitk::TensorModel<>* model = dynamic_cast<mitk::TensorModel<>*>(signalModel);
                m_Controls->m_TensorWidget1->SetT2(model->GetT2());
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
            if (dynamic_cast<mitk::StickModel<>*>(signalModel))
            {
                mitk::StickModel<>* model = dynamic_cast<mitk::StickModel<>*>(signalModel);
                m_Controls->m_StickWidget2->SetT2(model->GetT2());
                m_Controls->m_StickWidget2->SetD(model->GetDiffusivity());
                m_Controls->m_Compartment2Box->setCurrentIndex(1);
                break;
            }
            else if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
            {
                mitk::TensorModel<>* model = dynamic_cast<mitk::TensorModel<>*>(signalModel);
                m_Controls->m_TensorWidget2->SetT2(model->GetT2());
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
            if (dynamic_cast<mitk::BallModel<>*>(signalModel))
            {
                mitk::BallModel<>* model = dynamic_cast<mitk::BallModel<>*>(signalModel);
                m_Controls->m_BallWidget1->SetT2(model->GetT2());
                m_Controls->m_BallWidget1->SetD(model->GetDiffusivity());
                m_Controls->m_Compartment3Box->setCurrentIndex(0);
                break;
            }
            else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
            {
                mitk::AstroStickModel<>* model = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);
                m_Controls->m_AstrosticksWidget1->SetT2(model->GetT2());
                m_Controls->m_AstrosticksWidget1->SetD(model->GetDiffusivity());
                m_Controls->m_AstrosticksWidget1->SetRandomizeSticks(model->GetRandomizeSticks());
                m_Controls->m_Compartment3Box->setCurrentIndex(1);
                break;
            }
            else if (dynamic_cast<mitk::DotModel<>*>(signalModel))
            {
                mitk::DotModel<>* model = dynamic_cast<mitk::DotModel<>*>(signalModel);
                m_Controls->m_DotWidget1->SetT2(model->GetT2());
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
            if (dynamic_cast<mitk::BallModel<>*>(signalModel))
            {
                mitk::BallModel<>* model = dynamic_cast<mitk::BallModel<>*>(signalModel);
                m_Controls->m_BallWidget2->SetT2(model->GetT2());
                m_Controls->m_BallWidget2->SetD(model->GetDiffusivity());
                m_Controls->m_Compartment4Box->setCurrentIndex(1);
                break;
            }
            else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
            {
                mitk::AstroStickModel<>* model = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);
                m_Controls->m_AstrosticksWidget2->SetT2(model->GetT2());
                m_Controls->m_AstrosticksWidget2->SetD(model->GetDiffusivity());
                m_Controls->m_AstrosticksWidget2->SetRandomizeSticks(model->GetRandomizeSticks());
                m_Controls->m_Compartment4Box->setCurrentIndex(2);
                break;
            }
            else if (dynamic_cast<mitk::DotModel<>*>(signalModel))
            {
                mitk::DotModel<>* model = dynamic_cast<mitk::DotModel<>*>(signalModel);
                m_Controls->m_DotWidget2->SetT2(model->GetT2());
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
}

void QmitkFiberfoxView::ShowAdvancedOptions(int state)
{
    if (state)
    {
        m_Controls->m_AdvancedFiberOptionsFrame->setVisible(true);
        m_Controls->m_AdvancedSignalOptionsFrame->setVisible(true);
        m_Controls->m_AdvancedOptionsBox->setChecked(true);
        m_Controls->m_AdvancedOptionsBox_2->setChecked(true);
    }
    else
    {
        m_Controls->m_AdvancedFiberOptionsFrame->setVisible(false);
        m_Controls->m_AdvancedSignalOptionsFrame->setVisible(false);
        m_Controls->m_AdvancedOptionsBox->setChecked(false);
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

    switch (index)
    {
    case 0:
        break;
    case 1:
        m_Controls->m_StickWidget2->setVisible(true);
        break;
    case 2:
        m_Controls->m_ZeppelinWidget2->setVisible(true);
        break;
    case 3:
        m_Controls->m_TensorWidget2->setVisible(true);
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

void QmitkFiberfoxView::OnConstantRadius(int value)
{
    if (value>0 && m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnAddMotion(int value)
{
    if (value>0)
        m_Controls->m_MotionArtifactFrame->setVisible(true);
    else
        m_Controls->m_MotionArtifactFrame->setVisible(false);
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

void QmitkFiberfoxView::OnDistributionChanged(int value)
{
    if (value==1)
        m_Controls->m_VarianceBox->setVisible(true);
    else
        m_Controls->m_VarianceBox->setVisible(false);

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnVarianceChanged(double)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFiberDensityChanged(int)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFiberSamplingChanged(double)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnTensionChanged(double)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnContinuityChanged(double)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnBiasChanged(double)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::AlignOnGrid()
{
    for (unsigned int i=0; i<m_SelectedFiducials.size(); i++)
    {
        mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(i)->GetData());
        mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

        mitk::DataStorage::SetOfObjects::ConstPointer parentFibs = GetDataStorage()->GetSources(m_SelectedFiducials.at(i));
        for( mitk::DataStorage::SetOfObjects::const_iterator it = parentFibs->begin(); it != parentFibs->end(); ++it )
        {
            mitk::DataNode::Pointer pFibNode = *it;
            if ( pFibNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(pFibNode->GetData()) )
            {
                mitk::DataStorage::SetOfObjects::ConstPointer parentImgs = GetDataStorage()->GetSources(pFibNode);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = parentImgs->begin(); it2 != parentImgs->end(); ++it2 )
                {
                    mitk::DataNode::Pointer pImgNode = *it2;
                    if ( pImgNode.IsNotNull() && dynamic_cast<mitk::Image*>(pImgNode->GetData()) )
                    {
                        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(pImgNode->GetData());
                        mitk::BaseGeometry::Pointer geom = img->GetGeometry();
                        itk::Index<3> idx;
                        geom->WorldToIndex(wc0, idx);

                        mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
                        mitk::Point3D world;
                        geom->IndexToWorld(cIdx,world);

                        mitk::Vector3D trans = world - wc0;
                        pe->GetGeometry()->Translate(trans);

                        break;
                    }
                }
                break;
            }
        }
    }

    for(unsigned int i=0; i<m_SelectedBundles2.size(); i++ )
    {
        mitk::DataNode::Pointer fibNode = m_SelectedBundles2.at(i);

        mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(fibNode);
        for( mitk::DataStorage::SetOfObjects::const_iterator it = sources->begin(); it != sources->end(); ++it )
        {
            mitk::DataNode::Pointer imgNode = *it;
            if ( imgNode.IsNotNull() && dynamic_cast<mitk::Image*>(imgNode->GetData()) )
            {
                mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(fibNode);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
                {
                    mitk::DataNode::Pointer fiducialNode = *it2;
                    if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                    {
                        mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
                        mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

                        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(imgNode->GetData());
                        mitk::BaseGeometry::Pointer geom = img->GetGeometry();
                        itk::Index<3> idx;
                        geom->WorldToIndex(wc0, idx);
                        mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
                        mitk::Point3D world;
                        geom->IndexToWorld(cIdx,world);

                        mitk::Vector3D trans = world - wc0;
                        pe->GetGeometry()->Translate(trans);
                    }
                }

                break;
            }
        }
    }

    for(unsigned int i=0; i<m_SelectedImages.size(); i++ )
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_SelectedImages.at(i)->GetData());

        mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(m_SelectedImages.at(i));
        for( mitk::DataStorage::SetOfObjects::const_iterator it = derivations->begin(); it != derivations->end(); ++it )
        {
            mitk::DataNode::Pointer fibNode = *it;
            if ( fibNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(fibNode->GetData()) )
            {
                mitk::DataStorage::SetOfObjects::ConstPointer derivations2 = GetDataStorage()->GetDerivations(fibNode);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations2->begin(); it2 != derivations2->end(); ++it2 )
                {
                    mitk::DataNode::Pointer fiducialNode = *it2;
                    if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                    {
                        mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
                        mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

                        mitk::BaseGeometry::Pointer geom = img->GetGeometry();
                        itk::Index<3> idx;
                        geom->WorldToIndex(wc0, idx);
                        mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
                        mitk::Point3D world;
                        geom->IndexToWorld(cIdx,world);

                        mitk::Vector3D trans = world - wc0;
                        pe->GetGeometry()->Translate(trans);
                    }
                }
            }
        }
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFlipButton()
{
    if (m_SelectedFiducial.IsNull())
        return;

    std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(m_SelectedFiducial.GetPointer());
    if( it != m_DataNodeToPlanarFigureData.end() )
    {
        QmitkPlanarFigureData& data = it->second;
        data.m_Flipped += 1;
        data.m_Flipped %= 2;
    }

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
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
    double C = sqrt(4*M_PI);
    phi(0) = 0.0;
    phi(NPoints-1) = 0.0;
    for(int i=0; i<NPoints; i++)
    {
        theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - M_PI / 2.0;
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

void QmitkFiberfoxView::OnAddBundle()
{
    if (m_SelectedImage.IsNull())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedImage);

    mitk::FiberBundleX::Pointer bundle = mitk::FiberBundleX::New();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( bundle );
    QString name = QString("Bundle_%1").arg(children->size());
    node->SetName(name.toStdString());
    m_SelectedBundles.push_back(node);
    UpdateGui();

    GetDataStorage()->Add(node, m_SelectedImage);
}

void QmitkFiberfoxView::OnDrawROI()
{
    if (m_SelectedBundles.empty())
        OnAddBundle();
    if (m_SelectedBundles.empty())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(0));

    mitk::PlanarEllipse::Pointer figure = mitk::PlanarEllipse::New();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( figure );
    node->SetBoolProperty("planarfigure.3drendering", true);

    QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
    for( int i=0; i<nodes.size(); i++)
        nodes.at(i)->SetSelected(false);

    m_SelectedFiducial = node;

    QString name = QString("Fiducial_%1").arg(children->size());
    node->SetName(name.toStdString());
    node->SetSelected(true);

    this->DisableCrosshairNavigation();

    mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());
    if(figureInteractor.IsNull())
    {
        figureInteractor = mitk::PlanarFigureInteractor::New();
        us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
        figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
        figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
        figureInteractor->SetDataNode( node );
    }

    UpdateGui();
    GetDataStorage()->Add(node, m_SelectedBundles.at(0));
}

bool CompareLayer(mitk::DataNode::Pointer i,mitk::DataNode::Pointer j)
{
    int li = -1;
    i->GetPropertyValue("layer", li);
    int lj = -1;
    j->GetPropertyValue("layer", lj);
    return li<lj;
}

void QmitkFiberfoxView::GenerateFibers()
{
    if (m_SelectedBundles.empty())
    {
        if (m_SelectedFiducial.IsNull())
            return;

        mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(m_SelectedFiducial);
        for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
            if(dynamic_cast<mitk::FiberBundleX*>((*it)->GetData()))
                m_SelectedBundles.push_back(*it);

        if (m_SelectedBundles.empty())
            return;
    }

    FiberfoxParameters<double> parameters = UpdateImageParameters<double>();

    for (unsigned int i=0; i<m_SelectedBundles.size(); i++)
    {
        mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(i));
        std::vector< mitk::DataNode::Pointer > childVector;
        for( mitk::DataStorage::SetOfObjects::const_iterator it = children->begin(); it != children->end(); ++it )
            childVector.push_back(*it);
        sort(childVector.begin(), childVector.end(), CompareLayer);

        vector< mitk::PlanarEllipse::Pointer > fib;
        vector< unsigned int > flip;
        float radius = 1;
        int count = 0;
        for( std::vector< mitk::DataNode::Pointer >::const_iterator it = childVector.begin(); it != childVector.end(); ++it )
        {
            mitk::DataNode::Pointer node = *it;

            if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
            {
                mitk::PlanarEllipse* ellipse = dynamic_cast<mitk::PlanarEllipse*>(node->GetData());
                if (m_Controls->m_ConstantRadiusBox->isChecked())
                {
                    ellipse->SetTreatAsCircle(true);
                    mitk::Point2D c = ellipse->GetControlPoint(0);
                    mitk::Point2D p = ellipse->GetControlPoint(1);
                    mitk::Vector2D v = p-c;
                    if (count==0)
                    {
                        radius = v.GetVnlVector().magnitude();
                        ellipse->SetControlPoint(1, p);
                        ellipse->Modified();
                    }
                    else
                    {
                        v.Normalize();
                        v *= radius;
                        ellipse->SetControlPoint(1, c+v);
                        ellipse->Modified();
                    }
                }
                fib.push_back(ellipse);

                std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(node.GetPointer());
                if( it != m_DataNodeToPlanarFigureData.end() )
                {
                    QmitkPlanarFigureData& data = it->second;
                    flip.push_back(data.m_Flipped);
                }
                else
                    flip.push_back(0);
            }
            count++;
        }
        if (fib.size()>1)
        {
            parameters.m_FiberGen.m_Fiducials.push_back(fib);
            parameters.m_FiberGen.m_FlipList.push_back(flip);
        }
        else if (fib.size()>0)
            m_SelectedBundles.at(i)->SetData( mitk::FiberBundleX::New() );

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
    filter->SetParameters(parameters.m_FiberGen);
    filter->Update();
    vector< mitk::FiberBundleX::Pointer > fiberBundles = filter->GetFiberBundles();

    for (unsigned int i=0; i<fiberBundles.size(); i++)
    {
        m_SelectedBundles.at(i)->SetData( fiberBundles.at(i) );
        if (fiberBundles.at(i)->GetNumFibers()>50000)
            m_SelectedBundles.at(i)->SetVisibility(false);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::GenerateImage()
{
    if (m_SelectedBundles.empty() && m_SelectedDWI.IsNull())
    {
        mitk::Image::Pointer image = mitk::ImageGenerator::GenerateGradientImage<unsigned int>(
                    m_Controls->m_SizeX->value(),
                    m_Controls->m_SizeY->value(),
                    m_Controls->m_SizeZ->value(),
                    m_Controls->m_SpacingX->value(),
                    m_Controls->m_SpacingY->value(),
                    m_Controls->m_SpacingZ->value());

        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData( image );
        node->SetName("Dummy");
        unsigned int window = m_Controls->m_SizeX->value()*m_Controls->m_SizeY->value()*m_Controls->m_SizeZ->value();
        unsigned int level = window/2;
        mitk::LevelWindow lw; lw.SetLevelWindow(level, window);
        node->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );
        GetDataStorage()->Add(node);
        m_SelectedImage = node;

        mitk::BaseData::Pointer basedata = node->GetData();
        if (basedata.IsNotNull())
        {
            mitk::RenderingManager::GetInstance()->InitializeViews( basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
        UpdateGui();
    }
    else if (!m_SelectedBundles.empty())
        SimulateImageFromFibers(m_SelectedBundles.at(0));
    else if (m_SelectedDWI.IsNotNull())
        SimulateForExistingDwi(m_SelectedDWI);
}

void QmitkFiberfoxView::SimulateForExistingDwi(mitk::DataNode* imageNode)
{
    if (!dynamic_cast<mitk::DiffusionImage<short>*>(imageNode->GetData()))
        return;

    FiberfoxParameters<short> parameters = UpdateImageParameters<short>();

    if (parameters.m_NoiseModel==NULL &&
            parameters.m_SignalGen.m_Spikes==0 &&
            parameters.m_SignalGen.m_FrequencyMap.IsNull() &&
            parameters.m_SignalGen.m_KspaceLineOffset<=0.000001 &&
            !parameters.m_SignalGen.m_DoAddGibbsRinging &&
            !(parameters.m_SignalGen.m_EddyStrength>0) &&
            parameters.m_SignalGen.m_CroppingFactor>0.999)
    {
        QMessageBox::information( NULL, "Simulation cancelled", "No valid artifact enabled! Motion artifacts and relaxation effects can NOT be added to an existing diffusion weighted image.");
        return;
    }

    mitk::DiffusionImage<short>::Pointer diffImg = dynamic_cast<mitk::DiffusionImage<short>*>(imageNode->GetData());
    m_ArtifactsToDwiFilter = itk::AddArtifactsToDwiImageFilter< short >::New();
    m_ArtifactsToDwiFilter->SetInput(diffImg->GetVectorImage());
    parameters.m_Misc.m_ParentNode = imageNode;
    m_ArtifactsToDwiFilter->SetParameters(parameters);
    m_Worker.m_FilterType = 1;
    m_Thread.start(QThread::LowestPriority);
}

void QmitkFiberfoxView::SimulateImageFromFibers(mitk::DataNode* fiberNode)
{
    mitk::FiberBundleX::Pointer fiberBundle = dynamic_cast<mitk::FiberBundleX*>(fiberNode->GetData());
    if (fiberBundle->GetNumFibers()<=0)
        return;

    FiberfoxParameters<double> parameters = UpdateImageParameters<double>();

    m_TractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    parameters.m_Misc.m_ParentNode = fiberNode;
    if (m_SelectedDWI.IsNotNull())
    {
        bool first = true;
        bool ok = true;
        mitk::DiffusionImage<short>::Pointer diffImg = dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedDWI->GetData());
        itk::Image< itk::DiffusionTensor3D< double >, 3 >::Pointer tensorImage = NULL;
        const int shOrder = 2;
        typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,shOrder,QBALL_ODFSIZE> QballFilterType;
        QballFilterType::CoefficientImageType::Pointer itkFeatureImage = NULL;
        ItkDoubleImgType::Pointer adcImage = NULL;

        for (unsigned int i=0; i<parameters.m_FiberModelList.size()+parameters.m_NonFiberModelList.size(); i++)
        {
            mitk::RawShModel<>* model = NULL;
            if (i<parameters.m_FiberModelList.size())
                model = dynamic_cast<  mitk::RawShModel<>* >(parameters.m_FiberModelList.at(i));
            else
                model = dynamic_cast<  mitk::RawShModel<>* >(parameters.m_NonFiberModelList.at(i-parameters.m_FiberModelList.size()));

            if (model!=0 && model->GetNumberOfKernels()<=0)
            {
                if (first==true)
                {
                    typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, double > TensorReconstructionImageFilterType;
                    TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
                    filter->SetGradientImage( diffImg->GetDirections(), diffImg->GetVectorImage() );
                    filter->SetBValue(diffImg->GetReferenceBValue());
                    filter->Update();
                    tensorImage = filter->GetOutput();

                    const int NumCoeffs = (shOrder*shOrder + shOrder + 2)/2 + shOrder;
                    QballFilterType::Pointer qballfilter = QballFilterType::New();
                    qballfilter->SetGradientImage( diffImg->GetDirections(), diffImg->GetVectorImage() );
                    qballfilter->SetBValue(diffImg->GetReferenceBValue());
                    qballfilter->SetLambda(0.006);
                    qballfilter->SetNormalizationMethod(QballFilterType::QBAR_RAW_SIGNAL);
                    qballfilter->Update();
                    itkFeatureImage = qballfilter->GetCoefficientImage();

                    itk::AdcImageFilter< short, double >::Pointer adcFilter = itk::AdcImageFilter< short, double >::New();
                    adcFilter->SetInput(diffImg->GetVectorImage());
                    adcFilter->SetGradientDirections(diffImg->GetDirections());
                    adcFilter->SetB_value(diffImg->GetReferenceBValue());
                    adcFilter->Update();
                    adcImage = adcFilter->GetOutput();
                }
                ok = model->SampleKernels(diffImg, parameters.m_SignalGen.m_MaskImage, tensorImage, itkFeatureImage, adcImage);
                if (!ok)
                    break;
            }
        }

        if (!ok)
        {
            QMessageBox::information( NULL, "Simulation cancelled", "No valid prototype signals could be sampled.");
            return;
        }
    }
    else if ( m_Controls->m_Compartment1Box->currentIndex()==3 || m_Controls->m_Compartment3Box->currentIndex()==3 || m_Controls->m_Compartment4Box->currentIndex()==4 )
    {
        QMessageBox::information( NULL, "Simulation cancelled", "Prototype signal but no diffusion-weighted image selected to sample signal from.");
        return;
    }

    m_TractsToDwiFilter->SetParameters(parameters);
    m_TractsToDwiFilter->SetFiberBundle(fiberBundle);
    m_Worker.m_FilterType = 0;
    m_Thread.start(QThread::LowestPriority);
}

void QmitkFiberfoxView::ApplyTransform()
{
    vector< mitk::DataNode::Pointer > selectedBundles;
    for(unsigned int i=0; i<m_SelectedImages.size(); i++ )
    {
        mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(m_SelectedImages.at(i));
        for( mitk::DataStorage::SetOfObjects::const_iterator it = derivations->begin(); it != derivations->end(); ++it )
        {
            mitk::DataNode::Pointer fibNode = *it;
            if ( fibNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(fibNode->GetData()) )
                selectedBundles.push_back(fibNode);
        }
    }
    if (selectedBundles.empty())
        selectedBundles = m_SelectedBundles2;

    if (!selectedBundles.empty())
    {
        for (std::vector<mitk::DataNode::Pointer>::const_iterator it = selectedBundles.begin(); it!=selectedBundles.end(); ++it)
        {
            mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
            fib->RotateAroundAxis(m_Controls->m_XrotBox->value(), m_Controls->m_YrotBox->value(), m_Controls->m_ZrotBox->value());
            fib->TranslateFibers(m_Controls->m_XtransBox->value(), m_Controls->m_YtransBox->value(), m_Controls->m_ZtransBox->value());
            fib->ScaleFibers(m_Controls->m_XscaleBox->value(), m_Controls->m_YscaleBox->value(), m_Controls->m_ZscaleBox->value());

            // handle child fiducials
            if (m_Controls->m_IncludeFiducials->isChecked())
            {
                mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(*it);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
                {
                    mitk::DataNode::Pointer fiducialNode = *it2;
                    if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                    {
                        mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
                        mitk::BaseGeometry* geom = pe->GetGeometry();

                        // translate
                        mitk::Vector3D world;
                        world[0] = m_Controls->m_XtransBox->value();
                        world[1] = m_Controls->m_YtransBox->value();
                        world[2] = m_Controls->m_ZtransBox->value();
                        geom->Translate(world);

                        // calculate rotation matrix
                        double x = m_Controls->m_XrotBox->value()*M_PI/180;
                        double y = m_Controls->m_YrotBox->value()*M_PI/180;
                        double z = m_Controls->m_ZrotBox->value()*M_PI/180;

                        itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
                        rotX[1][1] = cos(x);
                        rotX[2][2] = rotX[1][1];
                        rotX[1][2] = -sin(x);
                        rotX[2][1] = -rotX[1][2];

                        itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
                        rotY[0][0] = cos(y);
                        rotY[2][2] = rotY[0][0];
                        rotY[0][2] = sin(y);
                        rotY[2][0] = -rotY[0][2];

                        itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
                        rotZ[0][0] = cos(z);
                        rotZ[1][1] = rotZ[0][0];
                        rotZ[0][1] = -sin(z);
                        rotZ[1][0] = -rotZ[0][1];

                        itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

                        // transform control point coordinate into geometry translation
                        geom->SetOrigin(pe->GetWorldControlPoint(0));
                        mitk::Point2D cp; cp.Fill(0.0);
                        pe->SetControlPoint(0, cp);

                        // rotate fiducial
                        geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());

                        // implicit translation
                        mitk::Vector3D trans;
                        trans[0] = geom->GetOrigin()[0]-fib->GetGeometry()->GetCenter()[0];
                        trans[1] = geom->GetOrigin()[1]-fib->GetGeometry()->GetCenter()[1];
                        trans[2] = geom->GetOrigin()[2]-fib->GetGeometry()->GetCenter()[2];
                        mitk::Vector3D newWc = rot*trans;
                        newWc = newWc-trans;
                        geom->Translate(newWc);

                        pe->Modified();
                    }
                }
            }
        }
    }
    else
    {
        for (unsigned int i=0; i<m_SelectedFiducials.size(); i++)
        {
            mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(i)->GetData());
            mitk::BaseGeometry* geom = pe->GetGeometry();

            // translate
            mitk::Vector3D world;
            world[0] = m_Controls->m_XtransBox->value();
            world[1] = m_Controls->m_YtransBox->value();
            world[2] = m_Controls->m_ZtransBox->value();
            geom->Translate(world);

            // calculate rotation matrix
            double x = m_Controls->m_XrotBox->value()*M_PI/180;
            double y = m_Controls->m_YrotBox->value()*M_PI/180;
            double z = m_Controls->m_ZrotBox->value()*M_PI/180;
            itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
            rotX[1][1] = cos(x);
            rotX[2][2] = rotX[1][1];
            rotX[1][2] = -sin(x);
            rotX[2][1] = -rotX[1][2];
            itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
            rotY[0][0] = cos(y);
            rotY[2][2] = rotY[0][0];
            rotY[0][2] = sin(y);
            rotY[2][0] = -rotY[0][2];
            itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
            rotZ[0][0] = cos(z);
            rotZ[1][1] = rotZ[0][0];
            rotZ[0][1] = -sin(z);
            rotZ[1][0] = -rotZ[0][1];
            itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

            // transform control point coordinate into geometry translation
            geom->SetOrigin(pe->GetWorldControlPoint(0));
            mitk::Point2D cp; cp.Fill(0.0);
            pe->SetControlPoint(0, cp);

            // rotate fiducial
            geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());
            pe->Modified();
        }
        if (m_Controls->m_RealTimeFibers->isChecked())
            GenerateFibers();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::CopyBundles()
{
    if ( m_SelectedBundles.size()<1 ){
        QMessageBox::information( NULL, "Warning", "Select at least one fiber bundle!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least one fiber bundle!";
        return;
    }

    for (std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin(); it!=m_SelectedBundles.end(); ++it)
    {
        // find parent image
        mitk::DataNode::Pointer parentNode;
        mitk::DataStorage::SetOfObjects::ConstPointer parentImgs = GetDataStorage()->GetSources(*it);
        for( mitk::DataStorage::SetOfObjects::const_iterator it2 = parentImgs->begin(); it2 != parentImgs->end(); ++it2 )
        {
            mitk::DataNode::Pointer pImgNode = *it2;
            if ( pImgNode.IsNotNull() && dynamic_cast<mitk::Image*>(pImgNode->GetData()) )
            {
                parentNode = pImgNode;
                break;
            }
        }

        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
        mitk::FiberBundleX::Pointer newBundle = fib->GetDeepCopy();
        QString name((*it)->GetName().c_str());
        name += "_copy";

        mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
        fbNode->SetData(newBundle);
        fbNode->SetName(name.toStdString());
        fbNode->SetVisibility(true);
        if (parentNode.IsNotNull())
            GetDataStorage()->Add(fbNode, parentNode);
        else
            GetDataStorage()->Add(fbNode);

        // copy child fiducials
        if (m_Controls->m_IncludeFiducials->isChecked())
        {
            mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(*it);
            for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
            {
                mitk::DataNode::Pointer fiducialNode = *it2;
                if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                {
                    mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData())->Clone();
                    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
                    newNode->SetData(pe);
                    newNode->SetName(fiducialNode->GetName());
                    newNode->SetBoolProperty("planarfigure.3drendering", true);
                    GetDataStorage()->Add(newNode, fbNode);

                }
            }
        }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::JoinBundles()
{
    if ( m_SelectedBundles.size()<2 ){
        QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
        return;
    }

    std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
    mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
    QString name("");
    name += QString((*it)->GetName().c_str());
    ++it;
    for (; it!=m_SelectedBundles.end(); ++it)
    {
        newBundle = newBundle->AddBundle(dynamic_cast<mitk::FiberBundleX*>((*it)->GetData()));
        name += "+"+QString((*it)->GetName().c_str());
    }

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::UpdateGui()
{
    m_Controls->m_FiberBundleLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_GeometryFrame->setEnabled(true);
    m_Controls->m_GeometryMessage->setVisible(false);
    m_Controls->m_DiffusionPropsMessage->setVisible(false);
    m_Controls->m_FiberGenMessage->setVisible(true);

    m_Controls->m_TransformBundlesButton->setEnabled(false);
    m_Controls->m_CopyBundlesButton->setEnabled(false);
    m_Controls->m_GenerateFibersButton->setEnabled(false);
    m_Controls->m_FlipButton->setEnabled(false);
    m_Controls->m_CircleButton->setEnabled(false);
    m_Controls->m_BvalueBox->setEnabled(true);
    m_Controls->m_NumGradientsBox->setEnabled(true);
    m_Controls->m_JoinBundlesButton->setEnabled(false);
    m_Controls->m_AlignOnGrid->setEnabled(false);

    if (m_SelectedFiducial.IsNotNull())
    {
        m_Controls->m_TransformBundlesButton->setEnabled(true);
        m_Controls->m_FlipButton->setEnabled(true);
        m_Controls->m_AlignOnGrid->setEnabled(true);
    }

    if (m_SelectedImage.IsNotNull() || !m_SelectedBundles.empty())
    {
        m_Controls->m_TransformBundlesButton->setEnabled(true);
        m_Controls->m_CircleButton->setEnabled(true);
        m_Controls->m_FiberGenMessage->setVisible(false);
        m_Controls->m_AlignOnGrid->setEnabled(true);
    }

    if (m_MaskImageNode.IsNotNull() || m_SelectedImage.IsNotNull())
    {
        m_Controls->m_GeometryMessage->setVisible(true);
        m_Controls->m_GeometryFrame->setEnabled(false);
    }

    if (m_SelectedDWI.IsNotNull())
    {
        m_Controls->m_DiffusionPropsMessage->setVisible(true);
        m_Controls->m_BvalueBox->setEnabled(false);
        m_Controls->m_NumGradientsBox->setEnabled(false);
        m_Controls->m_GeometryMessage->setVisible(true);
        m_Controls->m_GeometryFrame->setEnabled(false);
    }

    if (!m_SelectedBundles.empty())
    {
        m_Controls->m_CopyBundlesButton->setEnabled(true);
        m_Controls->m_GenerateFibersButton->setEnabled(true);
        m_Controls->m_FiberBundleLabel->setText(m_SelectedBundles.at(0)->GetName().c_str());

        if (m_SelectedBundles.size()>1)
            m_Controls->m_JoinBundlesButton->setEnabled(true);
    }
}

void QmitkFiberfoxView::OnSelectionChanged( berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes )
{
    m_SelectedBundles2.clear();
    m_SelectedImages.clear();
    m_SelectedFiducials.clear();
    m_SelectedFiducial = NULL;
    m_SelectedBundles.clear();
    m_SelectedImage = NULL;
    m_SelectedDWI = NULL;
    m_MaskImageNode = NULL;
    m_Controls->m_TissueMaskLabel->setText("<font color='grey'>optional</font>");

    // iterate all selected objects, adjust warning visibility
    for( int i=0; i<nodes.size(); i++)
    {
        mitk::DataNode::Pointer node = nodes.at(i);

        if ( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
        {
            m_SelectedDWI = node;
            m_SelectedImage = node;
            m_SelectedImages.push_back(node);
        }
        else if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            m_SelectedImages.push_back(node);
            m_SelectedImage = node;
            bool isbinary = false;
            node->GetPropertyValue<bool>("binary", isbinary);
            if (isbinary)
            {
                m_MaskImageNode = node;
                m_Controls->m_TissueMaskLabel->setText(m_MaskImageNode->GetName().c_str());
            }
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            m_SelectedBundles2.push_back(node);
            if (m_Controls->m_RealTimeFibers->isChecked())
            {
                m_SelectedBundles.push_back(node);
                mitk::FiberBundleX::Pointer newFib = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
                if (newFib->GetNumFibers()!=m_Controls->m_FiberDensityBox->value())
                    GenerateFibers();
            }
            else
                m_SelectedBundles.push_back(node);
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
        {
            m_SelectedFiducials.push_back(node);
            m_SelectedFiducial = node;
            m_SelectedBundles.clear();
            mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(node);
            for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
            {
                mitk::DataNode::Pointer pNode = *it;
                if ( pNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(pNode->GetData()) )
                    m_SelectedBundles.push_back(pNode);
            }
        }
    }
    UpdateGui();
}


void QmitkFiberfoxView::EnableCrosshairNavigation()
{
    MITK_DEBUG << "EnableCrosshairNavigation";

    // enable the crosshair navigation
    if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
            dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
    {
        MITK_DEBUG << "enabling linked navigation";
        linkedRenderWindow->EnableLinkedNavigation(true);
        //        linkedRenderWindow->EnableSlicingPlanes(true);
    }

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::DisableCrosshairNavigation()
{
    MITK_DEBUG << "DisableCrosshairNavigation";

    // disable the crosshair navigation during the drawing
    if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
            dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
    {
        MITK_DEBUG << "disabling linked navigation";
        linkedRenderWindow->EnableLinkedNavigation(false);
        //        linkedRenderWindow->EnableSlicingPlanes(false);
    }
}

void QmitkFiberfoxView::NodeRemoved(const mitk::DataNode* node)
{
    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(nonConstNode);

    if (dynamic_cast<FiberBundleX*>(node->GetData()))
    {
        m_SelectedBundles.clear();
        m_SelectedBundles2.clear();
    }
    else if (dynamic_cast<Image*>(node->GetData()))
        m_SelectedImages.clear();

    if( it != m_DataNodeToPlanarFigureData.end() )
    {
        QmitkPlanarFigureData& data = it->second;

        // remove observers
        data.m_Figure->RemoveObserver( data.m_EndPlacementObserverTag );
        data.m_Figure->RemoveObserver( data.m_SelectObserverTag );
        data.m_Figure->RemoveObserver( data.m_StartInteractionObserverTag );
        data.m_Figure->RemoveObserver( data.m_EndInteractionObserverTag );

        m_DataNodeToPlanarFigureData.erase( it );
    }
}

void QmitkFiberfoxView::NodeAdded( const mitk::DataNode* node )
{
    // add observer for selection in renderwindow
    mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    bool isPositionMarker (false);
    node->GetBoolProperty("isContourMarker", isPositionMarker);
    if( figure && !isPositionMarker )
    {
        MITK_DEBUG << "figure added. will add interactor if needed.";
        mitk::PlanarFigureInteractor::Pointer figureInteractor
                = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());

        mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>( node );
        if(figureInteractor.IsNull())
        {
            figureInteractor = mitk::PlanarFigureInteractor::New();
            us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
            figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
            figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
            figureInteractor->SetDataNode( nonConstNode );
        }

        MITK_DEBUG << "will now add observers for planarfigure";
        QmitkPlanarFigureData data;
        data.m_Figure = figure;

        //        // add observer for event when figure has been placed
        typedef itk::SimpleMemberCommand< QmitkFiberfoxView > SimpleCommandType;
        //        SimpleCommandType::Pointer initializationCommand = SimpleCommandType::New();
        //        initializationCommand->SetCallbackFunction( this, &QmitkFiberfoxView::PlanarFigureInitialized );
        //        data.m_EndPlacementObserverTag = figure->AddObserver( mitk::EndPlacementPlanarFigureEvent(), initializationCommand );

        // add observer for event when figure is picked (selected)
        typedef itk::MemberCommand< QmitkFiberfoxView > MemberCommandType;
        MemberCommandType::Pointer selectCommand = MemberCommandType::New();
        selectCommand->SetCallbackFunction( this, &QmitkFiberfoxView::PlanarFigureSelected );
        data.m_SelectObserverTag = figure->AddObserver( mitk::SelectPlanarFigureEvent(), selectCommand );

        // add observer for event when interaction with figure starts
        SimpleCommandType::Pointer startInteractionCommand = SimpleCommandType::New();
        startInteractionCommand->SetCallbackFunction( this, &QmitkFiberfoxView::DisableCrosshairNavigation);
        data.m_StartInteractionObserverTag = figure->AddObserver( mitk::StartInteractionPlanarFigureEvent(), startInteractionCommand );

        // add observer for event when interaction with figure starts
        SimpleCommandType::Pointer endInteractionCommand = SimpleCommandType::New();
        endInteractionCommand->SetCallbackFunction( this, &QmitkFiberfoxView::EnableCrosshairNavigation);
        data.m_EndInteractionObserverTag = figure->AddObserver( mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand );

        m_DataNodeToPlanarFigureData[nonConstNode] = data;
    }
}

void QmitkFiberfoxView::PlanarFigureSelected( itk::Object* object, const itk::EventObject& )
{
    mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPf = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

    mitk::DataStorage::SetOfObjects::ConstPointer allPfs = this->GetDataStorage()->GetSubset( isPf );
    for ( mitk::DataStorage::SetOfObjects::const_iterator it = allPfs->begin(); it!=allPfs->end(); ++it)
    {
        mitk::DataNode* node = *it;

        if( node->GetData() == object )
        {
            node->SetSelected(true);
            m_SelectedFiducial = node;
        }
        else
            node->SetSelected(false);
    }
    UpdateGui();
    this->RequestRenderWindowUpdate();
}

void QmitkFiberfoxView::SetFocus()
{
    m_Controls->m_CircleButton->setFocus();
}


void QmitkFiberfoxView::SetOutputPath()
{
    // SELECT FOLDER DIALOG

    string outputPath = QFileDialog::getExistingDirectory(NULL, "Save images to...", QString(outputPath.c_str())).toStdString();

    if (outputPath.empty())
        m_Controls->m_SavePathEdit->setText("-");
    else
    {
        outputPath += "/";
        m_Controls->m_SavePathEdit->setText(QString(outputPath.c_str()));
    }
}
