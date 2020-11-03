/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMAPAlgorithmInfoObject.h"

namespace mitk
{

MAPAlgorithmInfoObject::MAPAlgorithmInfoObject() :
  m_Info(nullptr)
{

}

MAPAlgorithmInfoObject::MAPAlgorithmInfoObject(::map::deployment::DLLInfo::ConstPointer info) :
  m_Info(info)
{

}

const ::map::deployment::DLLInfo* MAPAlgorithmInfoObject::GetInfo() const
{
  return m_Info;
}

bool MAPAlgorithmInfoObject::operator==(const berry::Object* obj) const
{
  if (const MAPAlgorithmInfoObject* other = dynamic_cast<const MAPAlgorithmInfoObject*>(obj))
  {
    return m_Info == other->m_Info;
  }

  return false;
}

}
