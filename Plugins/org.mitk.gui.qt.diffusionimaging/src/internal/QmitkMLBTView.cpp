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
#include "QmitkMLBTView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QString>

#include "mitkNodePredicateDataType.h"
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkImageCast.h>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkMLBTView::VIEW_ID = "org.mitk.views.mlbtview";
using namespace berry;

QmitkMLBTView::QmitkMLBTView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
{
    m_TrackingTimer = new QTimer(this);
}

// Destructor
QmitkMLBTView::~QmitkMLBTView()
{

}

void QmitkMLBTView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkMLBTViewControls;
        m_Controls->setupUi( parent );

        connect( m_Controls->m_StartTrainingButton, SIGNAL ( clicked() ), this, SLOT( StartTrainingThread() ) );
        connect( &m_TrainingWatcher, SIGNAL ( finished() ), this, SLOT( OnTrainingThreadStop() ) );
        connect( m_Controls->m_StartTrackingButton, SIGNAL ( clicked() ), this, SLOT( StartTrackingThread() ) );
        connect( &m_TrackingWatcher, SIGNAL ( finished() ), this, SLOT( OnTrackingThreadStop() ) );
        connect( m_Controls->m_SaveForestButton, SIGNAL ( clicked() ), this, SLOT( SaveForest() ) );
        connect( m_Controls->m_LoadForestButton, SIGNAL ( clicked() ), this, SLOT( LoadForest() ) );
        connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(BuildFibers()) );
        connect( m_Controls->m_TimerIntervalBox, SIGNAL(valueChanged(int)), this, SLOT( ChangeTimerInterval(int) ));
        connect( m_Controls->m_DemoModeBox, SIGNAL(stateChanged(int)), this, SLOT( ToggleDemoMode(int) ));
        connect( m_Controls->m_PauseTrackingButton, SIGNAL ( clicked() ), this, SLOT( PauseTracking() ) );
        connect( m_Controls->m_AbortTrackingButton, SIGNAL ( clicked() ), this, SLOT( AbortTracking() ) );

        int numThread = itk::MultiThreader::GetGlobalDefaultNumberOfThreads();
        m_Controls->m_NumberOfThreadsBox->setMaximum(numThread);
        m_Controls->m_NumberOfThreadsBox->setValue(numThread);

        m_Controls->m_TrackingMaskImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingSeedImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingStopImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingRawImageBox->SetDataStorage(this->GetDataStorage());

        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
        mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
        mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
        mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
        isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        finalPredicate = mitk::NodePredicateAnd::New(finalPredicate, isBinaryPredicate);
        m_Controls->m_TrackingMaskImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingSeedImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingStopImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingRawImageBox->SetPredicate(isDwi);
    }
}

void QmitkMLBTView::AbortTracking()
{
    if (tracker.IsNotNull())
    {
        tracker->m_AbortTracking = true;
    }
}

void QmitkMLBTView::PauseTracking()
{
    if (tracker.IsNotNull())
    {
        tracker->m_PauseTracking = !tracker->m_PauseTracking;
    }
}

void QmitkMLBTView::ChangeTimerInterval(int value)
{
    m_TrackingTimer->setInterval(value);
}

void QmitkMLBTView::ToggleDemoMode(int state)
{
    if (tracker.IsNotNull())
    {
        tracker->SetDemoMode(m_Controls->m_DemoModeBox->isChecked());
        tracker->m_Stop = false;
    }
}

void QmitkMLBTView::BuildFibers()
{
    if (m_Controls->m_DemoModeBox->isChecked() && tracker.IsNotNull() && tracker->m_BuildFibersFinished)
    {
        vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
        mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
        outFib->SetFiberColors(255,0,0);
        m_TractogramNode->SetData(outFib);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        tracker->m_BuildFibersFinished = false;
        tracker->m_Stop = false;
        tracker->m_BuildFibersReady = 0;
    }
}

void QmitkMLBTView::LoadForest()
{

    QString filename = QFileDialog::getOpenFileName(0, tr("Load Forest"), QDir::currentPath(), tr("HDF5 random forest file (*.rf)") );
    if(filename.isEmpty() || filename.isNull())
        return;

    m_ForestHandler.LoadForest( filename.toStdString() );
}

void QmitkMLBTView::StartTrackingThread()
{
    m_TractogramNode = mitk::DataNode::New();
    m_TractogramNode->SetName("MLBT Result");
    m_TractogramNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(20));
    m_TractogramNode->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(false));
    m_TractogramNode->SetProperty("LineWidth", mitk::IntProperty::New(2));
    m_TractogramNode->SetProperty("color",mitk::ColorProperty::New(0, 1, 1));
    this->GetDataStorage()->Add(m_TractogramNode);

    QFuture<void> future = QtConcurrent::run( this, &QmitkMLBTView::StartTracking );
    m_TrackingWatcher.setFuture(future);
    m_TrackingThreadIsRunning = true;
    m_Controls->m_StartTrackingButton->setEnabled(false);
    m_TrackingTimer->start(10);
}

void QmitkMLBTView::OnTrackingThreadStop()
{
    m_TrackingThreadIsRunning = false;
    m_Controls->m_StartTrackingButton->setEnabled(true);


    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
    outFib->SetFiberColors(255,0,0);
//    mitk::DataNode::Pointer node = mitk::DataNode::New();
    m_TractogramNode->SetData(outFib);

    tracker = NULL;
    m_TrackingTimer->stop();
}

void QmitkMLBTView::StartTracking()
{
    if ( m_Controls->m_TrackingRawImageBox->GetSelectedNode().IsNull() )
        return;

    mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingRawImageBox->GetSelectedNode()->GetData());
    tracker = TrackerType::New();
    tracker->SetNumberOfThreads(m_Controls->m_NumberOfThreadsBox->value());
    tracker->SetInput(0,  mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi) );
    tracker->SetGradientDirections( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi) );
    tracker->SetB_Value( mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi) );
    tracker->SetDemoMode(m_Controls->m_DemoModeBox->isChecked());
    if (m_Controls->m_TrackingUseMaskImageBox->isChecked() && m_Controls->m_TrackingMaskImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingMaskImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
        mitk::CastToItkImage(mask, itkMask);
        tracker->SetMaskImage(itkMask);
    }
    if (m_Controls->m_TrackingUseSeedImageBox->isChecked() && m_Controls->m_TrackingSeedImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingSeedImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        tracker->SetSeedImage(itkImg);
    }
    if (m_Controls->m_TrackingUseStopImageBox->isChecked() && m_Controls->m_TrackingStopImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingStopImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        tracker->SetStoppingRegions(itkImg);
    }
    tracker->SetSeedsPerVoxel(m_Controls->m_NumberOfSeedsBox->value());
    tracker->SetUseDirection(true);
    tracker->SetStepSize(m_Controls->m_TrackingStepSizeBox->value());
    tracker->SetAngularThreshold(cos((float)m_Controls->m_AngularThresholdBox->value()*M_PI/180));
    tracker->SetMinTractLength(m_Controls->m_MinLengthBox->value());
    tracker->SetMaxTractLength(m_Controls->m_MaxLengthBox->value());

    vigra::RandomForest<int> forest = m_ForestHandler.GetForest();
    tracker->SetDecisionForest(&forest);
    tracker->SetSamplingDistance(m_Controls->m_SamplingDistanceBox->value());
    tracker->SetNumberOfSamples(m_Controls->m_NumSamplesBox->value());
    tracker->Update();
//    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
//    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
//    outFib->SetColorCoding(mitk::FiberBundle::COLORCODING_CUSTOM);
//    mitk::DataNode::Pointer node = mitk::DataNode::New();
//    m_TractogramNode->SetData(outFib);
//    node->SetData(outFib);
//    node->SetName("MLBT Result");
//    this->GetDataStorage()->Add(node);
//    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMLBTView::SaveForest()
{
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Forest"), QDir::currentPath()+"/forest.rf", tr("HDF5 random forest file (*.rf)") );

    if(filename.isEmpty() || filename.isNull())
        return;
    if(!filename.endsWith(".rf"))
        filename += ".rf";

    m_ForestHandler.SaveForest( filename.toStdString() );
}

void QmitkMLBTView::StartTrainingThread()
{
    QFuture<void> future = QtConcurrent::run( this, &QmitkMLBTView::StartTraining );
    m_TrainingWatcher.setFuture(future);
    m_Controls->m_StartTrainingButton->setEnabled(false);
    m_Controls->m_SaveForestButton->setEnabled(false);
    m_Controls->m_LoadForestButton->setEnabled(false);
}

void QmitkMLBTView::OnTrainingThreadStop()
{
    m_Controls->m_StartTrainingButton->setEnabled(true);
    m_Controls->m_SaveForestButton->setEnabled(true);
    m_Controls->m_LoadForestButton->setEnabled(true);
}

void QmitkMLBTView::StartTraining()
{
    m_ForestHandler.SetRawData(m_SelectedDiffImages);
    m_ForestHandler.SetTractograms(m_SelectedFB);
    m_ForestHandler.SetNumTrees(m_Controls->m_NumTreesBox->value());
    m_ForestHandler.SetMaxTreeDepth(m_Controls->m_MaxDepthBox->value());
    m_ForestHandler.SetGrayMatterSamplesPerVoxel(m_Controls->m_GmSamplingBox->value());
    m_ForestHandler.SetSampleFraction(m_Controls->m_SampleFractionBox->value());
    m_ForestHandler.SetStepSize(m_Controls->m_TrainingStepSizeBox->value());
    m_ForestHandler.SetUsePreviousDirection(m_Controls->m_TrackingUsePreviousDirectionBox->isChecked());
    m_ForestHandler.StartTraining();
}

void QmitkMLBTView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}


void QmitkMLBTView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkMLBTView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    if ( !this->IsVisible() )
    {
        // do nothing if nobody wants to see me :-(
        return;
    }

    m_SelectedFB.clear();
    m_SelectedDiffImages.clear();
    m_MaskImages.clear();
    m_WhiteMatterImages.clear();

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;
        if ( dynamic_cast<mitk::FiberBundle*>(node->GetData()) )
            m_SelectedFB.push_back( dynamic_cast<mitk::FiberBundle*>(node->GetData()) );
        else if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(node))
            m_SelectedDiffImages.push_back( dynamic_cast<mitk::Image*>(node->GetData()) );
    }
}

void QmitkMLBTView::Activated()
{

}


