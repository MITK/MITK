/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QMessageBox>
#include <QThreadPool>

#include "mitkWorkbenchUtil.h"

#include "PerfusionCurveDescriptionParameterView.h"
#include "QmitkDescriptionParameterBackgroundJob.h"
#include "mitkAreaUnderTheCurveDescriptionParameter.h"
#include "mitkAreaUnderFirstMomentDescriptionParameter.h"
#include "mitkMeanResidenceTimeDescriptionParameter.h"
#include "mitkTimeToPeakCurveDescriptionParameter.h"
#include "mitkMaximumCurveDescriptionParameter.h"
#include "mitkPixelBasedDescriptionParameterImageGenerator.h"
#include "mitkCurveParameterFunctor.h"
#include "mitkExtractTimeGrid.h"

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include "mitkNodePredicateFunction.h"
#include <mitkMultiLabelPredicateHelper.h>

#include <iostream>

const std::string PerfusionCurveDescriptionParameterView::VIEW_ID =
  "org.mitk.views.pharmacokinetics.curvedescriptor";

void PerfusionCurveDescriptionParameterView::SetFocus()
{
  m_Controls.btnCalculateParameters->setFocus();
}

void PerfusionCurveDescriptionParameterView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
  m_Controls.btnCalculateParameters->setEnabled(false);


  m_Controls.timeSeriesNodeSelector->SetNodePredicate(this->m_isValidTimeSeriesImagePredicate);
  m_Controls.timeSeriesNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.timeSeriesNodeSelector->SetSelectionIsOptional(false);
  m_Controls.timeSeriesNodeSelector->SetInvalidInfo("Please select time series.");

  connect(m_Controls.timeSeriesNodeSelector,
    &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this,
    &PerfusionCurveDescriptionParameterView::OnNodeSelectionChanged);

  connect(m_Controls.btnCalculateParameters, SIGNAL(clicked()), this,
          SLOT(OnCalculateParametersButtonClicked()));

  // Should be done last, if everything else is configured because it triggers the autoselection of data.
  m_Controls.timeSeriesNodeSelector->SetAutoSelectNewNodes(true);

  InitParameterList();
}



void PerfusionCurveDescriptionParameterView::OnNodeSelectionChanged( const QList<mitk::DataNode::Pointer>& /*nodes*/)
{

  m_Controls.btnCalculateParameters->setEnabled(false);


  if (m_Controls.timeSeriesNodeSelector->GetSelectedNode().IsNotNull())
  {
    this->m_selectedNode = m_Controls.timeSeriesNodeSelector->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
  }
  else
  {
    this->m_selectedNode = nullptr;
    this->m_selectedImage = nullptr;
  }


  if (m_selectedImage.IsNotNull())
  {
      m_Controls.btnCalculateParameters->setEnabled(true);
  }
  else if (m_selectedNode.IsNotNull())
  {
    this->OnJobStatusChanged("Cannot compute parameters. Selected node is not an image.");
  }
  else
  {
    this->OnJobStatusChanged("Cannot compute parameters. No node selected.");
  }
}

PerfusionCurveDescriptionParameterView::PerfusionCurveDescriptionParameterView()
{
  m_selectedNode = nullptr;
  m_selectedImage = nullptr;

  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");

  auto isNoMask = mitk::NodePredicateNot::New(mitk::GetMultiLabelSegmentationPredicate());

  auto isDynamicData = mitk::NodePredicateFunction::New([](const mitk::DataNode* node)
  {
    return  (node && node->GetData() && node->GetData()->GetTimeSteps() > 1);
  });


  this->m_isValidTimeSeriesImagePredicate = mitk::NodePredicateAnd::New(isDynamicData, isImage, isNoMask);
}

void PerfusionCurveDescriptionParameterView::InitParameterList()
{
  m_ParameterMap.clear();

  mitk::CurveDescriptionParameterBase::Pointer parameterFunction =
    mitk::AreaUnderTheCurveDescriptionParameter::New().GetPointer();
  m_ParameterMap.insert(std::make_pair(ParameterNameType("AUC"), parameterFunction));
  parameterFunction = mitk::AreaUnderFirstMomentDescriptionParameter::New().GetPointer();
  m_ParameterMap.insert(std::make_pair(ParameterNameType("AUMC"), parameterFunction));
  parameterFunction = mitk::MeanResidenceTimeDescriptionParameter::New().GetPointer();
  m_ParameterMap.insert(std::make_pair(ParameterNameType("MRT"), parameterFunction));
  parameterFunction = mitk::MaximumCurveDescriptionParameter::New().GetPointer();
  m_ParameterMap.insert(std::make_pair(ParameterNameType("Maximum"), parameterFunction));
  parameterFunction = mitk::TimeToPeakCurveDescriptionParameter::New().GetPointer();
  m_ParameterMap.insert(std::make_pair(ParameterNameType("TimeToPeak"), parameterFunction));

  for (ParameterMapType::const_iterator pos = m_ParameterMap.begin(); pos != m_ParameterMap.end();
       ++pos)
  {
    QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(pos->first),
        this->m_Controls.parameterlist);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
  }
};

void PerfusionCurveDescriptionParameterView::ConfigureFunctor(mitk::CurveParameterFunctor* functor)
const
{
  functor->SetGrid(mitk::ExtractTimeGrid(m_selectedImage));

  for (int pos = 0; pos < this->m_Controls.parameterlist->count(); ++pos)
  {
    QListWidgetItem* item = this->m_Controls.parameterlist->item(pos);
    mitk::CurveDescriptionParameterBase::Pointer parameterFunction = m_ParameterMap.at(
          item->text().toStdString());

    if (item->checkState() == Qt::Checked)
    {
      functor->RegisterDescriptionParameter(item->text().toStdString(), parameterFunction);
    }
  }
}

void PerfusionCurveDescriptionParameterView::OnCalculateParametersButtonClicked()
{
  mitk::PixelBasedDescriptionParameterImageGenerator::Pointer generator =
    mitk::PixelBasedDescriptionParameterImageGenerator::New();
  mitk::CurveParameterFunctor::Pointer functor = mitk::CurveParameterFunctor::New();
  this->ConfigureFunctor(functor);



  generator->SetFunctor(functor);
  generator->SetDynamicImage(m_selectedImage);


  /////////////////////////
  //create job and put it into the thread pool
  DescriptionParameterBackgroundJob* pJob = new DescriptionParameterBackgroundJob(generator,
      this->m_selectedNode);
  pJob->setAutoDelete(true);

  connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnJobError(QString)));
  connect(pJob, SIGNAL(Finished()), this, SLOT(OnJobFinished()));
  connect(pJob, SIGNAL(ResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType,
                       const DescriptionParameterBackgroundJob*)), this,
          SLOT(OnJobResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType,
                                        const DescriptionParameterBackgroundJob*)), Qt::BlockingQueuedConnection);

  connect(pJob, SIGNAL(JobProgress(double)), this, SLOT(OnJobProgress(double)));
  connect(pJob, SIGNAL(JobStatusChanged(QString)), this, SLOT(OnJobStatusChanged(QString)));

  QThreadPool* threadPool = QThreadPool::globalInstance();
  threadPool->start(pJob);
}



void PerfusionCurveDescriptionParameterView::OnJobFinished()
{
  this->m_Controls.infoBox->append(QString("Fitting finished"));
};

void PerfusionCurveDescriptionParameterView::OnJobError(QString err)
{
  MITK_ERROR << err.toStdString().c_str();
  m_Controls.infoBox->append(QString("<font color='red'><b>") + err + QString("</b></font>"));
};

void PerfusionCurveDescriptionParameterView::OnJobResultsAreAvailable(
  mitk::modelFit::ModelFitResultNodeVectorType results, const DescriptionParameterBackgroundJob* pJob)
{
  for (auto image : results)
  {
    this->GetDataStorage()->Add(image, pJob->GetParentNode());
  }
};

void PerfusionCurveDescriptionParameterView::OnJobProgress(double progress)
{
  this->m_Controls.progressBar->setValue(100 * progress);
  QString report = QString("Progress. ") + QString::number(progress);
  this->m_Controls.infoBox->append(report);
};

void PerfusionCurveDescriptionParameterView::OnJobStatusChanged(QString info)
{
  this->m_Controls.infoBox->append(info);
  MITK_INFO << info.toStdString().c_str();
}

