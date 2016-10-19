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

#include "mitkMAPAlgorithmInfoObject.h"

namespace mitk
{

MAPAlgorithmInfoObject::MAPAlgorithmInfoObject() :
  m_Info(NULL)
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
