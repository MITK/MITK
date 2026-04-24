/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneJsonFileReader.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkSceneJsonReader.h>
#include <mitkStandaloneDataStorage.h>

#include <algorithm>

namespace mitk
{
  SceneJsonFileReader::SceneJsonFileReader() : AbstractFileReader()
  {
    CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".scene.json");
    mimeType.SetComment("MITK JSON Scene Files");
    mimeType.SetCategory("MITK Scenes");
    mimeType.AddExtension("mitkscene.json");

    this->SetDescription("MITK JSON Scene Reader");
    this->SetMimeType(mimeType);

    this->RegisterService();
  }

  DataStorage::SetOfObjects::Pointer SceneJsonFileReader::Read(DataStorage &ds)
  {
    DataStorage::SetOfObjects::ConstPointer oldNodes = ds.GetAll();
    SceneJsonReader::Pointer reader = SceneJsonReader::New();
    reader->LoadScene(this->GetLocalFileName(), &ds, false);
    DataStorage::SetOfObjects::ConstPointer newNodes = ds.GetAll();

    // Compute the difference: nodes present after load but not before.
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

  std::vector<BaseData::Pointer> SceneJsonFileReader::DoRead()
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

  SceneJsonFileReader *SceneJsonFileReader::Clone() const { return new SceneJsonFileReader(*this); }
}
