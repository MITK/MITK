/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDescriptionParameterBackgroundJob.h"
#include "mitkModelFitInfo.h"

void DescriptionParameterBackgroundJob::OnComputeEvent(::itk::Object* caller,
    const itk::EventObject& event)
{
  itk::ProgressEvent progressEvent;
  itk::InitializeEvent initEvent;
  itk::StartEvent startEvent;
  itk::EndEvent endEvent;

  if (progressEvent.CheckEvent(&event))
  {
    mitk::DescriptionParameterImageGeneratorBase* castedReporter =
      dynamic_cast<mitk::DescriptionParameterImageGeneratorBase*>(caller);
    emit JobProgress(castedReporter->GetProgress());
  }
  else  if (initEvent.CheckEvent(&event))
  {
    emit JobStatusChanged(QString("Initializing description parameter generator"));
  }
  else  if (startEvent.CheckEvent(&event))
  {
    emit JobStatusChanged(QString("Started parameter computation process."));
  }
  else  if (endEvent.CheckEvent(&event))
  {
    emit JobStatusChanged(QString("Finished parameter computation process."));
  }
}

DescriptionParameterBackgroundJob::
DescriptionParameterBackgroundJob(mitk::DescriptionParameterImageGeneratorBase* generator,
                                  mitk::DataNode* parentNode)
{
  if (!generator)
  {
    mitkThrow() << "Cannot create description parameter background job. Passed fit generator is NULL.";
  }

  m_Generator = generator;
  m_ParentNode = parentNode;

  m_spCommand = ::itk::MemberCommand<DescriptionParameterBackgroundJob>::New();
  m_spCommand->SetCallbackFunction(this, &DescriptionParameterBackgroundJob::OnComputeEvent);
  m_ObserverID = m_Generator->AddObserver(::itk::AnyEvent(), m_spCommand);
};

mitk::DataNode*
DescriptionParameterBackgroundJob::
GetParentNode() const
{
  return m_ParentNode;
};

DescriptionParameterBackgroundJob::
~DescriptionParameterBackgroundJob()
{
  m_Generator->RemoveObserver(m_ObserverID);
};

mitk::modelFit::ModelFitResultNodeVectorType DescriptionParameterBackgroundJob::CreateResultNodes(
  const mitk::DescriptionParameterImageGeneratorBase::ParameterImageMapType& paramimages)
{
  mitk::modelFit::ModelFitResultNodeVectorType results;

  for (const auto &image : paramimages)
  {
    if (image.second.IsNull())
    {
      mitkThrow() << "Cannot generate result node. Passed parameterImage is null. parameter name: " <<
                  image.first;
    }

    mitk::DataNode::Pointer result = mitk::DataNode::New();
    result->SetData(image.second);
    result->SetName(image.first);
    result->SetVisibility(true);
    results.push_back(result);
  }

  return results;
};

void
DescriptionParameterBackgroundJob::
run()
{
  try
  {
    emit JobStatusChanged(QString("Started session..."));

    m_Generator->Generate();

    emit JobStatusChanged(QString("Generate result nodes."));

    m_Results = CreateResultNodes(m_Generator->GetParameterImages());

    emit ResultsAreAvailable(m_Results, this);
  }
  catch (::std::exception& e)
  {
    emit Error(QString("Error while processing data. Details: ") + QString::fromLatin1(e.what()));
  }
  catch (...)
  {
    emit Error(QString("Unkown error when processing the data."));
  }

  emit Finished();
};
