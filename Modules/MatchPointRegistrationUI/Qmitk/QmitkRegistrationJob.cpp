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

#include "QmitkRegistrationJob.h"

// Mitk
#include <mitkAlgorithmHelper.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageMappingHelper.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkMaskedAlgorithmHelper.h>
#include <mitkMatchPointPropertyTags.h>
#include <mitkUIDHelper.h>

// Qt
#include <QThreadPool>

// MatchPoint
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmInterface.h>

const mitk::Image *QmitkRegistrationJob::GetTargetDataAsImage() const
{
  return dynamic_cast<const mitk::Image *>(m_spTargetData.GetPointer());
};

const mitk::Image *QmitkRegistrationJob::GetMovingDataAsImage() const
{
  return dynamic_cast<const mitk::Image *>(m_spMovingData.GetPointer());
};

const map::algorithm::RegistrationAlgorithmBase *QmitkRegistrationJob::GetLoadedAlgorithm() const
{
  return m_spLoadedAlgorithm;
};

void QmitkRegistrationJob::OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event)
{
  const map::events::AlgorithmEvent *pAlgEvent = dynamic_cast<const map::events::AlgorithmEvent *>(&event);
  const map::events::AlgorithmIterationEvent *pIterationEvent =
    dynamic_cast<const map::events::AlgorithmIterationEvent *>(&event);
  const map::events::AlgorithmWrapperEvent *pWrapEvent =
    dynamic_cast<const map::events::AlgorithmWrapperEvent *>(&event);
  const map::events::AlgorithmResolutionLevelEvent *pLevelEvent =
    dynamic_cast<const map::events::AlgorithmResolutionLevelEvent *>(&event);

  const map::events::InitializingAlgorithmEvent *pInitEvent =
    dynamic_cast<const map::events::InitializingAlgorithmEvent *>(&event);
  const map::events::StartingAlgorithmEvent *pStartEvent =
    dynamic_cast<const map::events::StartingAlgorithmEvent *>(&event);
  const map::events::StoppingAlgorithmEvent *pStoppingEvent =
    dynamic_cast<const map::events::StoppingAlgorithmEvent *>(&event);
  const map::events::StoppedAlgorithmEvent *pStoppedEvent =
    dynamic_cast<const map::events::StoppedAlgorithmEvent *>(&event);
  const map::events::FinalizingAlgorithmEvent *pFinalizingEvent =
    dynamic_cast<const map::events::FinalizingAlgorithmEvent *>(&event);
  const map::events::FinalizedAlgorithmEvent *pFinalizedEvent =
    dynamic_cast<const map::events::FinalizedAlgorithmEvent *>(&event);

  if (pInitEvent)
  {
    emit AlgorithmStatusChanged(QString("Initializing algorithm ..."));
  }
  else if (pStartEvent)
  {
    emit AlgorithmStatusChanged(QString("Starting algorithm ..."));
  }
  else if (pStoppingEvent)
  {
    emit AlgorithmStatusChanged(QString("Stopping algorithm ..."));
  }
  else if (pStoppedEvent)
  {
    emit AlgorithmStatusChanged(QString("Stopped algorithm ..."));

    if (!pStoppedEvent->getComment().empty())
    {
      emit AlgorithmInfo(QString("Stopping condition: ") + QString::fromStdString(pStoppedEvent->getComment()));
    }
  }
  else if (pFinalizingEvent)
  {
    emit AlgorithmStatusChanged(QString("Finalizing algorithm and results ..."));
  }
  else if (pFinalizedEvent)
  {
    emit AlgorithmStatusChanged(QString("Finalized algorithm ..."));
  }
  else if (pIterationEvent)
  {
    const IIterativeAlgorithm *pIterative =
      dynamic_cast<const IIterativeAlgorithm *>(this->m_spLoadedAlgorithm.GetPointer());

    map::algorithm::facet::IterativeAlgorithmInterface::IterationCountType count = 0;
    bool hasCount = false;

    if (pIterative && pIterative->hasIterationCount())
    {
      hasCount = true;
      count = pIterative->getCurrentIteration();
    }

    emit AlgorithmIterated(QString::fromStdString(pIterationEvent->getComment()), hasCount, count);
  }
  else if (pLevelEvent)
  {
    const IMultiResAlgorithm *pResAlg =
      dynamic_cast<const IMultiResAlgorithm *>(this->m_spLoadedAlgorithm.GetPointer());

    map::algorithm::facet::MultiResRegistrationAlgorithmInterface::ResolutionLevelCountType count = 0;
    bool hasCount = false;
    QString info = QString::fromStdString(pLevelEvent->getComment());

    if (pResAlg && pResAlg->hasLevelCount())
    {
      count = pResAlg->getCurrentLevel() + 1;
      hasCount = true;
      info = QString("Level #") + QString::number(pResAlg->getCurrentLevel() + 1) + QString(" ") + info;
    }

    emit LevelChanged(info, hasCount, count);
  }
  else if (pAlgEvent && !pWrapEvent)
  {
    emit AlgorithmInfo(QString::fromStdString(pAlgEvent->getComment()));
  }
}

QmitkRegistrationJob::QmitkRegistrationJob(map::algorithm::RegistrationAlgorithmBase *pAlgorithm)
{
  m_MapEntity = false;
  m_StoreReg = false;
  m_ErrorOccured = false;
  m_spLoadedAlgorithm = pAlgorithm;
  m_JobName = "Unnamed RegJob";
  m_MovingDataUID = "Missing moving UID";
  m_TargetDataUID = "Missing target UID";

  m_spTargetMask = nullptr;
  m_spMovingMask = nullptr;

  m_spCommand = ::itk::MemberCommand<QmitkRegistrationJob>::New();
  m_spCommand->SetCallbackFunction(this, &QmitkRegistrationJob::OnMapAlgorithmEvent);
  m_ObserverID = m_spLoadedAlgorithm->AddObserver(::map::events::AlgorithmEvent(), m_spCommand);
};

QmitkRegistrationJob::~QmitkRegistrationJob()
{
  m_spLoadedAlgorithm->RemoveObserver(m_ObserverID);
};

void QmitkRegistrationJob::run()
{
  try
  {
    mitk::MITKAlgorithmHelper helper(m_spLoadedAlgorithm);
    mitk::MaskedAlgorithmHelper maskedHelper(m_spLoadedAlgorithm);

    //*@TODO Data Check and failure handle
    helper.SetData(this->m_spMovingData, this->m_spTargetData);
    maskedHelper.SetMasks(this->m_spMovingMask, this->m_spTargetMask);

    // perform registration
    m_spResultRegistration = helper.GetRegistration();

    // wrap the registration in a data node
    if (m_spResultRegistration.IsNull())
    {
      emit Error(QString("Error. No registration was determined. No results to store."));
    }
    else
    {
      mitk::MAPRegistrationWrapper::Pointer spRegWrapper = mitk::MAPRegistrationWrapper::New();
      spRegWrapper->SetRegistration(m_spResultRegistration);

      emit RegResultIsAvailable(spRegWrapper, this);
    }
  }
  catch (::std::exception &e)
  {
    emit Error(QString("Error while registering data. Details: ") + QString::fromLatin1(e.what()));
  }
  catch (...)
  {
    emit Error(QString("Unkown error when registering data."));
  }

  emit Finished();
};
