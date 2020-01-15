/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __QMITK_DESCRIPTION_PARAMETER_BACKGROUND_JOB_H
#define __QMITK_DESCRIPTION_PARAMETER_BACKGROUND_JOB_H


//QT
#include <QRunnable>
#include <QObject>

//MITK
#include <mitkDataNode.h>

#include <mitkDescriptionParameterImageGeneratorBase.h>
#include <mitkModelFitResultHelper.h>
#include <mitkModelFitInfo.h>

// ITK
#include <itkCommand.h>

#include "MitkPharmacokineticsUIExports.h"

class MITKPHARMACOKINETICSUI_EXPORT DescriptionParameterBackgroundJob : public QObject, public QRunnable
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:
  DescriptionParameterBackgroundJob(mitk::DescriptionParameterImageGeneratorBase* generator, mitk::DataNode* parentNode = nullptr);
    ~DescriptionParameterBackgroundJob() override;

		void run() override;

    /**Returns the node (if defined), that is the parent object for the results of the job.
    May be null.*/
    mitk::DataNode* GetParentNode() const;

signals:
    void Finished();
    void Error(QString err);
    void ResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType resultMap, const DescriptionParameterBackgroundJob* pJob);
    void JobProgress(double progress);
    void JobStatusChanged(QString info);

protected:
  static mitk::modelFit::ModelFitResultNodeVectorType CreateResultNodes(const mitk::DescriptionParameterImageGeneratorBase::ParameterImageMapType& paramimages);

	//Inputs
	mitk::DescriptionParameterImageGeneratorBase::Pointer m_Generator;
  mitk::DataNode::Pointer m_ParentNode;

  // Results
  mitk::modelFit::ModelFitResultNodeVectorType m_Results;

  ::itk::MemberCommand<DescriptionParameterBackgroundJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;

  void OnComputeEvent(::itk::Object *, const itk::EventObject &event);
};

#endif

