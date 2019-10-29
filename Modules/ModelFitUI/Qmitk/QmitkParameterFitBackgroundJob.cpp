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


#include "QmitkParameterFitBackgroundJob.h"
#include "mitkModelFitInfo.h"

void ParameterFitBackgroundJob::OnFitEvent(::itk::Object* caller, const itk::EventObject & event)
{
  itk::ProgressEvent progressEvent;
  itk::InitializeEvent initEvent;
  itk::StartEvent startEvent;
  itk::EndEvent endEvent;

  if (progressEvent.CheckEvent(&event))
  {
    mitk::ParameterFitImageGeneratorBase* castedReporter = dynamic_cast<mitk::ParameterFitImageGeneratorBase*>(caller);
    emit JobProgress(castedReporter->GetProgress());
  }
  else  if (initEvent.CheckEvent(&event))
  {
    emit JobStatusChanged(QString("Initializing parameter fit generator"));
  }
  else  if (startEvent.CheckEvent(&event))
  {
    emit JobStatusChanged(QString("Started fitting process."));
  }
  else  if (endEvent.CheckEvent(&event))
  {
    emit JobStatusChanged(QString("Finished fitting process."));
  }
}

ParameterFitBackgroundJob::
ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode) : ParameterFitBackgroundJob(generator, fitInfo, parentNode, {})
{
};

ParameterFitBackgroundJob::
ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode, mitk::modelFit::ModelFitResultNodeVectorType additionalRelevantNodes)
{
  if (!generator)
  {
    mitkThrow() << "Cannot create parameter fit background job. Passed fit generator is NULL.";
  }

  if (!fitInfo)
  {
    mitkThrow() << "Cannot create parameter fit background job. Passed model traits interface is NULL.";
  }

  m_Generator = generator;
  m_ModelFitInfo = fitInfo;
  m_ParentNode = parentNode;
  m_AdditionalRelevantNodes = additionalRelevantNodes;

  m_spCommand = ::itk::MemberCommand<ParameterFitBackgroundJob>::New();
  m_spCommand->SetCallbackFunction(this, &ParameterFitBackgroundJob::OnFitEvent);
  m_ObserverID = m_Generator->AddObserver(::itk::AnyEvent(), m_spCommand);
};

mitk::DataNode*
  ParameterFitBackgroundJob::
GetParentNode() const
{
  return m_ParentNode;
};

mitk::modelFit::ModelFitResultNodeVectorType ParameterFitBackgroundJob::GetAdditionalRelevantNodes() const
{
  return m_AdditionalRelevantNodes;
};


ParameterFitBackgroundJob::
~ParameterFitBackgroundJob()
{
    m_Generator->RemoveObserver(m_ObserverID);
};

void
ParameterFitBackgroundJob::
run()
{
    try
    {
      emit JobStatusChanged(QString("Started fit session.Generate UID: ")+QString::fromStdString(m_ModelFitInfo->uid));

      m_Generator->Generate();

      emit JobStatusChanged(QString("Generate result nodes."));

      m_Results = mitk::modelFit::CreateResultNodeMap(m_Generator->GetParameterImages(), m_Generator->GetDerivedParameterImages(), m_Generator->GetCriterionImages(), m_Generator->GetEvaluationParameterImages(), m_ModelFitInfo);

      emit ResultsAreAvailable(m_Results, this);
    }
    catch (::std::exception& e)
    {
        emit Error(QString("Error while fitting data. Details: ")+QString::fromLatin1(e.what()));
    }
    catch (...)
    {
        emit Error(QString("Unkown error when fitting the data."));
    }

    emit Finished();
};
