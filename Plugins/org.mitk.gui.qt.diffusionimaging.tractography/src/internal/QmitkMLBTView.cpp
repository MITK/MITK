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

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4244)
#endif

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
    connect( m_Controls->m_AddTwButton, SIGNAL ( clicked() ), this, SLOT( AddTrainingWidget() ) );
    connect( m_Controls->m_RemoveTwButton, SIGNAL ( clicked() ), this, SLOT( RemoveTrainingWidget() ) );

    mitk::NodePredicateIsDWI::Pointer isDiffusionImage = mitk::NodePredicateIsDWI::New();

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
    mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    finalPredicate = mitk::NodePredicateAnd::New(finalPredicate, isBinaryPredicate);
    AddTrainingWidget();
  }

}

void QmitkMLBTView::SetFocus()
{
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

void QmitkMLBTView::StartTrainingThread()
{
  if (!this->IsTrainingInputValid())
  {
    QMessageBox::warning(nullptr, "Training aborted", "Training could not be started. Not all necessary datasets were selected.");
    return;
  }

  if (m_Controls->m_FeatureTypeBox->currentIndex()==0)
    m_ForestHandler = new mitk::TrackingHandlerRandomForest<6,28>();
  else
    m_ForestHandler = new mitk::TrackingHandlerRandomForest<6,100>();

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

  switch (m_Controls->m_FeatureTypeBox->currentIndex())
  {
  case 0:
    if (dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->IsForestValid())
      node->SetData(dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->GetForest());
    break;
  case 1:
    if (dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->IsForestValid())
      node->SetData(dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->GetForest());
    break;
  default:
    mitkThrow() << "Unknown feature type!";
  }

  QString name("Forest");
  node->SetName(name.toStdString());
  GetDataStorage()->Add(node);

  delete m_ForestHandler;

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

  switch (m_Controls->m_FeatureTypeBox->currentIndex())
  {
  case 0:
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetDwis(m_SelectedDiffImages);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetTractograms(m_SelectedFB);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetMaskImages(m_MaskImages);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetWhiteMatterImages(m_WhiteMatterImages);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetNumTrees(m_Controls->m_NumTreesBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetMaxTreeDepth(m_Controls->m_MaxDepthBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetGrayMatterSamplesPerVoxel(m_Controls->m_GmSamplingBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetSampleFraction(m_Controls->m_SampleFractionBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetFiberSamplingStep(m_Controls->m_TrainingStepSizeBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->SetNumPreviousDirections(m_Controls->m_NumPrevDirs->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_ForestHandler)->StartTraining();
    break;
  case 1:
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetDwis(m_SelectedDiffImages);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetTractograms(m_SelectedFB);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetMaskImages(m_MaskImages);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetWhiteMatterImages(m_WhiteMatterImages);
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetNumTrees(m_Controls->m_NumTreesBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetMaxTreeDepth(m_Controls->m_MaxDepthBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetGrayMatterSamplesPerVoxel(m_Controls->m_GmSamplingBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetSampleFraction(m_Controls->m_SampleFractionBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetFiberSamplingStep(m_Controls->m_TrainingStepSizeBox->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->SetNumPreviousDirections(m_Controls->m_NumPrevDirs->value());
    dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_ForestHandler)->StartTraining();
    break;
  default:
    mitkThrow() << "Unknown feature type!";
  }
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

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
