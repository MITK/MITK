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

#include "GenericDataFittingView.h"

#include "mitkWorkbenchUtil.h"

#include <mitkLinearModelFactory.h>
#include <mitkLinearModelParameterizer.h>
#include <mitkGenericParamModelFactory.h>
#include <mitkGenericParamModelParameterizer.h>
#include <mitkT2DecayModelFactory.h>
#include <mitkT2DecayModelParameterizer.h>


#include <mitkValueBasedParameterizationDelegate.h>

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkPixelBasedParameterFitImageGenerator.h>
#include <mitkROIBasedParameterFitImageGenerator.h>
#include <mitkLevenbergMarquardtModelFitFunctor.h>
#include <mitkSumOfSquaredDifferencesFitCostFunction.h>
#include <mitkNormalizedSumOfSquaredDifferencesFitCostFunction.h>
#include <mitkSimpleBarrierConstraintChecker.h>
#include <mitkModelFitResultHelper.h>
#include <mitkImageTimeSelector.h>
#include <mitkMaskedDynamicImageStatisticsGenerator.h>
#include <mitkExtractTimeGrid.h>

#include <QMessageBox>
#include <QThreadPool>
#include <QmitkDataStorageComboBox.h>

// Includes for image casting between ITK and MITK
#include <mitkImage.h>
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <iostream>


const std::string GenericDataFittingView::VIEW_ID = "org.mitk.gui.qt.fit.genericfitting";

void GenericDataFittingView::SetFocus()
{
  m_Controls.btnModelling->setFocus();
}

void GenericDataFittingView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.btnModelling->setEnabled(false);

  this->InitModelComboBox();

  connect(m_Controls.btnModelling, SIGNAL(clicked()), this, SLOT(OnModellingButtonClicked()));

  connect(m_Controls.comboModel, SIGNAL(currentIndexChanged(int)), this, SLOT(OnModellSet(int)));
  connect(m_Controls.radioPixelBased, SIGNAL(toggled(bool)), this, SLOT(UpdateGUIControls()));

  //Gerneric setting
  m_Controls.groupGeneric->hide();
  m_Controls.labelFormulaInfo->hide();
  connect(m_Controls.editFormula, SIGNAL(textChanged(const QString&)), this,
          SLOT(UpdateGUIControls()));
  connect(m_Controls.checkFormulaInfo, SIGNAL(toggled(bool)), m_Controls.labelFormulaInfo,
          SLOT(setVisible(bool)));
  connect(m_Controls.nrOfParams, SIGNAL(valueChanged(int)), this, SLOT(OnNrOfParamsChanged()));


  //Model fit configuration
  m_Controls.groupBox_FitConfiguration->hide();

  m_Controls.checkBox_Constraints->setEnabled(false);
  m_Controls.constraintManager->setEnabled(false);
  m_Controls.initialValuesManager->setEnabled(false);

  connect(m_Controls.radioButton_StartParameters, SIGNAL(toggled(bool)), this,
          SLOT(UpdateGUIControls()));
  connect(m_Controls.checkBox_Constraints, SIGNAL(toggled(bool)), this,
          SLOT(UpdateGUIControls()));

  connect(m_Controls.radioButton_StartParameters, SIGNAL(toggled(bool)),
          m_Controls.initialValuesManager,
          SLOT(setEnabled(bool)));
  connect(m_Controls.checkBox_Constraints, SIGNAL(toggled(bool)), m_Controls.constraintManager,
          SLOT(setEnabled(bool)));
  connect(m_Controls.checkBox_Constraints, SIGNAL(toggled(bool)), m_Controls.constraintManager,
          SLOT(setVisible(bool)));

  UpdateGUIControls();
}

void GenericDataFittingView::UpdateGUIControls()
{
  m_Controls.lineFitName->setPlaceholderText(QString::fromStdString(this->GetDefaultFitName()));
  m_Controls.lineFitName->setEnabled(!m_FittingInProgress);

  m_Controls.checkBox_Constraints->setEnabled(m_modelConstraints.IsNotNull());

  bool isGenericFactory = dynamic_cast<mitk::GenericParamModelFactory*>
                          (m_selectedModelFactory.GetPointer()) != NULL;

  m_Controls.groupGeneric->setVisible(isGenericFactory);

  m_Controls.groupBox_FitConfiguration->setVisible(m_selectedModelFactory);

  m_Controls.groupBox->setEnabled(!m_FittingInProgress);
  m_Controls.comboModel->setEnabled(!m_FittingInProgress);
  m_Controls.groupGeneric->setEnabled(!m_FittingInProgress);
  m_Controls.groupBox_FitConfiguration->setEnabled(!m_FittingInProgress);

  m_Controls.radioROIbased->setEnabled(m_selectedMask.IsNotNull());

  m_Controls.btnModelling->setEnabled(m_selectedImage.IsNotNull()
                                      && m_selectedModelFactory.IsNotNull() && !m_FittingInProgress && CheckModelSettings());
}

std::string GenericDataFittingView::GetFitName() const
{
  std::string fitName = m_Controls.lineFitName->text().toStdString();
  if (fitName.empty())
  {
    fitName = m_Controls.lineFitName->placeholderText().toStdString();
  }
  return fitName;
}

std::string GenericDataFittingView::GetDefaultFitName() const
{
    std::string defaultName = "undefined model";

    if (this->m_selectedModelFactory.IsNotNull())
    {
        defaultName = this->m_selectedModelFactory->GetClassID();
    }

    if (this->m_Controls.radioPixelBased->isChecked())
    {
        defaultName += "_pixel";
    }
    else
    {
        defaultName += "_roi";
    }

    return defaultName;
}


void GenericDataFittingView::OnNrOfParamsChanged()
{
  PrepareFitConfiguration();
  UpdateGUIControls();
}


void GenericDataFittingView::OnModellSet(int index)
{
  m_selectedModelFactory = NULL;

  if (index > 0)
  {
    if (static_cast<ModelFactoryStackType::size_type>(index) <= m_FactoryStack.size() )
    {
        m_selectedModelFactory = m_FactoryStack[index - 1];
    }
    else
    {
        MITK_WARN << "Invalid model index. Index outside of the factory stack. Factory stack size: "<< m_FactoryStack.size() << "; invalid index: "<< index;
    }
  }

  UpdateGUIControls();
}

bool GenericDataFittingView::IsGenericParamFactorySelected() const
{
  return dynamic_cast<mitk::GenericParamModelFactory*>
    (m_selectedModelFactory.GetPointer()) != nullptr;
}

void GenericDataFittingView::PrepareFitConfiguration()
{
  if (m_selectedModelFactory)
  {
    mitk::ModelBase::ParameterNamesType paramNames = m_selectedModelFactory->GetParameterNames();
    unsigned int nrOfPools = this->m_Controls.nrOfParams->value();

    //init values
    if (this->IsGenericParamFactorySelected())
    {
      mitk::modelFit::ModelFitInfo::Pointer fitInfo = mitk::modelFit::ModelFitInfo::New();
      fitInfo->staticParamMap.Add(mitk::GenericParamModel::NAME_STATIC_PARAMETER_number, { static_cast<double>(nrOfPools) });

      auto parameterizer = m_selectedModelFactory->CreateParameterizer(fitInfo);
      paramNames = parameterizer->GetParameterNames();

      m_Controls.initialValuesManager->setInitialValues(paramNames, parameterizer->GetDefaultInitialParameterization());
    }
    else
    {
      m_Controls.initialValuesManager->setInitialValues(paramNames, this->m_selectedModelFactory->GetDefaultInitialParameterization());
    }

    //constraints
    this->m_modelConstraints = dynamic_cast<mitk::SimpleBarrierConstraintChecker*>
      (m_selectedModelFactory->CreateDefaultConstraints().GetPointer());

    if (this->m_modelConstraints.IsNull())
    {
      this->m_modelConstraints = mitk::SimpleBarrierConstraintChecker::New();
    }

    m_Controls.constraintManager->setChecker(this->m_modelConstraints, paramNames);
  }
};

void GenericDataFittingView::OnModellingButtonClicked()
{
  //check if all static parameters set
  if (m_selectedModelFactory.IsNotNull() && CheckModelSettings())
  {
    mitk::ParameterFitImageGeneratorBase::Pointer generator = NULL;
    mitk::modelFit::ModelFitInfo::Pointer fitSession = NULL;

    bool isLinearFactory = dynamic_cast<mitk::LinearModelFactory*>
                           (m_selectedModelFactory.GetPointer()) != NULL;
    bool isGenericFactory = dynamic_cast<mitk::GenericParamModelFactory*>
                            (m_selectedModelFactory.GetPointer()) != NULL;
    bool isT2DecayFactory = dynamic_cast<mitk::T2DecayModelFactory*>
      (m_selectedModelFactory.GetPointer()) != NULL;

    if (isLinearFactory)
    {
      if (this->m_Controls.radioPixelBased->isChecked())
      {
        GenerateModelFit_PixelBased<mitk::LinearModelParameterizer>(fitSession, generator);
      }
      else
      {
        GenerateModelFit_ROIBased<mitk::LinearModelParameterizer>(fitSession, generator);
      }
    }
    else if (isGenericFactory)
    {
      if (this->m_Controls.radioPixelBased->isChecked())
      {
        GenerateModelFit_PixelBased<mitk::GenericParamModelParameterizer>(fitSession, generator);
      }
      else
      {
        GenerateModelFit_ROIBased<mitk::GenericParamModelParameterizer>(fitSession, generator);
      }
    }
    else if (isT2DecayFactory)
    {
      if (this->m_Controls.radioPixelBased->isChecked())
      {
        GenerateModelFit_PixelBased<mitk::T2DecayModelParameterizer>(fitSession, generator);
      }
      else
      {
        GenerateModelFit_ROIBased<mitk::T2DecayModelParameterizer>(fitSession, generator);
      }
    }
    //add other models with else if

    if (generator.IsNotNull() && fitSession.IsNotNull())
    {
      m_FittingInProgress = true;
      UpdateGUIControls();
      DoFit(fitSession, generator);
    }
    else
    {
      QMessageBox box;
      box.setText("Fitting error!");
      box.setInformativeText("Could not establish fitting job. Error when setting ab generator, model parameterizer or session info.");
      box.setStandardButtons(QMessageBox::Ok);
      box.setDefaultButton(QMessageBox::Ok);
      box.setIcon(QMessageBox::Warning);
      box.exec();
    }

  }
  else
  {
    QMessageBox box;
    box.setText("Static parameters for model are not set!");
    box.setInformativeText("Some static parameters, that are needed for calculation are not set and equal to zero. Modeling not possible");
    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Warning);
    box.exec();
  }
}


void GenericDataFittingView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  m_selectedNode = NULL;
  m_selectedImage = NULL;
  m_selectedMaskNode = NULL;
  m_selectedMask = NULL;

  m_Controls.masklabel->setText("No (valid) mask selected.");
  m_Controls.timeserieslabel->setText("No (valid) series selected.");

  QList<mitk::DataNode::Pointer> nodes = selectedNodes;

  mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);

  mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

  if (nodes.size() > 0 && isImage->CheckNode(nodes.front()))
  {
    this->m_selectedNode = nodes.front();
    this->m_selectedImage = dynamic_cast<mitk::Image*>(this->m_selectedNode->GetData());
    m_Controls.timeserieslabel->setText((this->m_selectedNode->GetName()).c_str());
    nodes.pop_front();
  }

  if (nodes.size() > 0 && maskPredicate->CheckNode(nodes.front()))
  {
      this->m_selectedMaskNode = nodes.front();
      this->m_selectedMask = dynamic_cast<mitk::Image*>(this->m_selectedMaskNode->GetData());

      if (this->m_selectedMask->GetTimeSteps() > 1)
      {
        MITK_INFO <<
                  "Selected mask has multiple timesteps. Only use first timestep to mask model fit. Mask name: " <<
                  m_selectedMaskNode->GetName();
        mitk::ImageTimeSelector::Pointer maskedImageTimeSelector = mitk::ImageTimeSelector::New();
        maskedImageTimeSelector->SetInput(this->m_selectedMask);
        maskedImageTimeSelector->SetTimeNr(0);
        maskedImageTimeSelector->UpdateLargestPossibleRegion();
        this->m_selectedMask = maskedImageTimeSelector->GetOutput();
      }

      m_Controls.masklabel->setText((this->m_selectedMaskNode->GetName()).c_str());
  }

  if (m_selectedMask.IsNull())
  {
    this->m_Controls.radioPixelBased->setChecked(true);
  }

  UpdateGUIControls();
}

bool GenericDataFittingView::CheckModelSettings() const
{
  bool ok = true;

  //check wether any model is set at all. Otherwise exit with false
  if (m_selectedModelFactory.IsNotNull())
  {
    bool isGenericFactory = dynamic_cast<mitk::GenericParamModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL; 

    if (isGenericFactory)
    {
      ok = !m_Controls.editFormula->text().isEmpty();
    }
  }
  else
  {
    ok = false;
  }

  return ok;
}

void GenericDataFittingView::ConfigureInitialParametersOfParameterizer(mitk::ModelParameterizerBase*
    parameterizer) const
{
  if (m_Controls.radioButton_StartParameters->isChecked())
  {
    //use user defined initial parameters
    mitk::ValueBasedParameterizationDelegate::Pointer paramDelegate =
      mitk::ValueBasedParameterizationDelegate::New();
    paramDelegate->SetInitialParameterization(m_Controls.initialValuesManager->getInitialValues());

    parameterizer->SetInitialParameterizationDelegate(paramDelegate);
  }

  mitk::GenericParamModelParameterizer* genericParameterizer =
    dynamic_cast<mitk::GenericParamModelParameterizer*>(parameterizer);

  if (genericParameterizer)
  {
    genericParameterizer->SetFunctionString(m_Controls.editFormula->text().toStdString());
  }
}

template <typename TParameterizer>
void GenericDataFittingView::GenerateModelFit_PixelBased(mitk::modelFit::ModelFitInfo::Pointer&
    modelFitInfo, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::PixelBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::PixelBasedParameterFitImageGenerator::New();

  typename TParameterizer::Pointer modelParameterizer =
    TParameterizer::New();

  auto genericParameterizer = dynamic_cast<mitk::GenericParamModelParameterizer*>(modelParameterizer.GetPointer());
  if (genericParameterizer)
  {
    genericParameterizer->SetNumberOfParameters(this->m_Controls.nrOfParams->value());
  }

  this->ConfigureInitialParametersOfParameterizer(modelParameterizer);

  //Specify fitting strategy and criterion parameters
  mitk::ModelFitFunctorBase::Pointer fitFunctor = CreateDefaultFitFunctor(modelParameterizer);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  std::string roiUID = "";

  if (m_selectedMask.IsNotNull())
  {
    fitGenerator->SetMask(m_selectedMask);
    roiUID = mitk::EnsureModelFitUID(this->m_selectedMaskNode);
  }

  mitk::EnsureModelFitUID(this->m_selectedNode);

  fitGenerator->SetDynamicImage(this->m_selectedImage);
  fitGenerator->SetFitFunctor(fitFunctor);

  generator = fitGenerator.GetPointer();

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    m_selectedNode->GetData(), mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED(), this->GetFitName(),
                 roiUID);
}

template <typename TParameterizer>
void GenericDataFittingView::GenerateModelFit_ROIBased(
  mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
  mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::ROIBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::ROIBasedParameterFitImageGenerator::New();

  typename TParameterizer::Pointer modelParameterizer =
    TParameterizer::New();

  auto genericParameterizer = dynamic_cast<mitk::GenericParamModelParameterizer*>(modelParameterizer.GetPointer());
  if (genericParameterizer)
  {
    genericParameterizer->SetNumberOfParameters(this->m_Controls.nrOfParams->value());
  }

  this->ConfigureInitialParametersOfParameterizer(modelParameterizer);

  //Compute ROI signal
  mitk::MaskedDynamicImageStatisticsGenerator::Pointer signalGenerator =
    mitk::MaskedDynamicImageStatisticsGenerator::New();
  signalGenerator->SetMask(m_selectedMask);
  signalGenerator->SetDynamicImage(m_selectedImage);
  signalGenerator->Generate();

  mitk::MaskedDynamicImageStatisticsGenerator::ResultType roiSignal = signalGenerator->GetMean();

  //Specify fitting strategy and criterion parameters
  mitk::ModelFitFunctorBase::Pointer fitFunctor = CreateDefaultFitFunctor(modelParameterizer);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  fitGenerator->SetMask(m_selectedMask);
  fitGenerator->SetFitFunctor(fitFunctor);
  fitGenerator->SetSignal(roiSignal);
  fitGenerator->SetTimeGrid(mitk::ExtractTimeGrid(m_selectedImage));

  generator = fitGenerator.GetPointer();

  mitk::EnsureModelFitUID(this->m_selectedNode);
  std::string roiUID = mitk::EnsureModelFitUID(this->m_selectedMaskNode);

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    m_selectedNode->GetData(), mitk::ModelFitConstants::FIT_TYPE_VALUE_ROIBASED(), this->GetFitName(),
                 roiUID);

  mitk::ScalarListLookupTable::ValueType infoSignal;

  for (mitk::MaskedDynamicImageStatisticsGenerator::ResultType::const_iterator pos =
         roiSignal.begin(); pos != roiSignal.end(); ++pos)
  {
    infoSignal.push_back(*pos);
  }

  modelFitInfo->inputData.SetTableValue("ROI", infoSignal);
}

void GenericDataFittingView::DoFit(const mitk::modelFit::ModelFitInfo* fitSession,
                                   mitk::ParameterFitImageGeneratorBase* generator)
{
  QString message = "<font color='green'>Fitting Data Set . . .</font>";
  m_Controls.infoBox->append(message);

  /////////////////////////
  //create job and put it into the thread pool
  ParameterFitBackgroundJob* pJob = new ParameterFitBackgroundJob(generator, fitSession, this->m_selectedNode);
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

GenericDataFittingView::GenericDataFittingView() : m_FittingInProgress(false)
{
  m_selectedImage = NULL;
  m_selectedMask = NULL;

  mitk::ModelFactoryBase::Pointer factory =
    mitk::LinearModelFactory::New().GetPointer();
  m_FactoryStack.push_back(factory);
  factory = mitk::GenericParamModelFactory::New().GetPointer();
  m_FactoryStack.push_back(factory);
  factory = mitk::T2DecayModelFactory::New().GetPointer();
  m_FactoryStack.push_back(factory);

  this->m_IsNotABinaryImagePredicate = mitk::NodePredicateAnd::New(
                                         mitk::TNodePredicateDataType<mitk::Image>::New(),
                                         mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("binary",
                                             mitk::BoolProperty::New(true))),
                                         mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();
  this->m_IsBinaryImagePredicate = mitk::NodePredicateAnd::New(
                                     mitk::TNodePredicateDataType<mitk::Image>::New(),
                                     mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)),
                                     mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();
}

void GenericDataFittingView::OnJobFinished()
{
  this->m_Controls.infoBox->append(QString("<font color='green'>Fitting finished</font>"));
  this->m_FittingInProgress = false;
  this->UpdateGUIControls();
};

void GenericDataFittingView::OnJobError(QString err)
{
  MITK_ERROR << err.toStdString().c_str();

  m_Controls.infoBox->append(QString("<font color='red'><b>") + err + QString("</b></font>"));

};

void GenericDataFittingView::OnJobResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType
    results,
    const ParameterFitBackgroundJob* pJob)
{
  //Store the resulting parameter fit image via convenience helper function in data storage
  //(handles the correct generation of the nodes and their properties)
  mitk::modelFit::StoreResultsInDataStorage(this->GetDataStorage(), results, pJob->GetParentNode());
};

void GenericDataFittingView::OnJobProgress(double progress)
{
  QString report = QString("Progress. ") + QString::number(progress);
  this->m_Controls.infoBox->append(report);
};

void GenericDataFittingView::OnJobStatusChanged(QString info)
{
  this->m_Controls.infoBox->append(info);
}


void GenericDataFittingView::InitModelComboBox() const
{
  this->m_Controls.comboModel->clear();
  this->m_Controls.comboModel->addItem(tr("No model selected"));

  for (ModelFactoryStackType::const_iterator pos = m_FactoryStack.begin();
       pos != m_FactoryStack.end(); ++pos)
  {
    this->m_Controls.comboModel->addItem(QString::fromStdString((*pos)->GetClassID()));
  }

  this->m_Controls.comboModel->setCurrentIndex(0);
};

mitk::ModelFitFunctorBase::Pointer GenericDataFittingView::CreateDefaultFitFunctor(
  const mitk::ModelParameterizerBase* parameterizer) const
{
  mitk::LevenbergMarquardtModelFitFunctor::Pointer fitFunctor =
    mitk::LevenbergMarquardtModelFitFunctor::New();

  mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::Pointer chi2 =
    mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::New();
  fitFunctor->RegisterEvaluationParameter("Chi^2", chi2);

  if (m_Controls.checkBox_Constraints->isChecked())
  {
    fitFunctor->SetConstraintChecker(m_modelConstraints);
  }

  mitk::ModelBase::Pointer refModel = parameterizer->GenerateParameterizedModel();

  ::itk::LevenbergMarquardtOptimizer::ScalesType scales;
  scales.SetSize(refModel->GetNumberOfParameters());
  scales.Fill(1.0);
  fitFunctor->SetScales(scales);

  fitFunctor->SetDebugParameterMaps(m_Controls.checkDebug->isChecked());

  return fitFunctor.GetPointer();
}
