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

#include "org_mitk_gui_qt_matchpoint_algorithmcontrol_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionProvider.h>
#include <berryQModelIndexObject.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkPointSet.h>
#include <mitkImageTimeSelector.h>
#include <mitkMAPAlgorithmInfoSelection.h>
#include <mitkRegistrationHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// Qmitk
#include "QmitkMatchPoint.h"
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

const std::string QmitkMatchPoint::VIEW_ID = "org.mitk.views.matchpoint.algorithm.control";

QmitkMatchPoint::QmitkMatchPoint()
  : m_Parent(NULL), m_LoadedDLLHandle(NULL), m_LoadedAlgorithm(NULL)
{
  m_CanLoadAlgorithm = false;
  m_ValidInputs = false;
  m_Working = false;
  m_spSelectedTargetData = NULL;
  m_spSelectedMovingData = NULL;
  m_spSelectedTargetMaskData = NULL;
  m_spSelectedMovingMaskData = NULL;
}

QmitkMatchPoint::~QmitkMatchPoint()
{
  // remove selection service
  berry::ISelectionService* s = this->GetSite()->GetWorkbenchWindow()->GetSelectionService();

  if (s)
  {
    s->RemoveSelectionListener(m_AlgorithmSelectionListener.data());
  }
}

void QmitkMatchPoint::SetFocus()
{
}

void QmitkMatchPoint::CreateConnections()
{

  connect(m_Controls.checkMovingMask, SIGNAL(toggled(bool)), this,
          SLOT(OnMaskCheckBoxToggeled(bool)));
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
  connect(m_Controls.m_pbStopReg, SIGNAL(clicked()), this, SLOT(OnStopRegBtnPushed()));
  connect(m_Controls.m_pbSaveLog, SIGNAL(clicked()), this, SLOT(OnSaveLogBtnPushed()));
}

const map::deployment::DLLInfo* QmitkMatchPoint::GetSelectedAlgorithmDLL() const
{
  return m_SelectedAlgorithmInfo;
}

void QmitkMatchPoint::OnMaskCheckBoxToggeled(bool checked)
{
  if (!m_Working)
  {
    CheckInputs();
    ConfigureRegistrationControls();
  }
};

void QmitkMatchPoint::OnSelectedAlgorithmChanged()
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
  this->AdaptFolderGUIElements();
}

void QmitkMatchPoint::OnLoadAlgorithmButtonPushed()
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
  m_Controls.checkMovingMask->setChecked(false);
  m_Controls.checkTargetMask->setChecked(false);

  this->AdaptFolderGUIElements();
  this->CheckInputs();
  this->ConfigureRegistrationControls();
  this->ConfigureProgressInfos();
  this->m_Controls.m_tabs->setCurrentIndex(1);
}

void QmitkMatchPoint::Error(QString msg)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
  MITK_ERROR << msg.toStdString().c_str();

  m_Controls.m_teLog->append(QString("<font color='red'><b>") + msg + QString("</b></font>"));
}

void QmitkMatchPoint::AdaptFolderGUIElements()
{
  m_Controls.m_pbLoadSelected->setEnabled(m_CanLoadAlgorithm);
}

void QmitkMatchPoint::CreateQtPartControl(QWidget* parent)
{

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Parent = parent;

  m_Controls.checkMovingMask->setChecked(false);
  m_Controls.checkTargetMask->setChecked(false);
  m_Controls.m_tabs->setCurrentIndex(0);

  m_AlgorithmSelectionListener.reset(new berry::SelectionChangedAdapter<QmitkMatchPoint>(this,
                                     &QmitkMatchPoint::OnAlgorithmSelectionChanged));

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

bool QmitkMatchPoint::CheckInputs()
{
  bool validMoving = false;
  bool validTarget = false;

  bool validMovingMask = false;
  bool validTargetMask = false;

  mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);

  mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

  if (m_LoadedAlgorithm.IsNull())
  {
    m_Controls.m_lbMovingName->setText(QString("<font color='red'>No algorithm seleted!</font>"));
    m_spSelectedMovingNode = NULL;
    m_spSelectedMovingData = NULL;
    m_Controls.m_lbTargetName->setText(QString("<font color='red'>No algorithm seleted!</font>"));
    m_spSelectedTargetNode = NULL;
    m_spSelectedTargetData = NULL;

    m_spSelectedMovingMaskNode = NULL;
    m_spSelectedMovingMaskData = NULL;
    m_spSelectedTargetMaskNode = NULL;
    m_spSelectedTargetMaskData = NULL;
  }
  else
  {
    QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

    mitk::Image* movingImage = NULL;
    mitk::PointSet* movingPointSet = NULL;
    mitk::Image* targetImage = NULL;
    mitk::PointSet* targetPointSet = NULL;

    mitk::Image* movingMaskImage = NULL;
    mitk::Image* targetMaskImage = NULL;

    typedef ::map::core::continuous::Elements<3>::InternalPointSetType InternalDefaultPointSetType;
    typedef ::map::algorithm::facet::PointSetRegistrationAlgorithmInterface<InternalDefaultPointSetType, InternalDefaultPointSetType>
    PointSetRegInterface;
    typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3> MaskRegInterface;

    PointSetRegInterface* pPointSetInterface = dynamic_cast<PointSetRegInterface*>
        (m_LoadedAlgorithm.GetPointer());
    MaskRegInterface* pMaskInterface = dynamic_cast<MaskRegInterface*>(m_LoadedAlgorithm.GetPointer());

    if (nodes.count() < 1)
    {
      m_Controls.m_lbMovingName->setText(QString("<font color='red'>no data selected!</font>"));
      m_spSelectedMovingNode = NULL;
      m_spSelectedMovingData = NULL;
    }
    else
    {
      m_spSelectedMovingNode = nodes.front();
      m_spSelectedMovingData = m_spSelectedMovingNode->GetData();
      movingImage = dynamic_cast<mitk::Image*>(m_spSelectedMovingNode->GetData());
      movingPointSet = dynamic_cast<mitk::PointSet*>(m_spSelectedMovingNode->GetData());

      if (movingPointSet && pPointSetInterface)
      {
        validMoving = true;
      }
      else if (movingImage && !pPointSetInterface)
      {
        if (movingImage->GetDimension() - 1 == m_LoadedAlgorithm->getMovingDimensions()
            && movingImage->GetTimeSteps() > 1)
        {
          //images has multiple time steps and a time step has the correct dimensionality
          mitk::ImageTimeSelector::Pointer imageTimeSelector =   mitk::ImageTimeSelector::New();
          imageTimeSelector->SetInput(movingImage);
          imageTimeSelector->SetTimeNr(0);
          imageTimeSelector->UpdateLargestPossibleRegion();

          m_spSelectedMovingData = imageTimeSelector->GetOutput();
          validMoving = true;
          m_Controls.m_teLog->append(
            QString("<font color='gray'><i>Selected moving image has multiple time steps. First time step is used as moving image.</i></font>"));
        }
        else if (movingImage->GetDimension() != m_LoadedAlgorithm->getMovingDimensions())
        {
          m_Controls.m_lbMovingName->setText(QString("<font color='red'>wrong image dimension. ") +
                                             QString::number(m_LoadedAlgorithm->getMovingDimensions()) + QString("D needed</font>"));
        }
        else
        {
          validMoving = true;
        }
      }
      else
      {
        m_Controls.m_lbMovingName->setText(QString("<font color='red'>no supported data selected!</font>"));
      }

      nodes.removeFirst();
    }

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
      targetPointSet = dynamic_cast<mitk::PointSet*>(m_spSelectedTargetNode->GetData());

      if (targetPointSet && pPointSetInterface)
      {
        validTarget = true;
      }
      else if (targetImage && !pPointSetInterface)
      {
        if (targetImage->GetDimension() - 1 == m_LoadedAlgorithm->getTargetDimensions()
            && targetImage->GetTimeSteps() > 1)
        {
          //images has multiple time steps and a time step has the correct dimensionality
          mitk::ImageTimeSelector::Pointer imageTimeSelector =   mitk::ImageTimeSelector::New();
          imageTimeSelector->SetInput(targetImage);
          imageTimeSelector->SetTimeNr(0);
          imageTimeSelector->UpdateLargestPossibleRegion();

          m_spSelectedTargetData = imageTimeSelector->GetOutput();
          validTarget = true;
          m_Controls.m_teLog->append(
            QString("<font color='gray'><i>Selected target image has multiple time steps. First time step is used as target image.</i></font>"));
        }
        else if (targetImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions())
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
        m_Controls.m_lbTargetName->setText(QString("<font color='red'>no supported data selected!</font>"));
      }

      nodes.removeFirst();
    }

    if (m_Controls.checkMovingMask->isChecked())
    {
      if (nodes.count() < 1)
      {
        m_Controls.m_lbMovingMaskName->setText(QString("<font color='red'>no data selected!</font>"));
        m_spSelectedMovingMaskNode = NULL;
        m_spSelectedMovingMaskData = NULL;
      }
      else
      {
        m_spSelectedMovingMaskNode = nodes.front();
        m_spSelectedMovingMaskData = NULL;

        movingMaskImage = dynamic_cast<mitk::Image*>(m_spSelectedMovingMaskNode->GetData());

        bool isMask = maskPredicate->CheckNode(m_spSelectedMovingMaskNode);

        if (!isMask)
        {
          m_Controls.m_lbMovingMaskName->setText(QString("<font color='red'>no mask selected!</font>"));
        }
        else if (movingMaskImage && pMaskInterface)
        {
          if (movingMaskImage->GetDimension() - 1 == m_LoadedAlgorithm->getMovingDimensions()
              && movingMaskImage->GetTimeSteps() > 1)
          {
            //images has multiple time steps and a time step has the correct dimensionality
            mitk::ImageTimeSelector::Pointer imageTimeSelector =   mitk::ImageTimeSelector::New();
            imageTimeSelector->SetInput(movingMaskImage);
            imageTimeSelector->SetTimeNr(0);
            imageTimeSelector->UpdateLargestPossibleRegion();

            m_spSelectedMovingMaskData = imageTimeSelector->GetOutput();
            validMovingMask = true;
            m_Controls.m_teLog->append(
              QString("<font color='gray'><i>Selected moving mask has multiple time steps. First time step is used as moving mask.</i></font>"));
          }
          else if (movingMaskImage->GetDimension() != m_LoadedAlgorithm->getMovingDimensions())
          {
            m_Controls.m_lbMovingMaskName->setText(QString("<font color='red'>wrong image dimension. ") +
                                                   QString::number(m_LoadedAlgorithm->getMovingDimensions()) + QString("D needed</font>"));
          }
          else
          {
            m_spSelectedMovingMaskData = movingMaskImage;
            validMovingMask = true;
          }
        }
        else
        {
          m_Controls.m_lbMovingMaskName->setText(
            QString("<font color='red'>no supported data selected!</font>"));
        }

        nodes.removeFirst();
      }
    }
    else
    {
      m_Controls.m_lbMovingMaskName->setText(QString("mask deactivated"));
      validMovingMask = true;
      m_spSelectedMovingMaskNode = NULL;
      m_spSelectedMovingMaskData = NULL;
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
          if (targetMaskImage->GetDimension() - 1 == m_LoadedAlgorithm->getTargetDimensions()
              && targetMaskImage->GetTimeSteps() > 1)
          {
            //images has multiple time steps and a time step has the correct dimensionality
            mitk::ImageTimeSelector::Pointer imageTimeSelector =   mitk::ImageTimeSelector::New();
            imageTimeSelector->SetInput(targetMaskImage);
            imageTimeSelector->SetTimeNr(0);
            imageTimeSelector->UpdateLargestPossibleRegion();

            m_spSelectedTargetMaskData = imageTimeSelector->GetOutput();
            validTargetMask = true;
            m_Controls.m_teLog->append(
              QString("<font color='gray'><i>Selected target mask has multiple time steps. First time step is used as target mask.</i></font>"));
          }
          else if (targetMaskImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions())
          {
            m_Controls.m_lbTargetMaskName->setText(QString("<font color='red'>wrong image dimension. ") +
                                                   QString::number(m_LoadedAlgorithm->getTargetDimensions()) + QString("D needed</font>"));
          }
          else
          {
            m_spSelectedTargetMaskData = targetMaskImage;
            validTargetMask = true;
          }
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

  if (validMoving)
  {
    m_Controls.m_lbMovingName->setText(QString::fromStdString(GetInputNodeDisplayName(
                                         m_spSelectedMovingNode)));
  }

  if (validTarget)
  {
    m_Controls.m_lbTargetName->setText(QString::fromStdString(GetInputNodeDisplayName(
                                         m_spSelectedTargetNode)));
  }

  if (validMovingMask && m_Controls.checkMovingMask->isChecked())
  {
    m_Controls.m_lbMovingMaskName->setText(QString::fromStdString(GetInputNodeDisplayName(
        m_spSelectedMovingMaskNode)));
  }

  if (validTargetMask && m_Controls.checkTargetMask->isChecked())
  {
    m_Controls.m_lbTargetMaskName->setText(QString::fromStdString(GetInputNodeDisplayName(
        m_spSelectedTargetMaskNode)));
  }

  m_ValidInputs = validMoving && validTarget && validMovingMask && validTargetMask;
  return m_ValidInputs;
}

std::string QmitkMatchPoint::GetInputNodeDisplayName(const mitk::DataNode* node) const
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

mitk::DataStorage::SetOfObjects::Pointer QmitkMatchPoint::GetRegNodes() const
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

std::string QmitkMatchPoint::GetDefaultRegJobName() const
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

void QmitkMatchPoint::ConfigureRegistrationControls()
{
  m_Controls.m_tabSelection->setEnabled(!m_Working);
  m_Controls.m_leRegJobName->setEnabled(!m_Working);
  m_Controls.groupMasks->setEnabled(!m_Working);

  m_Controls.m_pbStartReg->setEnabled(false);
  m_Controls.m_pbStopReg->setEnabled(false);
  m_Controls.m_pbStopReg->setVisible(false);

  m_Controls.m_lbMovingMaskName->setVisible(m_Controls.checkMovingMask->isChecked());
  m_Controls.m_lbTargetMaskName->setVisible(m_Controls.checkTargetMask->isChecked());

  if (m_LoadedAlgorithm.IsNotNull())
  {
    m_Controls.m_tabSettings->setEnabled(!m_Working);
    m_Controls.m_tabExecution->setEnabled(true);
    m_Controls.m_pbStartReg->setEnabled(m_ValidInputs && !m_Working);
    m_Controls.m_leRegJobName->setEnabled(!m_Working);
    m_Controls.m_checkMapEntity->setEnabled(!m_Working);
    m_Controls.m_checkStoreReg->setEnabled(!m_Working);

    const IStoppableAlgorithm* pIterativ = dynamic_cast<const IStoppableAlgorithm*>
                                           (m_LoadedAlgorithm.GetPointer());

    if (pIterativ)
    {
      m_Controls.m_pbStopReg->setVisible(pIterativ->isStoppable());
    }

    typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3> MaskRegInterface;
    const MaskRegInterface* pMaskReg = dynamic_cast<const MaskRegInterface*>
                                       (m_LoadedAlgorithm.GetPointer());

    m_Controls.groupMasks->setVisible(pMaskReg != NULL);

    //if the stop button is set to visible and the algorithm is working ->
    //then the algorithm is stoppable, thus enable the button.
    m_Controls.m_pbStopReg->setEnabled(m_Controls.m_pbStopReg->isVisible() && m_Working);

    this->m_Controls.m_lbLoadedAlgorithmName->setText(
      QString::fromStdString(m_LoadedAlgorithm->getUID()->toStr()));
  }
  else
  {
    m_Controls.m_tabSettings->setEnabled(false);
    m_Controls.m_tabExecution->setEnabled(false);
    this->m_Controls.m_lbLoadedAlgorithmName->setText(
      QString("<font color='red'>no algorithm loaded!</font>"));
    m_Controls.groupMasks->setVisible(false);
  }

  if (!m_Working)
  {
    this->m_Controls.m_leRegJobName->setText(QString::fromStdString(this->GetDefaultRegJobName()));
  }
}

void QmitkMatchPoint::ConfigureProgressInfos()
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
}

void QmitkMatchPoint::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes)
{
  if (!m_Working)
  {
    CheckInputs();
    ConfigureRegistrationControls();
  }
}

void QmitkMatchPoint::OnStartRegBtnPushed()
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
  QmitkRegistrationJob* pJob = new QmitkRegistrationJob(m_LoadedAlgorithm);
  pJob->setAutoDelete(true);

  pJob->m_spTargetData = m_spSelectedTargetData;
  pJob->m_spMovingData = m_spSelectedMovingData;
  pJob->m_TargetDataUID = mitk::EnsureUID(this->m_spSelectedTargetNode->GetData());
  pJob->m_MovingDataUID = mitk::EnsureUID(this->m_spSelectedMovingNode->GetData());

  if (m_spSelectedTargetMaskData.IsNotNull())
  {
    pJob->m_spTargetMask = m_spSelectedTargetMaskData;
    pJob->m_TargetMaskDataUID = mitk::EnsureUID(this->m_spSelectedTargetMaskNode->GetData());
  }

  if (m_spSelectedMovingMaskData.IsNotNull())
  {
    pJob->m_spMovingMask = m_spSelectedMovingMaskData;
    pJob->m_MovingMaskDataUID = mitk::EnsureUID(this->m_spSelectedMovingMaskNode->GetData());
  }

  pJob->m_JobName = m_Controls.m_leRegJobName->text().toStdString();

  pJob->m_StoreReg = m_Controls.m_checkStoreReg->checkState() == Qt::Checked;

  connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnRegJobError(QString)));
  connect(pJob, SIGNAL(Finished()), this, SLOT(OnRegJobFinished()));
  connect(pJob, SIGNAL(RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer,
                       const QmitkRegistrationJob*)), this,
          SLOT(OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)),
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
}

void QmitkMatchPoint::OnStopRegBtnPushed()
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

      m_Controls.m_pbStopReg->setEnabled(false);
    }
    else
    {
    }
  }
}

void QmitkMatchPoint::OnSaveLogBtnPushed()
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

void QmitkMatchPoint::OnRegJobError(QString err)
{
  Error(err);
};

void QmitkMatchPoint::OnRegJobFinished()
{
  this->m_Working = false;

  this->GetRenderWindowPart()->RequestUpdate();

  this->CheckInputs();
  this->ConfigureRegistrationControls();
  this->ConfigureProgressInfos();
};


void QmitkMatchPoint::OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer
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

    pMapJob->m_MappedName = pRegJob->m_JobName + std::string(" mapped moving data");
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

void QmitkMatchPoint::OnMapJobError(QString err)
{
  Error(err);
};

void QmitkMatchPoint::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
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

void QmitkMatchPoint::OnAlgorithmIterated(QString info, bool hasIterationCount,
    unsigned long currentIteration)
{
  if (hasIterationCount)
  {
    m_Controls.m_progBarIteration->setValue(currentIteration);
  }

  m_Controls.m_teLog->append(info);
};

void QmitkMatchPoint::OnLevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel)
{
  if (hasLevelCount)
  {
    m_Controls.m_progBarLevel->setValue(currentLevel);
  }

  m_Controls.m_teLog->append(QString("<b><font color='green'>") + info + QString("</font></b>"));
};

void QmitkMatchPoint::OnAlgorithmStatusChanged(QString info)
{
  m_Controls.m_teLog->append(QString("<b><font color='blue'>") + info + QString(" </font></b>"));
};

void QmitkMatchPoint::OnAlgorithmInfo(QString info)
{
  m_Controls.m_teLog->append(QString("<font color='gray'><i>") + info + QString("</i></font>"));
};

void QmitkMatchPoint::OnAlgorithmSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcepart,
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

void QmitkMatchPoint::UpdateAlgorithmSelection(berry::ISelection::ConstPointer selection)
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
