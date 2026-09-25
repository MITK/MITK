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
    SceneIO::Pointer sceneIO = SceneIO::New();

    // The scene reports into this reader's task, so that reading the file
    // and reading the scene inside it share one notification.
    sceneIO->SetProgressTask(this->GetProgressTask());

    sceneIO->LoadScene(this->GetLocalFileName(), &ds, false);

    // Taken from the reader that added them rather than worked out by comparing
    // the storage before and after. Reading runs on a worker thread while the
    // thread that owns the storage keeps handling events, so anything another
    // handler adds meanwhile would land in such a difference and be reported
    // as part of the scene.
    auto loadedNodes = sceneIO->GetLoadedNodes();
    auto result = DataStorage::SetOfObjects::New();

    for (const auto &node : *loadedNodes)
      result->push_back(node);

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
