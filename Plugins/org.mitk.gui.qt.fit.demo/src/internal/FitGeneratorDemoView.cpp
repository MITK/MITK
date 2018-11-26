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

#include "FitGeneratorDemoView.h"
#include <QMessageBox>
#include <QThreadPool>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include "mitkWorkbenchUtil.h"

#include <mitkImage.h>
#include <mitkImageTimeSelector.h>

#include <mitkPixelBasedParameterFitImageGenerator.h>
#include <mitkLinearModelFactory.h>
#include <mitkLinearModelParameterizer.h>
#include <mitkLevenbergMarquardtModelFitFunctor.h>
#include <mitkSumOfSquaredDifferencesFitCostFunction.h>
#include <mitkModelFitResultHelper.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>

#include <mitkTestArtifactGenerator.h>

#include <iostream>

const std::string FitGeneratorDemoView::VIEW_ID = "org.mitk.gui.qt.fit.demo";

typedef itk::Image<double, 3> FrameITKImageType;
typedef itk::Image<double, 4> DynamicITKImageType;

void FitGeneratorDemoView::SetFocus()
{
  m_Controls.btnModelling->setFocus();
}

void FitGeneratorDemoView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
  m_Controls.btnModelling->setEnabled(false);

  connect(m_Controls.btnModelling, SIGNAL(clicked()), this, SLOT(OnModellingButtonClicked()));
  connect(m_Controls.btnGenerateTestData, SIGNAL(clicked()), this,
          SLOT(OnGenerateTestDataButtonClicked()));

  m_Controls.leFitName->setText(tr("demo"));
}

void FitGeneratorDemoView::OnModellingButtonClicked()
{
  Generate();
}

void FitGeneratorDemoView::OnGenerateTestDataButtonClicked()
{
  mitk::Image::Pointer testImage = mitk::GenerateDynamicTestImageMITK();

  mitk::DataNode::Pointer testNode = mitk::DataNode::New();

  testNode->SetData(testImage);
  testNode->SetName("LinearModel_4DTestImage");

  this->GetDataStorage()->Add(testNode);
}

void FitGeneratorDemoView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& /*nodes*/)
{

  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();

  m_selectedNode = NULL;
  m_selectedImage = NULL;
  m_selectedMaskNode = NULL;
  m_selectedMask = NULL;

  if (!dataNodes.empty())
  {
    m_selectedNode = dataNodes[0];
    mitk::Image* selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());

    if (selectedImage && selectedImage->GetDimension(3) > 1)
    {
      m_selectedImage = selectedImage;
    }

    if (dataNodes.size() > 1)
    {
      m_selectedMaskNode = dataNodes[1];

      mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
      mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
      mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
      mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);

      mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

      mitk::Image* selectedMask = dynamic_cast<mitk::Image*>(this->m_selectedMaskNode->GetData());

      if (selectedMask && maskPredicate->CheckNode(m_selectedMaskNode))
      {
        m_selectedMask = selectedMask;

        if (this->m_selectedMask->GetTimeSteps() > 1)
        {
          MITK_INFO <<
                    "Selected mask has multiple timesteps. Only use first timestep to mask model fit. Mask name: " <<
                    m_selectedMaskNode->GetName() ;
          mitk::ImageTimeSelector::Pointer maskedImageTimeSelector = mitk::ImageTimeSelector::New();
          maskedImageTimeSelector->SetInput(this->m_selectedMask);
          maskedImageTimeSelector->SetTimeNr(0);
          maskedImageTimeSelector->UpdateLargestPossibleRegion();
          this->m_selectedMask = maskedImageTimeSelector->GetOutput();
        }
      }

    }
  }

  if (m_selectedImage)
  {
    m_Controls.lTimeseries->setText((m_selectedNode->GetName()).c_str());
  }
  else
  {
    if (m_selectedNode.IsNull())
    {
      m_Controls.lTimeseries->setText("None");
    }
    else
    {
      m_Controls.lTimeseries->setText("Error. Selected node #1 is no 4D image!");
    }
  }

  if (m_selectedMask)
  {
    m_Controls.lMask->setText((m_selectedMaskNode->GetName()).c_str());
  }
  else
  {
    if (m_selectedMaskNode.IsNull())
    {
      m_Controls.lMask->setText("None");
    }
    else
    {
      m_Controls.lMask->setText("Error. Selected node #2 is no mask!");
    }
  }

  m_Controls.btnModelling->setEnabled(m_selectedImage.IsNotNull());
}

void FitGeneratorDemoView::Generate()
{
  mitk::PixelBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::PixelBasedParameterFitImageGenerator::New();

  //Model configuration (static parameters) can be done now

  //Specify fitting strategy and evaluation parameters
  mitk::LevenbergMarquardtModelFitFunctor::Pointer fitFunctor =
    mitk::LevenbergMarquardtModelFitFunctor::New();
  mitk::SumOfSquaredDifferencesFitCostFunction::Pointer evaluation =
    mitk::SumOfSquaredDifferencesFitCostFunction::New();

  fitFunctor->RegisterEvaluationParameter("sum_diff^2", evaluation);

  //Parametrize fit generator
  mitk::LinearModelParameterizer::Pointer parameterizer = mitk::LinearModelParameterizer::New();
  fitGenerator->SetModelParameterizer(parameterizer);
  fitGenerator->SetDynamicImage(m_selectedImage);
  fitGenerator->SetFitFunctor(fitFunctor);

  if (m_selectedMask.IsNotNull())
  {
    fitGenerator->SetMask(m_selectedMask);
  }

  mitk::modelFit::ModelFitInfo::Pointer fitSession =
    mitk::modelFit::CreateFitInfoFromModelParameterizer(parameterizer, m_selectedNode->GetData(),
        mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED(), m_Controls.leFitName->text().toStdString());

  /////////////////////////
  //create job and put it into the thread pool
  ParameterFitBackgroundJob* pJob = new ParameterFitBackgroundJob(fitGenerator, fitSession, m_selectedNode);
  pJob->setAutoDelete(true);

  connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnJobError(QString)));
  connect(pJob, SIGNAL(Finished()), this, SLOT(OnJobFinished()));
  connect(pJob, SIGNAL(ResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType,
                       const ParameterFitBackgroundJob*)), this,
          SLOT(OnJobResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType,
                                        const ParameterFitBackgroundJob*)), Qt::BlockingQueuedConnection);

  connect(pJob, SIGNAL(JobProgress(double)), this, SLOT(OnJobProgress(double)));
  connect(pJob, SIGNAL(JobStatusChanged(QString)), this, SLOT(OnJobStatusChanged(QString)));

  QThreadPool* threadPool = QThreadPool::globalInstance();
  threadPool->start(pJob);
}

FitGeneratorDemoView::FitGeneratorDemoView()
{
  m_selectedImage = NULL;
  m_selectedNode = NULL;

}

void FitGeneratorDemoView::OnJobFinished()
{
  this->m_Controls.textEdit->append(QString("Fitting finished"));
};

void FitGeneratorDemoView::OnJobError(QString err)
{
  MITK_ERROR << err.toStdString().c_str();

  m_Controls.textEdit->append(QString("<font color='red'><b>") + err + QString("</b></font>"));

};

void FitGeneratorDemoView::OnJobResultsAreAvailable(
  mitk::modelFit::ModelFitResultNodeVectorType results, const ParameterFitBackgroundJob* pJob)
{
  //Store the resulting parameter fit image via convenience helper function in data storage
  //(handles the correct generation of the nodes and their properties)
  mitk::modelFit::StoreResultsInDataStorage(this->GetDataStorage(), results, pJob->GetParentNode());
};

void FitGeneratorDemoView::OnJobProgress(double progress)
{
  QString report = QString("Progress. ") + QString::number(progress);
  this->m_Controls.textEdit->append(report);
};

void FitGeneratorDemoView::OnJobStatusChanged(QString info)
{
  this->m_Controls.textEdit->append(info);
}
