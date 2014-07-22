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


#include "mitkSegmentationDataNodeReader.h"
#include "mitkIOUtil.h"
#include "mitkLabelSetImage.h"


namespace mitk {

int SegmentationDataNodeReader::Read(const std::string &fileName, mitk::DataStorage &storage)
{
  int n = 0;
  /*
   * The following code could be used to set the segmentation node as child to the original image
   * The information for this was saved in the exterior label
   * */

//  if (fileName.substr(fileName.size()-5) == ".lset")
//  {
//    int oldSize = storage.GetAll()->Size();
//    mitk::Image::Pointer image = mitk::IOUtil::LoadImage(fileName);
//    mitk::LabelSetImage * lsImage = dynamic_cast<mitk::LabelSetImage *>(image.GetPointer());
//    mitk::Label * extLabel = lsImage->GetExteriorLabel();
//    std::string nameParent("");
//    std::string nameImage("");
//    // if paranet name was set in LabelSetImage
//    if(extLabel->GetMap()->find("name.parent") != extLabel->GetMap()->end())
//      extLabel->GetStringProperty("name.parent",nameParent);

//    if(extLabel->GetMap()->find("name.image") != extLabel->GetMap()->end())
//      extLabel->GetStringProperty("name.image",nameImage);

//    mitk::DataStorage::SetOfObjects::ConstPointer SoO = storage.GetAll();
//    mitk::DataStorage::SetOfObjects::ConstIterator it = SoO->Begin();
//    mitk::DataStorage::SetOfObjects::ConstIterator end = SoO->End();

//    mitk::DataNode::Pointer lsDataNode = mitk::DataNode::New();
//    lsDataNode->SetName(nameImage);
//    mitk::DataNode::Pointer parentDataNode = NULL;
//    lsDataNode->SetData(lsImage);
//    while(it != end)
//    {
//      if(it.Value()->GetName().compare(nameParent) == 0){
//        parentDataNode = it.Value();
//      }
//      ++it;
//    }
//    storage.Add(lsDataNode,parentDataNode.GetPointer());
//    n = storage.GetAll()->Size() - oldSize;
//  }
  return n;
}

}
