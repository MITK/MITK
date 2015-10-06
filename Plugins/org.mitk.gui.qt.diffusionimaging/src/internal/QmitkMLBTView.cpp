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

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkImageCast.h>
#include <mitkEnumerationProperty.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkPointSet.h>
#include <mitkNodePredicateIsDWI.h>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkMLBTView::VIEW_ID = "org.mitk.views.mlbtview";
using namespace berry;

QmitkMLBTView::QmitkMLBTView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
{
    m_TrackingTimer = std::make_shared<QTimer>(this);
    m_LastLoadedForestName = "(none)";
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
        connect( m_TrackingTimer.get(), SIGNAL(timeout()), this, SLOT(BuildFibers()) );
        connect( m_Controls->m_TimerIntervalBox, SIGNAL(valueChanged(int)), this, SLOT( ChangeTimerInterval(int) ));
        connect( m_Controls->m_DemoModeBox, SIGNAL(stateChanged(int)), this, SLOT( ToggleDemoMode(int) ));
        connect( m_Controls->m_PauseTrackingButton, SIGNAL ( clicked() ), this, SLOT( PauseTracking() ) );
        connect( m_Controls->m_AbortTrackingButton, SIGNAL ( clicked() ), this, SLOT( AbortTracking() ) );
        connect( m_Controls->m_AddTwButton, SIGNAL ( clicked() ), this, SLOT( AddTrainingWidget() ) );
        connect( m_Controls->m_RemoveTwButton, SIGNAL ( clicked() ), this, SLOT( RemoveTrainingWidget() ) );

        m_Controls->m_TrackingMaskImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingSeedImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingStopImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingRawImageBox->SetDataStorage(this->GetDataStorage());

        mitk::NodePredicateIsDWI::Pointer isDiffusionImage = mitk::NodePredicateIsDWI::New();

        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        finalPredicate = mitk::NodePredicateAnd::New(finalPredicate, isBinaryPredicate);
        m_Controls->m_TrackingMaskImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingSeedImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingStopImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingRawImageBox->SetPredicate(isDiffusionImage);

        m_Controls->m_TrackingMaskImageBox->SetZeroEntryText("--");
        m_Controls->m_TrackingSeedImageBox->SetZeroEntryText("--");
        m_Controls->m_TrackingStopImageBox->SetZeroEntryText("--");
        AddTrainingWidget();

        UpdateGui();
    }
}

void QmitkMLBTView::AddTrainingWidget()
{
    std::shared_ptr<QmitkMlbstTrainingDataWidget> tw = std::make_shared<QmitkMlbstTrainingDataWidget>();
    tw->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TwFrame->layout()->addWidget(tw.get());
    m_TrainingWidgets.push_back(tw);
}

void QmitkMLBTView::RemoveTrainingWidget()
{
    if(m_TrainingWidgets.size()>1)
    {
        m_TrainingWidgets.back().reset();
        m_TrainingWidgets.pop_back();
    }
}

void QmitkMLBTView::UpdateGui()
{
    if (m_ForestHandler.IsForestValid())
    {
        std::string label_text="Random forest available: "+m_LastLoadedForestName;
        m_Controls->statusLabel->setText( QString(label_text.c_str()) );
        m_Controls->m_SaveForestButton->setEnabled(true);
        m_Controls->m_StartTrackingButton->setEnabled(true);
    }
    else
    {
        m_Controls->statusLabel->setText("Please load or train random forest!");
        m_Controls->m_SaveForestButton->setEnabled(false);
        m_Controls->m_StartTrackingButton->setEnabled(false);
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
        outFib->SetFiberColors(255,255,255);
        m_TractogramNode->SetData(outFib);

        m_SamplingPointsNode->SetData(tracker->m_SamplingPointset);
        m_AlternativePointsNode->SetData(tracker->m_AlternativePointset);

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        tracker->m_BuildFibersFinished = false;
        tracker->m_BuildFibersReady = 0;
        tracker->m_Stop = false;
    }
}

void QmitkMLBTView::LoadForest()
{

    QString filename = QFileDialog::getOpenFileName(0, tr("Load Forest"), QDir::currentPath(), tr("HDF5 random forest file (*.rf)") );
    if(filename.isEmpty() || filename.isNull())
        return;

    m_ForestHandler.LoadForest( filename.toStdString() );
    QFileInfo fi( filename );
    m_LastLoadedForestName = QString( fi.baseName() + "." + fi.completeSuffix() ).toStdString();

    UpdateGui();
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

    m_SamplingPointsNode = mitk::DataNode::New();
    m_SamplingPointsNode->SetName("SamplingPoints");
    m_SamplingPointsNode->SetProperty("pointsize", mitk::FloatProperty::New(0.2));
    m_SamplingPointsNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
    mitk::PointSetShapeProperty::Pointer bla = mitk::PointSetShapeProperty::New(); bla->SetValue(8);
    m_SamplingPointsNode->SetProperty("Pointset.2D.shape", bla);
    m_SamplingPointsNode->SetProperty("Pointset.2D.distance to plane", mitk::FloatProperty::New(1.5));
    m_SamplingPointsNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
    m_SamplingPointsNode->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
    this->GetDataStorage()->Add(m_SamplingPointsNode);

    m_AlternativePointsNode = mitk::DataNode::New();
    m_AlternativePointsNode->SetName("AlternativePoints");
    m_AlternativePointsNode->SetProperty("pointsize", mitk::FloatProperty::New(0.2));
    m_AlternativePointsNode->SetProperty("color", mitk::ColorProperty::New(1,0,0));
    m_AlternativePointsNode->SetProperty("Pointset.2D.shape", bla);
    m_AlternativePointsNode->SetProperty("Pointset.2D.distance to plane", mitk::FloatProperty::New(1.5));
    m_AlternativePointsNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
    m_AlternativePointsNode->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
    this->GetDataStorage()->Add(m_AlternativePointsNode);

    QFuture<void> future = QtConcurrent::run( this, &QmitkMLBTView::StartTracking );
    m_TrackingWatcher.setFuture(future);
    m_TrackingThreadIsRunning = true;
    m_Controls->m_StartTrackingButton->setEnabled(false);
    m_TrackingTimer->start(m_Controls->m_TimerIntervalBox->value());
}

void QmitkMLBTView::OnTrackingThreadStop()
{
    m_TrackingThreadIsRunning = false;
    m_Controls->m_StartTrackingButton->setEnabled(true);

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
    m_TractogramNode->SetData(outFib);
    m_TractogramNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(1));
    if (m_Controls->m_DemoModeBox->isChecked())
    {
        m_SamplingPointsNode->SetData(tracker->m_SamplingPointset);
        m_AlternativePointsNode->SetData(tracker->m_AlternativePointset);
    }

    tracker = NULL;
    m_TrackingTimer->stop();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMLBTView::StartTracking()
{
    if ( m_Controls->m_TrackingRawImageBox->GetSelectedNode().IsNull() || !m_ForestHandler.IsForestValid())
        return;

    mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingRawImageBox->GetSelectedNode()->GetData());
    m_ForestHandler.AddRawData(dwi);

//    int numThread = itk::MultiThreader::GetGlobalDefaultNumberOfThreads();

    tracker = TrackerType::New();
    tracker->SetInput(0,  mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi) );
    tracker->SetDemoMode(m_Controls->m_DemoModeBox->isChecked());
    if (m_Controls->m_DemoModeBox->isChecked())
        tracker->SetNumberOfThreads(1);
    if (m_Controls->m_TrackingMaskImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingMaskImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
        mitk::CastToItkImage(mask, itkMask);
        tracker->SetMaskImage(itkMask);
    }
    if (m_Controls->m_TrackingSeedImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingSeedImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        tracker->SetSeedImage(itkImg);
    }
    if (m_Controls->m_TrackingStopImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingStopImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        tracker->SetStoppingRegions(itkImg);
    }
    tracker->SetSeedsPerVoxel(m_Controls->m_NumberOfSeedsBox->value());
    tracker->SetStepSize(m_Controls->m_TrackingStepSizeBox->value());
    tracker->SetMinTractLength(m_Controls->m_MinLengthBox->value());
    tracker->SetMaxTractLength(m_Controls->m_MaxLengthBox->value());
    tracker->SetAposterioriCurvCheck(m_Controls->m_Curvcheck2->isChecked());
    tracker->SetRemoveWmEndFibers(false);
    tracker->SetAvoidStop(m_Controls->m_AvoidStop->isChecked());
    tracker->SetForestHandler(m_ForestHandler);
    tracker->SetSamplingDistance(m_Controls->m_SamplingDistanceBox->value());
    tracker->SetNumberOfSamples(m_Controls->m_NumSamplesBox->value());
    tracker->SetRandomSampling(m_Controls->m_RandomSampling->isChecked());
    tracker->Update();
}

void QmitkMLBTView::SaveForest()
{
    if (!m_ForestHandler.IsForestValid())
    {
        UpdateGui();
        return;
    }
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Forest"), QDir::currentPath()+"/forest.rf", tr("HDF5 random forest file (*.rf)") );

    if(filename.isEmpty() || filename.isNull())
        return;
    if(!filename.endsWith(".rf"))
        filename += ".rf";

    m_ForestHandler.SaveForest( filename.toStdString() );
}

void QmitkMLBTView::StartTrainingThread()
{
    if (!this->IsTrainingInputValid())
    {
        QMessageBox::warning(nullptr, "Training aborted", "Training could not be started. Not all necessary datasets were selected.");
        return;
    }

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
    UpdateGui();
}

void QmitkMLBTView::StartTraining()
{
    std::vector< mitk::Image::Pointer > m_SelectedDiffImages;
    std::vector< mitk::FiberBundle::Pointer > m_SelectedFB;
    std::vector< ItkUcharImgType::Pointer > m_MaskImages;
    std::vector< ItkUcharImgType::Pointer > m_WhiteMatterImages;

    for (auto w : m_TrainingWidgets)
    {
        m_SelectedDiffImages.push_back(dynamic_cast<mitk::Image*>(w->GetImage()->GetData()));
        m_SelectedFB.push_back(dynamic_cast<mitk::FiberBundle*>(w->GetFibers()->GetData()));

        if (w->GetMask().IsNotNull())
        {
            mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(w->GetMask()->GetData());
            ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
            mitk::CastToItkImage(img, itkMask);
            m_MaskImages.push_back(itkMask);
        }
        else
            m_MaskImages.push_back(nullptr);

        if (w->GetWhiteMatter().IsNotNull())
        {
            mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(w->GetWhiteMatter()->GetData());
            ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
            mitk::CastToItkImage(img, itkMask);
            m_WhiteMatterImages.push_back(itkMask);
        }
        else
            m_WhiteMatterImages.push_back(nullptr);
    }

    m_ForestHandler.SetRawData(m_SelectedDiffImages);
    m_ForestHandler.SetTractograms(m_SelectedFB);
    m_ForestHandler.SetMaskImages(m_MaskImages);
    m_ForestHandler.SetWhiteMatterImages(m_WhiteMatterImages);
    m_ForestHandler.SetNumTrees(m_Controls->m_NumTreesBox->value());
    m_ForestHandler.SetMaxTreeDepth(m_Controls->m_MaxDepthBox->value());
    m_ForestHandler.SetGrayMatterSamplesPerVoxel(m_Controls->m_GmSamplingBox->value());
    m_ForestHandler.SetSampleFraction(m_Controls->m_SampleFractionBox->value());
    m_ForestHandler.SetStepSize(m_Controls->m_TrainingStepSizeBox->value());
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

void QmitkMLBTView::Activated()
{

}

bool QmitkMLBTView::IsTrainingInputValid(void) const
{
  for (auto widget : m_TrainingWidgets)
  {
    if (widget->GetImage().IsNull() || widget->GetFibers().IsNull())
    {
      return false;
    }
  }

  return true;
}
