/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegChangeOperationBase.h"

mitk::SegChangeOperationBase::SegChangeOperationBase(mitk::MultiLabelSegmentation* segmentation, OperationType operationType)
  : Operation(operationType)
{
  m_Segmentation = segmentation;
}

bool mitk::SegChangeOperationBase::IsValid() const
{
  return !m_Segmentation.IsExpired();
}

mitk::MultiLabelSegmentation::Pointer mitk::SegChangeOperationBase::GetSegmentation()
{
  return this->m_Segmentation.Lock();
}

const mitk::MultiLabelSegmentation::ConstPointer mitk::SegChangeOperationBase::GetSegmentation() const
{
  return this->m_Segmentation.Lock();
}