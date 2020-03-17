/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataGenerationJobBase.h"


mitk::DataStorage::SetOfObjects::ConstPointer QmitkDataGenerationJobBase::GetOutputDataNodes() const
{
  return m_OutputDataNodes;
}

QmitkDataGenerationJobBase::QmitkDataGenerationJobBase(const mitk::DataStorage::SetOfObjects* outputDataNodes, const std::vector<mitk::BaseData::ConstPointer>& inputBaseData) :m_OutputDataNodes(outputDataNodes), m_InputBaseData(inputBaseData)
{
}

QmitkDataGenerationJobBase::QmitkDataGenerationJobBase(mitk::DataNode* outputDataNode, const std::vector<mitk::BaseData::ConstPointer>& inputBaseData): m_InputBaseData(inputBaseData)
{
  if (!outputDataNode) {
    mitkThrow() << "No output data node defined.";
  }

  auto nodes = mitk::DataStorage::SetOfObjects::New();
  nodes->InsertElement(0, outputDataNode);
  m_OutputDataNodes = nodes;
}

QmitkDataGenerationJobBase::QmitkDataGenerationJobBase(const mitk::DataStorage::SetOfObjects* outputDataNodes) : m_OutputDataNodes(outputDataNodes)
{
}

QmitkDataGenerationJobBase::QmitkDataGenerationJobBase(mitk::DataNode* outputDataNode) : QmitkDataGenerationJobBase(outputDataNode, std::vector<mitk::BaseData::ConstPointer>())
{
}

QmitkDataGenerationJobBase::~QmitkDataGenerationJobBase()
{
}

void QmitkDataGenerationJobBase::SetInputBaseData(const std::vector<mitk::BaseData::ConstPointer>& baseData)
{
  m_InputBaseData = baseData;
}

void QmitkDataGenerationJobBase::run()
{
  try
  {
    if (this->RunComputation())
    {
      emit ResultsAvailable(m_OutputDataNodes, this);
    }
  }
  catch (std::exception& e)
  {
    emit Error(QString("Error while running computation. Error description: ") + QString::fromLatin1(
      e.what()), this);
  }
  catch (...)
  {
    emit Error(QString("Unknown error while running computation."), this);
  }
}
