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

#include "mitkSceneFileReader.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkSceneIO.h>

namespace mitk {

SceneFileReader::SceneFileReader()
  : AbstractFileReader()
{
  CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".scene");
  mimeType.SetComment("MITK Scene Files");
  mimeType.SetCategory("MITK Scenes");
  mimeType.AddExtension("mitk");

  this->SetDescription("MITK Scene Reader");
  this->SetMimeType(mimeType);

  this->RegisterService();
}

DataStorage::SetOfObjects::Pointer SceneFileReader::Read(DataStorage& ds)
{
  //const DataStorage::SetOfObjects::STLContainerType& oldNodes = ds.GetAll()->CastToSTLConstContainer();
  DataStorage::SetOfObjects::ConstPointer oldNodes = ds.GetAll();
  SceneIO::Pointer sceneIO = SceneIO::New();
  sceneIO->LoadScene(this->GetLocalFileName(), &ds, false);
  DataStorage::SetOfObjects::ConstPointer newNodes = ds.GetAll();

  // Compute the difference
  DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();

  unsigned int index = 0;
  for (DataStorage::SetOfObjects::ConstIterator iter = newNodes->Begin(),
        iterEnd = newNodes->End(); iter != iterEnd; ++iter)
  {
    if (!oldNodes->empty())
    {
      if (std::find(oldNodes->begin(), oldNodes->end(), iter.Value()) == oldNodes->end())
        result->InsertElement(index++, iter.Value());
    }
    else
    {
      result->InsertElement(index++, iter.Value());
    }
  }

  return result;
}

std::vector<BaseData::Pointer> SceneFileReader::Read()
{
  return AbstractFileReader::Read();
}

SceneFileReader* SceneFileReader::Clone() const
{
  return new SceneFileReader(*this);
}

}
