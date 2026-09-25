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
    auto result = DataStorage::SetOfObjects::New();

    SceneJsonReader::Pointer reader = SceneJsonReader::New();

    // Taken from the reader that added them rather than worked out by comparing
    // the storage before and after. Reading runs on a worker thread while the
    // thread that owns the storage keeps handling events, so anything another
    // handler adds meanwhile would land in such a difference and be reported
    // as part of the scene.
    reader->SetLoadedNodes(result);

    // The scene reports into this reader's task, so that reading the file
    // and reading the scene inside it share one notification.
    reader->SetProgressTask(this->GetProgressTask());

    reader->LoadScene(this->GetLocalFileName(), &ds, false);

    return result;
  }

  std::vector<BaseData::Pointer> SceneJsonFileReader::DoRead()
  {
    // The AbstractFileReader::DoRead contract returns BaseData only, so
    // node properties and parent/child relationships are intentionally
    // lost on this code path. Callers that need the scene graph should
    // use Read(DataStorage&) instead. Data-less nodes are skipped here
    // to avoid handing nullptrs back to IOUtil::Load.
    std::vector<BaseData::Pointer> result;

    DataStorage::Pointer ds = StandaloneDataStorage::New().GetPointer();
    this->Read(*ds);
    DataStorage::SetOfObjects::ConstPointer dataNodes = ds->GetAll();
    for (DataStorage::SetOfObjects::ConstIterator iter = dataNodes->Begin(), iterEnd = dataNodes->End();
      iter != iterEnd;
      ++iter)
    {
      if (BaseData::Pointer data = iter.Value()->GetData())
      {
        result.push_back(data);
      }
    }
    return result;
  }

  SceneJsonFileReader *SceneJsonFileReader::Clone() const { return new SceneJsonFileReader(*this); }
}
