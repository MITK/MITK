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

#include "QmitkUSAbstractNavigationStep.h"
#include "ui_QmitkUSAbstractNavigationStep.h"

#include "mitkNavigationDataToNavigationDataFilter.h"

const char* QmitkUSAbstractNavigationStep::DATANAME_SETTINGS = "Settings";
const char* QmitkUSAbstractNavigationStep::DATANAME_IMAGESTREAM = "US Image Stream";
const char* QmitkUSAbstractNavigationStep::DATANAME_BASENODE = QmitkUSAbstractNavigationStep::DATANAME_IMAGESTREAM;

QmitkUSAbstractNavigationStep::QmitkUSAbstractNavigationStep(QWidget *parent) :
  QWidget(parent), m_NavigationStepState(State_Stopped)
{
}

QmitkUSAbstractNavigationStep::~QmitkUSAbstractNavigationStep()
{
}

bool QmitkUSAbstractNavigationStep::OnRestartStep()
{
  return this->OnStopStep() && this->OnStartStep();
}

bool QmitkUSAbstractNavigationStep::GetIsRestartable()
{
  return true;
}

QmitkUSAbstractNavigationStep::FilterVector QmitkUSAbstractNavigationStep::GetFilter()
{
  return FilterVector();
}

void QmitkUSAbstractNavigationStep::SetDataStorage(itk::SmartPointer<mitk::DataStorage> dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkUSAbstractNavigationStep::SetCombinedModality(itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> combinedModality)
{
  m_CombinedModality = combinedModality;
  MITK_INFO << "Combined modality set to NULL: " << m_CombinedModality.IsNull();

  this->OnSetCombinedModality();
}

bool QmitkUSAbstractNavigationStep::StartStep()
{
  if ( this->GetNavigationStepState() == State_Stopped && this->OnStartStep() )
  {
    m_NavigationStepState = State_Started;
    return true;
  }

  return false;
}

bool QmitkUSAbstractNavigationStep::StopStep()
{
  if ( this->GetNavigationStepState() == State_Started ||
    ( this->GetNavigationStepState() > State_Started && this->DeactivateStep() ) )
  {
    if ( this->OnStopStep() )
    {
      m_NavigationStepState = State_Stopped;
      return true;
    }
  }

  return false;
}

bool QmitkUSAbstractNavigationStep::RestartStep()
{
  return this->OnRestartStep();
}

bool QmitkUSAbstractNavigationStep::FinishStep()
{
  if ( this->GetNavigationStepState() > State_Started && this->DeactivateStep() )
  {
    if ( this->OnFinishStep() )
    {
      m_NavigationStepState = State_Started;
      return true;
    }
  }

  return false;
}

bool QmitkUSAbstractNavigationStep::ActivateStep()
{
  if ( this->GetNavigationStepState() == State_Started ||
    ( this->GetNavigationStepState() < State_Started && this->StartStep() ) )
  {
    if ( this->OnActivateStep() )
    {
      m_NavigationStepState = State_Active;
      return true;
    }
  }

  return false;
}

bool QmitkUSAbstractNavigationStep::DeactivateStep()
{
  if ( this->GetNavigationStepState() == State_Active )
  {
    if ( this->OnDeactivateStep() )
    {
      m_NavigationStepState = State_Started;
      return true;
    }
  }

  return false;
}

void QmitkUSAbstractNavigationStep::Update()
{
  this->OnUpdate();
}

QmitkUSAbstractNavigationStep::NavigationStepState QmitkUSAbstractNavigationStep::GetNavigationStepState()
{
  return m_NavigationStepState;
}

itk::SmartPointer<mitk::DataStorage> QmitkUSAbstractNavigationStep::GetDataStorage(bool throwNull)
{
  if ( throwNull && m_DataStorage.IsNull() )
  {
    MITK_ERROR << "Data storage must be set to step widget before.";
    mitkThrow() << "Data storage must be set to step widget before.";
  }

  return m_DataStorage;
}

itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> QmitkUSAbstractNavigationStep::GetCombinedModality(bool throwNull)
{
  if ( throwNull && m_CombinedModality.IsNull() )
  {
    MITK_INFO << "Combined modality is not set yet for this widget.";
    mitkThrow() << "Combined modality is not set yet for this widget.";
  }

  return m_CombinedModality;
}

itk::SmartPointer<mitk::DataNode> QmitkUSAbstractNavigationStep::GetNamedDerivedNode(const char *name, const char *sourceName)
{
  if ( m_DataStorage.IsNull() )
  {
    MITK_ERROR << "Data storage must be set to step widget before.";
    mitkThrow() << "Data storage must be set to step widget before.";
  }

  mitk::DataNode::Pointer sourceNode = m_DataStorage->GetNamedNode(sourceName);
  if ( sourceNode.IsNull() )
  {
    MITK_WARN << "Source node cannot be found in data storage. Returning null.";
    return nullptr;
  }

  return m_DataStorage->GetNamedDerivedNode(name, m_DataStorage->GetNamedNode(sourceName));
}

itk::SmartPointer<mitk::DataNode> QmitkUSAbstractNavigationStep::GetNamedDerivedNodeAndCreate(const char* name, const char* sourceName)
{
  if ( m_DataStorage.IsNull() )
  {
    MITK_ERROR << "Data storage must be set to step widget before.";
    mitkThrow() << "Data storage must be set to step widget before.";
  }

  mitk::DataNode::Pointer sourceNode = m_DataStorage->GetNamedNode(sourceName);
  mitk::DataNode::Pointer dataNode = m_DataStorage->GetNamedDerivedNode(name, sourceNode);

  if ( dataNode.IsNull() )
  {
    dataNode = mitk::DataNode::New();
    dataNode->SetName(name);
    if ( sourceNode.IsNotNull() )
      {this->GetDataStorage()->Add(dataNode, sourceNode);}
    else
      {this->GetDataStorage()->Add(dataNode);}
  }

  return dataNode;
}
