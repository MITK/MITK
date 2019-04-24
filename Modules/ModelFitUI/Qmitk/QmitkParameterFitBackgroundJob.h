/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Software Development for
Integrated Diagnostic and Therapy. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __QMITK_PARAMETER_FIT_BACKGROUND_JOB_H
#define __QMITK_PARAMETER_FIT_BACKGROUND_JOB_H


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
  ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode = NULL);
  /** */
  ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode, mitk::modelFit::ModelFitResultNodeVectorType additionalRelevantNodes);

  ~ParameterFitBackgroundJob();

		void run();

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

