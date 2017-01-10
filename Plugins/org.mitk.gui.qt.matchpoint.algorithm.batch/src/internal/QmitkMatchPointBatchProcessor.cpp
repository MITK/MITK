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

#include "org_mitk_gui_qt_matchpoint_algorithm_batch_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionProvider.h>
#include <berryQModelIndexObject.h>

// Mitk
#include <mitkImageAccessByItk.h>
#include <mitkStatusBar.h>
#include <mitkMAPAlgorithmInfoSelection.h>
#include <QmitkRegistrationJob.h>
#include <QmitkMappingJob.h>
#include <mitkRegistrationHelper.h>
#include <mitkResultNodeGenerationHelper.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QErrorMessage>
#include <QThreadPool>

// MatchPoint
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmInterface.h>
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapConvert.h>
#include <mapDeploymentDLLAccess.h>

// Qmitk
#include "QmitkMatchPointBatchProcessor.h"


const std::string QmitkMatchPointBatchProcessor::VIEW_ID =
  "org.mitk.views.matchpoint.algorithm.batchprocessing";

QmitkMatchPointBatchProcessor::QmitkMatchPointBatchProcessor()
  : m_Parent(NULL), m_LoadedDLLHandle(NULL), m_LoadedAlgorithm(NULL)
{
  m_CanLoadAlgorithm = false;
  m_ValidInputs = false;
  m_Working = false;
}

QmitkMatchPointBatchProcessor::~QmitkMatchPointBatchProcessor()
{
  // remove selection service
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();

  if (s)
  {
    s->RemoveSelectionListener(m_AlgorithmSelectionListener.data());
  }
}

void QmitkMatchPointBatchProcessor::SetFocus()
{
}

void QmitkMatchPointBatchProcessor::CreateConnections()
{
  // show first page
  m_Controls.m_tabs->setCurrentIndex(0);

  // ------
  // Tab 1 - Shared library loading interface
  // ------

  connect(m_Controls.m_pbLoadSelected, SIGNAL(clicked()), this, SLOT(OnLoadAlgorithmButtonPushed()));

  // -----
  // Tab 2 - Input
  // -----
  connect(m_Controls.m_pbLockTarget, SIGNAL(clicked()), this, SLOT(OnLockTargetButtonPushed()));
  connect(m_Controls.m_pbLockMoving, SIGNAL(clicked()), this, SLOT(OnLockMovingButtonPushed()));

  // -----
  // Tab 3 - Execution
  // -----
  connect(m_Controls.m_pbStartReg, SIGNAL(clicked()), this, SLOT(OnStartRegBtnPushed()));
}

const map::deployment::DLLInfo* QmitkMatchPointBatchProcessor::GetSelectedAlgorithmDLL() const
{
  return m_SelectedAlgorithmInfo;
}

void QmitkMatchPointBatchProcessor::OnSelectedAlgorithmChanged()
{
  std::stringstream descriptionString;

  ::map::deployment::DLLInfo::ConstPointer currentItemInfo = GetSelectedAlgorithmDLL();

  if (!currentItemInfo)
  {
    Error(QString("No valid algorithm is selected. ABORTING."));
    return;
  }

  m_Controls.m_teAlgorithmDetails->updateInfo(currentItemInfo);

  m_Controls.m_lbSelectedAlgorithm->setText(QString::fromStdString(
        currentItemInfo->getAlgorithmUID().getName()));

  // enable loading
  m_CanLoadAlgorithm = true;
  this->UpdateAlgorithmSelectionGUI();
}

void QmitkMatchPointBatchProcessor::OnLoadAlgorithmButtonPushed()
{
  map::deployment::DLLInfo::ConstPointer dllInfo = GetSelectedAlgorithmDLL();

  if (!dllInfo)
  {
    Error(QString("No valid algorithm is selected. Cannot load algorithm. ABORTING."));
    return;
  }

  ::map::deployment::DLLHandle::Pointer tempDLLHandle = ::map::deployment::openDeploymentDLL(
        dllInfo->getLibraryFilePath());
  ::map::algorithm::RegistrationAlgorithmBase::Pointer tempAlgorithm
    = ::map::deployment::getRegistrationAlgorithm(tempDLLHandle);

  if (tempAlgorithm.IsNull())
  {
    Error(QString("Error. Cannot load selected algorithm."));
    return;
  }

  this->m_LoadedAlgorithm = tempAlgorithm;
  this->m_LoadedDLLHandle = tempDLLHandle;

  this->m_Controls.m_AlgoConfigurator->setAlgorithm(m_LoadedAlgorithm);

  this->CheckInputs();
  this->ConfigureRegistrationControls();
  this->ConfigureProgressInfos();
  this->m_Controls.m_tabs->setCurrentIndex(1);
  this->UpdateAlgorithmSelectionGUI();
}

void QmitkMatchPointBatchProcessor::OnLockTargetButtonPushed()
{
  if (this->m_Controls.m_pbLockTarget->isChecked())
  {
    if (this->m_spSelectedTargetNode.IsNotNull())
    {
      this->m_spSelectedTargetNode->SetSelected(false);
      this->GetDataStorage()->Modified();
    }
  }

  this->CheckInputs();
  this->ConfigureRegistrationControls();
}

void QmitkMatchPointBatchProcessor::OnLockMovingButtonPushed()
{
  if (this->m_Controls.m_pbLockMoving->isChecked())
  {
    if (!this->m_selectedMovingNodes.empty())
    {
      for (NodeListType::const_iterator pos = m_selectedMovingNodes.begin();
           pos != m_selectedMovingNodes.end(); ++pos)
      {
        (*pos)->SetSelected(false);
      }

      this->GetDataStorage()->Modified();
    }
  }

  this->CheckInputs();
  this->ConfigureRegistrationControls();
}


void QmitkMatchPointBatchProcessor::Error(QString msg)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
  MITK_ERROR << msg.toStdString().c_str();

  m_Controls.m_teLog->append(QString("<font color='red'><b>") + msg + QString("</b></font>"));
}

void QmitkMatchPointBatchProcessor::UpdateAlgorithmSelectionGUI()
{
  m_Controls.m_pbLoadSelected->setEnabled(m_CanLoadAlgorithm);
}

void QmitkMatchPointBatchProcessor::CreateQtPartControl(QWidget* parent)
{

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Parent = parent;

  m_AlgorithmSelectionListener.reset(new
                                     berry::SelectionChangedAdapter<QmitkMatchPointBatchProcessor>(this,
                                         &QmitkMatchPointBatchProcessor::OnAlgorithmSelectionChanged));

  // register selection listener
  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(
    m_AlgorithmSelectionListener.data());

  this->CreateConnections();
  this->UpdateAlgorithmSelectionGUI();
  this->CheckInputs();
  this->ConfigureProgressInfos();
  this->ConfigureRegistrationControls();

  berry::ISelection::ConstPointer selection =
    GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.matchpoint.algorithm.browser");

  this->UpdateAlgorithmSelection(selection);
}

void QmitkMatchPointBatchProcessor::CheckInputs()
{
  bool validMoving = false;
  bool validTarget = false;

  NodeListType dataNodes = this->GetSelectedDataNodes();


  //first set the internal nodes according to selection
  if (!m_Controls.m_pbLockTarget->isChecked())
  {
    mitk::DataNode::Pointer targetNode = NULL;

    if (dataNodes.size() > 0)
    {
      targetNode = dataNodes.front();
      dataNodes.pop_front();

      mitk::Image* targetImage = dynamic_cast<mitk::Image*>(targetNode->GetData());

      if (targetImage)
      {
        if (m_LoadedAlgorithm.IsNotNull())
        {
          if (targetImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions())
          {
            m_Controls.m_lbTargetName->setText(QString("<font color='red'>wrong image dimension. ") +
                                               QString::number(m_LoadedAlgorithm->getTargetDimensions()) + QString("D needed</font>"));
          }
          else
          {
            validTarget = true;
          }
        }
        else
        {
          validTarget = true;
        }

        if (validTarget)
        {
          m_Controls.m_lbTargetName->setText(QString::fromStdString(targetNode->GetName()));
        }
      }
    }

    this->m_spSelectedTargetNode = targetNode;
  }
  else
  {
    validTarget = true;
  }

  if (this->m_spSelectedTargetNode.IsNull())
  {
    m_Controls.m_lbTargetName->setText(QString("<font color='red'>no data selected!</font>"));
  }

  if (!m_Controls.m_pbLockMoving->isChecked())
  {
    m_selectedMovingNodes.clear();
    this->m_Controls.m_listMovingNames->clear();

    if (dataNodes.size() > 0)
    {
      for (NodeListType::const_iterator pos = dataNodes.begin(); pos != dataNodes.end(); ++pos)
      {
        this->m_Controls.m_listMovingNames->addItem(QString::fromStdString((*pos)->GetName()));
      }
    }

    m_selectedMovingNodes = dataNodes;
    validMoving = !m_selectedMovingNodes.empty();
  }
  else
  {
    validMoving = true;
  }

  if (this->m_selectedMovingNodes.size() == 0)
  {
    this->m_Controls.m_listMovingNames->addItem(QString("no data selected!"));
  }

  this->m_Controls.m_pbLockMoving->setEnabled(this->m_selectedMovingNodes.size() > 0);
  this->m_Controls.m_pbLockTarget->setEnabled(this->m_spSelectedTargetNode.IsNotNull());

  m_ValidInputs = validMoving && validTarget;
}

mitk::DataStorage::SetOfObjects::Pointer QmitkMatchPointBatchProcessor::GetRegNodes() const
{

  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetAll();
  mitk::DataStorage::SetOfObjects::Pointer result = mitk::DataStorage::SetOfObjects::New();

  for (mitk::DataStorage::SetOfObjects::const_iterator pos = nodes->begin(); pos != nodes->end();
       ++pos)
  {
    if (mitk::MITKRegistrationHelper::IsRegNode(*pos))
    {
      result->push_back(*pos);
    }
  }

  return result;
}

QList<mitk::DataNode::Pointer> QmitkMatchPointBatchProcessor::GetSelectedDataNodes()
{
  NodeListType nodes = m_currentlySelectedNodes; //this->GetDataManagerSelection();
  NodeListType result;


  /**@TODO rework to use mitk node filter mechanism*/
  for (NodeListType::iterator pos = nodes.begin(); pos != nodes.end(); ++pos)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>((*pos)->GetData());

    if (image)
    {
      result.push_back(*pos);
    }
  }

  return result;
}

std::string QmitkMatchPointBatchProcessor::GetDefaultRegJobName() const
{

  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetRegNodes().GetPointer();
  mitk::DataStorage::SetOfObjects::ElementIdentifier estimatedIndex = nodes->Size();

  bool isUnique = false;
  std::string result = "Unnamed Reg";

  while (!isUnique)
  {
    ++estimatedIndex;
    result = "Reg #" +::map::core::convert::toStr(estimatedIndex);
    isUnique =  this->GetDataStorage()->GetNamedNode(result) == NULL;
  }

  return result;
}

void QmitkMatchPointBatchProcessor::ConfigureRegistrationControls()
{
  m_Controls.m_pageSelection->setEnabled(!m_Working);
  m_Controls.m_leRegJobName->setEnabled(!m_Working);

  m_Controls.m_pbStartReg->setEnabled(false);

  /**@TODO reactivate as soon as crex processor allows to stop batch processing.*/
  //m_Controls.m_pbStopReg->setEnabled(false);
  //m_Controls.m_pbStopReg->setVisible(false);

  if (m_LoadedAlgorithm.IsNotNull())
  {
    m_Controls.m_pageSettings->setEnabled(!m_Working);
    m_Controls.m_pageExecution->setEnabled(true);
    m_Controls.m_pbStartReg->setEnabled(m_ValidInputs && !m_Working);
    /////////////////////////////////////////////

    const IStoppableAlgorithm* pIterativ = dynamic_cast<const IStoppableAlgorithm*>
                                           (m_LoadedAlgorithm.GetPointer());

    /**@TODO reactivate as soon as crex processor allows to stop batch processing.*/
    //if (pIterativ)
    //{
    //    m_Controls.m_pbStopReg->setVisible(pIterativ->isStoppable());
    //}

    ////if the stop button is set to visible and the algorithm is working ->
    ////then the algorithm is stoppable, thus enable the button.
    //m_Controls.m_pbStopReg->setEnabled(m_Controls.m_pbStopReg->isVisible() && m_Working);

    this->m_Controls.m_lbLoadedAlgorithmName->setText(
      QString::fromStdString(m_LoadedAlgorithm->getUID()->toStr()));
  }
  else
  {
    m_Controls.m_pageSettings->setEnabled(false);
    m_Controls.m_pageExecution->setEnabled(false);
    this->m_Controls.m_lbLoadedAlgorithmName->setText(
      QString("<font color='red'>no algorithm loaded!</font>"));
  }

  if (!m_Working)
  {
    this->m_Controls.m_leRegJobName->setText(QString::fromStdString(this->GetDefaultRegJobName()));
  }
}

void QmitkMatchPointBatchProcessor::ConfigureProgressInfos()
{
  const IIterativeAlgorithm* pIterative = dynamic_cast<const IIterativeAlgorithm*>
                                          (m_LoadedAlgorithm.GetPointer());
  const IMultiResAlgorithm* pMultiRes = dynamic_cast<const IMultiResAlgorithm*>
                                        (m_LoadedAlgorithm.GetPointer());

  m_Controls.m_progBarIteration->setVisible(pIterative);
  m_Controls.m_lbProgBarIteration->setVisible(pIterative);


  if (pIterative)
  {
    QString format = "%p% (%v/%m)";

    if (!pIterative->hasMaxIterationCount())
    {
      format = "%v";
      m_Controls.m_progBarIteration->setMaximum(0);
    }
    else
    {
      m_Controls.m_progBarIteration->setMaximum(pIterative->getMaxIterations());
    }

    m_Controls.m_progBarIteration->setFormat(format);
  }

  if (pMultiRes)
  {
    m_Controls.m_progBarLevel->setMaximum(pMultiRes->getResolutionLevels());

  }
  else
  {
    m_Controls.m_progBarLevel->setMaximum(1);
  }

  m_Controls.m_progBarJob->setMaximum(this->m_selectedMovingNodes.size());
  m_Controls.m_progBarJob->reset();

  m_Controls.m_progBarIteration->reset();
  m_Controls.m_progBarLevel->reset();
}

void QmitkMatchPointBatchProcessor::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes)
{
  m_currentlySelectedNodes = nodes;

  if (!m_Working)
  {
    CheckInputs();
    ConfigureRegistrationControls();
  }
}

void QmitkMatchPointBatchProcessor::OnStartRegBtnPushed()
{
  this->m_Working = true;

  ////////////////////////////////
  //configure GUI
  this->ConfigureProgressInfos();

  this->ConfigureRegistrationControls();

  if (m_Controls.m_checkClearLog->checkState() == Qt::Checked)
  {
    this->m_Controls.m_teLog->clear();
  }

  this->m_nextNodeToSpawn = 0;

  SpawnNextJob();
}

bool QmitkMatchPointBatchProcessor::SpawnNextJob()
{
  if (this->m_nextNodeToSpawn < this->m_selectedMovingNodes.size())
  {
    QmitkRegistrationJob* pJob = new QmitkRegistrationJob(m_LoadedAlgorithm);
    pJob->setAutoDelete(true);

    pJob->m_spTargetData = this->m_spSelectedTargetNode->GetData();
    pJob->m_spMovingData = this->m_selectedMovingNodes[m_nextNodeToSpawn]->GetData();
    pJob->m_TargetDataUID = mitk::EnsureUID(this->m_spSelectedTargetNode->GetData());
    pJob->m_MovingDataUID = mitk::EnsureUID(this->m_selectedMovingNodes[m_nextNodeToSpawn]->GetData());

    QString jobName = m_Controls.m_leRegJobName->text() + QString(" ") + QString::fromStdString(
                        this->m_selectedMovingNodes[m_nextNodeToSpawn]->GetName());
    pJob->m_JobName = jobName.toStdString();

    pJob->m_StoreReg = m_Controls.m_checkStoreReg->checkState() == Qt::Checked;

    connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnRegJobError(QString)));
    connect(pJob, SIGNAL(Finished()), this, SLOT(OnRegJobFinished()));
    connect(pJob, SIGNAL(RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer,
                         const QmitkRegistrationJob*)), this,
            SLOT(OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)),
            Qt::BlockingQueuedConnection);
    connect(pJob, SIGNAL(MapResultNodeIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
            this, SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
            Qt::BlockingQueuedConnection);

    connect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));
    connect(pJob, SIGNAL(AlgorithmStatusChanged(QString)), this,
            SLOT(OnAlgorithmStatusChanged(QString)));
    connect(pJob, SIGNAL(AlgorithmIterated(QString, bool, unsigned long)), this,
            SLOT(OnAlgorithmIterated(QString, bool, unsigned long)));
    connect(pJob, SIGNAL(LevelChanged(QString, bool, unsigned long)), this, SLOT(OnLevelChanged(QString,
            bool, unsigned long)));

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pJob);

    this->m_nextNodeToSpawn++;
    return true;
  }

  return false;
}

void QmitkMatchPointBatchProcessor::OnRegJobFinished()
{
  if (SpawnNextJob())
  {
    m_Controls.m_teLog->append(
      QString("<p/><b><font color='purple'>Commencing new registration job...</font></b><p/>"));
    m_Controls.m_progBarJob->setValue(this->m_nextNodeToSpawn);
  }
  else
  {
    this->m_Working = false;

    this->GetRenderWindowPart()->RequestUpdate();

    this->CheckInputs();
    this->ConfigureRegistrationControls();
    this->ConfigureProgressInfos();
  }
};


void QmitkMatchPointBatchProcessor::OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer
    spResultRegistration, const QmitkRegistrationJob* pRegJob)
{
  mitk::DataNode::Pointer spResultRegistrationNode = mitk::generateRegistrationResultNode(
        pRegJob->m_JobName, spResultRegistration, pRegJob->GetLoadedAlgorithm()->getUID()->toStr(),
        pRegJob->m_MovingDataUID, pRegJob->m_TargetDataUID);

  if (pRegJob->m_StoreReg)
  {
    m_Controls.m_teLog->append(
      QString("<b><font color='blue'> Storing registration object in data manager ... </font></b>"));

    this->GetDataStorage()->Add(spResultRegistrationNode);
    this->GetRenderWindowPart()->RequestUpdate();
  }

  if (m_Controls.m_checkMapEntity->checkState() == Qt::Checked)
  {
    QmitkMappingJob* pMapJob = new QmitkMappingJob();
    pMapJob->setAutoDelete(true);

    pMapJob->m_spInputData = pRegJob->m_spMovingData;
    pMapJob->m_InputDataUID = pRegJob->m_MovingDataUID;
    pMapJob->m_spRegNode = spResultRegistrationNode;
    pMapJob->m_doGeometryRefinement = false;
    pMapJob->m_spRefGeometry = pRegJob->m_spTargetData->GetGeometry()->Clone().GetPointer();

    pMapJob->m_MappedName = pRegJob->m_JobName + std::string(" mapped");
    pMapJob->m_allowUndefPixels = true;
    pMapJob->m_paddingValue = 100;
    pMapJob->m_allowUnregPixels = true;
    pMapJob->m_errorValue = 200;
    pMapJob->m_InterpolatorLabel = "Linear Interpolation";
    pMapJob->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;

    connect(pMapJob, SIGNAL(Error(QString)), this, SLOT(OnMapJobError(QString)));
    connect(pMapJob, SIGNAL(MapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
            this, SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
            Qt::BlockingQueuedConnection);
    connect(pMapJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));

    m_Controls.m_teLog->append(
      QString("<b><font color='blue'>Started mapping input data...</font></b>"));

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pMapJob);
  }
};

void QmitkMatchPointBatchProcessor::OnMapJobError(QString err)
{
  Error(err);
};

void QmitkMatchPointBatchProcessor::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
    const QmitkMappingJob* job)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>Mapped entity stored. Name: ") +
                             QString::fromStdString(job->m_MappedName) + QString("</font></b>"));

  mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
                                         spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID,
                                         job->m_doGeometryRefinement, job->m_InterpolatorLabel);
  this->GetDataStorage()->Add(spMappedNode);
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointBatchProcessor::OnStopRegBtnPushed()
{
  if (m_LoadedAlgorithm.IsNotNull())
  {
    IStoppableAlgorithm* pIterativ = dynamic_cast<IStoppableAlgorithm*>(m_LoadedAlgorithm.GetPointer());

    if (pIterativ && pIterativ->isStoppable())
    {
      if (pIterativ->stopAlgorithm())
      {

      }
      else
      {

      }

      /**@TODO reactivate as soon as crex processor allows to stop batch processing.*/
      //m_Controls.m_pbStopReg->setEnabled(false);
    }
    else
    {
    }
  }
}

void QmitkMatchPointBatchProcessor::OnRegJobError(QString err)
{
  Error(err);
};

void QmitkMatchPointBatchProcessor::OnAlgorithmIterated(QString info, bool hasIterationCount,
    unsigned long currentIteration)
{
  if (hasIterationCount)
  {
    m_Controls.m_progBarIteration->setValue(currentIteration);
  }

  m_Controls.m_teLog->append(info);
};

void QmitkMatchPointBatchProcessor::OnLevelChanged(QString info, bool hasLevelCount,
    unsigned long currentLevel)
{
  if (hasLevelCount)
  {
    m_Controls.m_progBarLevel->setValue(currentLevel);
  }

  m_Controls.m_teLog->append(QString("<b><font color='green'>") + info + QString("</font></b>"));
};

void QmitkMatchPointBatchProcessor::OnAlgorithmStatusChanged(QString info)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>") + info + QString(" </font></b>"));
};

void QmitkMatchPointBatchProcessor::OnAlgorithmInfo(QString info)
{
  m_Controls.m_teLog->append(QString("<font color='gray'><i>") + info + QString("</i></font>"));
};

void QmitkMatchPointBatchProcessor::UpdateAlgorithmSelection(berry::ISelection::ConstPointer
    selection)
{
  mitk::MAPAlgorithmInfoSelection::ConstPointer currentSelection =
    selection.Cast<const mitk::MAPAlgorithmInfoSelection>();

  if (currentSelection)
  {
    mitk::MAPAlgorithmInfoSelection::AlgorithmInfoVectorType infoVector =
      currentSelection->GetSelectedAlgorithmInfo();

    if (!infoVector.empty())
    {
      // only the first selection is of interest, the rest will be skipped.
      this->m_SelectedAlgorithmInfo = infoVector[0];
    }
  }

  this->OnSelectedAlgorithmChanged();
};

void QmitkMatchPointBatchProcessor::OnAlgorithmSelectionChanged(const
    berry::IWorkbenchPart::Pointer& sourcepart,
    const berry::ISelection::ConstPointer& selection)
{
  // check for null selection
  if (selection.IsNull())
  {
    return;
  }

  if (sourcepart != this)
  {
    UpdateAlgorithmSelection(selection);
  }
}
