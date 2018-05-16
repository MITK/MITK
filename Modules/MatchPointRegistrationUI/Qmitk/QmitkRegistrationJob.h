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

#ifndef __QMITK_REGISTRATION_JOB_H
#define __QMITK_REGISTRATION_JOB_H

// QT
#include <QObject>
#include <QRunnable>

// ITK
#include <itkCommand.h>

// MITK
#include "mitkUIDHelper.h"
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkMAPRegistrationWrapper.h>

// MatchPoint
#include <mapDeploymentDLLInfo.h>
#include <mapIterativeAlgorithmInterface.h>
#include <mapMultiResRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapRegistrationBase.h>

#include <MitkMatchPointRegistrationUIExports.h>

class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegistrationJob : public QObject, public QRunnable
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  QmitkRegistrationJob(::map::algorithm::RegistrationAlgorithmBase *pAlgorithm);
  ~QmitkRegistrationJob() override;

  void run() override;

signals:
  void Finished();
  void Error(QString err);
  void RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration,
                            const QmitkRegistrationJob *pJob);
  void AlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration);
  void LevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel);
  void AlgorithmStatusChanged(QString info);
  void AlgorithmInfo(QString info);

public:
  // Inputs
  mitk::BaseData::ConstPointer m_spTargetData;
  mitk::BaseData::ConstPointer m_spMovingData;

  mitk::Image::ConstPointer m_spTargetMask;
  mitk::Image::ConstPointer m_spMovingMask;

  // job settings
  bool m_MapEntity;
  bool m_StoreReg;
  bool m_ErrorOccured;
  std::string m_JobName;
  mitk::NodeUIDType m_TargetDataUID;
  mitk::NodeUIDType m_MovingDataUID;
  mitk::NodeUIDType m_TargetMaskDataUID;
  mitk::NodeUIDType m_MovingMaskDataUID;

  const ::map::algorithm::RegistrationAlgorithmBase *GetLoadedAlgorithm() const;

protected:
  typedef ::map::algorithm::facet::IterativeAlgorithmInterface IIterativeAlgorithm;
  typedef ::map::algorithm::facet::MultiResRegistrationAlgorithmInterface IMultiResAlgorithm;

  // Result registration.
  ::map::core::RegistrationBase::Pointer m_spResultRegistration;
  mitk::DataNode::Pointer m_spRegNode;
  // mapped image. May be null if m_MapEntity is false.
  mitk::DataNode::Pointer m_spMappedImageNode;

  ::itk::MemberCommand<QmitkRegistrationJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;
  ::map::algorithm::RegistrationAlgorithmBase::Pointer m_spLoadedAlgorithm;

  // Helper functions
  const mitk::Image *GetTargetDataAsImage() const;
  const mitk::Image *GetMovingDataAsImage() const;

  void OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
