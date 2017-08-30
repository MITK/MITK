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

#include "QmitkFramesRegistrationJob.h"

// Mitk
#include <mitkImageAccessByItk.h>

// Qt
#include <QThreadPool>

// Map4CTK
#include <mitkImageMappingHelper.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkMatchPointPropertyTags.h>
#include <mitkUIDHelper.h>

// MatchPoint
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmInterface.h>

const mitk::Image *QmitkFramesRegistrationJob::GetTargetDataAsImage() const
{
  return dynamic_cast<const mitk::Image *>(m_spTargetData.GetPointer());
};

const map::algorithm::RegistrationAlgorithmBase *QmitkFramesRegistrationJob::GetLoadedAlgorithm() const
{
  return m_spLoadedAlgorithm;
};

void QmitkFramesRegistrationJob::OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event)
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
  const itk::ProgressEvent *pProgressEvent = dynamic_cast<const itk::ProgressEvent *>(&event);
  const mitk::FrameRegistrationEvent *pFrameRegEvent = dynamic_cast<const mitk::FrameRegistrationEvent *>(&event);
  const mitk::FrameMappingEvent *pFrameMapEvent = dynamic_cast<const mitk::FrameMappingEvent *>(&event);

  if (pProgressEvent)
  {
    emit FrameProcessed(m_helper->GetProgress());
  }
  else if (pFrameRegEvent)
  {
    emit FrameRegistered(m_helper->GetProgress());
  }
  else if (pFrameMapEvent)
  {
    emit FrameMapped(m_helper->GetProgress());
  }
  else if (pInitEvent)
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

QmitkFramesRegistrationJob::QmitkFramesRegistrationJob(map::algorithm::RegistrationAlgorithmBase *pAlgorithm)
  : m_TargetDataUID("Missing target UID"), m_spLoadedAlgorithm(pAlgorithm)
{
  m_MappedName = "Unnamed RegJob";

  m_spTargetMask = nullptr;

  m_spCommand = ::itk::MemberCommand<QmitkFramesRegistrationJob>::New();
  m_spCommand->SetCallbackFunction(this, &QmitkFramesRegistrationJob::OnMapAlgorithmEvent);
  m_ObserverID = m_spLoadedAlgorithm->AddObserver(::map::events::AlgorithmEvent(), m_spCommand);
};

QmitkFramesRegistrationJob::~QmitkFramesRegistrationJob()
{
  m_spLoadedAlgorithm->RemoveObserver(m_ObserverID);
};

void QmitkFramesRegistrationJob::run()
{
  try
  {
    m_helper = mitk::TimeFramesRegistrationHelper::New();

    m_helper->Set4DImage(this->GetTargetDataAsImage());
    m_helper->SetTargetMask(this->m_spTargetMask);
    m_helper->SetAlgorithm(this->m_spLoadedAlgorithm);
    m_helper->SetIgnoreList(this->m_IgnoreList);

    m_helper->SetAllowUndefPixels(this->m_allowUndefPixels);
    m_helper->SetAllowUnregPixels(this->m_allowUnregPixels);
    m_helper->SetErrorValue(this->m_errorValue);
    m_helper->SetPaddingValue(this->m_paddingValue);
    m_helper->SetInterpolatorType(this->m_InterpolatorType);

    m_helper->AddObserver(::map::events::AnyMatchPointEvent(), m_spCommand);
    m_helper->AddObserver(::itk::ProgressEvent(), m_spCommand);

    // perform registration
    m_spMappedImageNode = m_helper->GetRegisteredImage();

    // wrap the registration in a data node
    if (m_spMappedImageNode.IsNull())
    {
      emit Error(QString("Error. No registration was determined. No results to store."));
    }
    else
    {
      emit ResultIsAvailable(m_spMappedImageNode, this);
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
