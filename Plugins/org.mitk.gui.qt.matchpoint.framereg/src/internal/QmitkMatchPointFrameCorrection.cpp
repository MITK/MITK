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

#include "org_mitk_gui_qt_matchpoint_framereg_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionProvider.h>
#include <berryQModelIndexObject.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkPointSet.h>
#include <mitkImageTimeSelector.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkMAPAlgorithmInfoSelection.h>
#include <mitkRegistrationHelper.h>
#include <mitkResultNodeGenerationHelper.h>

// Qmitk
#include "QmitkMatchPointFrameCorrection.h"
#include <QmitkRegistrationJob.h>
#include <QmitkMappingJob.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QErrorMessage>
#include <QThreadPool>
#include <QDateTime>

// MatchPoint
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapPointSetRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmInterface.h>
#include <mapMaskedRegistrationAlgorithmInterface.h>
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapConvert.h>
#include <mapDeploymentDLLAccess.h>

const std::string QmitkMatchPointFrameCorrection::VIEW_ID =
  "org.mitk.views.matchpoint.algorithm.framereg";

QmitkMatchPointFrameCorrection::QmitkMatchPointFrameCorrection()
  : m_Parent(NULL), m_LoadedDLLHandle(NULL), m_LoadedAlgorithm(NULL), m_CanLoadAlgorithm(false),
    m_ValidInputs(false), m_Working(false)
{
  m_spSelectedTargetData = NULL;
  m_spSelectedTargetMaskData = NULL;
}

QmitkMatchPointFrameCorrection::~QmitkMatchPointFrameCorrection()
{
  // remove selection service
  berry::ISelectionService* s = this->GetSite()->GetWorkbenchWindow()->GetSelectionService();

  if (s)
  {
    s->RemoveSelectionListener(m_AlgorithmSelectionListener.data());
  }
}

void QmitkMatchPointFrameCorrection::SetFocus()
{
}

void QmitkMatchPointFrameCorrection::CreateConnections()
{

  connect(m_Controls.checkTargetMask, SIGNAL(toggled(bool)), this,
          SLOT(OnMaskCheckBoxToggeled(bool)));

  // ------
  // Tab 1 - Shared library loading interface
  // ------

  connect(m_Controls.m_pbLoadSelected, SIGNAL(clicked()), this, SLOT(OnLoadAlgorithmButtonPushed()));

  // -----
  // Tab 2 - Execution
  // -----
  connect(m_Controls.m_pbStartReg, SIGNAL(clicked()), this, SLOT(OnStartRegBtnPushed()));
  connect(m_Controls.m_pbSaveLog, SIGNAL(clicked()), this, SLOT(OnSaveLogBtnPushed()));

  // -----
  // Tab 4 - Frames
  // -----
  connect(m_Controls.m_btnFrameSelAll, SIGNAL(clicked()), this, SLOT(OnFramesSelectAllPushed()));
  connect(m_Controls.m_btnFrameDeSelAll, SIGNAL(clicked()), this, SLOT(OnFramesDeSelectAllPushed()));
  connect(m_Controls.m_btnFrameInvert, SIGNAL(clicked()), this, SLOT(OnFramesInvertPushed()));

}

const map::deployment::DLLInfo* QmitkMatchPointFrameCorrection::GetSelectedAlgorithmDLL() const
{
  return m_SelectedAlgorithmInfo;
}

void QmitkMatchPointFrameCorrection::OnMaskCheckBoxToggeled(bool checked)
{
  if (!m_Working)
  {
    CheckInputs();
    ConfigureRegistrationControls();
  }
};

void QmitkMatchPointFrameCorrection::OnSelectedAlgorithmChanged()
{
  std::stringstream descriptionString;

  ::map::deployment::DLLInfo::ConstPointer currentItemInfo = GetSelectedAlgorithmDLL();

  if (!currentItemInfo)
  {
    return;
  }

  m_Controls.m_teAlgorithmDetails->updateInfo(currentItemInfo);

  m_Controls.m_lbSelectedAlgorithm->setText(QString::fromStdString(
        currentItemInfo->getAlgorithmUID().getName()));

  // enable loading
  m_CanLoadAlgorithm = true;
  this->AdaptFolderGUIElements();
}

void QmitkMatchPointFrameCorrection::OnLoadAlgorithmButtonPushed()
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
  m_Controls.checkTargetMask->setChecked(false);

  this->AdaptFolderGUIElements();
  this->CheckInputs();
  this->ConfigureRegistrationControls();
  this->ConfigureProgressInfos();
  this->m_Controls.m_tabs->setCurrentIndex(1);
}

void QmitkMatchPointFrameCorrection::Error(QString msg)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
  MITK_ERROR << msg.toStdString().c_str();

  m_Controls.m_teLog->append(QString("<font color='red'><b>") + msg + QString("</b></font>"));
}

void QmitkMatchPointFrameCorrection::AdaptFolderGUIElements()
{
  m_Controls.m_pbLoadSelected->setEnabled(m_CanLoadAlgorithm);
}

void QmitkMatchPointFrameCorrection::CreateQtPartControl(QWidget* parent)
{

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Parent = parent;

  m_Controls.checkTargetMask->setChecked(false);
  m_Controls.m_tabs->setCurrentIndex(0);

  m_Controls.m_mapperSettings->AllowSampling(false);

  m_AlgorithmSelectionListener.reset(new
                                     berry::SelectionChangedAdapter<QmitkMatchPointFrameCorrection>(this,
                                         &QmitkMatchPointFrameCorrection::OnAlgorithmSelectionChanged));

  // register selection listener
  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(
    m_AlgorithmSelectionListener.data());

  this->CreateConnections();
  this->AdaptFolderGUIElements();
  this->CheckInputs();
  this->ConfigureProgressInfos();
  this->ConfigureRegistrationControls();

  berry::ISelection::ConstPointer selection =
    GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.matchpoint.algorithm.browser");

  this->UpdateAlgorithmSelection(selection);
}

bool QmitkMatchPointFrameCorrection::CheckInputs()
{
  bool validTarget = false;

  bool validTargetMask = false;

  mitk::DataNode::Pointer oldTargetNode = m_spSelectedTargetNode;

  mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);

  mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

  if (m_LoadedAlgorithm.IsNull())
  {
    m_Controls.m_lbLoadedAlgorithmName->setText(
      QString("<font color='red'>No algorithm seleted!</font>"));
    m_spSelectedTargetNode = NULL;
    m_spSelectedTargetData = NULL;

    m_spSelectedTargetMaskNode = NULL;
    m_spSelectedTargetMaskData = NULL;
  }
  else
  {
    QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

    mitk::Image* targetImage = NULL;
    mitk::PointSet* targetPointSet = NULL;

    mitk::Image* targetMaskImage = NULL;

    typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3> MaskRegInterface;

    MaskRegInterface* pMaskInterface = dynamic_cast<MaskRegInterface*>(m_LoadedAlgorithm.GetPointer());

    if (nodes.count() < 1)
    {
      m_Controls.m_lbTargetName->setText(QString("<font color='red'>no data selected!</font>"));
      m_spSelectedTargetNode = NULL;
      m_spSelectedTargetData = NULL;
    }
    else
    {
      m_spSelectedTargetNode = nodes.front();
      m_spSelectedTargetData = m_spSelectedTargetNode->GetData();
      targetImage = dynamic_cast<mitk::Image*>(m_spSelectedTargetNode->GetData());

      if (targetImage)
      {
        if (targetImage->GetTimeSteps() < 1)
        {
          m_Controls.m_lbTargetName->setText(QString("<font color='red'>Image has no mulitple time steps"));
        }
        else if (targetImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions() + 1)
        {
          m_Controls.m_lbTargetName->setText(QString("<font color='red'>wrong image dimension. ") +
                                             QString::number(m_LoadedAlgorithm->getTargetDimensions()) + QString("D+t needed</font>"));
        }
        else
        {
          validTarget = true;
        }
      }
      else
      {
        m_Controls.m_lbTargetName->setText(QString("<font color='red'>no supported data selected!</font>"));
      }

      nodes.removeFirst();
    }

    if (m_Controls.checkTargetMask->isChecked())
    {
      if (nodes.count() < 1)
      {
        m_Controls.m_lbTargetMaskName->setText(QString("<font color='red'>no data selected!</font>"));
        m_spSelectedTargetMaskNode = NULL;
        m_spSelectedTargetMaskData = NULL;
      }
      else
      {
        m_spSelectedTargetMaskNode = nodes.front();
        m_spSelectedTargetMaskData = NULL;
        targetMaskImage = dynamic_cast<mitk::Image*>(m_spSelectedTargetMaskNode->GetData());

        bool isMask = maskPredicate->CheckNode(m_spSelectedTargetMaskNode);

        if (!isMask)
        {
          m_Controls.m_lbTargetMaskName->setText(QString("<font color='red'>no mask selected!</font>"));
        }
        else if (targetMaskImage && pMaskInterface)
        {
          m_spSelectedTargetMaskData = targetMaskImage;
          validTargetMask = true;
        }
        else
        {
          m_Controls.m_lbTargetMaskName->setText(
            QString("<font color='red'>no supported data selected!</font>"));
        }
      }

    }
    else
    {
      m_Controls.m_lbTargetMaskName->setText(QString("mask deactivated"));
      validTargetMask = true;
      m_spSelectedTargetMaskNode = NULL;
      m_spSelectedTargetMaskData = NULL;
    }

  }

  if (validTarget)
  {
    m_Controls.m_lbTargetName->setText(QString::fromStdString(GetInputNodeDisplayName(
                                         m_spSelectedTargetNode)));

    if (oldTargetNode != m_spSelectedTargetNode)
    {
      ConfigureFrameList();
    }
  }
  else
  {
    m_Controls.m_listFrames->clear();
  }

  if (validTargetMask && m_Controls.checkTargetMask->isChecked())
  {
    m_Controls.m_lbTargetMaskName->setText(QString::fromStdString(GetInputNodeDisplayName(
        m_spSelectedTargetMaskNode)));
  }

  m_ValidInputs = validTarget && validTargetMask;
  return m_ValidInputs;
}

std::string QmitkMatchPointFrameCorrection::GetInputNodeDisplayName(const mitk::DataNode* node)
const
{
  std::string result = "UNDEFINED/NULL";

  if (node)
  {
    result = node->GetName();

    const mitk::PointSet* pointSet = dynamic_cast<const mitk::PointSet*>(node->GetData());

    if (pointSet)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer sources = this->GetDataStorage()->GetSources(node);

      if (sources.IsNotNull() && sources->Size() > 0)
      {
        result = result + " (" + sources->GetElement(0)->GetName() + ")";
      }

    }
  }

  return result;
}

mitk::DataStorage::SetOfObjects::Pointer QmitkMatchPointFrameCorrection::GetRegNodes() const
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

std::string QmitkMatchPointFrameCorrection::GetDefaultJobName() const
{

  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetRegNodes().GetPointer();
  mitk::DataStorage::SetOfObjects::ElementIdentifier newIndex = 0;

  bool isUnique = false;

  std::string baseName = "corrected #";

  if (m_spSelectedTargetNode.IsNotNull())
  {
    baseName = m_spSelectedTargetNode->GetName() + "corrected #";
  }

  std::string result = baseName;

  while (!isUnique)
  {
    ++newIndex;
    result = baseName + ::map::core::convert::toStr(newIndex);
    isUnique =  this->GetDataStorage()->GetNamedNode(result) == NULL;
  }

  return result;
}

void QmitkMatchPointFrameCorrection::ConfigureRegistrationControls()
{
  m_Controls.m_tabSelection->setEnabled(!m_Working);
  m_Controls.m_leRegJobName->setEnabled(!m_Working);
  m_Controls.groupMasks->setEnabled(!m_Working);

  m_Controls.m_pbStartReg->setEnabled(false);

  m_Controls.m_lbTargetMaskName->setVisible(m_Controls.checkTargetMask->isChecked());

  if (m_LoadedAlgorithm.IsNotNull())
  {
    m_Controls.m_tabSettings->setEnabled(!m_Working);
    m_Controls.m_tabExclusion->setEnabled(!m_Working);
    m_Controls.m_tabExecution->setEnabled(true);
    m_Controls.m_pbStartReg->setEnabled(m_ValidInputs && !m_Working);
    m_Controls.m_leRegJobName->setEnabled(!m_Working);

    typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3> MaskRegInterface;
    const MaskRegInterface* pMaskReg = dynamic_cast<const MaskRegInterface*>
                                       (m_LoadedAlgorithm.GetPointer());

    m_Controls.groupMasks->setVisible(pMaskReg != NULL);

    this->m_Controls.m_lbLoadedAlgorithmName->setText(
      QString::fromStdString(m_LoadedAlgorithm->getUID()->toStr()));
  }
  else
  {
    m_Controls.m_tabSettings->setEnabled(false);
    m_Controls.m_tabExclusion->setEnabled(false);
    m_Controls.m_tabExecution->setEnabled(false);
    this->m_Controls.m_lbLoadedAlgorithmName->setText(
      QString("<font color='red'>no algorithm loaded!</font>"));
    m_Controls.groupMasks->setVisible(false);
  }

  if (!m_Working)
  {
    this->m_Controls.m_leRegJobName->setText(QString::fromStdString(this->GetDefaultJobName()));
  }
}

void QmitkMatchPointFrameCorrection::ConfigureProgressInfos()
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

  m_Controls.m_progBarIteration->reset();
  m_Controls.m_progBarLevel->reset();
  m_Controls.m_progBarFrame->reset();
}

void QmitkMatchPointFrameCorrection::ConfigureFrameList()
{
  m_Controls.m_listFrames->clear();

  if (m_spSelectedTargetData.IsNotNull())
  {
    mitk::TimeGeometry::ConstPointer tg = m_spSelectedTargetData->GetTimeGeometry();

    for (unsigned int i = 1; i < tg->CountTimeSteps(); ++i)
    {
      QString lable = "Timepoint #" + QString::number(i) + QString(" (") + QString::number(
                        tg->GetMinimumTimePoint(i)) + QString(" ms - " + QString::number(tg->GetMaximumTimePoint(
                              i)) + QString(" ms)"));
      QListWidgetItem* item = new QListWidgetItem(lable, m_Controls.m_listFrames);
      item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      item->setCheckState(Qt::Checked);
    }
  }
}

void QmitkMatchPointFrameCorrection::OnFramesSelectAllPushed()
{
  for (unsigned int row = 0; row < m_Controls.m_listFrames->count(); row++)
  {
    QListWidgetItem* item = m_Controls.m_listFrames->item(row);
    item->setCheckState(Qt::Checked);
  }
};

void QmitkMatchPointFrameCorrection::OnFramesDeSelectAllPushed()
{
  for (unsigned int row = 0; row < m_Controls.m_listFrames->count(); row++)
  {
    QListWidgetItem* item = m_Controls.m_listFrames->item(row);
    item->setCheckState(Qt::Unchecked);
  }
};

void QmitkMatchPointFrameCorrection::OnFramesInvertPushed()
{
  for (unsigned int row = 0; row < m_Controls.m_listFrames->count(); row++)
  {
    QListWidgetItem* item = m_Controls.m_listFrames->item(row);

    if (item->checkState() == Qt::Unchecked)
    {
      item->setCheckState(Qt::Checked);
    }
    else
    {
      item->setCheckState(Qt::Unchecked);
    }
  }
};

mitk::TimeFramesRegistrationHelper::IgnoreListType
QmitkMatchPointFrameCorrection::GenerateIgnoreList() const
{
  mitk::TimeFramesRegistrationHelper::IgnoreListType result;

  for (unsigned int row = 0; row < m_Controls.m_listFrames->count(); row++)
  {
    QListWidgetItem* item = m_Controls.m_listFrames->item(row);

    if (item->checkState() == Qt::Unchecked)
    {
      result.push_back(row + 1);
    }
  }

  return result;
}

void QmitkMatchPointFrameCorrection::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes)
{
  if (!m_Working)
  {
    CheckInputs();
    ConfigureRegistrationControls();
  }
}

void QmitkMatchPointFrameCorrection::OnStartRegBtnPushed()
{
  this->m_Working = true;

  ////////////////////////////////
  //configure GUI
  this->ConfigureProgressInfos();

  m_Controls.m_progBarIteration->reset();
  m_Controls.m_progBarLevel->reset();

  this->ConfigureRegistrationControls();

  if (m_Controls.m_checkClearLog->checkState() == Qt::Checked)
  {
    this->m_Controls.m_teLog->clear();
  }


  /////////////////////////
  //create job and put it into the thread pool
  QmitkFramesRegistrationJob* pJob = new QmitkFramesRegistrationJob(m_LoadedAlgorithm);
  pJob->setAutoDelete(true);

  pJob->m_spTargetData = m_spSelectedTargetData;
  pJob->m_TargetNodeUID = mitk::EnsureUID(this->m_spSelectedTargetNode);
  pJob->m_IgnoreList = this->GenerateIgnoreList();

  if (m_spSelectedTargetMaskData.IsNotNull())
  {
    pJob->m_spTargetMask = m_spSelectedTargetMaskData;
    pJob->m_TargetMaskNodeUID = mitk::EnsureUID(this->m_spSelectedTargetMaskNode);
  }

  pJob->m_MappedName = m_Controls.m_leRegJobName->text().toStdString();

  m_Controls.m_mapperSettings->ConfigureJobSettings(pJob);

  connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnRegJobError(QString)));
  connect(pJob, SIGNAL(Finished()), this, SLOT(OnRegJobFinished()));
  connect(pJob, SIGNAL(ResultIsAvailable(mitk::Image::Pointer, const QmitkFramesRegistrationJob*)),
          this, SLOT(OnMapResultIsAvailable(mitk::Image::Pointer, const QmitkFramesRegistrationJob*)),
          Qt::BlockingQueuedConnection);

  connect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));
  connect(pJob, SIGNAL(AlgorithmStatusChanged(QString)), this,
          SLOT(OnAlgorithmStatusChanged(QString)));
  connect(pJob, SIGNAL(AlgorithmIterated(QString, bool, unsigned long)), this,
          SLOT(OnAlgorithmIterated(QString, bool, unsigned long)));
  connect(pJob, SIGNAL(LevelChanged(QString, bool, unsigned long)), this, SLOT(OnLevelChanged(QString,
          bool, unsigned long)));
  connect(pJob, SIGNAL(FrameRegistered(double)), this, SLOT(OnFrameRegistered(double)));
  connect(pJob, SIGNAL(FrameMapped(double)), this, SLOT(OnFrameMapped(double)));
  connect(pJob, SIGNAL(FrameProcessed(double)), this, SLOT(OnFrameProcessed(double)));

  QThreadPool* threadPool = QThreadPool::globalInstance();
  threadPool->start(pJob);
}

void QmitkMatchPointFrameCorrection::OnSaveLogBtnPushed()
{
  QDateTime currentTime = QDateTime::currentDateTime();
  QString fileName = tr("registration_log_") + currentTime.toString(tr("yyyy-MM-dd_hh-mm-ss")) +
                     tr(".txt");
  fileName = QFileDialog::getSaveFileName(NULL, tr("Save registration log"), fileName,
                                          tr("Text files (*.txt)"));

  if (fileName.isEmpty())
  {
    QMessageBox::critical(NULL, tr("No file selected!"),
                          tr("Cannot save registration log file. Please selected a file."));
  }
  else
  {
    std::ofstream file;

    std::ios_base::openmode iOpenFlag = std::ios_base::out | std::ios_base::trunc;
    file.open(fileName.toStdString().c_str(), iOpenFlag);

    if (!file.is_open())
    {
      mitkThrow() << "Cannot open or create specified file to save. File path: "
                  << fileName.toStdString();
    }

    file << this->m_Controls.m_teLog->toPlainText().toStdString() << std::endl;

    file.close();
  }
}

void QmitkMatchPointFrameCorrection::OnRegJobError(QString err)
{
  Error(err);
};

void QmitkMatchPointFrameCorrection::OnRegJobFinished()
{
  this->m_Working = false;

  this->GetRenderWindowPart()->RequestUpdate();

  this->CheckInputs();
  this->ConfigureRegistrationControls();
  this->ConfigureProgressInfos();
};


void QmitkMatchPointFrameCorrection::OnMapResultIsAvailable(mitk::Image::Pointer spMappedData,
    const QmitkFramesRegistrationJob* job)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>Corrected image stored. Name: ") +
                             QString::fromStdString(job->m_MappedName) + QString("</font></b>"));

  mitk::DataNode::Pointer spResultNode = mitk::generateMappedResultNode(job->m_MappedName,
                                         spMappedData.GetPointer(), "", job->m_TargetNodeUID, false, job->m_InterpolatorLabel);

  this->GetDataStorage()->Add(spResultNode, this->m_spSelectedTargetNode);
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointFrameCorrection::OnMapJobError(QString err)
{
  Error(err);
};

void QmitkMatchPointFrameCorrection::OnAlgorithmIterated(QString info, bool hasIterationCount,
    unsigned long currentIteration)
{
  if (hasIterationCount)
  {
    m_Controls.m_progBarIteration->setValue(currentIteration);
  }

  m_Controls.m_teLog->append(info);
};

void QmitkMatchPointFrameCorrection::OnLevelChanged(QString info, bool hasLevelCount,
    unsigned long currentLevel)
{
  if (hasLevelCount)
  {
    m_Controls.m_progBarLevel->setValue(currentLevel);
  }

  m_Controls.m_teLog->append(QString("<b><font color='green'>") + info + QString("</font></b>"));
};

void QmitkMatchPointFrameCorrection::OnAlgorithmStatusChanged(QString info)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>") + info + QString(" </font></b>"));
};

void QmitkMatchPointFrameCorrection::OnAlgorithmInfo(QString info)
{
  m_Controls.m_teLog->append(QString("<font color='gray'><i>") + info + QString("</i></font>"));
};

void QmitkMatchPointFrameCorrection::OnFrameProcessed(double progress)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>Frame processed...</font></b>"));
  m_Controls.m_progBarFrame->setValue(100 * progress);
};

void QmitkMatchPointFrameCorrection::OnFrameRegistered(double progress)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>Frame registered...</font></b>"));
  m_Controls.m_progBarFrame->setValue(100 * progress);
};

void QmitkMatchPointFrameCorrection::OnFrameMapped(double progress)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>Frame mapped...</font></b>"));
  m_Controls.m_progBarFrame->setValue(100 * progress);
};

void QmitkMatchPointFrameCorrection::OnAlgorithmSelectionChanged(const
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

void QmitkMatchPointFrameCorrection::UpdateAlgorithmSelection(berry::ISelection::ConstPointer
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
