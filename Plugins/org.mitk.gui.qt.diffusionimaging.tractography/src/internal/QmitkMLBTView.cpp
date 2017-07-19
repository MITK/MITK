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
#include <mitkTractographyForest.h>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkMLBTView::VIEW_ID = "org.mitk.views.mlbtview";
using namespace berry;

QmitkMLBTView::QmitkMLBTView()
    : QmitkAbstractView()
    , m_Controls( 0 )
{
    m_TrackingTimer = std::make_shared<QTimer>(this);
    m_ForestHandler = new mitk::TrackingHandlerRandomForest<6,100>();
}

// Destructor
QmitkMLBTView::~QmitkMLBTView()
{
    delete m_ForestHandler;
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
        connect( m_TrackingTimer.get(), SIGNAL(timeout()), this, SLOT(BuildFibers()) );
        connect( m_Controls->m_TimerIntervalBox, SIGNAL(valueChanged(int)), this, SLOT( ChangeTimerInterval(int) ));
        connect( m_Controls->m_DemoModeBox, SIGNAL(stateChanged(int)), this, SLOT( ToggleDemoMode(int) ));
        connect( m_Controls->m_PauseTrackingButton, SIGNAL ( clicked() ), this, SLOT( PauseTracking() ) );
        connect( m_Controls->m_AbortTrackingButton, SIGNAL ( clicked() ), this, SLOT( AbortTracking() ) );
        connect( m_Controls->m_AddTwButton, SIGNAL ( clicked() ), this, SLOT( AddTrainingWidget() ) );
        connect( m_Controls->m_RemoveTwButton, SIGNAL ( clicked() ), this, SLOT( RemoveTrainingWidget() ) );
        connect( m_Controls->m_TrackingForestBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ForestSwitched()) );
        connect( m_Controls->m_TrackingRawImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DwiSwitched()) );

        m_Controls->m_TrackingMaskImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingSeedImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingStopImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingRawImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TissueImageBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_TrackingForestBox->SetDataStorage(this->GetDataStorage());

        mitk::TNodePredicateDataType<mitk::TractographyForest>::Pointer isTractographyForest = mitk::TNodePredicateDataType<mitk::TractographyForest>::New();

        mitk::NodePredicateIsDWI::Pointer isDiffusionImage = mitk::NodePredicateIsDWI::New();

        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        m_Controls->m_TissueImageBox->SetPredicate(finalPredicate);
        mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        finalPredicate = mitk::NodePredicateAnd::New(finalPredicate, isBinaryPredicate);
        m_Controls->m_TrackingMaskImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingSeedImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingStopImageBox->SetPredicate(finalPredicate);
        m_Controls->m_TrackingRawImageBox->SetPredicate(isDiffusionImage);
        m_Controls->m_TrackingForestBox->SetPredicate(isTractographyForest);

        m_Controls->m_TrackingMaskImageBox->SetZeroEntryText("--");
        m_Controls->m_TrackingSeedImageBox->SetZeroEntryText("--");
        m_Controls->m_TrackingStopImageBox->SetZeroEntryText("--");
        m_Controls->m_TissueImageBox->SetZeroEntryText("--");
        AddTrainingWidget();

        UpdateGui();
    }

}


void QmitkMLBTView::DwiSwitched()
{
  if ( m_Controls->m_TrackingRawImageBox->GetSelectedNode().IsNotNull() )
  {
    mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingRawImageBox->GetSelectedNode()->GetData());
    m_ForestHandler->AddDwi(dwi);
  }
  UpdateGui();
}

void QmitkMLBTView::ForestSwitched()
{
  if ( m_Controls->m_TrackingForestBox->GetSelectedNode().IsNotNull() )
  {
    mitk::TractographyForest::Pointer forest = dynamic_cast<mitk::TractographyForest*>(m_Controls->m_TrackingForestBox->GetSelectedNode()->GetData());
    m_ForestHandler->SetForest(forest);
  }
  UpdateGui();
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
    if (m_Controls->m_TrackingForestBox->GetSelectedNode().IsNotNull() && m_Controls->m_TrackingRawImageBox->GetSelectedNode().IsNotNull())
    {
        m_Controls->m_StartTrackingButton->setEnabled(true);
    }
    else
    {
        m_Controls->m_StartTrackingButton->setEnabled(false);
    }
}

void QmitkMLBTView::SetFocus()
{
  m_Controls->toolBox->setFocus();
}

void QmitkMLBTView::AbortTracking()
{
    if (m_Tracker.IsNotNull())
    {
        m_Tracker->m_AbortTracking = true;
    }
}

void QmitkMLBTView::PauseTracking()
{
    if (m_Tracker.IsNotNull())
    {
        m_Tracker->m_PauseTracking = !m_Tracker->m_PauseTracking;
    }
}

void QmitkMLBTView::ChangeTimerInterval(int value)
{
    m_TrackingTimer->setInterval(value);
}

void QmitkMLBTView::ToggleDemoMode(int )
{
    if (m_Tracker.IsNotNull())
    {
        m_Tracker->SetDemoMode(m_Controls->m_DemoModeBox->isChecked());
        m_Tracker->m_Stop = false;
    }
}

void QmitkMLBTView::BuildFibers()
{
    if (m_Controls->m_DemoModeBox->isChecked() && m_Tracker.IsNotNull() && m_Tracker->m_BuildFibersFinished)
    {
        vtkSmartPointer< vtkPolyData > poly = m_Tracker->GetFiberPolyData();
        mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
        outFib->SetFiberColors(255,255,255);
        m_TractogramNode->SetData(outFib);

        m_SamplingPointsNode->SetData(m_Tracker->m_SamplingPointset);
        m_AlternativePointsNode->SetData(m_Tracker->m_AlternativePointset);
        m_StopVotePointsNode->SetData(m_Tracker->m_StopVotePointset);

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        m_Tracker->m_BuildFibersFinished = false;
        m_Tracker->m_BuildFibersReady = 0;
        m_Tracker->m_Stop = false;
    }
}

void QmitkMLBTView::StartTrackingThread()
{
    m_TractogramNode = mitk::DataNode::New();
    m_TractogramNode->SetName("MLBT Result");
    //m_TractogramNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(20));
    m_TractogramNode->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(true));
    m_TractogramNode->SetProperty("LineWidth", mitk::IntProperty::New(1));
    //m_TractogramNode->SetProperty("color",mitk::ColorProperty::New(0, 1, 1));
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
    m_AlternativePointsNode->SetProperty("color", mitk::ColorProperty::New(0,1,0));
    m_AlternativePointsNode->SetProperty("Pointset.2D.shape", bla);
    m_AlternativePointsNode->SetProperty("Pointset.2D.distance to plane", mitk::FloatProperty::New(1.5));
    m_AlternativePointsNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
    m_AlternativePointsNode->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
    this->GetDataStorage()->Add(m_AlternativePointsNode);

    m_StopVotePointsNode = mitk::DataNode::New();
    m_StopVotePointsNode->SetName("StopVotes");
    m_StopVotePointsNode->SetProperty("pointsize", mitk::FloatProperty::New(0.2));
    m_StopVotePointsNode->SetProperty("color", mitk::ColorProperty::New(1,0,0));
    m_StopVotePointsNode->SetProperty("Pointset.2D.shape", bla);
    m_StopVotePointsNode->SetProperty("Pointset.2D.distance to plane", mitk::FloatProperty::New(1.5));
    m_StopVotePointsNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
    m_StopVotePointsNode->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
    this->GetDataStorage()->Add(m_StopVotePointsNode);

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

    vtkSmartPointer< vtkPolyData > poly = m_Tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
    m_TractogramNode->SetData(outFib);
    m_TractogramNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(1));
    if (m_Controls->m_DemoModeBox->isChecked())
    {
        m_SamplingPointsNode->SetData(m_Tracker->m_SamplingPointset);
        m_AlternativePointsNode->SetData(m_Tracker->m_AlternativePointset);
    }

    m_Tracker = nullptr;
    m_TrackingTimer->stop();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMLBTView::StartTracking()
{
    m_ForestHandler->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);

//    int numThread = itk::MultiThreader::GetGlobalDefaultNumberOfThreads();

    m_Tracker = TrackerType::New();
    m_Tracker->SetDemoMode(m_Controls->m_DemoModeBox->isChecked());
    if (m_Controls->m_DemoModeBox->isChecked())
        m_Tracker->SetNumberOfThreads(1);
    if (m_Controls->m_TrackingMaskImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingMaskImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
        mitk::CastToItkImage(mask, itkMask);
        m_Tracker->SetMaskImage(itkMask);
    }
    if (m_Controls->m_TrackingSeedImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingSeedImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        m_Tracker->SetSeedImage(itkImg);
    }
    if (m_Controls->m_TrackingStopImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TrackingStopImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        m_Tracker->SetStoppingRegions(itkImg);
    }
    if (m_Controls->m_TissueImageBox->GetSelectedNode().IsNotNull())
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_TissueImageBox->GetSelectedNode()->GetData());
        ItkUcharImgType::Pointer itkImg = ItkUcharImgType::New();
        mitk::CastToItkImage(img, itkImg);
        m_Tracker->SetTissueImage(itkImg);
    }

    m_Tracker->SetSeedsPerVoxel(m_Controls->m_NumberOfSeedsBox->value());
    m_Tracker->SetStepSize(m_Controls->m_TrackingStepSizeBox->value());
    m_Tracker->SetMinTractLength(m_Controls->m_MinLengthBox->value());
    m_Tracker->SetMaxTractLength(m_Controls->m_MaxLengthBox->value());
    m_Tracker->SetAposterioriCurvCheck(false);
    m_Tracker->SetNumberOfSamples(m_Controls->m_NumSamplesBox->value());
    m_Tracker->SetTrackingHandler(m_ForestHandler);
    m_Tracker->SetSamplingDistance(m_Controls->m_SamplingDistanceBox->value());
    m_Tracker->SetUseStopVotes(m_Controls->m_OnlyForwardSamples->isChecked());
    m_Tracker->SetOnlyForwardSamples(m_Controls->m_OnlyForwardSamples->isChecked());
    m_Tracker->SetNumPreviousDirections(m_Controls->m_NumPrevDirs->value());
    m_Tracker->SetSeedOnlyGm(m_Controls->m_SeedGm->isChecked());
    m_Tracker->Update();
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
    m_Controls->m_StartTrainingButton->setText("Training in progress ...");
    m_Controls->m_StartTrainingButton->setToolTip("Training in progress. This can take up to a couple of hours.");
    m_Controls->m_StartTrainingButton->setCursor(Qt::WaitCursor);
    QApplication::processEvents();
}

void QmitkMLBTView::OnTrainingThreadStop()
{
    m_Controls->m_StartTrainingButton->setEnabled(true);
    m_Controls->m_StartTrainingButton->setCursor(Qt::ArrowCursor);
    m_Controls->m_StartTrainingButton->setText("Start Training");
    m_Controls->m_StartTrainingButton->setToolTip("Start Training. This can take up to a couple of hours.");

    mitk::DataNode::Pointer node = mitk::DataNode::New();

    if (m_ForestHandler->IsForestValid())
    {
      node->SetData(m_ForestHandler->GetForest());
      QString name("Forest");
      node->SetName(name.toStdString());
      GetDataStorage()->Add(node);
    }

    UpdateGui();
    QApplication::processEvents();
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

    m_ForestHandler->SetDwis(m_SelectedDiffImages);
    m_ForestHandler->SetTractograms(m_SelectedFB);
    m_ForestHandler->SetMaskImages(m_MaskImages);
    m_ForestHandler->SetWhiteMatterImages(m_WhiteMatterImages);
    m_ForestHandler->SetNumTrees(m_Controls->m_NumTreesBox->value());
    m_ForestHandler->SetMaxTreeDepth(m_Controls->m_MaxDepthBox->value());
    m_ForestHandler->SetGrayMatterSamplesPerVoxel(m_Controls->m_GmSamplingBox->value());
    m_ForestHandler->SetSampleFraction(m_Controls->m_SampleFractionBox->value());
    m_ForestHandler->SetFiberSamplingStep(m_Controls->m_TrainingStepSizeBox->value());
    m_ForestHandler->SetNumPreviousDirections(m_Controls->m_NumPrevDirs->value());
    m_ForestHandler->StartTraining();
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
