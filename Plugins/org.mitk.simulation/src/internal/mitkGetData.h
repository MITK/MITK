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

#ifndef mitkGetData_h
#define mitkGetData_h

#include "mitkGetDataStorage.h"
#include <mitkNodePredicateDataType.h>

namespace mitk
{
  template <class T>
  typename T::Pointer GetData(const std::string& name)
  {
    DataStorage::Pointer dataStorage = GetDataStorage();

    if (dataStorage.IsNull())
      return NULL;

    typename TNodePredicateDataType<T>::Pointer predicate = TNodePredicateDataType<T>::New();
    DataStorage::SetOfObjects::ConstPointer subset = dataStorage->GetSubset(predicate);

    for (DataStorage::SetOfObjects::ConstIterator it = subset->Begin(); it != subset->End(); ++it)
    {
      DataNode::Pointer dataNode = it.Value();

      if (dataNode->GetName() == name)
      {
        typename T::Pointer data = static_cast<T*>(dataNode->GetData());

        if (data.IsNotNull())
          return data;
      }
    }

    return NULL;
  }
}

#endif
