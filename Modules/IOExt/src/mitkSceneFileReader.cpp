/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneFileReader.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkSceneIO.h>
#include <mitkStandaloneDataStorage.h>

namespace mitk
{
  SceneFileReader::SceneFileReader() : AbstractFileReader()
  {
    CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".scene");
    mimeType.SetComment("MITK Scene Files");
    mimeType.SetCategory("MITK Scenes");
    mimeType.AddExtension("mitk");

    this->SetDescription("MITK Scene Reader");
    this->SetMimeType(mimeType);

    this->RegisterService();
  }

  DataStorage::SetOfObjects::Pointer SceneFileReader::Read(DataStorage &ds)
  {
    // const DataStorage::SetOfObjects::STLContainerType& oldNodes = ds.GetAll()->CastToSTLConstContainer();
    DataStorage::SetOfObjects::ConstPointer oldNodes = ds.GetAll();
    SceneIO::Pointer sceneIO = SceneIO::New();
    sceneIO->LoadScene(this->GetLocalFileName(), &ds, false);
    DataStorage::SetOfObjects::ConstPointer newNodes = ds.GetAll();

    // Compute the difference
    DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();

    unsigned int index = 0;
    for (DataStorage::SetOfObjects::ConstIterator iter = newNodes->Begin(), iterEnd = newNodes->End(); iter != iterEnd;
         ++iter)
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

  std::vector<BaseData::Pointer> SceneFileReader::DoRead()
  {
    std::vector<BaseData::Pointer> result;

    DataStorage::Pointer ds = StandaloneDataStorage::New().GetPointer();
    this->Read(*ds);
    DataStorage::SetOfObjects::ConstPointer dataNodes = ds->GetAll();
    for (DataStorage::SetOfObjects::ConstIterator iter = dataNodes->Begin(), iterEnd = dataNodes->End();
      iter != iterEnd;
      ++iter)
    {
      result.push_back(iter.Value()->GetData());
    }
    return result;
  }

  SceneFileReader *SceneFileReader::Clone() const { return new SceneFileReader(*this); }
}
