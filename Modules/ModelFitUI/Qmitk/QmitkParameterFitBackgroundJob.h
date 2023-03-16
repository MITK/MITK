/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkParameterFitBackgroundJob_h
#define QmitkParameterFitBackgroundJob_h


//QT
#include <QRunnable>
#include <QObject>

//MITK
#include <mitkDataNode.h>

#include <mitkParameterFitImageGeneratorBase.h>
#include <mitkModelFitResultHelper.h>
#include <mitkModelFitInfo.h>

// ITK
#include <itkCommand.h>

#include "MitkModelFitUIExports.h"

class MITKMODELFITUI_EXPORT ParameterFitBackgroundJob : public QObject, public QRunnable
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:
  ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode = nullptr);
  /** */
  ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode, mitk::modelFit::ModelFitResultNodeVectorType additionalRelevantNodes);

  ~ParameterFitBackgroundJob() override;

		void run() override;

    /**Returns the node (if defined), that is the parent object for the results of the job.
    May be null.*/
    mitk::DataNode* GetParentNode() const;

    mitk::modelFit::ModelFitResultNodeVectorType GetAdditionalRelevantNodes() const;

signals:
    void Finished();
    void Error(QString err);
    void ResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType resultMap, const ParameterFitBackgroundJob* pJob);
    void JobProgress(double progress);
    void JobStatusChanged(QString info);

protected:
	//Inputs
	mitk::ParameterFitImageGeneratorBase::Pointer m_Generator;
  mitk::modelFit::ModelFitInfo::ConstPointer m_ModelFitInfo;
  mitk::DataNode::Pointer m_ParentNode;
  mitk::modelFit::ModelFitResultNodeVectorType m_AdditionalRelevantNodes;

  // Results
  mitk::modelFit::ModelFitResultNodeVectorType m_Results;

  ::itk::MemberCommand<ParameterFitBackgroundJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;

  void OnFitEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
