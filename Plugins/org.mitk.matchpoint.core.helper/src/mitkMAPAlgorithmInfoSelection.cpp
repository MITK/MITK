/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMAPAlgorithmInfoSelection.h"

#include "mitkMAPAlgorithmInfoObject.h"

namespace mitk
{

MAPAlgorithmInfoSelection::MAPAlgorithmInfoSelection() :
  m_Selection(new ContainerType())
{

}

MAPAlgorithmInfoSelection::MAPAlgorithmInfoSelection(AlgorithmInfoType::ConstPointer info) :
  m_Selection(new ContainerType())
{
  MAPAlgorithmInfoObject::Pointer obj(new MAPAlgorithmInfoObject(info));
  m_Selection->push_back(obj);
}

MAPAlgorithmInfoSelection::MAPAlgorithmInfoSelection(const AlgorithmInfoVectorType& infos) :
  m_Selection(new ContainerType())
{
  for (AlgorithmInfoVectorType::const_iterator i = infos.begin(); i != infos.end(); ++i)
  {
    MAPAlgorithmInfoObject::Pointer obj(new MAPAlgorithmInfoObject(*i));
    m_Selection->push_back(obj);
  }
}

berry::Object::Pointer MAPAlgorithmInfoSelection::GetFirstElement() const
{
  if (m_Selection->empty())
    return berry::Object::Pointer();

  return *(m_Selection->begin());
}

berry::IStructuredSelection::iterator MAPAlgorithmInfoSelection::Begin() const
{
  return m_Selection->begin();
}

berry::IStructuredSelection::iterator MAPAlgorithmInfoSelection::End() const
{
  return m_Selection->end();
}

int MAPAlgorithmInfoSelection::Size() const
{
  return m_Selection->size();
}

berry::IStructuredSelection::ContainerType::Pointer MAPAlgorithmInfoSelection::ToVector() const
{
  return m_Selection;
}

MAPAlgorithmInfoSelection::AlgorithmInfoVectorType MAPAlgorithmInfoSelection::GetSelectedAlgorithmInfo() const
{
  AlgorithmInfoVectorType selectedInfos;
  if(IsEmpty())
    return selectedInfos;

  MAPAlgorithmInfoObject::Pointer infoObject;
  AlgorithmInfoType::ConstPointer info;

  for(iterator it = Begin(); it != End(); ++it)
  {
    infoObject = it->Cast<MAPAlgorithmInfoObject>();
    if(infoObject.IsNotNull())
    {
      info = infoObject->GetInfo();
      if(info.IsNotNull())
        selectedInfos.push_back(info);
    }
  }
  return selectedInfos;
}

bool MAPAlgorithmInfoSelection::IsEmpty() const
{
  return m_Selection->empty();
}

bool MAPAlgorithmInfoSelection::operator==(const berry::Object* obj) const
{
  if (const berry::IStructuredSelection* other = dynamic_cast<const berry::IStructuredSelection*>(obj))
  {
    return m_Selection == other->ToVector();
  }

  return false;
}

}
